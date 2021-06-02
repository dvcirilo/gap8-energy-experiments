#!/usr/bin/env bash

CURRDIR=`pwd`
OUTDIR=$CURRDIR/out
mkdir -p $OUTDIR

F_MIN=300
F_MAX=400
TEST_RUNS=10
PROBLEM_SIZE=10000  # Random number generator iterations
TEST_REPEAT=1       # RNG calls at same voltage/frequency
VOLTAGE=1150        # Initial applied voltage
MAX_V=1200          # Maximum applied voltage
REPEATS=2           # Repeats the same voltage after timeout.
REPEAT_COUNTER=$REPEATS
TIMEOUT=2

while (( $VOLTAGE <= $MAX_V ))
do
    TIMESTAMP=$(date "+%Y.%m.%d-%H.%M.%S")
    OUTFILE=output-$TIMESTAMP
    export APP_CFLAGS="-O0 -DF_MIN=${F_MIN} -DTEST_REPEAT=${TEST_REPEAT}\
                -DPROBLEM_SIZE=${PROBLEM_SIZE} -DF_MAX=${F_MAX}\
                -DTEST_RUNS=${TEST_RUNS} -DVOLTAGE=${VOLTAGE}"
    echo $APP_CFLAGS
    make clean all run &>> $OUTDIR/$OUTFILE &
    #make clean all run platform=gvsoc &
    TEST_PID=$!
    python $CURRDIR/script/energy-measurement.py &>> $OUTDIR/measurements-$TIMESTAMP &
    MEAS_PID=$!
    END=0

    # While end is not reached and test is still running...
    while [[ "$END" == "0" ]]  && kill -s 0 $TEST_PID > /dev/null 2>&1
    do
        STARTMOD=$(stat -c '%Y' $OUTDIR/$OUTFILE)
        sleep $TIMEOUT
        MODIFIED=$(stat -c '%Y' $OUTDIR/$OUTFILE)

        # If the last modification time is equal current, the file has not
        # been updated and thus the test is frozen.
        if [[ $STARTMOD == $MODIFIED ]]; then
            echo "Timeout!!"
            # Saves timeout message and timestamp to output file.
            echo "Timeout!!" $(date "+%Y.%m.%d-%H.%M.%S") \
                $((`date '+%s'`-$MODIFIED))s >> $OUTDIR/$OUTFILE
            END=1

            kill $TEST_PID
            kill $MEAS_PID

            # Sometimes the gap8 communication driver keeps running, kill it
            # to prevent errors on the next execution.
            killall -9 gap8-openocd
        fi
    done

    # Kill measurement script when test is finished.
    kill $MEAS_PID
    let "REPEAT_COUNTER--"
    echo $REPEAT_COUNTER
    if [[ $REPEAT_COUNTER == 0 ]]; then
        let "VOLTAGE+=50"
        REPEAT_COUNTER=$REPEATS
    fi
done
