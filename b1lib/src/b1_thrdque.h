#ifndef _B1_THRDQUE_H_
#define _B1_THRDQUE_H_

#include "event2/event.h"
#include "event2/event_struct.h"
#include <pthread.h>
// 通常使用该头文件的时候也会用到b1_mq.h，所以一并包含
#include "b1_mq.h"

#ifdef WIN32
#include "win32/win32.h"
#endif

typedef struct b1_thread_s b1_thread_t;
struct b1_thread_s {
    pthread_t			thread_id_;
    struct event_base	*base_;
    struct event		*notify_event_;
    int					notify_receive_fd_;
    int					notify_send_fd_;
    b1_mq_t             *mq_;
    void                *ud_;   /*private data for special thread*/
};

/* nthreads: the num of creation */
/**
 * 注意：
 * 这里的cb并不是线程执行函数，而是线程队列中有事件发生时的回调函数。
 * 所以，只有当该线程有任务需要完成时才会执行。
 * 原型：
 * typedef void (*event_callback_fn)(evutil_socket_t, short, void *);
 */
void b1_thread_create(int nthreads, event_callback_fn cb, void *data, b1_thread_t **threads);
void b1_thread_delete(b1_thread_t *t);

/* send a char to notify thread of something happened. */
void b1_thread_send_notice(b1_thread_t *stid, void *data, char c);
/* waiting for notification of other thread. It will block self-thread. */
void b1_thread_wait_notice(b1_thread_t *rtid, char *c);
/* get a notice item from thread queue. */
b1_mq_item_t *b1_thread_get_notice(b1_thread_t *rtid);

#define b1_lock(x) while (pthread_mutex_trylock(x));
#define b1_unlock(x) pthread_mutex_unlock(x)

#endif
