#ifndef TEST_H
#define TEST_H

#include "pmsis.h"

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

#ifndef F_MAX
#define F_MAX        (350000000)
#endif /* F_MAX */

#ifndef F_MIN
#define F_MIN        (210000000)
#endif /* F_MIN */

#define F_STEP       (5000000)
#define F_DIV        (1000)
#ifndef PROBLEM_SIZE
#define PROBLEM_SIZE (1000000)
#endif /* PROBLEM_SIZE */
#define TEST_RUNS    (100)
#define TEST_REPEAT  (1)
#define MHZ          (1000000)

struct run_info {
    int success_counter[CORE_NUMBER];
    int failure_counter[CORE_NUMBER];
    int failures;
    int successes;    
    int call_total;
    int total_time;
};

void rand_test(void);
struct run_info test_rand(struct pi_device *cluster_dev, int verbose);
void random_gen(void *arg);
void cluster_entry(void *arg);

#endif /* TEST_H */
