#include "b1_mq.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

void b1_mq_item_create(void *data, b1_mq_item_t **item)
{
    b1_mq_item_t *i = (b1_mq_item_t *)malloc(sizeof(b1_mq_item_t));
    if (!i) {
        printf("[fatal] can't alloc mcq item");
        exit(1);
    }

    memset(i, 0, sizeof(b1_mq_item_t));
    i->data_ = data;

    *item = i;
}

void b1_mq_item_delete(b1_mq_item_t *item)
{
    if (!item) return;

    /*the data is belong to owner of item*/
    free(item);
}

void b1_mq_init(b1_mq_t *mcq)
{
    pthread_mutex_init(&mcq->lock_, NULL);
    mcq->head_ = NULL;
    mcq->tail_ = NULL;
}

void b1_mq_unint(b1_mq_t *mcq)
{
    if (!mcq) return;

    b1_mq_item_t *item = NULL;
    while ((item = b1_mq_pop(mcq)) != NULL) {
        printf("got an item: %p\n", item);
        b1_mq_item_delete(item);
    }

    pthread_mutex_destroy(&mcq->lock_);
}

b1_mq_item_t *b1_mq_pop(b1_mq_t *mcq)
{
    if (!mcq) return NULL;

    b1_mq_item_t *item;

    pthread_mutex_lock(&mcq->lock_);
    item = mcq->head_;
    if (item != NULL) {
        mcq->head_ = item->next_;
        if (mcq->head_ == NULL) {
            mcq->tail_ = NULL;
        }
    }
    pthread_mutex_unlock(&mcq->lock_);

    return item;
}

void b1_mq_push(b1_mq_t *mcq, b1_mq_item_t *item)
{
    if (!mcq) return;

    item->next_ = NULL;

    pthread_mutex_lock(&mcq->lock_);
    if (mcq->tail_ == NULL) {
        mcq->head_ = item;
    } else {
        mcq->tail_->next_ = item;
    }
    mcq->tail_ = item;
    pthread_mutex_unlock(&mcq->lock_);
}
