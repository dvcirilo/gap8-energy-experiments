#include <stdio.h>
#include <stdlib.h>
#include "pmsis.h"
#include "pmsis/implem/drivers/pmsis_it.h"
#include "test.h"
#include "util.h"

volatile unsigned int rand_ret;
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
    struct pi_device gpio;
    struct pi_gpio_conf gpio_conf = {0};
    pi_gpio_conf_init(&gpio_conf);
    pi_open_from_conf(&gpio, &gpio_conf);
    pi_gpio_open(&gpio);
    pi_gpio_e trigger = PI_GPIO_A17_PAD_31_B11;
    pi_gpio_pin_configure(&gpio, trigger, PI_GPIO_OUTPUT);

    int i, j, counter, timeout_flag;

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

    printf("Done. Running proceeding with test...\n");

    struct pi_device cluster_dev;
    struct pi_cluster_conf conf;

    // First open the cluster
    pi_cluster_conf_init(&conf);

    pi_open_from_conf(&cluster_dev, &conf);

    if (pi_cluster_open(&cluster_dev)) pmsis_exit(-1);

    struct pi_cluster_task cluster_task;
    struct pi_task wait_task = {};
    pi_task_callback(&wait_task, end_cluster, NULL);
    /*pi_task_block(&wait_task);*/

    int fmax, fstep, freq;
    int fstep2, f_mid;

    freq = F_MIN*MHZ;
    fstep =  20*MHZ;
    fstep2 = 2*MHZ;
    f_mid = 200*MHZ;
    if (VOLTAGE > 1050)
        f_mid=250*MHZ;

    printf("voltage,set_freq,meas_freq,TEST_RUNS,PROBLEM_SIZE,SEED,rand_ret,timeout\n");

    while (freq <= F_MAX*MHZ) {
        if(set_voltage_current(freq, VOLTAGE)){
            printf("Failed to assign freq/voltage\n");
            break;
        }

        for (int k = 0; k < TEST_REPEAT; k++) {
            /* Delay to allow measurements */
            pi_time_wait_us(1000000);
            cluster_flag = 0;

            /* Set trigger */
            pi_gpio_pin_write(&gpio, trigger, 1);

            /* Run call the test */
            pi_cluster_send_task_to_cl_async(
                    &cluster_dev,
                    pi_cluster_task(&cluster_task, cluster_entry, NULL),
                    &wait_task
            );

            counter = 0; 
            timeout_flag = 0;
            while(cluster_flag == 0){
                pi_time_wait_us(500*1000);
                counter++; 
                if (counter > 10){
                    counter = 0;
                    timeout_flag = 1;
                }
            }


            /* Unset trigger */
            pi_gpio_pin_write(&gpio, trigger, 0);
 
            printf("%d,%d,%d", VOLTAGE, freq, pi_fll_get_frequency(FLL_CLUSTER,1));
            printf(",%d,%d,%d,%u", TEST_RUNS, PROBLEM_SIZE, SEED, rand_ret);
            printf(",%d", timeout_flag);
            printf("\n");
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

/* The CLUSTER function: Generate a random number */
void random_gen(void *arg)
{
    /*unsigned int *L1_mem = (unsigned int *) arg;*/
    /*unsigned int rand_num = rand_ret;*/
    unsigned int rand_num = SEED;
    int i,j;

    /* Reset SEED for each run */
    /*if(pi_core_id()==3){*/
        /*L1_mem[pi_core_id()] = SEED;*/
    /*}*/

    for (i = 0; i < TEST_RUNS; i++) {
        for (j = 0; j < PROBLEM_SIZE; j++) {
            rand_r(&rand_num);
        }
    }
    rand_ret = rand_num;
}

/* Forks the job to the cores */
void cluster_entry(void *arg)
{
    /*printf("(%ld, %ld) Entering cluster controller\n", pi_cluster_id(), pi_core_id());*/

    // Just fork the execution on all cores
    /*pi_cl_team_fork(0, random_gen, (void *) arg);*/
    random_gen(arg);
}
