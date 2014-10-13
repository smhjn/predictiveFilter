# How to found here: http://docs.cython.org/src/userguide/wrapping_CPlusPlus.html
# Include the standard namespace:
cdef extern from "PredictiveFilter.h":
    # Redefine the public portion of the class:
    cdef cppclass PredictiveFilter:
        PredictiveFilter() except +
        void reset()
        int configure(unsigned int numPointsToFilter,unsigned flags)
        int addData(double* timestamps, double* datapoints, unsigned int length)
        int filter(double cutoffFrequency, int order, unsigned int numPointsToFit)
        double getPrediction(double timestamp)

        # Get state functions:
        double* getData()
        double* getFilteredData()
        double* getDataToFit()
        double* getTimeToFit()
        double* getTime()
        long double* getCoeffs()
        unsigned int getNumDataPoints()
        unsigned int getNumDataPointsToFit()
        unsigned int getNumDataPointsToFilter()
        double getStartTime()


# Import C level numpy:
cimport numpy as np

# # Numpy must be initialized. When using numpy from C or Cython you must
# # _always_ do that, or you will have segfaults
np.import_array()

# This is the cython wrapper class for the predictive filter 
# c++ shared library:
cdef class PyPredictiveFilter:

    # Declare a predictive filter object from c++ land on the stack:
    # cdef PredictiveFilter pf

    # Declare a predictive filter object from c++ land on the heap:
    cdef PredictiveFilter *thisptr      # hold a C++ instance which we're wrapping

    def __cinit__(self):
        self.thisptr = NULL
        self.thisptr = new PredictiveFilter()
        
    def __dealloc__(self):
        if self.thisptr:
            del self.thisptr

    def reset(self):
        self.thisptr.reset()

    def configure(self, numPointsToFilter=512, flags="FFTW_ESTIMATE"):
        fftwPlan = {"FFTW_ESTIMATE":64, "FFTW_MEASURE":0, "FFTW_PATIENT": 32, "FFTW_EXHAUSTIVE": 8, "FFTW_WISDOM_ONLY": 2097152}
        return self.thisptr.configure(numPointsToFilter, fftwPlan[flags])

    def addData(self, np.ndarray[double, ndim=1] timestamps, np.ndarray[double, ndim=1] datapoints):
        return self.thisptr.addData(&timestamps[0], &datapoints[0], len(timestamps))

    def filter(self, cutoffFrequency, order, numPointsToFit=0):
        return self.thisptr.filter(cutoffFrequency, order, numPointsToFit)

    def getPrediction(self, timestamp=0):
        return self.thisptr.getPrediction(timestamp)

    def getData(self):
        # Call the C function:
        cdef double* data_ptr
        data_ptr = self.thisptr.getData()

        # Configure array size:
        cdef int size = self.thisptr.getNumDataPoints()
        cdef np.npy_intp shape[1]
        shape[0] = <np.npy_intp> size

        # Cast C array as a numpy array:
        return  np.PyArray_SimpleNewFromData(1, shape, np.NPY_DOUBLE, data_ptr) 

    def getFilteredData(self):
        # Call the C function:
        cdef double* data_ptr
        data_ptr = self.thisptr.getFilteredData()

        # Configure array size:
        cdef int size = self.thisptr.getNumDataPointsToFilter()
        cdef np.npy_intp shape[1]
        shape[0] = <np.npy_intp> size

        # Cast C array as a numpy array:
        return  np.PyArray_SimpleNewFromData(1, shape, np.NPY_DOUBLE, data_ptr) 

    def getDataToFit(self):
        # Call the C function:
        cdef double* data_ptr
        data_ptr = self.thisptr.getDataToFit()

        # Configure array size:
        cdef int size = self.thisptr.getNumDataPointsToFit()
        cdef np.npy_intp shape[1]
        shape[0] = <np.npy_intp> size

        # Cast C array as a numpy array:
        return  np.PyArray_SimpleNewFromData(1, shape, np.NPY_DOUBLE, data_ptr) 

    def getTimeToFit(self):
        # Call the C function:
        cdef double* data_ptr
        data_ptr = self.thisptr.getTimeToFit()

        # Configure array size:
        cdef int size = self.thisptr.getNumDataPointsToFit()
        cdef np.npy_intp shape[1]
        shape[0] = <np.npy_intp> size

        # Cast C array as a numpy array:
        return  np.PyArray_SimpleNewFromData(1, shape, np.NPY_DOUBLE, data_ptr) 

    def getTime(self):
        # Call the C function:
        cdef double* data_ptr
        data_ptr = self.thisptr.getTime()

        # Configure array size:
        cdef int size = self.thisptr.getNumDataPoints()
        cdef np.npy_intp shape[1]
        shape[0] = <np.npy_intp> size

        # Cast C array as a numpy array:
        return  np.PyArray_SimpleNewFromData(1, shape, np.NPY_DOUBLE, data_ptr)

    def getCoeffs(self):
        # Call the C function:
        cdef long double* data_ptr
        data_ptr = self.thisptr.getCoeffs()

        # Configure array size:
        cdef int size = 9
        cdef np.npy_intp shape[1]
        shape[0] = <np.npy_intp> size

        # Cast C array as a numpy array:
        return  np.PyArray_SimpleNewFromData(1, shape, np.NPY_LONGDOUBLE, data_ptr) 

    def getNumDataPoints(self):
        return self.thisptr.getNumDataPoints()

    def getNumDataPointsToFit(self):
        return self.thisptr.getNumDataPointsToFit()

    def getNumDataPointsToFilter(self):
        return self.thisptr.getNumDataPointsToFilter()

    def getStartTime(self):
        return self.thisptr.getStartTime()

#     return ndarray

if __name__ == "__main__":
    pf = PyPredictiveFilter