OBJS=Timer.o 
LIBS=libtimer.so

all: $(OBJS) $(LIBS) $(BIN)	


Timer.o: Timer.cpp
	$(CPP) $(LIBFLAGS) Timer.cpp -o Timer.o

libtimer.so: Timer.o
	$(CPP) $(LDFLAGS) -shared -o libtimer.so Timer.o

TimerTest.bin: TimerTest.cpp Timer.cpp
	$(CPP) -Wall TimerTest.cpp Timer.cpp -pthread -o TimerTest.bin 



install:

clean:
	-rm -f $(OBJS) $(LIBS) $(BIN) $(TMP)

