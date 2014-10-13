#!/usr/bin/python

import PyPredictiveFilter as pypfilt
import argparse, thread, pickle
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import sys

DEBUG = False

RUNTEST1 = True

def runTest(timestamps, data):

    # How much data we got?
    numDataPoints = len(data)

    # Create a predictive filter object:
    print
    print "Creating predictive filter object..."
    pf = pypfilt.PyPredictiveFilter()
    print

    # Configure the predictive filter object:
    if numDataPoints >= 128:
        numPointsToFilter = 128
    elif numDataPoints >= 32:
        numPointsToFilter = 32
    else:
        print "Not enough data in file!"
        return

    print "Configuring filter to use " + str(numPointsToFilter) + " points in in the low pass filter..."
    ret = pf.configure(numPointsToFilter=numPointsToFilter)
    if not ret:
        print "configure() ran successfully returning", ret
    else:
        print "*** configure() failed! returned", ret

    # Configurable filter values:
    order = 2
    cutoffFrequency = 1.0 # Hz

    #######################################################
    ##################### TEST 1 ##########################
    #######################################################
    if RUNTEST1:
        print "Filling filter completely with " + str(numPointsToFilter) + " datapoints..."
        ret = pf.addData(timestamps[0:numPointsToFilter], data[0:numPointsToFilter])
        if not ret:
            print "addData() ran successfully returning", ret
        else:
            print "*** addData() failed! returned", ret
        print

        print
        ret = pf.addData(timestamps[numPointsToFilter:numPointsToFilter+50], data[numPointsToFilter:numPointsToFilter+50])
        if not ret:
            print "addData() ran successfully returning", ret
        else:
            print "*** addData() failed! returned", ret
        print

        print "Running filter with cutoffFrequency=" + str(cutoffFrequency) + "Hz and order=" + str(order) + "..."
        ret = pf.filter(cutoffFrequency, order, numPointsToFit=30)
        if not ret:
            print "filter() ran successfully returning", ret
            pass
        else:
            print "*** filter() failed! returned", ret
        print

        # Extract predicted point from filter:
        print "Getting prediction for the next timestamp=" + str(timestamps[numPointsToFilter+50]) + " ns, data=" + str(data[numPointsToFilter+50]) + "..."
        predictedTime = timestamps[numPointsToFilter+50+1]
        predictedPoint = pf.getPrediction(predictedTime)
        print "getPrediction() returned a value of: " + str(predictedPoint)
        print
        print "Getting prediction for the current timestamp..."
        print "getPrediction() returned a value of: " + str(pf.getPrediction())
        print
    #######################################################
    #######################################################

    #######################################################
    ##################### TEST 2 ##########################
    #######################################################
    # else:
        # Add data to filter sequentially:
        # dataPointsToGoThrough = 20
        # for i in xrange(dataPointsToGoThrough):
        #     ret = pf.addData(timestamps[i:i+1], data[i:i+1])
        #     if not ret:
        #         #print "addData() ran successfully returning", ret
        #         pass
        #     else:
        #         print "*** addData() failed! returned", ret
        
        #     # Run filter on data:
        #     # print "Running filter with cutoffFrequency=" + str(cutoffFrequency) + "Hz and order=" + str(order) + "..."
        #     ret = pf.filter(cutoffFrequency, order)
        #     if not ret:
        #         #print "filter() ran successfully returning", ret
        #         pass
        #     else:
        #         print "*** filter() failed! returned", ret

        #     # predictedPoint = pf.getPrediction(timestamps[i])
        #     # print 'predicted point',predictedPoint

        #     # coeffs = pf.getCoeffs()
        #     # coeffs = coeffs.tolist()
        #     # coeffs.reverse()
        #     # fitData = pf.getDataToFit()
        #     # fitTime = pf.getTimeToFit()
        #     # print "input", data[i:i+1]
        #     # rawData = pf.getData()
        #     # print "raw data:", rawData
        #     # print "fit data", fitData
        #     # print "fit Time", fitTime
        #     # print "coeffs", coeffs

        # Extract predicted point from filter:
        # print "Getting prediction for the next timestamp=" + str(timestamps[dataPointsToGoThrough-1]) + " ns, data=" + str(data[dataPointsToGoThrough-1]) + "..."
        # predictedTime = timestamps[dataPointsToGoThrough-1] + timestamps[dataPointsToGoThrough-1] - timestamps[dataPointsToGoThrough-2]
        # predictedPoint = pf.getPrediction(predictedTime)
        # print "getPrediction() at " + str(predictedTime) + " returned a value of: " + str(predictedPoint)
        # print
        # print "Getting prediction for the current timestamp..."
        # print "getPrediction() returned a value of: " + str(pf.getPrediction())
        # print
    #######################################################
    #######################################################

    # Extract data from filter:
    print "Getting data from filter..."
    numDataPoints = pf.getNumDataPoints()
    numDataPointsToFit = pf.getNumDataPointsToFit()
    numDataPointsToFilter = pf.getNumDataPointsToFilter()

    # Print some data:
    print "Number of data points:", numDataPoints
    print "Number of filtered points:", numDataPointsToFilter
    print "Number of fit points:", numDataPointsToFit

    rawData = pf.getData()
    filteredData = pf.getFilteredData()
    fitData = pf.getDataToFit()
    fitTime = pf.getTimeToFit()
    time = pf.getTime()
    #print time
    firstTime = time[0]
    time = (time - firstTime).astype(np.float64)/1000.0/1000.0/1000.0
    fitTimeToPlot = time[(numDataPoints - numDataPointsToFit):]
    coeffs = pf.getCoeffs()
    coeffs = coeffs.tolist()
    coeffs.reverse() 

    # Print results:
    print "Polynomial Coefficients:", coeffs

    # Make filter plot:
    plt.figure()
    plt.plot(time, rawData, 'k.', label="input data")
    plt.plot(time, filteredData, 'b', label="filtered data")
    plt.plot(fitTimeToPlot, fitData, 'gs', label='fit data')
    plt.plot(fitTimeToPlot, np.polyval(coeffs, fitTime), 'r', label='polynomial')
    plt.plot((predictedTime - firstTime)/1000.0/1000.0/1000.0, predictedPoint, 'ro', label='predicted point')
    plt.xlabel('Time [s]')
    plt.ylabel('Data Value')
    plt.grid()
    plt.legend(loc="upper left")

    # plt.figure()
    # plt.plot(time)
    # plt.figure()
    # plt.plot(data)
    # plt.figure()
    # plt.plot(time,rawData)

    plt.show()
    

    print "Test complete."

def generatePredictionResiduals(time,dataset,polyorder=1, numPointsToFilter=128, cutoffFreq=1, showPlot=True, pf=None):
    """Generate Residual plot for dataset. Basic for now

    Args:
        time:       time array
        dataset:    1D dataset tied to time

    Kwargs:
        polyorder:          Polynomial prediction order to use
        numPointsToFilter:  Number of points to use in the polynomial prediction
        cutoffFreq:         Cutoff Frequency of lowpass filter in Hz
        showPlot:           Show a plot of the results
        pf:                 Predictive Filter object (assumes this has been pre-configured...)

    Returns:
        residuals:          Residual difference between next the actual observation and what that 
                            observation was predicted to be
        predictedPoints:    Predicted observation point at each time. That is to say, 
                            predictedPoints[ii] is the result of calculations using obs[ii-numPtstoFilter-1:ii]
    Examples:

    """
    ###### Create and configure the predictive filter
    if pf is None:
        if DEBUG:
            print "Creating a predictive filter object"
        pf = pypfilt.PyPredictiveFilter()
        if DEBUG:
            print "Configuring filter to use " + str(numPointsToFilter) + " points in in the low pass filter..."
        ret = pf.configure(numPointsToFilter=numPointsToFilter)
        if ret:
            print "*** configure() failed! returned", ret
            return 
    else:
        polypoints = pf.getNumDataPointsToFit()
        numPointsToFilter = pf.getNumDataPointsToFilter()
        if DEBUG:
            print "Using existing predictive filter object"

    ###### Determine order and filter frequency
    ## Possibly transform data, find power spikes, guess at cutoff frequency...

    cdresiduals = np.zeros(len(dataset))
    predictedPoints = np.zeros(len(dataset))

    ii = np.arange(len(dataset)-numPointsToFilter-1)
    n = len(dataset)-numPointsToFilter-1
    # predictedPoints = processDataset,ii,[time]*n, [dataset]*n, [numPointsToFilter]*n, [pf]*n, [cutoffFreq]*n, [polyorder]*n)
    ###### Loop through dataset, generate residuals 
    for ii in np.arange(len(dataset)-numPointsToFilter-1):
        # if ii < numPointsToFilter:
        #     continue
        predictedPoints[ii+1] = getPrediction(ii,time,dataset,numPointsToFilter, pf, cutoffFreq,polyorder)
        if np.mod(ii,100) == 0: print "Iter ", str(ii),"/",str(len(dataset))

    #     currIdx = ii + numPointsToFilter
    #     ## Add data to filter
    #     if DEBUG:
    #         print "Adding dataset[",str(ii),":",str(currIdx),"]"
    #     ret = pf.addData(time[ii:currIdx], dataset[ii:currIdx])
    #     if ret:
    #         print "*** addData() failed! returned", ret
        
    #     ## Apply filter
    #     ret = pf.filter(cutoffFreq, polyorder)
    #     if ret:
    #         print "*** filter() failed! returned", ret

    #     ## Get next predicted point
    #     p = pf.getPrediction(time[currIdx+1])
    #     ## OOH SOMETIMES THERE IS A PROBREM HERE!!!
    #     if (abs(p) > 1000): #(Faster, this way is more correct though): > abs(5*np.std(dataset[ii:currIdx]) + np.mean(dataset[ii:currIdx]))):
    #         print "UH OH! PROBLEM WITH PREDICTION ON RANGE [",str(ii),":",str(currIdx),"]"
    #         print "Truth: ",str(dataset[currIdx+1])," Predicted: ",p
    #         ## ADD A VERTICAL LINE TO THE PLOT SO WE CAN TELL WHERE THIS HAPPENED?
    #         p=0
    #     predictedPoints[currIdx+1] = p

    ###### Finally, compute residuals  
    residuals = dataset - predictedPoints
    print "Residual [mean, std]: [", str(np.mean(residuals[numPointsToFilter+1:])), ",",str(np.std(residuals[numPointsToFilter+1:])),"]"

    ###### Generate plot
    if showPlot:
        plt.figure()
        plt.plot(time,dataset,'b',label='RawData')
        plt.plot(time,residuals,'r',label='Residual')
        plt.plot(time,predictedPoints,'gs',label='Predicted Points')
        plt.grid()
        plt.legend()
        plt.show()

    return residuals,predictedPoints

def getPrediction(index,time,dataset,numPointsToFilter,pf,cutoffFreq,polyorder):
    # currIdx = offset + numPointsToFilter
    ## Add data to filter
    if index < numPointsToFilter:
        print "Can't get prediction with dataset at index: ", index, ' with numPointsToFilter: ', numPointsToFilter
        return None
    if DEBUG:
        print "Adding dataset[",str(index-numPointsToFilter),":",str(index),"]"
    ret = pf.addData(time[index-numPointsToFilter:index], dataset[index-numPointsToFilter:index])
    if ret:
        print "*** addData() failed! returned", ret
    
    ## Apply filter
    ret = pf.filter(cutoffFreq, polyorder)
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


def AzElPredictionResiduals(time,Az,El,AzFilter=None,ElFilter=None):
    ## Hey...thread this?
    AzFilter = pypfilt.PyPredictiveFilter()
    AzFilter.configure(numPointstoFilter=128)
    ElFilter = pypfilt.PyPredictiveFilter()
    ElFilter.configure(numPointstoFilter=128)


if __name__ == "__main__":
    # parser = argparse.ArgumentParser(description='This test script opens and parses a control law csv file, loads it into the predictive filter, and plots results.')
    # parser.add_argument('fname', metavar='controllawfile', type=str, help='a controllaw file to ingest')
    # args = parser.parse_args()

    # Extract data from file:
    # Do this eventually...
    # controlNames =  ["loopIter", "initialX", "initialY", "currX", "currY", "errorX", "errorY", "actAz", "actEl", "predAz", "predEl", "DACAz", "DACEl", "filteredAz", "filteredEl","polyAz","polyEl"]
    # centroidNames = ["brightPixelCount","numBackgroundPixels","numGoodPix","mean","std","limit",'peaklimit','nuStarsFound','falseStarCount','numBlobSaturated','numBlobLowPeak','numBlobTooSmall','numBlobTooBig','numBlobTooOblong','numBlobOnEdge','xCenterBrightest','yCenterBrightest','IWBBrightest','widthBrightest','heightBrightest','numPixBrightest','roundnessBrightest','maxValBrightest','efyBrightest','subWinLeft','subWinTop','subWinRight','subWinBottom','xCentroid','yCentroid','xCentroid2','yCentroid2','iCentroid']
    # # Check number of columns in the file maybe
    # cols = controlNames + centroidNames
    # data = pd.read_csv(fname,names=cols)
    # func = lambda x: x/1e9a
    # data['time'] = data['loopIter'].apply(func) - data['loopIter'].apply(func).iloc[0]  # playing with lambdas and apply
    # data.dt = map(lambda x,y:x-y,data.time.iloc[1:].tolist(),data.time.iloc[:-1].tolist())
    # print "Loading file: ", args.fname
    # data = zip(*np.genfromtxt(args.fname, dtype=None, delimiter=','))
    time = np.arange(0,400,dtype='double')
    azimuth = np.array(np.sin(2*np.pi*time/400.0), dtype='double') + (np.random.rand(len(time))-0.5)*0.5

    runTest(time,azimuth)
    #print "Processing azimuth data, generating residuals and predictions for whole dataset"
    # Run the test:
    residuals,predictions = generatePredictionResiduals(time, azimuth)  
