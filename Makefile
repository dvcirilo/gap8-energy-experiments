# User Test
#------------------------------------
APP              = test
APP_SRCS         = src/test.c src/util.c
#APP_SRCS         = test.c
APP_INC	         =
APP_CFLAGS       ?= -O0 #-DF_MIN=200 -DPROBLEM_SIZE=10000 -DTEST_RUNS=100 -DTEST_REPEAT=2

APP_LDFLAGS      =

PMSIS_OS 		 = freertos

include $(RULES_DIR)/pmsis_rules.mk
