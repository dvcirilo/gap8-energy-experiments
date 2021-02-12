# User Test
#------------------------------------
APP              = test
APP_SRCS         = src/test.c src/util.c
#APP_SRCS         = test.c
APP_INC	         =
APP_CFLAGS       = -O2 -DF_MIN=200 -DPROBLEM_SIZE=10000
APP_LDFLAGS      =

PMSIS_OS 		 = freertos

include $(RULES_DIR)/pmsis_rules.mk
