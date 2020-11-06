#include <stdlib.h>
#include "pmsis.h"
#include "util.h"

/* stdlib.h rand_r implementation */
int rand_r (unsigned int *seed)
{
    unsigned int            next = *seed;
    int result;

    next *= 1103515245;
    next += 12345;
    result = (unsigned int) (next / 65536) % 2048;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next / 65536) % 1024;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next / 65536) % 1024;

    *seed = next;

    return result;
}

/* Returns the current voltage */
int current_voltage(void)
{
    return pi_pmu_state_get(PI_PMU_DOMAIN_FC);
}

int set_voltage_current(int frequency, int voltage)
{
    
    if (pi_pmu_voltage_set(PI_PMU_DOMAIN_FC, voltage)) {
        return -1;
    }

    /* Set frequency */
    if (pi_fll_set_frequency(FLL_CLUSTER, frequency, 1) == -1) {
        return -1;
    }

    return 0;
}

void generate_rands(unsigned int *rand_values, unsigned int seed,
                    int num_tests, int runs)
{
    unsigned int rands = seed;
    int i, j;

    for (i=0; i < num_tests; i++) {
        for (j=0; j < runs; j++) {
            rand_r(&rands);
        }

        rand_values[i] = rands;
        /*rands = (unsigned int) SEED;*/
    }
}
