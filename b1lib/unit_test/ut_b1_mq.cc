#include "b1_mq.h"
#include "gtest/gtest.h"

static void smq_print(b1_mq_t *mcq)
{
    assert(mcq != NULL);
    printf("[smq_print begin]\n");
    b1_mq_item_t *item = mcq->head_;
    while (item) {
        printf("0x%p ~ 0x%p\n", item, item->data_);
        item = item->next_;
    }
    printf("[smq_print end]\n");
}

class mq_test_suite : public testing::Test {
protected:
    virtual void SetUp() {
        msg_queue = (b1_mq_t *)malloc(sizeof(b1_mq_t));
        memset(msg_queue, 0, sizeof(b1_mq_t));
        if (msg_queue == NULL) {
            printf("Failed to allocate memory for message queue\n");
        }
        b1_mq_init(msg_queue);
    }

    virtual void TearDown() {
        if (msg_queue) {
            b1_mq_unint(msg_queue);
            msg_queue = NULL;
        }
    }

    class mq_item_data {
    public:
        mq_item_data(int id) : id_(id) {}
        inline int get() const { return id_; }
    private:
        int     id_;
    };

    void test_mq_push(int num) {
        for (int i = 1; i <= num; ++i) {
            b1_mq_item_t *item = (b1_mq_item_t *)malloc(sizeof(b1_mq_item_t));
            memset(item, 0, sizeof(b1_mq_item_t));
            item->data_ = new mq_item_data(i);

            b1_mq_push(msg_queue, item);
        }
    }

    void test_mq_pop() {
        b1_mq_item_t *item = NULL;
        while (item = b1_mq_pop(msg_queue)) {
            delete ((mq_item_data *)item->data_);
            free(item);
        }
    }

    b1_mq_t *msg_queue;
};

TEST_F(mq_test_suite, init)
{
    EXPECT_TRUE(msg_queue != NULL);
    EXPECT_TRUE(msg_queue->head_ == NULL);
    EXPECT_TRUE(msg_queue->tail_ == NULL);
}

TEST_F(mq_test_suite, push_pop)
{
    test_mq_push(100);
    EXPECT_EQ(((mq_item_data *)(msg_queue->head_->data_))->get(), 1);
    EXPECT_EQ(((mq_item_data *)(msg_queue->head_->next_->data_))->get(), 2);
    EXPECT_EQ(((mq_item_data *)(msg_queue->tail_->data_))->get(), 100);
    test_mq_pop();
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
