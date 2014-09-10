#include "PredictiveFilter.h"

// Helpers:
void printBuffer(double* buffer, int length)
{
    for(int i = 0; i < length; i++)
    {
        cout << i << ": " << buffer[i] << endl;
    }
}

// Constructor:
PredictiveFilter::PredictiveFilter()
{  
    // pthread_mutex_init(&lock,NULL);
    // pthread_mutex_unlock(&lock);
    reset();
}

// Deconstructor:
PredictiveFilter::~PredictiveFilter()
{   
    // pthread_mutex_destroy(&lock);
    lpf.destroy();
}

//////////////////////////////
// Public Functions
//////////////////////////////
void PredictiveFilter::reset()
{
    // #if PREDICTIVE_FILTER_TIMING
    // resetSW.start();
    // #endif

    // Reset counters:
    numDataPoints = 0;
    numDataPointsToFit = 0;

    // Zero out data:
    bzero(data, sizeof(data));
    bzero(filteredData, sizeof(filteredData));
    bzero(dataToFit, sizeof(dataToFit));
    bzero(time, sizeof(time));
    bzero(timeToFit, sizeof(timeToFit));

    // LOCK THE MUTEX: //
    // pthread_mutex_lock(&lock);
    /////////////////////

    // Clear global state:
    bzero(Coeffs, sizeof(Coeffs));
    Order = 0;
    startTime = 0;
    
    // UNLOCK THE MUTEX: //
    // pthread_mutex_unlock(&lock);
    ///////////////////////

    // #if PREDICTIVE_FILTER_TIMING
    // resetSW.stop();
    // #endif
}

int PredictiveFilter::configure(unsigned int numPointsToFilter, unsigned flags)
{
    // #if PREDICTIVE_FILTER_TIMING
    // configureSW.start();
    // #endif

    if( numPointsToFilter > MAXNUMDATAPOINTS )
        return -1;

    numDataPointsToFilter = numPointsToFilter;

    if( lpf.configure(numDataPointsToFilter, flags) < 0 )
        return -1;

    // #if PREDICTIVE_FILTER_TIMING
    // configureSW.stop();
    // #endif

    return 0;
}

int PredictiveFilter::addData(unsigned long* timestamps, double* datapoints, unsigned int length)
{
    // #if PREDICTIVE_FILTER_TIMING
    // addSW.start();
    // #endif

    // Make sure we have valid data:
    if( timestamps == NULL || datapoints == NULL || length == 0)
        return -1;

    // Make sure we don't get too much data. If we get more than the number
    // of points the filter can handle, then just take numDataPointsToFilter
    // points from the end of the input data:
    // cout << "Trying to fill in " << length << " dps" << endl;
    int offset = 0;
    if( length > numDataPointsToFilter )
    {
        offset = length - numDataPointsToFilter;
        length = numDataPointsToFilter;
    }
    // cout << "We are actually going to copy " << length << " dps." << endl;

    int dataInBufferToKeep = numDataPointsToFilter - length;

    // Copy over new data:
    // Move data over to make room (must use memmove because data buffers overlap!):
    // cout << "Copying over " << dataInBufferToKeep << " old dps, starting at " << length << endl;
    // cout << "before moving " << dataInBufferToKeep << " dps: " << endl;
    // printBuffer(data, numDataPoints);
    memmove(&data[0], &data[length], dataInBufferToKeep*sizeof(double));
    memmove(&time[0], &time[length], dataInBufferToKeep*sizeof(unsigned long));
    //cout << "after: " << endl;
    //printBuffer(data, numDataPoints);

    // Copy over new data (can use memcpy because data buffers do not overlap!):
    // cout << "Copying over " << length << " data points [" << datapoints[offset] << "], indexing into new buffer at " << offset << " copying to " << dataInBufferToKeep << endl;
    memcpy(&data[dataInBufferToKeep], &datapoints[offset], length*sizeof(double));
    memcpy(&time[dataInBufferToKeep], &timestamps[offset], length*sizeof(unsigned long));
    // cout << "after: " << endl;
    // printBuffer(data, numDataPoints);

    // Increase out data point count:
    // cout << "had " << numDataPoints << " points." << endl;
    numDataPoints += length;
    if( numDataPoints > numDataPointsToFilter )
        numDataPoints = numDataPointsToFilter;
    // cout << "now have " << numDataPoints << endl;

    // #if PREDICTIVE_FILTER_TIMING
    // addSW.stop();
    // #endif

    return 0;
}

int PredictiveFilter::filter(double cutoffFrequency, int order, unsigned int numPointsToFit)
{
    // #if PREDICTIVE_FILTER_TIMING
    // fullpredictSW.start();
    // #endif

    // #if PREDICTIVE_FILTER_TIMING
    // filterSW.start();
    // #endif

    bool filtered = false;

    // Make sure we have some data:
    if( numDataPoints < 1 )
        return 0;

    // Check bounds:
    if( cutoffFrequency <= 1e-6 )
        return -1;

    // Low-pass filter the data is we have enough points:
    if( numDataPoints >= numDataPointsToFilter )
    {
        lpf.filter(cutoffFrequency, time, data, filteredData);
        filtered = true;
    }

    #if PREDICTIVE_FILTER_TIMING
    filterSW.stop();
    #endif

    #if PREDICTIVE_FILTER_TIMING
    numPtsSW.start();
    #endif

    // Higher order? Plz...
    if( order > MAXORDER )
        order = MAXORDER;

    // Lower order... come on:
    if( order < 0 )
        order = 0;

    // If polyfit autopoint picker is enabled, calculate the number of points
    if( numPointsToFit <= 0 )
    {
        // If we have enough data points use the point picker, use it:
        if( numDataPoints >= 2)
        {
             // Calculate the SI ratio
            double avedt = (time[numDataPoints-1] - time[0]) / NSINSEC / numDataPoints;
            // cout << "average dt: " << avedt << endl;
            double sampleFrequency = 1.0/avedt;
            // cout << "average sample freq: " << sampleFrequency << endl;
            double SIratio = sampleFrequency/cutoffFrequency;
            // cout << "SIrat: " << SIratio << endl;

            numPointsToFit = calculateNumberOfPolyPoints(order, SIratio);
        }
        // Otherwise just use all the points we have:
        else
        {
            numPointsToFit = numDataPoints;
        }
    }

    // Make sure we are not fitting more than we can handle:
    if( numPointsToFit > numDataPoints )
        numPointsToFit = numDataPoints;

    // Check size here to make things faster for fit (optimization, can remove if necessary):

    // #if PREDICTIVE_FILTER_TIMING
    // numPtsSW.stop();
    // #endif

    // #if PREDICTIVE_FILTER_TIMING
    // prefitSW.start();
    // #endif

    // Make sure the order will work with the number of points:
    if( ((int)(numPointsToFit-1)) < order )
        order = numPointsToFit-1;
    // Still can't go below zeroth order:
    if( order < 0 )
        order = 0;

    // Copy over filtered data for polynomial fitting:
    int index = numDataPointsToFilter - numPointsToFit;
    if( filtered )
        memcpy(&dataToFit[0], &filteredData[index], numPointsToFit*sizeof(double));
    else
        memcpy(&dataToFit[0], &data[index], numPointsToFit*sizeof(double));

    // Copy over time for polynomial fitting:
    unsigned long firstTimestamp = time[index];
    for( unsigned int i = 0; i < numPointsToFit; i++ )
    {
        // Zero time to first timestamp for polyfit to work correctly:
        timeToFit[i] = ((double) (time[index] - firstTimestamp))/NSINSEC;
        index++;
    }

    // #if PREDICTIVE_FILTER_TIMING
    // prefitSW.stop();
    // #endif

    // #if PREDICTIVE_FILTER_TIMING
    // fitSW.start();
    // #endif

    // Fit the filtered buffer with polynomial:
    long double coeffs[MAXORDER+1];
    bzero(coeffs, sizeof(coeffs));
    if( polyfit( order, timeToFit, dataToFit, numPointsToFit, coeffs ) < 0 )
    {
        // Use last point as zeroth order prediction on fail to fit:
        bzero(coeffs, sizeof(coeffs));
        coeffs[0] = dataToFit[numPointsToFit-1];
    }
    
    // Update some globals:
    numDataPointsToFit = numPointsToFit;

    // #if PREDICTIVE_FILTER_TIMING
    // fitSW.stop();
    // #endif

    // #if PREDICTIVE_FILTER_TIMING
    // mutexSW.start();
    // #endif

    // LOCK THE MUTEX: //
    // pthread_mutex_lock(&lock);
    /////////////////////

    // Set global state:
    memcpy(Coeffs, coeffs, sizeof(Coeffs));
    startTime = firstTimestamp;
    Order = order;

    // UNLOCK THE MUTEX: //
    // pthread_mutex_unlock(&lock);
    ///////////////////////

    // #if PREDICTIVE_FILTER_TIMING
    // mutexSW.stop();
    // #endif

    // #if PREDICTIVE_FILTER_TIMING
    // fullpredictSW.stop();
    // printTiming();
    // #endif

    return 0;
}

double PredictiveFilter::getPrediction(unsigned long timestamp)
{
    long double coeffs[MAXORDER+1];
    unsigned long firstTimestamp;
    int order;

    // LOCK THE MUTEX: //
    // pthread_mutex_lock(&lock);
    /////////////////////

    // Get global state:
    memcpy(coeffs, Coeffs, sizeof(Coeffs));
    firstTimestamp = startTime;
    order = Order;

    // If timetamp is not populated, then grab the current time:
    // If timestamp is less than first time stamp then grab the current time:
    if( !timestamp || timestamp < firstTimestamp )
        cout << "WOAH! PROVIDE A TIMESTAMP TO PREDICTIVE FILTER!!!"<<endl;
        // timestamp = rt_time();

    // UNLOCK THE MUTEX: //
    // pthread_mutex_unlock(&lock);
    ///////////////////////

    // Calculate the current timestep:
    // cout << "timestamp " << timestamp << " first TS " << firstTimestamp << " diff " << timestamp - firstTimestamp << endl;
    double timeToPredict = (timestamp - firstTimestamp)/NSINSEC;

    // cout << "Predicting for order" << order << " at time " << timeToPredict << " coeffs ";
    // for(int i = 0; i < MAXORDER+1; i++ )
    //     cout << coeffs[i] << " ";
    // cout << endl;

    return polyval( order, timeToPredict, coeffs);
}

//////////////////////////////
// Private Functions
//////////////////////////////
int PredictiveFilter::calculateNumberOfPolyPoints(int polyOrder, double SIratio)
{
    int numPoints = 1;

    if( polyOrder == 0 )
    {
        numPoints = (int) (SIratio/20.0 + 0.5);
        if( numPoints < 2 )
            numPoints = 2;
    }
    else if( polyOrder == 1 )
    {
        numPoints = (int) (SIratio/12.25 + 0.5);
        if( numPoints < 3 )
            numPoints = 3;
    }
    else if( polyOrder == 2 )
    {
        numPoints = (int) (SIratio/5.4444444 + 0.5);
        if( numPoints < 4 )
            numPoints = 4;
    }
    else
    {
        numPoints = (int) (SIratio/2.0 + 0.5);
        if( numPoints < 5 )
            numPoints = 5;
    }

    return numPoints;
}

#if PREDICTIVE_FILTER_TIMING
void PredictiveFilter::printTiming()
{
    cout << "      TIMES:          " << endl;
    cout << "      Configure time: " << configureSW.report()/1000000.0 << "ms" << endl;
    cout << "          Reset time: " << resetSW.report()/1000000.0 << "ms" << endl;
    cout << "       Add data time: " << addSW.report()/1000000.0 << " ms" << endl;
    cout << "         Filter time: " << filterSW.report()/1000000.0 << " ms" << endl;
    cout <<"    Point picker time: " << numPtsSW.report()/1000000.0 << " ms" << endl;
    cout << "        Pre-fit time: " << prefitSW.report()/1000000.0 << "ms" << endl;
    cout << "            Fit time: " << fitSW.report()/1000000.0 << "ms" << endl;
    cout << "          Mutex time: " << mutexSW.report()/1000000.0 << "ms" << endl;
    cout << "   Full predict time: " << fullpredictSW.report()/1000000.0 << "ms" << endl;
}
#endif
