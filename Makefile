# Compile Flags:
CPP = /usr/bin/g++
CPPFLAGS = -O3 -mtune=corei7-avx -march=corei7-avx -m64 -pipe -c -Wall -Wextra
LIBFLAGS = -O3 -mtune=corei7-avx -march=corei7-avx -m64 -pipe -c -Wall -Wextra -fPIC
LDFLAGS = -mtune=corei7-avx -march=corei7-avx -m64 -pipe -Wall -Wextra -lpthread -pthread 

OBJS=PolyFit.o LowPassFilter.o PredictiveFilter.o
LIBS=libpolyfit.so liblowpassfilter.so libpredictivefilter.so 
TEST=PolyFitTest.bin LowPassFilterTest.bin #PredictiveFilterTest.bin
PYTHON=PyPredictiveFilter.so

all: dep $(LIBS) $(TEST) $(PYTHON)

dep:
	# make -C ./cdhlib

PolyFit.o: PolyFit.cpp
	$(CPP) $(LIBFLAGS) PolyFit.cpp -o PolyFit.o 

libpolyfit.so: PolyFit.o
	$(CPP) -mtune=corei7-avx -march=corei7-avx -m64 -pipe -Wall -Wextra -fPIC -shared -o libpolyfit.so PolyFit.o -lm

LowPassFilter.o: LowPassFilter.cpp
	$(CPP) $(LIBFLAGS) LowPassFilter.cpp -o LowPassFilter.o 

liblowpassfilter.so: LowPassFilter.o
	$(CPP) -mtune=corei7-avx -march=corei7-avx -m64 -pipe -Wall -Wextra -fPIC -shared -o liblowpassfilter.so LowPassFilter.o -lm -lfftw3

PredictiveFilter.o: PredictiveFilter.cpp
	$(CPP) $(LIBFLAGS) PredictiveFilter.cpp -o PredictiveFilter.o 

libpredictivefilter.so: PredictiveFilter.o libpolyfit.so liblowpassfilter.so
	$(CPP) -mtune=corei7-avx -march=corei7-avx -m64 -pipe -Wall -Wextra -fPIC -shared -o libpredictivefilter.so PredictiveFilter.o -L.  -llowpassfilter -lpolyfit

PolyFitTest.bin: PolyFitTest.cpp PolyFit.cpp
	$(CPP) -Wall PolyFitTest.cpp PolyFit.cpp -pthread -o PolyFitTest.bin -lm

LowPassFilterTest.bin: LowPassFilterTest.cpp LowPassFilter.cpp
	$(CPP) -Wall -Wextra LowPassFilterTest.cpp LowPassFilter.cpp -o LowPassFilterTest.bin -lm -lfftw3

PyPredictiveFilter.so: libpredictivefilter.so
	python setup.py build_ext --inplace

install:
	cp $(LIBS) ../$(INSLIB)
	chmod a+x ../$(INSLIB)/$(LIBS)
	
clean:
	-rm -f $(OBJS) $(LIBS) $(TEST) build/* PyPredictiveFilter.cpp PyPredictiveFilter.c PyPredictiveFilter.so
	rm -rf build/*
	make -C ./cdhlib clean

