PULP_APP = test
PULP_APP_FC_SRCS = src/test.c src/util.c
PULP_APP_HOST_SRCS = src/test.c src/util.c
PULP_CFLAGS = -O2 -g -I$(GAP_SDK_HOME)/rtos/pulp/pulp-os/kernel/gap/

include $(GAP_SDK_HOME)/tools/rules/pulp_rules.mk
