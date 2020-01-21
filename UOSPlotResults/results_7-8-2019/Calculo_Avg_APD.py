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

def jfi(vetor): #Fairness Index
    sum0=0
    sum1=0
    jfi = [None] * 26
    for i in range(26):
        sum0 += vetor[i]
        sum1 += pow(vetor[i],2)
        jfi[i] = pow(sum0,2)/((i+1)*sum1)
    return jfi

with open('meanAPD_Sum') as MeanThroughput:

    data1 = np.array(list((int(Run),int(time), float(Throughput)) for Run, time, Throughput in csv.reader(MeanThroughput, delimiter= ',')))

with open('meanAPD_UABS_Sum') as MeanThroughputUABS:

    data2 = np.array(list((int(Run1),int(time1), float(Throughput1)) for Run1, time1, Throughput1 in csv.reader(MeanThroughputUABS, delimiter= ',')))    


Run,time,Throughput = data1.T
Run1,time1,Throughput1 = data2.T


# Mean general scenario
countarr = [None] * 26 #debe ser 33 o 30 o 20 o el # de runs
Throughputarr = [None] * 26  #debe ser 33 o 30 o 20 o el # de runs
ThroughputMean = [None] * 26 #debe ser 33 o 30 o 20 o el # de runs
for i in range(26):
    count=0
    sumplr=0
    for j in range(len(Run)):
        if (Run[j]==i):
            count+=1
            countarr[i]=count
            sumplr+= Throughput[j]
            Throughputarr[i] = sumplr
    ThroughputMean[i] = (Throughputarr[i]/countarr[i])

fairness_index_LTE = jfi(ThroughputMean)

# Mean UABS scenario
countarr1 = [None] * 26 #debe ser 33 o 30 o 20 o el # de runs
Throughputarr1 = [None] * 26 #debe ser 33 o 30 o 20 o el # de runs
ThroughputMean1 = [None] * 26 #debe ser 33 o 30 o 20 o el # de runs
for i in range(26):
    count1=0
    sumplr1=0
    for j in range(len(Run1)):
        if (Run1[j]==i):
            count1+=1
            countarr1[i]=count1
            sumplr1+= Throughput1[j]
            Throughputarr1[i] = sumplr1
    ThroughputMean1[i] = (Throughputarr1[i]/countarr1[i])
    
fairness_index_UOS = jfi(ThroughputMean1)
    
colors  = ["#b3ffd6","#809c8c" ]
patterns = ['\/\/','\/']
plt.style.use("classic")      
plt.figure()
#tight_layout()    
plt.grid(color='green')     
plt.xticks([0,1],('LTE','LTE + UOS') ,fontsize=18)#, plt.yticks([5,10,15,20,25])
plt.yticks(fontsize=16)
plt.xlabel('Scenarios',fontsize=18)
plt.ylabel('APD (ms)', fontsize=16)    
plt.xlim(-0.5,1.5)   
bars = plt.bar(np.arange(2),[np.mean(ThroughputMean),np.mean(ThroughputMean1)], yerr=[mean_confidence_interval(ThroughputMean),mean_confidence_interval(ThroughputMean1)],error_kw=dict(lw=2, capsize=5, capthick=2), width =0.30)
for color, bar,pattern in zip(colors, bars,patterns):
        bar.set_facecolor(color)
#        bar.set_hatch(pattern)
        bar.set_edgecolor("black")
        bar.set_linewidth(1)
plt.savefig("Graph_Avg_APD.pdf", format='pdf', dpi=1000)
plt.show()

#test= statistics.mean(Throughput)
tmean= np.mean(ThroughputMean)

print("APD Avg General: " + str(np.mean(ThroughputMean)))

#test1= statistics.mean(Throughput1)
tmean1= np.mean(ThroughputMean1)

print("APD Avg UABS: " + str(np.mean(ThroughputMean1)))

ImprovRatio =tmean1/tmean

print("Improvement ratio: " + str(ImprovRatio))  # improvement ratio = value after change / value before change

print("Improvement %:" + str( 100 * (ImprovRatio - 1)) +"%") 
      