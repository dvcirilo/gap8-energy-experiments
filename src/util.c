#include <stdlib.h>
#include "rt/rt_api.h"
#include "util.h"
#include "pmu_driver.h"
#include "pmu_driver.c"

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
uint32_t current_voltage(void)
{
    return DCDCSettingtomV(PMUState.DCDC_Settings[
                        REGULATOR_STATE(PMUState.State)]);
}

int set_voltage_current(int frequency, int voltage)
{

    if (PMU_set_voltage(voltage,0) != 0) {
        return -1;
    }

    /* Set frequency */
    if (rt_freq_set(__RT_FREQ_DOMAIN_CL,frequency) != 0) {
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
