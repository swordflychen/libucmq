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

#include "mq_util.h"
#include "mq_config.h"
#include "mq_store_wtag.h"
#include "mq_store_file.h"

//////////////////////////////////////////////////////////////////////////

#define STR_TO_LL(dest, str, len)                 \
{                                                 \
    uint64_t num = 0;                             \
    if (str_to_num(&num, str, len))               \
    {                                             \
        dest = (typeof(dest))num;                 \
    }                                             \
    else                                          \
    {                                             \
        return false;                             \
    }                                             \
}

//////////////////////////////////////////////////////////////////////////

static bool str_to_num(uint64_t* num, const char* str, int len);
static bool check_wtag_item(mq_queue_t *mq_queue);
static bool parse_wtag_item(mq_queue_t *mq_queue, char *wtag_str);

//////////////////////////////////////////////////////////////////////////

/* Open next wtag file */
int mq_sm_wtag_open_next_file(const char* file_name)
{
    int fd;

    fd = open(file_name, O_RDWR | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        log_debug("Can't open wtag file[%s],errno [%d] error message[%s]",
                file_name, errno, strerror(errno));
        return -1;
    }

    return fd;
}

/* Open wtag file */
bool mq_sm_wtag_open_file(mq_queue_t *mq_queue, queue_file_t *queue_file)
{
    int     fd;
    char    wtag_file_name[MAX_FULL_FILE_NAME_LEN + 1];

    GEN_DATA_FULL_PATH_BY_FNAME(wtag_file_name, mq_queue->qname, queue_file->wtag_fname);
    log_debug("Open wtag file for read item [%s]", wtag_file_name);

    /* Check wtag file size */
    if (get_file_size(wtag_file_name) <= 0)
    {
        log_debug("Wtag file is empty!");
        return false;
    }

    fd = open(wtag_file_name, O_RDWR | O_NONBLOCK | O_APPEND, S_IRUSR | S_IWUSR);
    if(fd < 0)
    {
        log_warn("Wtag file open fail, errno[%d], error msg[%s]",
                errno, strerror(errno));
        return false;
    }
    mq_queue->wtag_fd = fd;
    return true;
}

/* Close wtag file */
bool mq_sm_wtag_close_file(mq_queue_t *mq_queue)
{
    if (mq_queue->wtag_fd < 0)
    {
        return true;
    }
    if (fsync(mq_queue->wtag_fd) < 0)
    {   
        log_warn("Sync [%s] data file ... fail, errno[%d], error msg[%s]",
                mq_queue->qname, errno, strerror(errno));
    }   
    close(mq_queue->wtag_fd);

    return true;
}

bool mq_sm_wtag_read_item(mq_queue_t *mq_queue)
{
    int     i = 1;
    int     offset;
    int     read_len;
    int     read_interval;                                   /* Displacement interval */
    char    line[2 * (WTAG_ITEM_LEN + 1) + 1] = {'\0'};      /* ..wtag_item..\n..wtag_item..\n */

    /* 
     * Move the read position at last line,
     * If read line error then move the position in to prior line
     */
    do
    {
        read_interval = -(i * (WTAG_ITEM_LEN + 1));
        offset = lseek(mq_queue->wtag_fd, read_interval, SEEK_END);
        if (offset < WTAG_FILE_HEAD_LEN)
        {
            log_debug("Real len is no enough");
            break;
        }

        log_debug("Wtag file read_interval [%d], offset[%d]", read_interval, offset);
        if ((read_len = read_n(mq_queue->wtag_fd, FD_UNKNOWN, &line, 2 * (WTAG_ITEM_LEN + 1), READ_OPT_TIMEOUT)) >= 0)
        {
            if (WTAG_ITEM_LEN + 1 <= read_len)
            {
                log_debug("Wtag item [%s], read len [%d]", line, read_len);
                if (parse_wtag_item(mq_queue, line))
                {
                    log_debug("Wtag item parse success, Read line length [%d], source line [%s]",
                            read_len, line);
                    /* If the last line parse failure, skip this db file to avoid coverage */
                    if (i > 1)
                    {
                        mq_queue->cur_wdb.pos = MAX_DB_FILE_SIZE;
                    }
                    return true;
                }
            }
            log_warn("Wtag item read fail, read len [%d], error msg[%s]", read_len, line);
            i++;
            continue;
        }
        else
        {
            log_warn("Wtag file read fail, errno[%d], error msg[%s]", errno, strerror(errno));
            return false;
        }
    }while(1);

//    mq_queue->cur_rdb.pos       = 0;
    mq_queue->cur_wdb.pos       = MAX_DB_FILE_SIZE;
    mq_queue->maxque            = g_mq_conf.def_max_queue;

    return true;
}

bool mq_sm_wtag_write_item(mq_queue_t *mq_queue)
{
    int     ret = 0;
    int     buf_size;
    char    wtag_buf[WTAG_ITEM_LEN + 2];

    buf_size = sprintf(wtag_buf, "%020lu%010u%010u\n",
            mq_queue->put_num,
            mq_queue->cur_wdb.pos,
            mq_queue->maxque);

    log_debug("Write [%s]'s wtag, Wtag item: [%s], Write size[%d]",
            mq_queue->qname, wtag_buf, buf_size);

    ret = write_n(mq_queue->wtag_fd, FD_UNKNOWN, wtag_buf, buf_size, READ_OPT_TIMEOUT);
    if (ret < 0)
    {
        log_warn("Wtag write error, errno[%d] , errno msg[%s]", errno, strerror(errno));
        return false;
    }
    else if (ret == 0)
    {
        log_warn("Wtag write error, End of file");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool check_wtag_item(mq_queue_t *mq_queue)
{
    bool ret = false;

    do
    {
        /* if write position is invaild ,get next write data file */
        if (mq_queue->cur_wdb.pos > MAX_DB_FILE_SIZE || mq_queue->cur_wdb.pos < 0)
        {
            log_debug("Parse wtag item (wpos) error");
            break;
        }
        if (mq_queue->maxque > UINT_MAX || mq_queue->maxque < 0)
        {
            log_debug("Parse wtag item (max queue) error");
            break;
        }
        ret = true;
        return ret;
    }while(1);

    return ret;
}

static bool str_to_num(uint64_t* num, const char* str, int len)
{
    if (num == NULL || str == NULL || len <= 0)
    {   
        return false;
    }   

    *num = 0;
    int i;
    for(i = 0; i < len; i++)
    {   
        if (str[i] >= '0' && str[i] <= '9')
        {   
            *num = *num * 10 + (str[i] - '0');
        }   
        else
        {   
            log_debug("str invalid[%c]", str[i]);
            return false;
        }   
    }   
    log_debug("dest num[%lu]", *num);
    return true;
}

/* Parse wtag item */
static bool parse_wtag_item(mq_queue_t *mq_queue, char *wtag_str)
{
    char    *str1;
    char    *token;

    str1  = wtag_str;
    token = strtok(str1, "\n"); 
    while(token != NULL)
    {   
        log_debug("Token len [%d], token [%s]", strlen(token), token);
        if (strlen(token) >= WTAG_ITEM_LEN)
        {
            char* str;
            str = token;

            STR_TO_LL(mq_queue->put_num, str, WCOUNT_LEN);
            str += WCOUNT_LEN;
            STR_TO_LL(mq_queue->cur_wdb.pos, str, WPOS_LEN);
            str += WPOS_LEN;
            STR_TO_LL(mq_queue->maxque, str, QUEUE_MAX_SIZE_LEN);

            if(check_wtag_item(mq_queue))
            {
                log_debug("rcount [%lu] rpos [%u] wpos[%u] max queue [%u]",
                        mq_queue->put_num, //mq_queue->cur_rdb.opt_count,
                        mq_queue->cur_rdb.pos,
                        mq_queue->cur_wdb.pos,
                        mq_queue->maxque); //mq_queue->wlock);
                return true;
            }
        }
        token = strtok(NULL, "\n");
    }
    return false;
}
