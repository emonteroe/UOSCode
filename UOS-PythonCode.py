#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Apr 12 16:40:04 2019

@author: emanuel
"""
from sklearn.cluster import DBSCAN
import numpy as np
from matplotlib import pyplot as plt
from sklearn.datasets.samples_generator import make_blobs
from sklearn.preprocessing import StandardScaler
from pandas import DataFrame
from sklearn import metrics
import statistics
import csv



# generate 2d classification dataset (this will represent the users and the eNodeBs)
#X, y = make_blobs(n_samples=10000, centers= 4, n_features=2, shuffle = False, cluster_std=1.2)
# scatter plot, dots colored by class value
#print(X.shape)

with open('/home/emanuel/Desktop/ns-3/source/ns-3.29/enBs') as fenBs:

    data1 = np.array(list((float(x), float(y), float(z), int(cellid)) for x, y, z, cellid in csv.reader(fenBs, delimiter= ',')))
    
with open('/home/emanuel/Desktop/ns-3/source/ns-3.29/LTEUEs') as fUEs:
    data2 = np.array(list((float(x), float(y), float(z)) for x, y, z in csv.reader(fUEs, delimiter= ',')))
    
with open('/home/emanuel/Desktop/ns-3/source/ns-3.29/UABSs') as fUABS:
    data3 = np.array(list((float(x), float(y), float(z)) for x, y, z in csv.reader(fUABS, delimiter= ',')))

with open('/home/emanuel/Desktop/ns-3/source/ns-3.29/UEsLowSinr') as fUEsLow:
    data4 = np.array(list((float(x), float(y), float(z), float (Sinr), int (Imsi)) for x, y, z, Sinr,Imsi in csv.reader(fUEsLow, delimiter= ',')))

#print("enBs: "+ str(data1))
#print("UEs: "+ str(data2))
#print("UABSs: "+ str(data3))
#print("UEsLowSinr: "+ str(data4[0:2][0]))
x,y,z, cellid= data1.T
plt.scatter(x,y,c="blue")

x1,y1,z1= data2.T
plt.scatter(x1,y1,c="gray")

#x2,y2,z2= data3.T
#plt.scatter(x2,y2,c="green")

x3,y3,z3, sinr, imsi= data4.T
X = np.array(list(zip(x3,y3)))
#X = StandardScaler().fit_transform(X)
#print(X)

plt.scatter(x3,y3,c="red")

plt.title('BS and UABS Scenario 1')
plt.xlabel('x (meters)')
plt.ylabel('y (meters)')
plt.legend()
plt.show()
#print(X.size)

##Clustering with DBSCAN
DBClusters = DBSCAN( eps=500, min_samples=2, metric ='euclidean',algorithm = 'auto')
DBClusters.fit(X)
#DBClusters.labels_

# Number of clusters in labels, ignoring noise if present.
n_clusters_ = len(set(DBClusters.labels_)) - (1 if -1 in DBClusters.labels_ else 0)
core_samples = np.zeros_like(DBClusters.labels_, dtype = bool)
core_samples[DBClusters.core_sample_indices_] = True

# PRINT CLUSTERS & # of CLUSTERS
print("Clusters:"+str(DBClusters.labels_))

print('Estimated number of clusters: %d' % n_clusters_)

clusters = [X[DBClusters.labels_ == i] for i in range(n_clusters_)]
outliers = X[DBClusters.labels_ == -1]

# Plot Outliers
plt.scatter(outliers[:,0], outliers[:,1], c="black")
# Plot Clusters

x_clusters = [None] * len(clusters)
y_clusters = [None] * len(clusters)
colors = [0]
for i in range(len(clusters)):
    x_clusters[i] = []
    y_clusters[i] = []
    for j in range(len(clusters[i])):
        x_clusters[i].append(clusters[i][j][0])
        y_clusters[i].append(clusters[i][j][1])
#        
    plt.scatter(x_clusters[i], y_clusters[i])
    colors+=[i]

#median of clusters
x_clusters_mean = [None] * len(clusters)
y_clusters_mean = [None] * len(clusters)
for i in range(len(clusters)):
    x_clusters_mean[i] = []
    y_clusters_mean[i] = []
    x_clusters_mean[i].append(statistics.mean(x_clusters[i]))
    y_clusters_mean[i].append(statistics.mean(y_clusters[i]))
    
Centroids = list(zip([i[0] for i in x_clusters_mean],[i[0] for i in y_clusters_mean]))

print(Centroids)
#print(3.25)
#
plt.title("DBSCAN Clustering")
plt.legend(colors)
plt.show()
