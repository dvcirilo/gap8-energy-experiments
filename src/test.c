#include <stdio.h>
#include <stdlib.h>
#include "rt/rt_api.h"
#include "test.h"
#include "util.h"

unsigned int rand_values[TEST_RUNS];
int done = 0;
int watchdog = 0;

static void watchdog_handle(void *arg)
{
    /*printf("[%d] Entered user handler. Time's up!\n", rt_time_get_us());*/
    printf("Watchdog!!\n");
    watchdog = 1;
    rt_timer_start(arg, 100000);
}

int main()
{
    struct run_info runs;
    rt_timer_t timer;
    int i, j;

    rt_event_sched_t *p_sched = rt_event_internal_sched();
    if (rt_event_alloc(p_sched, 4)) return -1;

   rt_event_t *watchdog_event = rt_event_get(p_sched, watchdog_handle, 
                                            (void *) &timer);

   if(watchdog_event == NULL){
       printf("Watchdog event error\n");
       return -1;
   }

  /* Create a one-shot timer */
    if (rt_timer_create(&timer, RT_TIMER_ONE_SHOT, watchdog_event))
        return -1;

    printf("\n/*********** EXECUTION **********/\n");

    /*Set FC (SoC) Frequency (100MhZ)*/
    rt_freq_set(__RT_FREQ_DOMAIN_FC,100*MHZ);
    printf("Soc FC frequency: %d MHz\n", rt_freq_get(__RT_FREQ_DOMAIN_FC)/MHZ);

    /* Generate comparison values */
    printf("Generating %dM rand values with seed = %d\n",
                                PROBLEM_SIZE*TEST_RUNS/(1000*1000), SEED);
    generate_rands(rand_values, SEED, TEST_RUNS, PROBLEM_SIZE);

    printf("Done. Running proceeding with test...\n");

    /* GPIO Setup */
    rt_padframe_profile_t *profile_gpio = rt_pad_profile_get("hyper_gpio");
    if (profile_gpio == NULL) {
        printf("pad config error\n");
        return 1;
    }
    rt_padframe_set(profile_gpio);

    /* GPIO initialization */
    rt_gpio_init(0, TRIGGER);

    /* Configure TRIGGER pin as an output */
    rt_gpio_set_dir(0, 1<<TRIGGER, RT_GPIO_IS_OUT);

    rt_event_t *p_event = rt_event_get(p_sched, end_of_call, NULL);

    rt_cluster_mount(MOUNT, CID, 0, NULL);

    void *stacks = rt_alloc(RT_ALLOC_CL_DATA, STACK_SIZE*rt_nb_pe());
    if (stacks == NULL) return -1;

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
            
            if(set_voltage_current(freq, voltage)){
                printf("Failed to assign freq/voltage\n");
                break;
            }

            for (int k = 0; k < TEST_REPEAT; k++) {
                /* Delay to allow measurements */
                rt_time_wait_cycles(200*MHZ);

                /* Set trigger */
                rt_gpio_set_pin_value(0, TRIGGER, 1);
       
                /* Start watchdog */
                rt_timer_start(&timer, 1000000);

                /* Run call the test */
                runs = test_rand(p_sched, stacks, p_event, 0);

                /* Unset trigger */
                rt_gpio_set_pin_value(0, TRIGGER, 0);
     
                printf("%d,%d", (int) current_voltage(),
                                rt_freq_get(__RT_FREQ_DOMAIN_CL));
                for(int i=0; i<CORE_NUMBER;i++){
                    printf(",%d,%d", runs.success_counter[i],
                                     runs.failure_counter[i]);
                }

                printf("\n");
                
            }

            if (freq <= f_mid){
                freq += fstep;
            }else{
                freq += fstep2;
            }
        }

        voltage += vstep;
    }

    rt_cluster_mount(UNMOUNT, CID, 0, NULL);

    printf("Test success: Leaving main controller\n");
    return 0;
}

/* The actual test. */
struct run_info test_rand(rt_event_sched_t *p_sched, void *stacks,
                          rt_event_t *p_event, int verbose)
{
    int total_time=0, calls=0, call_total=0;
    struct run_info runs;


    /* Allocating a rand variable for each core, preventing race conditions */
    unsigned int *L1_mem = rt_alloc(RT_ALLOC_CL_DATA,
                                    CORE_NUMBER*sizeof(unsigned int));

    for(int i=0; i<CORE_NUMBER;i++){
    	L1_mem[i] = SEED;
        runs.success_counter[i] = 0;
        runs.failure_counter[i] = 0;
        runs.successes = 0;
        runs.failures = 0;
	}

    /* Runs NUM_TESTS tests. Each test with PROBLEM_SIZE calls to random_gen() */
    for(int j=0;j<TEST_RUNS;j++) {

        done = 0;
        watchdog = 0;
        rt_cluster_call(NULL, CID, cluster_entry, L1_mem, stacks, STACK_SIZE,
                        STACK_SIZE, rt_nb_pe(), p_event);

        /* Wait for cluster */
        rt_event_yield(p_sched);

        calls++;

        for (int i = 0; i < CORE_NUMBER; i++) {
			if (L1_mem[i]^rand_values[j]){
                runs.failure_counter[i]++;
                runs.failures++;
			} else {
				runs.success_counter[i]++;
                runs.successes++;
			}
           if (verbose == 1)
               printf("%d ", L1_mem[i]);
        }
    }    

    rt_free(RT_ALLOC_CL_DATA, L1_mem, CORE_NUMBER*sizeof(unsigned int));
    return runs;
}

/* The CLUSTER function: Generate a random number */
void random_gen(void *arg)
{
    unsigned int *L1_mem = (unsigned int *) arg;
    int i;

    /* Reset SEED for each run */
    /*if(rt_core_id()==3){*/
        /*L1_mem[rt_core_id()] = SEED;*/
    /*}*/

    for (i = 0; i < PROBLEM_SIZE; i++) {
        rand_r(&L1_mem[rt_core_id()]);
    }
}

/* Forks the job to the cores */
void cluster_entry(void *arg)
{
    rt_team_fork(CORE_NUMBER, random_gen, (void *) arg);
}

/* Called when cluster execution is ended */
void end_of_call(void *arg)
{
    /*printf("End of call\n");*/
    done = 1;
}
