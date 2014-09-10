#include "Timer.h"
#include <iostream>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

int main()
{	
	time_t sec;
	long nsec;

	cout << "starting test" << endl;
	Timer timeWrapper;
	StopWatch stopWatch;

	stopWatch.start();

	rt_sleep(1000100);

	stopWatch.stop();

	cout << "test took " << stopWatch.report() << " nanoseconds." << endl;

	cout << timeWrapper.timerCreate() << endl;
	sec = 0;
	nsec = 1000000;
	cout << timeWrapper.timerSet(sec, nsec) << endl;

	stopWatch.start();

	while(!timeWrapper.timerExpired()){ rt_sleep(0,100); cout << "."; }
	cout << endl;
	
	stopWatch.stop();

	cout << "test took " << stopWatch.report() << " nanoseconds." << endl;

	cout << timeWrapper.timerDelete() << endl;


	PeriodicTimer ptimer;
	cout << ptimer.timerCreate(syscall(SYS_gettid)) << endl;
	cout << ptimer.timerSet(2, 50000) << endl;
	int ret;
	int i = 0;
	while( i < 5)
	{
		cout << "waiting..." << endl;
		ret = ptimer.timerWait();

		if( ret == 0 )
		{
			cout << "timeout" << endl;
			cout << "unset: " << ptimer.timerUnset() << " errno: " << errno << endl;
		}
		else if( ret > 0 )
		{
			cout << "signal " << ret << " returned" << endl;
		}
		else
		{
			cout << "wait error" << endl;
		}
		i++;
	}

	cout << "disable " << ptimer.timerUnset() << endl;
	cout << "destroy " << ptimer.timerDelete() << endl;

	alarm( 2 );
	i = 0;
	while( i < 5)
	{
		cout << "waiting..." << endl;
		ret = ptimer.timerWait();

		if( ret == 0 )
		{
			cout << "timeout" << endl;
		}
		else if( ret > 0 )
		{
			cout << "signal " << ret << " returned" << endl;
		}
		else
		{
			cout << "wait error" << endl;
		}
		i++;
	}

	return 0;
}
