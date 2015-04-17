#
# date: 2014-10-20
#

TARGET=libucmq.a
TEST=test_ucmq
OBJ_DIR=./obj
SRC=./src

LIB_DIR=./
LIBS=-lrt -lpthread
LIBA=-lucmq

CFLAGS=-Wall -g -pipe -march=native -Wall -Wparentheses -Winline -Wuninitialized -Wunused -Wcomment -Wformat -Wimplicit -Wsequence-point -Wfloat-equal -Wshadow -fstack-protector-all -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 --std=gnu99
#CFLAGS=-Wall -O2

OBJ = $(addprefix $(OBJ_DIR)/, \
      util.o \
      log.o \
      file.o \
      crc16.o \
      mq_config.o \
      mq_util.o \
      mq_errno.o \
      mq_store_file.o \
      mq_store_manage.o \
      mq_store_msg.o \
      mq_store_rtag.o \
      mq_store_wtag.o \
      mq_queue_manage.o \
      mq_api.o \
      main.o \
      )
TEST_OBJ = $(addprefix $(OBJ_DIR)/, main.o)

all:$(OBJ) $(TEST_OBJ)
	ar -rs $(TARGET) $(OBJ)
	gcc $(CFLAGS) $(TEST_OBJ) -L$(LIB_DIR) $(LIBS) $(LIBA) -o $(TEST)

$(OBJ_DIR)/%.o:$(SRC)/%.c
	@-if [ ! -d $(OBJ_DIR) ];then mkdir $(OBJ_DIR); fi
	gcc $(CFLAGS) $(LIBS) -c $< -o $@

clean:
	rm -f $(TEST)
	rm -f $(TARGET)
	rm -rf $(OBJ_DIR)
