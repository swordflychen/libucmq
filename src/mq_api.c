/*
 * file: mq_api.c
 */


#include <pthread.h>
#include "mq_api.h"

extern ucmq_conf_t g_mq_conf;

typedef struct app_info
{
    uint32_t     put;
    uint32_t     get;
    uint32_t     times;
    uint32_t     total_time;
    uint32_t     max_time;
    uint64_t     count;
    uint64_t     put_size;
    pthread_mutex_t mutex;
} app_info_t;

app_info_t g_info;
app_info_t g_last_info;

bool mq_store_init(const char *log_path, const char *data_path)
{
    memset(&g_info, '\0', sizeof(app_info_t));
    memset(&g_last_info, '\0', sizeof(app_info_t));

    /* initial configure. */
    strcpy(g_mq_conf.data_file_path, data_path);
    strcpy(g_mq_conf.output_log_path, log_path);
#ifdef DISABLE_STORE_SPACE
    // g_mq_conf.res_store_space = 0;
#endif

    /* log open. */
    log_config_t log_conf;
    log_init_config(&log_conf);
    strcpy(log_conf.log_path, g_mq_conf.output_log_path);
    // log_conf.log_level = LOG_DEBUG;
    log_conf.log_level = LOG_INFO;
    if (log_init(&log_conf) != 0) {
        log_error("mq log init error.");
        return false;
    }

    /* UCMQ store open */
    log_info("Initial mq store.");
    if (!mq_qm_open_store())
    {
        log_error("Store Open fail");
        return false;
    }

    /* Get sum of all queue unread count */
    g_info.count = mq_qm_get_store_count();
    log_info("All queue's total unread msg count[%"PRIu64"]", g_info.count);

    return true;
}

void mq_store_close(void)
{
    log_info("Close mq store.");
    if (!mq_qm_close_store())
    {
        log_warn("Data base close fail");
    }
}

static int qname_check(const char* qname)
{
    if(qname == NULL)
    {
        return 0;
    }
    const int qname_len = (qname != NULL) ? strlen(qname) : -1;
    /* Queue name is too longer or empty */
    if (qname_len <= 0 || qname_len > MAX_QUEUE_NAME_LEN)
    {
        log_warn("Queue length invalid");
        return 0;
    }

    int i = 0;
    const char* str = qname;

    while (str[i] != '\0')
    {
        /* Check queue name */
        if (!((str[i] >= '0') && (str[i] <= '9'))
                && (!(str[i] >= 'a' && str[i]  <= 'z') && !(str[i] >= 'A' && str[i]  <= 'Z')))
        {
            if ((!(str[i] == '_'))
                    || ((str[i] == '_') && (i == 0))
                    || ((str[i + 1] == '\0') && (str[i] == '_')))
            {
                log_warn("Queue name is invalid, invalid char[%c]", str[i]);
                return 0;
            }
        }
        i++;
    }

    log_debug("queue_name [%s] len [%d]", qname, qname_len);

    return 1;
}

bool mq_store_put(const char *qname, const char *msg, int msg_len)
{

    msg_item_t msg_item;
    mq_queue_t* mq_queue = NULL;
    int result = -1;
    int put_position = -1;

    if (qname_check(qname) != 1) {
        return false;
    }

    if (msg_len <= 0) {
        log_warn("Msg length is invalid, invalid length[%d]", msg_len);
        return false;
    }

    msg_item.msg = (char *)msg;
    msg_item.len = msg_len;

    if (msg_item.len <= 0 || msg_item.len > MAX_DB_FILE_SIZE - DB_FILE_HEAD_LEN - MSG_HEAD_LEN) {
        log_error("Msg length is out or range, invalid length[%d]", msg_item.len);
        return false;
    }

    /* Find name from queue list, if not find create it. */
    mq_queue = mq_qm_find_queue(qname);
    if (mq_queue == NULL) {
        log_debug("Create the queue [%s]now", qname);
        mq_queue = mq_qm_add_queue(qname);
        if (mq_queue == NULL) {
            log_error("Unable create the queue[%s]", qname);
            return false;
        }
        log_info("Create the queue [%s] ...success", qname);
    }

    /* Write queue msg. */
    result = mq_qm_push_item(mq_queue, &msg_item);
    switch (result) {
    case QUEUE_WLOCK:
        log_warn("Queue [%s] is read only", qname);
        return false;

    case QUEUE_FULL:
        log_warn("Queue [%s] is full", qname);
        return false;

    case QUEUE_PUT_OK:
//        mq_queue->unread_count += 1;
        g_info.put_size        += msg_len;
        g_info.put             += 1;
        g_info.count           += 1;
        put_position = mq_queue->put_num;
        return true;

    default:
        log_error("Unknown error");
        return false;
    }
}

bool mq_store_get(const char *qname, char *msg, int msg_len)
{

    msg_item_t msg_item;
    mq_queue_t* mq_queue = NULL;
    int result = -1;

    if (qname_check(qname) != 1) {
        return false;
    }

    mq_queue = mq_qm_find_queue(qname);
    if (mq_queue == NULL) {
        log_warn("Queue [%s] is not exists", qname);
        return false;
    }

    /* Pop msg item from queue */
    result = mq_qm_pop_item(mq_queue, &msg_item);
    switch (result) {
    case QUEUE_GET_END:
        log_warn("Queue [%s] is empty", qname);
        return false;

    case QUEUE_GET_OK:
        g_info.get   += 1;
        g_info.count -= 1;

        assert(msg_len == msg_item.len);
        memcpy(msg, msg_item.msg, msg_item.len);
        return true;

    default:
        log_error("Unknown error");
        return false;
    }

}
