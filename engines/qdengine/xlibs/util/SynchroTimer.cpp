#include "StdAfx.h"
#include "SynchroTimer.h"
#include "XMath\xmath.h"

SyncroTimer::SyncroTimer()
{
	set(1, 15, 100);
	time_prev = time = 1; 
	clock_prev = 0;
	time_speed = 1;
	delta_ = 20;
	delta_prev = delta_;
	impossibleRevert=false;
}

void SyncroTimer::set(int syncro_by_clock_, int time_per_frame_, int max_time_interval_) 
{ 
	syncro_by_clock = syncro_by_clock_; 
	time_per_frame = time_per_frame_;
	max_time_interval = max_time_interval_;
}

void SyncroTimer::adjust()
{
	time_prev = time; //?

	if(syncro_by_clock){
		int clock = int_clock();
		float dtf = (clock - clock_prev)*time_speed;
		int dt = clamp(round(dtf), 0, max_time_interval);
		time += dt;
		clock_prev = clock;
	}
	calcDelta();
}

void SyncroTimer::next_frame()
{
	time_prev = time;
	delta_prev = delta_;
	if(syncro_by_clock)
		adjust();									 
	else
		time += round(time_per_frame*time_speed);
	calcDelta();
}

void SyncroTimer::skip()
{
	if(syncro_by_clock){
		clock_prev = int_clock();
		time_prev = time;
	}
	calcDelta();
}

void SyncroTimer::setTime(time_type t) 
{
	time_prev = time = t; 
	clock_prev = syncro_by_clock ? int_clock() : 0;
	calcDelta();
}

time_type SyncroTimer::int_clock()
{
	return xclock();
}

void SyncroTimer::calcDelta()
{
	delta_ = time - time_prev;
	impossibleRevert=false;
}

void SyncroTimer::revert()
{
	xassert(impossibleRevert==false);
	time = time_prev;
	time_prev -= delta_prev;
	delta_ = delta_prev;
	impossibleRevert=true;
}

