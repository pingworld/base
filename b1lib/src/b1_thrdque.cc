#include "b1_thrdque.h"
#include <memory.h>
#include <stdlib.h>
#include <assert.h>

#ifndef WIN32
#include <unistd.h>
#else
#include "win32/win32.h"
#include "win32/config.h"
#endif

#include "b1_mq.h"
#include "b1_utils.h"

typedef struct b1_thread_complete_signal_s b1_thread_complete_signal_t;
struct b1_thread_complete_signal_s {
    int	                init_sum_;
    b1_thread_t         *me;	
    int                 *init_count_;
    pthread_mutex_t     *init_lock_;
    pthread_cond_t      *init_cond_;
};

static void register_thread_initialized(b1_thread_complete_signal_t *cs) 
{
    b1_lock(cs->init_lock_);
    (*(cs->init_count_))++;
    pthread_cond_signal(cs->init_cond_);
    b1_unlock(cs->init_lock_);
}

static void *b1_thread_worker(void *arg)
{
    b1_thread_complete_signal_t *cs = (b1_thread_complete_signal_t *)arg;
    register_thread_initialized(cs);
    b1_thread_t *me = cs->me;
    free(cs);
    event_base_dispatch(me->base_);
    printf("pid: %d thread is exit\n", b1_getpid());
    return NULL;
}

static int b1_thread_init(b1_thread_t *t, event_callback_fn cb)
{
    int res = 0;
    t->base_ = event_base_new();
    t->notify_event_ = event_new(t->base_, t->notify_receive_fd_ , 
        EV_READ|EV_PERSIST, cb, t);
    res = event_add(t->notify_event_, NULL);
    if (res != 0) goto b1_tc_err;

    t->mq_ = (b1_mq_t *)malloc(sizeof(b1_mq_t));
    if (!t->mq_) goto b1_tc_err;

    b1_mq_init(t->mq_);
    return res;

b1_tc_err:
    event_free(t->notify_event_);
    t->notify_event_ = NULL;
    event_base_free(t->base_);    
    t->base_ = NULL;
    return -1;
}

static void b1_thread_create_worker(void *(*func) (void *), void *arg)
{
    pthread_t t;
    pthread_attr_t attr;
    int res;

    pthread_attr_init(&attr);
    if ((res = pthread_create(&t, &attr, func, arg)) != 0) {
        fprintf(stderr, "Can't create thread: %s\n", strerror(res));
        exit(1);
    }
}

void b1_thread_create(int nthreads, event_callback_fn cb, void *data, 
                       b1_thread_t **threads)
{
#ifdef WIN32
	struct sockaddr_in serv_addr;
	int sockfd;

	struct rlimit rlim;
	getrlimit(RLIMIT_CORE, &rlim);

	if ((sockfd = createLocalListSock(&serv_addr)) < 0)
		exit(1);
#endif

    b1_thread_t *t = (b1_thread_t *)calloc(nthreads, sizeof(b1_thread_t));
    if (!t) {
        printf("[fatal] oom\n");
        exit(1);
    }

    for (int i = 0; i < nthreads; ++i) {
        int fds[2];
#ifdef WIN32
		if (createLocalSocketPair(sockfd, fds, &serv_addr) == -1) {
			printf("[fatal] can't create pipe\n");
			exit(1);
		}
#else
        if (pipe(fds)) {
            printf("[fatal] can't create pipe\n");
            exit(1);
        }
#endif

        t[i].notify_receive_fd_ = fds[0];
        t[i].notify_send_fd_  = fds[1];
        t[i].ud_ = data;
        if (b1_thread_init(&t[i], cb) != 0) {
            assert(false);
            exit(1);
        }

#ifdef WIN32
		if (i == (nthreads - 1)) {
			shutdown(sockfd, 2);
		}
#endif
    }

    pthread_mutex_t init_lock;
    pthread_cond_t init_cond;
    int init_count = 0;

    pthread_mutex_init(&init_lock, NULL);
    pthread_cond_init(&init_cond, NULL);

    /* Create threads after we've done all the libevent setup. */
    for (int i = 0; i < nthreads; ++i) {
        b1_thread_complete_signal_t *cs = (b1_thread_complete_signal_t *)malloc(
            sizeof(b1_thread_complete_signal_t));
        cs->init_sum_ = nthreads;
        cs->init_count_ = &init_count;
        cs->init_lock_ = &init_lock;
        cs->init_cond_ = &init_cond;
        cs->me = &t[i];

        b1_thread_create_worker(b1_thread_worker, cs);
    }

    b1_lock(&init_lock);
    while (init_count < nthreads) {
        pthread_cond_wait(&init_cond, &init_lock);
    }
    b1_unlock(&init_lock);

    *threads = t;
}

void b1_thread_delete(b1_thread_t *t)
{
    if (!t) return;

    if (t->notify_event_) {
        event_del(t->notify_event_);
        event_free(t->notify_event_);
        t->notify_event_ = NULL;
    }

    if (t->base_) {        
        int ret = event_base_loopbreak(t->base_);
        event_base_free(t->base_);
        t->base_ = NULL;
    }

    if (t->mq_) {
        b1_mq_unint(t->mq_);
        free(t->mq_);
        t->mq_ = NULL;
    }

    free(t);
}

void b1_thread_send_notice(b1_thread_t *stid, void *data, char c)
{
    if (!stid) return;

    b1_mq_item_t *item = NULL;
    b1_mq_item_create(NULL, &item);
    assert(item != NULL);

    item->data_ = data;

    b1_mq_push(stid->mq_, item);
    char buf[1];
    buf[0] = c;
#ifndef WIN32
	if (write(stid->notify_send_fd_, buf, 1) != 1) {
#else
	if (mem_write(stid->notify_send_fd_, buf, 1) != 1) {
#endif
        printf("[fatal] can't write char into pipe");
		// FIXME
		// 这里delete了，但没有从队列中弹出来。。
        b1_mq_item_delete(item);
        exit(1);
    }
}

// NOTE 一般的用法是这样的
// char c;
// b1_thread_wait_notice(tid, &c);
// item = b1_thread_get_notice(tid);
// switch (c) { case x: do_x(); break; case y: do_y(); break; }
// 当有多个线程同时往tid中投递数据的时候，这时候c和item如何对应起来？？如何确定这个c对应的内容
// 就是这个item呢？mq是个队列，保证FIFO即可
// t1 --> send x to tid, then push itemx into tid.mq;
// tid --> get a char from pipe of x
// t2 --> send y to tid, then push itemy into tid.mq;
// tid --> get itemx from tid.mq;
// tid --> do_x();
// tid --> get itemy from tid.mq;
// tid --> do_y();

void b1_thread_wait_notice(b1_thread_t *rtid, char *c)
{
    char buf[1];
#ifndef WIN32
    if (read(rtid->notify_receive_fd_, buf, 1) != 1) {
#else
	if (mem_read(rtid->notify_receive_fd_, buf, 1) != 1) {
#endif
        printf("[fatal] can't read char from pipe");
        exit(1);
    }
    *c = buf[0];
}

b1_mq_item_t *b1_thread_get_notice(b1_thread_t *rtid)
{
    if (rtid) return b1_mq_pop(rtid->mq_);
    return NULL;
}
