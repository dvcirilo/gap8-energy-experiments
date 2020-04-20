#!/usr/bin/env python
import sys
import pyenergy
from time import sleep
from datetime import datetime

BOARD_ID = "EE00"
M_POINT = 1


em = pyenergy.EnergyMonitor(BOARD_ID)

# Calibration for STM32F407G-DISC1 - MB997D -  
em.measurement_params[1]['resistor'] = 0.55
em.measurement_params[1]['gain'] = 50
em.measurement_params[1]['vref'] = 2.946

em.enableMeasurementPoint(M_POINT)
first = True
print "timestamp,time,energy,avg_power,avg_current,avg_voltage"
while True:
    em.setTrigger("PA0", 1)

    while not em.isRunning(M_POINT):
        sleep(0.1)

    # print "triggered..."

    while not em.measurementCompleted(M_POINT):
        sleep(0.1)

    m = em.getMeasurement(M_POINT)
    output = ''
    title = ''
    # for attr, value in m.__dict__.iteritems():
        # title = title + attr + ','
        # output = output + str(value) + ','
    # if (first):
        # print title[:-1]
        # first = False
    # print datetime.now().strftime('%Y-%m-%d %H:%M:%S') + output[:-1]
    print datetime.now().strftime('%Y-%m-%d %H:%M:%S') + ',' + str(m.time) + ',' + str(m.energy) + ',' + str(m.avg_power) + ',' + str(m.avg_current) + ',' + str(m.avg_voltage)
    sys.stdout.flush()
    # print em.measurement_params[M_POINT]

