#ifndef _TIMER_H_
#define _TIMER_H_

#include <time.h>
#include <errno.h>
#include <signal.h>
#include <iostream>

using namespace std;

// A wrapper around the posix timer system. It implements
// a downwards ticking one-shot timer that expires after the 
// specified time.
class Timer
{
	public:
	Timer();
	~Timer();
	int timerCreate();
	int timerDelete();
	int timerSet(const time_t sec, const long nsec);
	int timerSet(const long usecs);
	int timerUnset(); // disable the timer
	int timerGet(time_t &sec, long &nsec);
	bool timerExpired();

	private:
	timer_t timerid;
};

// A wrapper around the posix timer system. It implements
// a downwards ticking periodic timer that expires after the 
// specified time and calls a callback function.
class PeriodicTimer
{
	public:
	PeriodicTimer();
	~PeriodicTimer();
	int timerCreate(long threadId = -1, int signalNum = SIGALRM);
	int timerDelete();
	int timerSet(const time_t sec, const long nsec);
	int timerSet(const long usecs);
	int timerUnset(); // disable the timer
	int timerGet(time_t &sec, long &nsec);
	int timerWait(long sec = -1, long nsec = -1);
	int timerIntervalGet(time_t &sec, long &nsec);
	void timerEnable();
	void timerDisable();
	bool isTimerSet();     // Is timer set to a value or is it infinite?
	bool isTimerEnabled(); // Is timer enabled or will it immediately return?

	private:
	timer_t timerid;
	int signum;
	sigset_t set;
	bool enabled;
};

// An upwards counting stopwatch-like timer that can be used
// for timing system performance.
class StopWatch
{
	public:
	int start();
	int stop();
	long report();

	private:
	timespec start_t;
	timespec stop_t;
};

// The most accurate real-time sleeper:
extern int rt_sleep(const time_t sec, const long nsec);
extern int rt_sleep(const long usec);

// Most accurate way to get system time
extern int rt_time( time_t &sec, long &nsec );
extern int rt_time( timespec &systemTime );
unsigned long rt_time();
unsigned long rt_time_ns(timespec timeSpecStruct);

// Utility functions:
extern void usec_2_sec_nsec(long usec, time_t& sec, long& nsec);

#endif
