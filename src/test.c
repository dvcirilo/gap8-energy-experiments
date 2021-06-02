#include <stdio.h>
#include <stdlib.h>
#include "pmsis.h"
#include "pmsis/implem/drivers/pmsis_it.h"
#include "test.h"
#include "util.h"

volatile unsigned int rand_ret = (unsigned int) SEED;

/* Stores the reference rand values */
unsigned int rand_values[TEST_RUNS];
int cluster_flag = 0;

void end_cluster(void *arg)
{
    cluster_flag = 1;
    pi_yield();
}

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
    printf("Voltage: %d V\n", current_voltage());

    /* Set FC (SoC) Frequency */
    int32_t cur_fc_freq = pi_fll_set_frequency(FLL_SOC, FC_FREQ*MHZ, 1);
    if (cur_fc_freq == -1)
    {
        printf("Error changing frequency !\nTest failed...\n");
        pmsis_exit(-3);
    }
    printf("Soc FC frequency: %d MHz\n", pi_fll_get_frequency(FLL_SOC, 1)/MHZ);

    /* Generate comparison values */
    printf("Generating %dk rand values with seed %d\n", 
                        PROBLEM_SIZE*TEST_RUNS/1000, SEED);
    generate_rands(rand_values, rand_ret, TEST_RUNS, PROBLEM_SIZE);

    printf("Done. Running proceeding with test...\n");

    struct pi_device cluster_dev;
    struct pi_cluster_conf conf;

    // First open the cluster
    pi_cluster_conf_init(&conf);

    pi_open_from_conf(&cluster_dev, &conf);

    if (pi_cluster_open(&cluster_dev)) pmsis_exit(-1);

    int fmax, fstep, freq;
    int fstep2, f_mid;

    freq = F_MIN*MHZ;
    fstep =  20*MHZ;
    fstep2 = 2*MHZ;
    f_mid = 200*MHZ;
    if (VOLTAGE > 1050)
        f_mid=250*MHZ;

    printf("voltage,set_freq,meas_freq,TEST_REPEAT,TEST_RUNS,PROBLEM_SIZE,SEED,success,failure,lockups\n");

    while (freq <= F_MAX*MHZ) {
        if(set_voltage_current(freq, VOLTAGE)){
            printf("Failed to assign freq/voltage\n");
            break;
        }

        for (int k = 0; k < TEST_REPEAT; k++) {
            /* Delay to allow measurements */
            pi_time_wait_us(1000*1000); /* 1s */

            /* Set trigger */
            pi_gpio_pin_write(&gpio, trigger, 1);

            /* Run call the test */
            runs = test_rand(&cluster_dev, &conf);

            /* Unset trigger */
            pi_gpio_pin_write(&gpio, trigger, 0);
 
            printf("%d,%d,%d", VOLTAGE, freq, pi_fll_get_frequency(FLL_CLUSTER,1));
            printf(",%d,%d,%d,%d",TEST_REPEAT, TEST_RUNS, PROBLEM_SIZE, SEED);

            printf(",%d,%d,%d", runs.successes, runs.failures, runs.lockups);
            printf("\n");

            /* exits if a lockup has happened */
            if (runs.lockups != 0){
                pmsis_exit(0);
            }

        }

        if (freq < f_mid){
            freq += fstep;
        }else{
            freq += fstep2;
        }
    }

    pi_cluster_close(&cluster_dev);

    printf("Test success: Leaving main controller\n");
    pmsis_exit(0);
}

struct run_info test_rand(struct pi_device *cluster_dev, struct pi_cluster_conf *conf)
{
    struct pi_cluster_task cluster_task;

    struct pi_task wait_task = {};
    pi_task_callback(&wait_task, end_cluster, NULL);

    struct run_info runs;
    int counter, timeout_flag;
   
    /* initialize with seed for each test */
    rand_ret = SEED;
    runs.successes = 0;
    runs.failures = 0;
    runs.lockups = 0;

    /* Runs NUM_TESTS tests. Each test with PROBLEM_SIZE calls to random_gen() */
    for(int j=0;j<TEST_RUNS;j++) {

        cluster_flag = 0;
        /* Then offload an entry point, this will get executed on the cluster controller */
        pi_cluster_send_task_to_cl_async(
                cluster_dev,
                pi_cluster_task(&cluster_task, cluster_entry, NULL),
                &wait_task
        );

        counter = 0; 
        timeout_flag = 0;
        while(cluster_flag == 0){
            /* wait for a time proportional to the problem size */
            pi_time_wait_us(PROBLEM_SIZE/10);
            /* using a counter to prevent waiting for too long */
            counter++; 
            if (counter > 5){
                counter = 0;
                timeout_flag = 1;
                /* calls the cluster end task */
                pi_task_push(&wait_task);
            }
        }

        if (timeout_flag == 1) {
            runs.lockups++;
            return runs;
        } else if (rand_ret^rand_values[j]){
            runs.failures++;
            /* sets the correct value for the next iteration */
            rand_ret = rand_values[j];
        } else
            runs.successes++;

        /*printf("%d,%d,%d\n", runs.successes, runs.failures, runs.lockups);*/

    }    

    return runs;
}

/* What runs on the cluster */
void cluster_entry(void *arg)
{
    int i;
    unsigned int rand_num = rand_ret;

    for (i = 0; i < PROBLEM_SIZE; i++)
        rand_r(&rand_num);

    rand_ret = rand_num;
}
