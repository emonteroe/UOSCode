#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Apr 12 16:40:04 2019

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
import statistics
import csv
import re



# generate 2d classification dataset (this will represent the users and the eNodeBs)
#X, y = make_blobs(n_samples=10000, centers= 4, n_features=2, shuffle = False, cluster_std=1.2)
# scatter plot, dots colored by class value
#print(X.shape)

with open('/home/emanuel/Desktop/ns-3/source/ns-3.29/enBs') as fenBs:

    data1 = np.array(list((float(x), float(y), float(z), int(cellid)) for x, y, z, cellid in csv.reader(fenBs, delimiter= ',')))
    
with open('/home/emanuel/Desktop/ns-3/source/ns-3.29/LTEUEs') as fUEs:
    data2 = np.array(list((float(x), float(y), float(z)) for x, y, z in csv.reader(fUEs, delimiter= ',')))
    
with open('/home/emanuel/Desktop/ns-3/source/ns-3.29/UABSs') as fUABS:
    data3 = np.array(list((float(x), float(y), float(z), int(cellid)) for x, y, z, cellid in csv.reader(fUABS, delimiter= ',')))

with open('/home/emanuel/Desktop/ns-3/source/ns-3.29/UEsLowSinr') as fUEsLow:
    data4 = np.array(list((float(x), float(y), float(z), float (Sinr), int (Imsi),int(cellid)) for x, y, z, Sinr,Imsi, cellid in csv.reader(fUEsLow, delimiter= ',')))

#print("enBs: "+ str(data1))
#print("UEs: "+ str(data2))
#print("UABSs: "+ str(data3))
#print("UEsLowSinr: "+ str(data4[0:2][0]))
x,y,z, cellid= data1.T
plt.scatter(x,y,c="blue")

x1,y1,z1= data2.T
plt.scatter(x1,y1,c="gray")

x2,y2,z2, cellid3= data3.T
plt.scatter(x2,y2,c="green")
UABSCoordinates = np.array(list(zip(x2,y2)))

x3,y3,z3, sinr, imsi, cellid4= data4.T
X = np.array(list(zip(x3,y3)))
#X = StandardScaler().fit_transform(X)
#print(X)

plt.scatter(x3,y3,c="red")

plt.title('BS and UABS Scenario 1')
plt.xlabel('x (meters)')
plt.ylabel('y (meters)')
plt.legend()
plt.show()
print(X.size)

##Clustering with DBSCAN
DBClusters = DBSCAN( eps=500, min_samples=2, metric ='euclidean',algorithm = 'auto')
DBClusters.fit(X)
#DBClusters.labels_

# Number of clusters in labels, ignoring noise if present.
n_clusters_ = len(set(DBClusters.labels_)) - (1 if -1 in DBClusters.labels_ else 0)
core_samples = np.zeros_like(DBClusters.labels_, dtype = bool)
core_samples[DBClusters.core_sample_indices_] = True

# PRINT CLUSTERS & # of CLUSTERS
#print("Clusters:"+str(DBClusters.labels_))

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
   # print("Tamano Cluster "+ str(i) + ": " + str(len(clusters[i])))
    for j in range(len(clusters[i])):
        x_clusters[i].append(clusters[i][j][0])
        y_clusters[i].append(clusters[i][j][1])
        
     
    plt.scatter(x_clusters[i], y_clusters[i])
    colors+=[i]
     
#plot the Clusters 
plt.title("DBSCAN Clustering")
plt.legend(colors)
plt.show()

 
#Sum of SINR and mean to later prioritize the clusters  
SUMSinr = [None] * len(clusters)

for i in range(len(clusters)):
    SUMSinrClusters = 0
    for j in range(len(clusters[i])):
        index_x3 = np.where(x3 == clusters[i][j][0])
#        print("Found x3: "+str(np.where(x3 == clusters[i][j][0]))) # para comparar con x3
#        print("Found y3: "+str(np.where(y3 == clusters[i][j][1]))) # para comparar con x3
        for k in range(len(index_x3)):
            if (y3[index_x3[k]] == clusters[i][j][1]):
#                print("SINR FOUND: " + str(sinr[index_x3[k]]))
                SUMSinrClusters += sinr[index_x3[k]]
#                print(sinr[index_x3[k]])
#                print(SUMSinrClusters)
#   SUMSinr[i] = sinr[index_x3[k]]
    SUMSinr[i] = SUMSinrClusters        

SINRAvg = [None] * len(clusters)

for i in range(len(SUMSinr)):
    SINRAvg[i] = SUMSinr[i]/len(clusters[i])

#Prioritize by greater SINR    
CopySINRAvg = SINRAvg.copy()
SINRAvgPrioritized = []
for i in range(len(SINRAvg)):
    #print("SINR Max:" + str(max(CopySINRAvg)))
    SINRAvgPrioritized.append(max(CopySINRAvg))
    CopySINRAvg.remove(max(CopySINRAvg))
   
       
     
#Centroids - median of clusters
x_clusters_mean = [None] * len(clusters)
y_clusters_mean = [None] * len(clusters)
for i in range(len(clusters)):
    x_clusters_mean[i] = []
    y_clusters_mean[i] = []
    x_clusters_mean[i].append(statistics.mean(x_clusters[i]))
    y_clusters_mean[i].append(statistics.mean(y_clusters[i]))
    
Centroids = list(zip([i[0] for i in x_clusters_mean],[i[0] for i in y_clusters_mean]))


    
#Reorder Centroides based on prioritized AVGSINR
CentroidsPrio = []   
for i in range(len(SINRAvg)):
    index_SAP = np.where(SINRAvg == SINRAvgPrioritized[i] )
#    print(index_SAP[0])
#    print(Centroids[int(index_SAP[0])])
    CentroidsPrio.append(Centroids[int(index_SAP[0])])
    
for i in CentroidsPrio:
    print("{} {} ".format(i[0], i[1]))
#centroidsarray = np.asarray(Centroids)
#print(centroidsarray)
    


#  KNN Implementation for finding the nearest UABS to the X Centroid.
# Create the knn model.
# Look at the five closest neighbors.
Kneighbors = 2
knn = KNeighborsClassifier(n_neighbors= Kneighbors, weights= "uniform" , algorithm="auto")
knn.fit(UABSCoordinates,cellid3)
#predict witch UABS will be serving to the X Centroid.
Knnpredict= knn.predict(CentroidsPrio)

#scores = {}
#scores_list = []
#for k in range(Kneighbors):
#    scores[k] = metrics.accuracy_score(cellid3,Knnpredict)
#    scores_list.append(metrics.accuracy_score(cellid3,Knnpredict))