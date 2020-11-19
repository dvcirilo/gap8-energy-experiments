# User Test
#------------------------------------

APP              = test
APP_SRCS         = src/test.c src/util.c
#APP_SRCS         = test.c
APP_INC	         =
APP_CFLAGS       = -O2
APP_LDFLAGS      =

PMSIS_OS 		 = freertos

include $(RULES_DIR)/pmsis_rules.mk
