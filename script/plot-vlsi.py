#!/usr/bin/env python3

import csv
import numpy as np
import pandas as pd
import sys
import scipy.stats as stats
import matplotlib
matplotlib.use('svg')
import matplotlib.pyplot as plt

def main():
    filename = "../out-clean/out_87_vsweep_01.csv"
    x_field = "voltage"
    y_field = "energy"

    df = pd.read_csv(filename)

    x = df[x_field]
    y = df[y_field]

    # # Plot the histogram.
    # plt.hist(data, bins=25, density=True, alpha=0.6, color='g')

    # plot the pdf.
    plt.rc('grid', linestyle="dotted", color='gray')
    plt.grid()
    plt.plot(x, y, '-or', label='f=87MHz', alpha=0.7, markersize=2)

    filename = "../out-clean/out_220_vsweep_01.csv"
    x_field = "voltage"
    y_field = "energy"

    df = pd.read_csv(filename)

    x = df[x_field]
    y = df[y_field]

    # # Plot the histogram.
    # plt.hist(data, bins=25, density=True, alpha=0.6, color='g')

    # plot the pdf.
    plt.plot(x, y, '-ob', label='f=220MHz', alpha=0.7, markersize=2)
    # plt.yticks(np.arange(y.min(), y.max()*1.02,0.001))
    plt.title("")
    plt.legend(loc='upper left')
    plt.xlabel("Voltage (mV)")
    plt.ylabel("Energy (J)")
    plt.savefig(filename.split('.')[0] + x_field + '_' + y_field + '.svg',bbox_inches="tight")
    plt.clf()
    
    filename = "../out_fsweep_long_1V_01.csv"
    x_field = "meas_freq"
    y_field = "energy"

    df = pd.read_csv(filename)

    x = df[x_field]
    y = df[y_field]

    # # Plot the histogram.
    # plt.hist(data, bins=25, density=True, alpha=0.6, color='g')

    # plot the pdf.
    plt.grid()
    plt.plot(x, y, 'or', label='V=1V', alpha=0.7, markersize=2)

    plt.title("")
    plt.legend(loc='upper left')
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Energy (J)")
    plt.savefig(filename.split('.')[0] + x_field + '_' + y_field + '.svg',bbox_inches="tight")

    plt.clf()

    filename = "../out_fsweep_long_1V_01.csv"
    x_field = "meas_freq"
    y_field = "time"

    df = pd.read_csv(filename)

    x = df[x_field]
    y = df[y_field]

    # # Plot the histogram.
    # plt.hist(data, bins=25, density=True, alpha=0.6, color='g')

    # plot the pdf.
    plt.grid()
    plt.plot(x, y, 'ob', label='V=1V', alpha=0.7, markersize=2)

    plt.title("")
    plt.legend(loc='upper left')
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Time (s)")
    plt.savefig(filename.split('.')[0] + x_field + '_' + y_field + '.svg',bbox_inches="tight")

    plt.clf()
    filename = "../out_fsweep_long_1V_01.csv"
    x_field = "meas_freq"
    y_field = "avg_power"

    df = pd.read_csv(filename)

    x = df[x_field]
    y = df[y_field]

    # # Plot the histogram.
    # plt.hist(data, bins=25, density=True, alpha=0.6, color='g')

    # plot the pdf.
    plt.grid()
    plt.plot(x, y, 'og', label='V=1V', alpha=0.7, markersize=2)

    plt.title("")
    plt.legend(loc='upper left')
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Avg. Power (W)")
    plt.savefig(filename.split('.')[0] + x_field + '_' + y_field + '.svg',bbox_inches="tight")

    plt.clf()

if __name__ == '__main__':
    main()
