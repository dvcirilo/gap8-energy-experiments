#!/usr/bin/env bash

DONE=5
CURRDIR=`pwd`
OUTDIR=$CURRDIR/out
mkdir -p $OUTDIR

TIMEOUT=120
F_MIN=200
F_MAX=350
PROBLEM_SIZE=100000
TEST_RUNS=1000
TEST_REPEAT=10
VOLTAGE=1000


while (( $VOLTAGE <= "1200" ))
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

    # While end is no reached and test is still running...
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
    let "VOLTAGE+=50"
done
