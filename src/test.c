#include <stdio.h>
#include <stdlib.h>
#include "pmsis.h"
#include "pmsis/implem/drivers/pmsis_it.h"
#include "test.h"
#include "util.h"

unsigned int rand_values[TEST_RUNS];
volatile unsigned int rand_ret;
struct pi_task test_end;

int main(void)
{
    printf("\n\n\t *** PMSIS Rand Test ***\n\n");
    return pmsis_kickoff((void *) rand_test);
}


void rand_test(void)
{
    struct run_info runs;

    struct pi_device gpio;
    struct pi_gpio_conf gpio_conf = {0};
    pi_gpio_conf_init(&gpio_conf);
    pi_open_from_conf(&gpio, &gpio_conf);
    pi_gpio_open(&gpio);
    pi_gpio_e trigger = PI_GPIO_A17_PAD_31_B11;
    pi_gpio_pin_configure(&gpio, trigger, PI_GPIO_OUTPUT);

    int i, j;

    printf("\n/*********** EXECUTION **********/\n");

    if (pi_pmu_voltage_set(PI_PMU_DOMAIN_FC, 1200))
    {
        printf("Error changing voltage !\nTest failed...\n");
        pmsis_exit(-2);
    }
    printf("%dmV\n", current_voltage());

    /* Set FC (SoC) Frequency */
    int32_t cur_fc_freq = pi_fll_set_frequency(FLL_SOC, FC_FREQ*MHZ, 1);
    if (cur_fc_freq == -1)
    {
        printf("Error changing frequency !\nTest failed...\n");
        pmsis_exit(-3);
    }
    printf("Soc FC frequency: %d MHz\n", pi_fll_get_frequency(FLL_SOC, 1)/MHZ);

    /* Generate comparison values */
    printf("Generating %dM rand values with seed = %d\n",
                               PROBLEM_SIZE*TEST_RUNS/MHZ, SEED);
    generate_rands(rand_values, SEED, TEST_RUNS, PROBLEM_SIZE);

    printf("Done. Running proceeding with test...\n");

    struct pi_device cluster_dev;
    struct pi_cluster_conf conf;
    struct pi_task task;

    // First open the cluster
    pi_cluster_conf_init(&conf);

    pi_open_from_conf(&cluster_dev, &conf);

    if (pi_cluster_open(&cluster_dev)) pmsis_exit(-1);

    int fmax, fstep, freq, finit;
    int voltage;
    int fstep2, f_mid;

    finit = F_MIN*MHZ;
    fstep =  20*MHZ;
    fstep2 = 2*MHZ;
    f_mid = 200*MHZ;

    voltage = V_MIN;
    printf("finit: %d\n", finit);

    printf("voltage,set_freq,meas_freq,success[i],failures[i]\n");

    while (voltage <= V_MAX) {       
        freq = finit;
        switch(voltage)
        {
            case 1000:
                fmax = 218*MHZ;
                break;
            case 1050:
                fmax = 245*MHZ;
                break;
            case 1100:
                fmax = 270*MHZ;
                break;
            case 1150:
                fmax = 295*MHZ;
                break;
            case 1200:
                fmax = F_MAX*MHZ;
                break;
            default:
                fmax = 87*MHZ;
        }

        while (freq <= fmax) {
            
            if(set_voltage_current(freq, voltage)){
                printf("Failed to assign freq/voltage\n");
                break;
            }

            for (int k = 0; k < TEST_REPEAT; k++) {
                /* Delay to allow measurements */
                pi_time_wait_us(1000000);

                /* Set trigger */
                pi_gpio_pin_write(&gpio, trigger, 1);
       
                /* Run call the test */
                runs = test_rand(&cluster_dev, 0);

                /* Unset trigger */
                pi_gpio_pin_write(&gpio, trigger, 0);
     
                printf("%d,%d,%d", voltage, freq, pi_fll_get_frequency(FLL_CLUSTER,1));
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

        voltage += V_STEP;
    }

    pi_cluster_close(&cluster_dev);

    printf("Test success: Leaving main controller\n");
    pmsis_exit(0);
}

/* The actual test. */
struct run_info test_rand(struct pi_device *cluster_dev, int verbose)
{
    int total_time=0, calls=0, call_total=0;
    struct pi_cluster_task cluster_task;
    struct run_info runs;
    
    rand_ret = SEED;
    for(int i=0; i<CORE_NUMBER;i++){
        runs.success_counter[i] = 0;
        runs.failure_counter[i] = 0;
        runs.successes = 0;
        runs.failures = 0;
    }

    /* Runs NUM_TESTS tests. Each test with PROBLEM_SIZE calls to random_gen() */
    for(int j=0;j<TEST_RUNS;j++) {


     /* Then offload an entry point, this will get executed on the cluster controller */
        pi_cluster_send_task_to_cl(cluster_dev,
                    pi_cluster_task(&cluster_task, cluster_entry, NULL));
        /*pi_cluster_send_task_async(cluster_dev,*/
                    /*pi_cluster_task(&cluster_task, cluster_entry, NULL));*/

        calls++;

        for (int i = 0; i < CORE_NUMBER; i++) {
            if (rand_ret^rand_values[j]){
                runs.failure_counter[i]++;
                runs.failures++;
            } else {
                runs.success_counter[i]++;
                runs.successes++;
            }
           if (verbose == 1)
               printf("%d ", rand_ret);
        }
    }    

    return runs;
}

/* The CLUSTER function: Generate a random number */
void random_gen(void *arg)
{
    /*unsigned int *L1_mem = (unsigned int *) arg;*/
    unsigned int rand_num = rand_ret;
    int i;

    /* Reset SEED for each run */
    /*if(pi_core_id()==3){*/
        /*L1_mem[pi_core_id()] = SEED;*/
    /*}*/

    for (i = 0; i < PROBLEM_SIZE; i++) {
        rand_r(&rand_num);
    }
    /*printf("rand_ret = %u\n", rand_num);*/
    rand_ret = rand_num;
}

/* Forks the job to the cores */
void cluster_entry(void *arg)
{
    /*printf("(%ld, %ld) Entering cluster controller\n", pi_cluster_id(), pi_core_id());*/

    // Just fork the execution on all cores
    pi_cl_team_fork(0, random_gen, (void *) arg);
}
