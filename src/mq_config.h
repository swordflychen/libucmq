/*
*  Copyright (c) 2013 UCWeb Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
* You may obtain a copy of the License at
*
*       http://www.gnu.org/licenses/gpl-2.0.html
*
* Email: osucmq@ucweb.com
* Author: ShaneYuan
*/

#ifndef  __MQ_CONFIG_H__
#define  __MQ_CONFIG_H__
#ifdef __cplusplus
extern "C" 
{
#endif

/////////////////////////////////////////////////////////////////////////////////

#define  DEF_OUTPUT_LOG_PATH    "../log"
#define  DEF_OUTPUT_LOG_LEVEL   "info"
#define  DEF_RES_STORE_SPACE    4
#define  DEF_MAX_QLIST_ITEMS    32       /* queue list items limit */

#define  DEF_SYNC_INTERVAL      100
#define  DEF_SYNC_TIME_INTERVAL 100

#define  DEF_DATA_FILE_PATH     "../data"
#define  DEF_DB_FILE_MAX_SIZE   64

#define  DEF_MAX_QUEUE          100000000

/////////////////////////////////////////////////////////////////////////////////

typedef struct ucmq_conf
{
    char         output_log_path[128];   /* log_path */
    char         output_log_level[16];   /* log_level */
    uint64_t     res_store_space;
    int          max_qlist_itmes;        /* queue list items limit */

    int          sync_interval;
    int          sync_time_interval;

    uint32_t     def_max_queue;          /* queue_size */

    char         data_file_path[128];    /* data_path */
    int          db_file_max_size;       /* db file size */
}ucmq_conf_t;

extern ucmq_conf_t g_mq_conf;

/////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
#endif /* __MQ_CONFIG_H__ */
