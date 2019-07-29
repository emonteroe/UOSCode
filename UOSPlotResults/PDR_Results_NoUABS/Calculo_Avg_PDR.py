#!/usr/bin/env python2
# -*- coding: utf-8 -*-
"""
Created on Mon Jul 29 13:34:40 2019

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

with open('mean') as MeanPDR:

    data1 = np.array(list((float(x), float(y), float(z), int(cellid)) for x, y, z, cellid in csv.reader(fenBs, delimiter= ',')))
    