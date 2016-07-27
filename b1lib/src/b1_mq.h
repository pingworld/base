#ifndef _B1_MQ_H_
#define _B1_MQ_H_

#include <pthread.h>

typedef struct b1_mq_item_s b1_mq_item_t;
struct b1_mq_item_s {
    void			*data_;
    b1_mq_item_t	*next_;
};

typedef struct b1_mq_s b1_mq_t;
struct b1_mq_s {
    b1_mq_item_t   *head_;
    b1_mq_item_t   *tail_;
    pthread_mutex_t lock_;
};

void b1_mq_item_create(void *data, b1_mq_item_t **item);
void b1_mq_item_delete(b1_mq_item_t *item);

void b1_mq_init(b1_mq_t *mq);
void b1_mq_unint(b1_mq_t *mq);
b1_mq_item_t *b1_mq_pop(b1_mq_t *mq);
void b1_mq_push(b1_mq_t *mq, b1_mq_item_t *item);

#endif
