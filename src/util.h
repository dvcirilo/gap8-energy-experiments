#ifndef UTIL_H
#define UTIL_H

#include "pmsis.h"

int rand_r (unsigned int *seed);
void generate_rands(unsigned int *rand_values, unsigned int seed,
                    int num_tests, int runs);
int current_voltage(void);
int set_voltage_current(int frequency, int voltage);

#endif /* UTIL_H */
