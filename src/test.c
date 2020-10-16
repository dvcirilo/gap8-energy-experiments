#include "test.h"
#include "functions.h"
#include "pin.h"

unsigned int rand_values[NUM_TESTS];

/* The CLUSTER function: Generate a random number */
void random_gen(void *arg)
{
    unsigned int *L1_mem = (unsigned int *) arg;

    /* Reset SEED for each run */
    /*if(__core_ID()==3){*/
        /*L1_mem[__core_ID()] = SEED;*/
    /*}*/

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
		/*printf("%d ", rand_values[i]);*/

   }
		printf("\n");

    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER, 0);

    int fmax, fstep, freq, finit;
    int vmin, vmax, vstep, voltage;
    int fstep2, f_mid;

    finit = 87000000; //210000000
    fstep =  20000000;
    fstep2 = 10000000;
    f_mid = 200000000;

    voltage = 1000;
    vmax = 1200;
    vstep = 50;

    printf("voltage,set_freq,meas_freq,success[i],failures[i]\n");
    while (voltage <= vmax) {       
        freq = finit;
        switch(voltage)
        {
            case 1000:
                fmax = 215000000;
                break;
            case 1050:
                fmax = 245000000;
                break;
            case 1100:
                fmax = 270000000;
                break;
            case 1150:
                fmax = 295000000;
                break;
            case 1200:
                fmax = 320000000;
                break;
            default:
                fmax = 87000000;
        }

        while (freq <= fmax) {
            
            if(set_voltage_current(freq, voltage, false)){
                printf("Failed to assign freq/voltage\n");
                break;
            }

            /* Set trigger */
            set_pin(trigger,1);
            //printf("Trigger set\n");

            /* Run call the test */
            runs = test_rand(false);

            /* Unset trigger */
            set_pin(trigger,0);
            //printf("Trigger unset\n");
 
            printf("%d,%d,%d",
                current_voltage(),freq,FLL_GetFrequency(uFLL_CLUSTER));
            for(int i=0; i<CORE_NUMBER;i++){
                printf(",%d,%d",runs.success_counter[i],runs.failure_counter[i]);
            }

            printf("\n");

            if (freq <= f_mid){
                freq += fstep;
            }else{
                freq += fstep2;
            }
            delay();

        }
        voltage += vstep;
    }

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);

    printf("Test finished\n");

    exit(0);
}
