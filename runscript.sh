#!/usr/bin/env bash

DONE=0
CURRDIR=`pwd`
OUTDIR=$CURRDIR/out
mkdir -p $OUTDIR

TIMEOUT=30

while [[ $DONE == 0 ]]
do
    TIMESTAMP=$(date "+%Y.%m.%d-%H.%M.%S")
    OUTFILE=output-$TIMESTAMP
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
    DONE=1
done
