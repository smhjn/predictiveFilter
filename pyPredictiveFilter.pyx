import numpy as np
cimport numpy as np
 
cdef extern from "PredictiveFilter.cpp": 
    cdef cppclass predictiveFilter:
    	## Variables:

        ## Methods:
        void reset();
		int configure(unsigned int numPointsToFilter=MAXNUMDATAPOINTS);
		int addData(unsigned long* timestamps, double* datapoints, unsigned int length=1);
		int filter(double cutoffFrequency, int order, unsigned int numPointsToFit=0);
		double getPrediction(unsigned long timestamp=0);