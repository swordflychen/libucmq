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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "ini.h"
#include "mq_util.h"
#include "mq_config.h"

/////////////////////////////////////////////////////////////////////////////////

/* -- initial global conf -- */
ucmq_conf_t g_mq_conf =
{
    DEF_OUTPUT_LOG_PATH,    /* output_log_path[128]; */
    DEF_OUTPUT_LOG_LEVEL,   /* output_log_level; */
    DEF_RES_STORE_SPACE,    /* real_store_space; */
    DEF_MAX_QLIST_ITEMS,    /* queue list items limit */

    DEF_SYNC_INTERVAL,      /* sync_interval; */
    DEF_SYNC_TIME_INTERVAL, /* sync_time_interval second; */

    DEF_MAX_QUEUE,          /* def_max_queue; */

    DEF_DATA_FILE_PATH,     /* data_file_path[128]; */
    DEF_DB_FILE_MAX_SIZE,   /* db_file_max_size; */
};
