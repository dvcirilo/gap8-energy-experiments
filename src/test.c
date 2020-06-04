#include "test.h"
#include "functions.h"
#include "pin.h"

unsigned int rand_values[NUM_TESTS];

/* The CLUSTER function: Generate a random number */
void random_gen(void *arg)
{
    unsigned int *L1_mem = (unsigned int *) arg;

    /* Reset SEED for each run */
    if(__core_ID()==3){
        /*L1_mem[__core_ID()] = SEED;*/
    }

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
    int total_time=0, calls=0, call_total=0;
    struct run_info runs;


    /* Allocating a rand variable for each core, preventing race conditions */
    unsigned int *L1_mem = L1_Malloc(CORE_NUMBER*sizeof(unsigned int));
    for(int i=0; i<CORE_NUMBER;i++){
    	L1_mem[i] = SEED;
        runs.success_counter[i] = 0;
        runs.failure_counter[i] = 0;
        runs.successes = 0;
        runs.failures = 0;
	}

    /* Runs NUM_TESTS tests. Each test with RUNS calls to random_gen() */
    for(int j=0;j<NUM_TESTS;j++) {
        CLUSTER_SendTask(0, Master_Entry, (void *) L1_mem, 0);
        CLUSTER_Wait(0);
        calls++;

        for (int i = 0; i < CORE_NUMBER; i++) {
			if (L1_mem[i]^rand_values[j]){
                runs.failure_counter[i]++;
                runs.failures++;
			} else {
				runs.success_counter[i]++;
                runs.successes++;
			}
           if (verbose)
               printf("%d ", L1_mem[i]);
        }
    }    

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
			rand_r(&rands);
		}

		//rands = (unsigned int) SEED;
		rand_values[i] = rands;

		/*Print rand_values*/
		printf("%d ", rand_values[i]);

   }
		printf("\n");

    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER, 0);

    int fmax, fstep, freq, finit, time;
    int vmin, vmax, vstep, voltage;

    finit = 200000000;
    fmax =  220000000; //300000000;
    fstep =  10000000;

    voltage = 1000;
    vmin = 1000;
    vmax = 1200;
    vstep = 50;

    while (voltage <= vmax) {       
        freq = finit;
        while (freq <= fmax) {
            printf("##  V:%d F:%d  ##\n",voltage,freq);
            if(set_voltage_current(freq, voltage, false)){
                printf("Failed to assign freq/voltage\n");
                break;
            }

            /* Set trigger */
            set_pin(trigger,1);

            /* Run call the test */
            runs = test_rand(false);

            /* Unset trigger */
            set_pin(trigger,0);
 
            printf("set_freq,meas_freq,voltage,successes,failures\n");
            printf("%d,%d,%d,%d,%d\n",
                freq,FLL_GetFrequency(uFLL_CLUSTER),current_voltage(), runs.successes, runs.failures);
            printf("id, successes, failures\n");
            for(int i=0; i<CORE_NUMBER;i++){
                printf("%d,%d,%d",i,runs.success_counter[i],runs.failure_counter[i]);
                printf("\n");
            }

            freq += fstep;

            /* Delay to allow measurement */
            time = 100;
            while (time) {
                osDelay(time--);
            }
        }
        printf("Voltage loop\n");
        voltage += vstep;
    }

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);

    printf("Test finished\n");

    exit(0);
}
