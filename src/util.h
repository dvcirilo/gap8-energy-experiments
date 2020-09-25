#ifndef UTIL_H
#define UTIL_H

#include "rt/rt_api.h"
#include "pmu_driver.h"

int rand_r (unsigned int *seed);
void generate_rands(unsigned int *rand_values, unsigned int seed,
                    int num_tests, int runs);
uint32_t current_voltage(void);
int set_voltage_current(int frequency, int voltage);

#endif /* UTIL_H */
