#include "functions.h"

/* Initialize seeds */
void initialize_seeds(unsigned int *L1_mem, int core_num, unsigned int seed)
{
    for (int i = 0; i < core_num; i++) {
       L1_mem[i] = seed; 
    }
}

/* Returns the current voltage */
uint32_t current_voltage(void)
{
    return DCDC_TO_mV(PMU_State.DCDC_Settings[
            READ_PMU_REGULATOR_STATE(PMU_State.State)]);
}

int set_voltage_current(int frequency, int voltage, bool verbose)
{
    int set_voltage_return;

    set_voltage_return = PMU_SetVoltage(voltage,1);

    if (set_voltage_return) {
        if (verbose)
            printf("Failed to change voltage! Code: %d\n",set_voltage_return);
        return set_voltage_return;
    }

    if(verbose)
        printf("Voltage: %lu - FCMaxFreq: %d kHz - ClusterMaxFreq: %d kHz\n",
                current_voltage(),FLL_SoCMaxFreqAtV(current_voltage())/F_DIV,
                FLL_ClusterMaxFreqAtV(current_voltage())/F_DIV);

    /* Set frequency */
    if (FLL_SetFrequency(uFLL_CLUSTER, frequency, 1) == -1) {
        if(verbose)
            printf("Error of changing frequency, check Voltage value!\n");
        return -1;
    } else {
        if(verbose)
            printf("Set: Cluster Freq: %d kHz - Voltage: %lu mV\n",
                    FLL_GetFrequency(uFLL_CLUSTER)/F_DIV,current_voltage());
    }
    return 0;
}
