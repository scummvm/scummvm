
/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "xglobal.h"

#pragma comment(lib, "winmm.lib")

#pragma warning(disable : 4073 )
#pragma init_seg(lib)

static const int FIXED_POINT = 12;
static const int ADJUST_PERIOD = 1000;
static const int PROCESSORS_MAX = 8;

__declspec (noinline)
__int64 getRDTSC()
{
	#define RDTSC __asm _emit 0xf __asm _emit 0x31
	__int64 timeRDTS;
	__asm {
		push ebx
		push ecx
		push edx
		RDTSC
		mov dword ptr [timeRDTS],eax
		mov dword ptr [timeRDTS+4],edx
		pop edx
		pop ecx
		pop ebx
	}
	return timeRDTS;
}

int getCPUID()
{
	unsigned int id;
	__asm {
		push ebx
		push ecx
		push edx
		mov eax, 1
		CPUID
		shr ebx, 24
		mov id, ebx
		pop edx
		pop ecx
		pop ebx
	}
	return id;
}

class XClock
{
public:
	XClock(int cpuID)
	{
		cpuID_ = cpuID;
		time_ = 0;
		counterPrev_ = 0;
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency_);
		frequency_ /= 1000;
		counterToAdjust_ = 0;
		clockToAdjust_ = 0;
	}

	int time()
	{
		__int64 counter = getRDTSC();
		unsigned int clock = timeGetTime();

		if(!counterPrev_){
			counterPrev_ = counterToAdjust_ = counter;
			clockToAdjust_ = clock + ADJUST_PERIOD;
		}

		__int64 timeRough = __int64(clock - clockGlobalPrev_) << FIXED_POINT;
		__int64 dt = ((counter - counterPrev_) << FIXED_POINT)/frequency_;
		__int64 dtMax = timeRough - timeGlobal_ + (__int64(1000) << FIXED_POINT);

		if(dt < 0)
			dt = 0;
		else if(dt > dtMax)
			dt = dtMax;

		time_ += dt;
		counterPrev_ = counter;

		if(timeGlobal_ < timeRough)
			timeGlobal_ = timeRough;
		
		if(time_ > timeGlobal_)
			timeGlobal_ = time_;
		else
			time_ = timeGlobal_;
		
		if(clock > clockToAdjust_){
			frequency_ = (counter - counterToAdjust_)/(clock - clockToAdjust_ + ADJUST_PERIOD);
			clockToAdjust_ = clock + ADJUST_PERIOD;
			counterToAdjust_ = counter;
		}

		return time_ >> FIXED_POINT;
	}

private:
	int cpuID_;
	__int64 time_;
	__int64 counterPrev_;
	__int64 frequency_;
	__int64 counterToAdjust_;
	unsigned int clockToAdjust_;

	static __int64 timeGlobal_;
	static unsigned int clockGlobalPrev_;
	static XClock clocks_[PROCESSORS_MAX];

	friend int xclock();
}; 

__int64 XClock::timeGlobal_ = 0;
unsigned int XClock::clockGlobalPrev_ = timeGetTime();

XClock XClock::clocks_[PROCESSORS_MAX] = { XClock(0), XClock(1), XClock(2), XClock(3), XClock(4), XClock(5), XClock(6), XClock(7) };

int xclock()
{
	return XClock::clocks_[getCPUID() & (PROCESSORS_MAX - 1)].time();
} 



/*
Best Practices for Windows Development - Timing

Be careful using RDTSC to get timing information for your game
Clock rate can vary with Power Management settings
Multi-processor and multi-core make no guarantees about syncing the cycle counts
If your thread gets run on a different core, time might appear to go backwards or jump a large amount
timeGetTime does not suffer from these issues, but is much lower resolution
Use QueryPerformanceCounter instead of RDTSC, which should cope with the multi-core issue
Use SetThreadAffinityMask to try to collect timing data always from same core
Only call from a single thread and only a few times per frame
Use QueryPerformanceFrequency every few frames to handle power management clock rate changes
Be sure your delta computation checks for negative time and bounds the range
*/