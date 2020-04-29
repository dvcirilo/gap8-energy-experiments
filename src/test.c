#include "test.h"
#include "functions.h"
#include "pin.h"

unsigned int rand_values[NUM_TESTS];

/* The CLUSTER function: Generate a random number */
void random_gen(void *arg)
{
    unsigned int *L1_mem = (unsigned int *) arg;
    L1_mem[__core_ID()] = SEED;
    for (int i = 0; i < RUNS; i++) {
        rand_r(&L1_mem[__core_ID()]);
    }
}

/* Master entry in charge of spawning the parallel function to all the cores */
void Master_Entry(int * L1_mem)
{
    CLUSTER_CoresFork(random_gen, (void *) L1_mem);
}

/* The actual test. */
struct run_info test_rand(bool verbose)
{
    int error=0;
    int success_counter=0, failure_counter=0;
    int total_time=0, calls=0, call_total=0;
    struct run_info runs;

    /* Allocating a rand variable for each core, preventing race conditions */
    unsigned int *L1_mem = L1_Malloc(CORE_NUMBER*sizeof(unsigned int));

    /* Runs NUM_TESTS tests. Each test with RUNS calls to random_gen() */
    for(int j=0;j<NUM_TESTS;j++) {
        CLUSTER_SendTask(0, Master_Entry, (void *) L1_mem, 0);
        CLUSTER_Wait(0);
        calls++;
        for (int i = 0; i < CORE_NUMBER; i++) {
           printf("%d ", L1_mem[i]);
        }
        printf("\n");
     	   printf("%d",rand_values[j]);
		printf("\n");
		 
        	if (L1_mem[0]^rand_values[j]){
            		failure_counter++;
        	} else {
            		success_counter++;
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

    /* Initialize trigger for external energy measurement */
    init_pin(trigger);

    /* Initialize FC Clock */
    FLL_SetFrequency(uFLL_SOC, FC_FREQ, 1);

    printf("FC Frequency: %d kHz - Voltage: %lu mV\n",
            FLL_GetFrequency(uFLL_SOC)/F_DIV, current_voltage());

    unsigned int rands = (unsigned int) SEED;
    for (int i=0; i < NUM_TESTS; i++){
	for (int j=0; j < RUNS; j++){
	    rand_values[i] = rand_r(&rands); 
	}
	rands = (unsigned int) SEED;

   	/*Print rand_values*/
	//printf("%d\n", rand_values[i]);
   }

    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER, 0);

    printf("set_freq,meas_freq,success,fail,total,voltage\n");

    int fmax, fstep, freq, time;
    int vmax, vstep, voltage;

    freq = 80000000;
    fmax = 100000000;
    fstep =  1000000;

    voltage = 1200;
    vmax = 1200;
    vstep = 50;
    /*while (freq < fmax) {*/
    //while (voltage < vmax) {
        freq += fstep;
        //voltage += vstep;
        if(set_voltage_current(freq, voltage, false)){
            printf("Failed to assign voltage\n");
            /*break;*/
        }

        for (int i = 0; i < 1; i++) {
            /* Set trigger */
            set_pin(trigger,1);

            /* Run call the test */
            runs = test_rand(false);

            /* Unset trigger */
            set_pin(trigger,0);

            printf("%d,%d,%d,%d,%d,%d\n",
                    freq,FLL_GetFrequency(uFLL_CLUSTER),runs.success_counter, 
                    runs.failure_counter, runs.call_total, current_voltage());

            /* Delay to allow measurement */
            time = 100;
            while (time) {
                osDelay(time--);
            }
        }
    /*}*/

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);

    printf("Test finished\n");

    exit(0);
}
