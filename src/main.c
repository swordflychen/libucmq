/*
 * file: main.c
 * desc: test mq.
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "log.h"
#include "mq_api.h"

typedef struct _queue
{
    char qname[16];
    char send[64];
    char recv[64];
    bool is_quit;
} queue_t;

int g_index = 0;

void *put_msg(void *args)
{
    int i;
    bool ret = true;
    queue_t *qt = (queue_t *)args;
    
    for (i = 0; !qt->is_quit; ++i) {
        sprintf(qt->send, "helloworld:[%10d]", i);
        ret = mq_store_put(qt->qname, qt->send, sizeof(qt->send));

        if (!ret) {
            printf("put item[%d] error.\n", i);
            break;
        }
    }

    return NULL;
}

void *get_msg(void *args)
{
    bool ret = true;
    queue_t *qt = (queue_t *)args;

    sleep(2);
    while (!qt->is_quit) {
        ret = mq_store_get(qt->qname, qt->recv, sizeof(qt->recv));

        if (!ret) {
            printf("get item error.\n");
            log_error("get item error.\n");
            sleep(1);
        }
    }

    return NULL;
}


void *put_a_msg(void *args)
{
    bool ret = true;
    queue_t *qt = (queue_t *)args;

    sprintf(qt->send, "helloworld:[%10d]", g_index ++);
    printf("put msg: %s\n", qt->send);
    ret = mq_store_put(qt->qname, qt->send, sizeof(qt->send));

    if (!ret) {
        printf("put item error.\n");
    }

    return NULL;
}

void *get_a_msg(void *args)
{
    bool ret = true;
    queue_t *qt = (queue_t *)args;

    ret = mq_store_get(qt->qname, qt->recv, sizeof(qt->recv));

    printf("get msg: %s\n", qt->recv);
    if (!ret) {
        printf("get item error.\n");
        log_error("get item error.");
    }

    return NULL;
}

int main()
{
    /* initial mq. */
    printf("mq_store_init... \n");
    int ret = mq_store_init("./mq_data/logs", "./mq_data/data");
    if (ret != 1) {
        printf("Error.\n");
        return -1;
    }
    printf("OK \n\n");

    queue_t queue;
    strcpy(queue.qname, "test_queue");
    queue.is_quit = false;

//    put_a_msg(&queue);
//    put_a_msg(&queue);
//    get_a_msg(&queue);
//    get_a_msg(&queue);

//    /*
    pthread_t put_pid, get_pid;
    ret = pthread_create(&put_pid, NULL, put_msg, &queue);
    if (ret != 0) {
        printf("create put thread error, errno[%d]\n", ret);
        exit(1);
    }

    ret = pthread_create(&get_pid, NULL, get_msg, &queue);
    if (ret != 0) {
        printf("create get thread error, errno[%d]\n", ret);
        exit(1); 
    }

    sleep(300);
    queue.is_quit = true;

    pthread_join(put_pid, NULL);
    pthread_join(get_pid, NULL);
//    */

    /* close mq. */
    printf("mq_store_close \n");
    mq_store_close();
    printf("OK\n");

    return 0;
}

