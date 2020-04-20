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
    if sys.argv[3:]:
        filename = sys.argv[1]
        x_field = sys.argv[2]
        y_field = sys.argv[3]
    else:
        raise Exception("Missing file name argument")

    df = pd.read_csv(filename)

    x = df[x_field]
    y = df[y_field]

    # # Plot the histogram.
    # plt.hist(data, bins=25, density=True, alpha=0.6, color='g')

    # plot the pdf.
    plt.scatter(x, y, 5)
    # plt.yticks(np.arange(y.min(), y.max()*1.02,0.001))
    plt.title(filename)
    plt.xlabel(x.name.title())
    plt.ylabel(y.name.title())
    plt.savefig(filename.split('.')[0] + '_' + x_field + '_' + y_field + '.svg')

if __name__ == '__main__':
    main()
