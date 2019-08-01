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

with open('teste_nagib') as MeanThroughput:

    data1 = np.array(list((int(Run),int(flow), float(Throughput)) for Run, flow, Throughput in csv.reader(MeanThroughput, delimiter= ' ')))
    


Run,flow,Throughput = data1.T

test= statistics.mean(Throughput)

print("Throughput Avg Total: " + str(test))

countarr = [None] * 30
Throughputarr = [None] * 30
ThroughputMean = [None] * 30
for i in range(30):
    count=0
    sumplr=0
    for j in range(len(Run)):
        if (Run[j]==i):
            count+=1
            countarr[i]=count
            sumplr+= Throughput[j]
            Throughputarr[i] = sumplr
    ThroughputMean[i] = (Throughputarr[i]/countarr[i])
    
plt.figure()
#tight_layout()    
plt.grid()    
plt.xticks([0,1,2],('General','OL enB w/o UABS','OL enB with UABS'))#, plt.yticks([5,10,15,20,25])
plt.xlabel('Scenarios')
plt.ylabel('Throughput (Mbps)')    
plt.bar(np.arange(3),[np.mean(ThroughputMean),np.mean(ThroughputMean),np.mean(ThroughputMean)], yerr=[mean_confidence_interval(ThroughputMean),mean_confidence_interval(ThroughputMean),mean_confidence_interval(ThroughputMean)])
plt.show()


      