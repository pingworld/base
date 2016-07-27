#include "b1_thrdque.h"
#include "gtest/gtest.h"
#include "b1_mq.h"
#include "b1_log.h"

static b1_thread_t *t1 = NULL;

static void test_thread_cb(int fd, short what, void *arg)
{
    printf("test_thread_cb start...\n");
    char c;
    b1_thread_wait_notice(t1, &c);
    b1_mq_item_t *item = b1_mq_pop(t1->mq_);
    if (item) {
        printf("pop item: %p\n", item);
    }
    if (c == 'e') {
        printf("got exit notification\n");
    }
    b1_thread_delete(t1);
}

TEST(b1_thrdque_suite, create)
{
    b1_thread_create(1, test_thread_cb, NULL, &t1);
    b1_thread_send_notice(t1, NULL, 'e');
    b1_thread_send_notice(t1, NULL, 'f');
    sleep(3);
}

TEST(b1_thrdque_suite, log_thrdque)
{
	log_open("b1_thrdque_suite.log", Logger::LEVEL_TRACE, true, 1024 * 1024);
	log_trace("hello b1log");
	log_debug("hello b1log");
	log_error("hello b1log");
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
