import PyPredictiveFilter as pypfilt
import argparse, thread, pickle
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import sys
import ctypes

_DEBUG=True

def getPrediction(index,time,dataset,numPointsToFilter,pf,cutoffFreq,polyorder,overridePointsToFit=0):
    # currIdx = offset + numPointsToFilter
    ## Add data to filter
    if index < numPointsToFilter:
        print "Can't get prediction with dataset at index: ", index, ' with numPointsToFilter: ', numPointsToFilter
        return None
    if _DEBUG:
        print "Adding dataset[",str(index-numPointsToFilter),":",str(index),"]"
    ret = pf.addData(time[index-numPointsToFilter:index], dataset[index-numPointsToFilter:index])
    if ret:
        print "*** addData() failed! returned", ret
    
    ## Apply filter
    ret = pf.filter(cutoffFreq, polyorder,numPointsToFit=overridePointsToFit)
    if ret:
        print "*** filter() failed! returned", ret

    ## Get next predicted point at next deltaT timestep
    p = pf.getPrediction(time[index+1])
    ## OOH SOMETIMES THERE IS A PROBREM HERE!!!
    if (abs(p) > 1000): #(Faster, this way is more correct though): > abs(5*np.std(dataset[ii:currIdx]) + np.mean(dataset[ii:currIdx]))):
        print "UH OH! PROBLEM WITH PREDICTION ON RANGE [",str(index-numPointsToFilter),":",str(index),"]"
        print "Truth: ",str(dataset[index+1])," Predicted: ",p
        ## ADD A VERTICAL LINE TO THE PLOT SO WE CAN TELL WHERE THIS HAPPENED?
        p=0

    # predictedPoints[currIdx+1] = p
    return p


print "Creating predictive filter object..."
pf = pypfilt.PyPredictiveFilter()


x=np.linspace(0,6*np.pi,300)
dataset=15*np.sin(x)+np.random.ranf(len(x))*3
time = np.array(np.arange(len(x)), dtype='double')


numPointsToFilter = 128
print "Configuring filter to use " + str(numPointsToFilter) + " points in in the low pass filter..."
ret = pf.configure(numPointsToFilter=numPointsToFilter)
print "returned: ",ret

# Configurable filter values:
polyorder = 2
cutoffFreq = 20.0 # Hz
overridePointsToFit=25

cdresiduals = np.zeros(len(dataset))
predictedPoints = np.zeros(len(dataset))

for ii in np.arange(len(dataset)-1):
    predictedPoints[ii+1] = getPrediction(ii,time,dataset,numPointsToFilter, pf,cutoffFreq,polyorder,overridePointsToFit=overridePointsToFit)
    if np.mod(ii,100) == 0: print "Iter ", str(ii),"/",str(len(dataset))
residuals = dataset - predictedPoints
print "Residual [mean, std]: [", str(np.mean(residuals[numPointsToFilter+1:])), ",",str(np.std(residuals[numPointsToFilter+1:])),"]"

###### Generate plot

plt.figure()
plt.plot(time,dataset,'b',label='RawData')
plt.plot(time,residuals,'r',label='Residual')
plt.plot(time,predictedPoints,'gs',label='Predicted Points')
plt.grid()
plt.legend()
plt.show()










