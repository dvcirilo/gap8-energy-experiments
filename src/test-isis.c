#include "test.h"
#include "functions.h"
#include "pin.h"

/* Reference rand values on L2 */
unsigned int rand_values[RUNS];

/* The CLUSTER function: Generate a random number */
//same
void random_gen(void *arg)
{
    unsigned int *L1_mem = (unsigned int *) arg;
    rand_r(&L1_mem[__core_ID()]);
    //unsigned int rand_var = (unsigned int) SEED;
    //rand_r(&rand_var);
}
//

/* Master entry in charge of spawning the parallel function to all the cores */
//same
void Master_Entry(int * L1_mem)
{
    CLUSTER_CoresFork(random_gen, (void *) L1_mem);
}
//

/* The actual test. */
//changes
struct run_info test_rand(bool verbose)
{
    int error=0, time_ms=0;
    int success_counter=0, failure_counter=0;
    int total_time=0, calls=0, call_total=0;
    struct run_info runs;

    unsigned int *L1_mem = L1_Malloc(CORE_NUMBER*sizeof(unsigned int));

    /* Runs NUM_TESTS tests. Each test with RUNS calls to random_gen() */
    for(int j=0;j<NUM_TESTS;j++) {

       initialize_seeds(L1_mem, CORE_NUMBER, (unsigned int) SEED);

        for(int i=0;i<RUNS;i++) {

            CLUSTER_SendTask(0, Master_Entry, (void *) L1_mem, 0);
            CLUSTER_Wait(0);
            calls++;
            if (L1_mem[CORE_NUMBER-1]^rand_values[i]){
                failure_counter++;
                if (verbose)
                    printf("%d,0x%08x,0x%08x,%d,%d\n", calls,
                            L1_mem[CORE_NUMBER-1], rand_values[i],
                            failure_counter,success_counter);
                call_total += calls;
                calls = 0;
                break;
            } else {
                success_counter++;
            }
        }

        /* Breaks if failed */
        if (failure_counter)
            break;
    }
    call_total += calls;
    runs.success_counter = success_counter;
    runs.failure_counter = failure_counter;
    runs.call_total = call_total;
    runs.total_time = 0;
    L1_Free(L1_mem);
    return runs;
}

int main()
{
    PinName trigger = GPIO_A17;

    struct run_info runs;

    init_pin(trigger);

    /* Initialize FC Clock */
    FLL_SetFrequency(uFLL_SOC, FC_FREQ, 1);

    printf("FC Frequency: %d kHz - Voltage: %lu mV\n",
            FLL_GetFrequency(uFLL_SOC)/F_DIV, current_voltage());

    /* Create a vector on L2 with RUN values of rand */
    unsigned int rand_var = (unsigned int) SEED;
    for (int i = 0; i < RUNS; i++) {
        rand_values[i] = rand_r(&rand_var);
    }

    printf("Done generating %dk items table for seed = %d\n", RUNS/1000, SEED);

    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER, 0);


    printf("set_freq,meas_freq,success,fail,total,voltage,time\n");

    int time, timer;
    int fmax, fstep, freq;
    int vmax, vstep, voltage;

    freq = 80000000;
    fmax = 100000000;
    fstep =  1000000;

    voltage = 1200;
    vmax = 1200;
    vstep = 50;
    while (freq < fmax) {
    //while (voltage < vmax) {
        freq += fstep;
        //voltage += vstep;
        if(set_voltage_current(freq, voltage, false)){
            printf("Failed to assign voltage\n");
            break;
        }

        for (int i = 0; i < 1; i++) {
            /* Set trigger */
            set_pin(trigger,1);

            /* Resets and initialize TIMER (on FC) */
            Timer_Initialize(TIMER, 0);
            Timer_Enable(TIMER);

            /* Run call the test */
	    //try verbose
            runs = test_rand(false);

            /* Get TIMER (on FC) */
            timer = (int)(Timer_ReadCycle(TIMER) 
                        / (FLL_GetFrequency(uFLL_SOC)/1000));
            Timer_Disable(TIMER);

             /* Unset trigger */
            set_pin(trigger,0);

            printf("%d,%d,%d,%d,%d,%d,%d\n",
                    freq,FLL_GetFrequency(uFLL_CLUSTER),runs.success_counter, 
                    runs.failure_counter, runs.call_total, current_voltage(),timer);

            /* Delay to allow measurement */
            time = 100;
            while (time) {
                osDelay(time--);
            }
        }
    }

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);

    printf("Test finished\n");

    exit(0);
}
