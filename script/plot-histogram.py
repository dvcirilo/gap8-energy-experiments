#!/usr/bin/env python3

import csv
import numpy as np
import pandas as pd
import sys
import scipy.stats as stats
import matplotlib
matplotlib.use('svg')
import matplotlib.pyplot as plt

fig, ax = plt.subplots(1,1)

def main():
    if sys.argv[2:]:
        filename = sys.argv[1]
        plot_var = sys.argv[2]
    else:
        raise Exception("Missing file name argument")

    df = pd.read_csv(filename)

    # # Plot the histogram.
    data = df[df['fail']!=0][plot_var].to_numpy()

    print("Length:    %d" % int(len(data)))
    print("Min:       %1.2f" % int(data.min()))
    print("Max:       %1.2f" % int(data.max()))
    print("Mean:      %1.2f" % float(data.mean()))
    print("StdDev:    %1.2f" % float(data.std()))
    print("Var:       %1.2f" % float(data.var()))
    print("Peak2Peak: %1.2f" % int(data.ptp()))

    ax.hist(data, bins=100, density=True, alpha=0.6, color='g')

    # mu, std = stats.norm.fit(data.tolist())
    # xmin, xmax = plt.xlim()
    # x = np.linspace(xmin, xmax, 100)
    # p = stats.norm.pdf(x, mu, std)
    # p = stats.norm.pdf(data, data.mean(), data.std())
    # ax.plot(data, stats.norm.pdf(data), 'k', linewidth=2)


    # # plot the pdf.
    # plt.scatter(x, y, 5)
    # # plt.yticks(np.arange(y.min(), y.max()*1.02,0.001))
    plt.title(filename)
    # plt.xlabel(x.name.title())
    # plt.ylabel(y.name.title())
    plt.savefig(filename.split('.')[0] + '.svg')

if __name__ == '__main__':
    main()
