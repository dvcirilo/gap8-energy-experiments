#ifndef TEST_H
#define TEST_H

#include "rt/rt_api.h"

#define WAIT_TIME    (12000000)
#define TRIGGER      (17)
#define STACK_SIZE   (2048)
#define MOUNT        (1)
#define UNMOUNT      (0)
#define CID          (0)
#define FC_FREQ      (150000000)
#define CORE_NUMBER  (1)
#define SEED         (10)
#define V_MAX        (1200)
#define V_MIN        (1000)
#define V_STEP       (50)
#define F_MAX        (350000000)
#define F_MIN        (250000000)
#define F_STEP       (5000000)
#define F_DIV        (1000)
#define PROBLEM_SIZE (1000000)
#define TEST_RUNS    (1)
#define TEST_REPEAT  (2)
#define MHZ          (1000000)

struct run_info {
    int success_counter[CORE_NUMBER];
    int failure_counter[CORE_NUMBER];
    int failures;
    int successes;    
    int call_total;
    int total_time;
};

struct run_info test_rand(rt_event_sched_t *p_sched, void *stacks,
                          rt_event_t *p_event, int verbose);
void random_gen(void *arg);
void cluster_entry(void *arg);
void end_of_call(void *arg);

#endif /* TEST_H */
