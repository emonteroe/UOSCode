#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Jul 29 15:12:25 2019

@author: emanuel
"""

from sklearn.cluster import DBSCAN
from sklearn.neighbors import KNeighborsClassifier
import numpy as np
from matplotlib import pyplot as plt
from sklearn.datasets.samples_generator import make_blobs
from sklearn.preprocessing import StandardScaler
from pandas import DataFrame
from sklearn import metrics
import scipy.stats
import statistics
import csv
import re

def mean_confidence_interval(data, confidence=0.95):
    a = 1.0 * np.array(data)
    n = len(a)
    m, se = np.mean(a), scipy.stats.sem(a)
    h = se * scipy.stats.t.ppf((1 + confidence) / 2., n-1)
    return h

with open('meanThroughput_Sum') as MeanThroughput:

    data1 = np.array(list((int(Run),int(time), float(Throughput)) for Run, time, Throughput in csv.reader(MeanThroughput, delimiter= ',')))

with open('meanThroughput_UABS_Sum') as MeanThroughputUABS:

    data2 = np.array(list((int(Run1),int(time1), float(Throughput1)) for Run1, time1, Throughput1 in csv.reader(MeanThroughputUABS, delimiter= ',')))    


Run,time,Throughput = data1.T
Run1,time1,Throughput1 = data2.T



# Mean general scenario
countarr = [None] * 8 #debe ser 33 o 30 o 20 o el # de runs
Throughputarr = [None] * 8  #debe ser 33 o 30 o 20 o el # de runs
ThroughputMean = [None] * 8 #debe ser 33 o 30 o 20 o el # de runs
for i in range(8):
    count=0
    sumplr=0
    for j in range(len(Run)):
        if (Run[j]==i):
            count+=1
            countarr[i]=count
            sumplr+= Throughput[j]
            Throughputarr[i] = sumplr
    ThroughputMean[i] = (Throughputarr[i]/countarr[i])

# Mean UABS scenario
countarr1 = [None] * 9 #debe ser 33 o 30 o 20 o el # de runs
Throughputarr1 = [None] * 9 #debe ser 33 o 30 o 20 o el # de runs
ThroughputMean1 = [None] * 9 #debe ser 33 o 30 o 20 o el # de runs
for i in range(9):
    count1=0
    sumplr1=0
    for j in range(len(Run1)):
        if (Run1[j]==i):
            count1+=1
            countarr1[i]=count1
            sumplr1+= Throughput1[j]
            Throughputarr1[i] = sumplr1
    ThroughputMean1[i] = (Throughputarr1[i]/countarr1[i])
    
plt.figure()
#tight_layout()    
plt.grid()    
plt.xticks([0,1],('General','UABS'))#, plt.yticks([5,10,15,20,25])
plt.xlabel('Scenarios')
plt.ylabel('Throughput (Kbps)')    
plt.bar(np.arange(2),[np.mean(ThroughputMean),np.mean(ThroughputMean1)], yerr=[mean_confidence_interval(ThroughputMean),mean_confidence_interval(ThroughputMean1)])
plt.show()

#test= statistics.mean(Throughput)
tmean= np.mean(ThroughputMean)

print("Throughput Avg General: " + str(np.mean(ThroughputMean)))

#test1= statistics.mean(Throughput1)
tmean1= np.mean(ThroughputMean1)

print("Throughput Avg UABS: " + str(np.mean(ThroughputMean1)))

ImprovRatio =tmean1/tmean

print("Improvement ratio: " + str(ImprovRatio))  # improvement ratio = value after change / value before change

print("Improvement %:" + str( 100 * (ImprovRatio - 1)) +"%") 
      