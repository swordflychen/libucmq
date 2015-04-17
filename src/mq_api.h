/*
 * file: mq_api.h
 */

#include <stdint.h>

#include "log.h"
#include "mq_queue_manage.h"

/* Open and close mq store. */
/*
 * Initial and open msg-queue store.
 * Parameters:
 *      1. log_path: log file path;
 *      2. data_path: msg data path.
 * Return:
 *      if initial ok then return 1, else return 0.
 *
 */
bool mq_store_init(const char *log_path, const char *data_path);

/*
 * Close msg-queue store.
 */
void mq_store_close(void);

/*
 * FIXME: no thread safe.
 * Put msg into specified msg-queue.
 * Args:
 *      1. qname: msg-queue name
 *      2. msg: msg context
 *      3. msg_len: msg length.
 * Return:
 *      if initial ok then return 1, else return 0.
 */
bool mq_store_put(const char *qname, const char *msg, int msg_len);

/*
 * FIXME: no thread safe.
 * Pop msg from specified msg-queue.
 * Args:
 *      1. qname: msg-queue name
 *      2. msg: msg pointer, not need free
 *      3. msg_len: msg length
 * Return:
 *      if initial ok then return 1, else return 0.
 */
bool mq_store_get(const char *qname, char *msg, int msg_len);

