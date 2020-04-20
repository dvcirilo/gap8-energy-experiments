#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "test.h"

void initialize_seeds(unsigned int *L1_mem, int core_num, unsigned int seed);
uint32_t current_voltage(void);
int set_voltage_current(int frequency, int voltage, bool verbose);

#endif /* FUNCTIONS_H */
