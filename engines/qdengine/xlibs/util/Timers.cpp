#include "Stdafx.h"
#include "Timers.h"
#include "Serialization\Serialization.h"
#include "DebugUtil.h"

SyncroTimer global_time;
SyncroTimer frame_time;
SyncroTimer scale_time;

void LogicTimer::serialize(Archive& ar) 
{
	if(ar.isOutput() && isUnderEditor())
		stop();

	ar.serialize(startTime_, "time", 0);

	if(ar.isInput() && timer() == 1)
		startTime_ = 0;
}

void GraphicsTimer::serialize(Archive& ar) 
{
	if(ar.isOutput() && isUnderEditor())
		stop();

	ar.serialize(startTime_, "time", 0);

	if(ar.isInput() && timer() == 1)
		startTime_ = 0;
}

void InterpolationLogicTimer::start(time_type duration)
{
	durationInv_ = duration ? 1/(float)duration : 0;
	LogicTimer::start(duration);
}

float InterpolationLogicTimer::factor() const 
{
	if(!started())
		return 0;
	
	if(busy())
		return 1.f - timeRest()*durationInv_;

	return 1;
}

void InterpolationGraphicsTimer::start(time_type duration)
{
	durationInv_ = duration ? 1/(float)duration : 0;
	GraphicsTimer::start(duration);
}

float InterpolationGraphicsTimer::factor() const 
{
	if(!started())
		return 0;

	if(busy())
		return 1.f - timeRest()*durationInv_;

	return 1;
}

void InterpolationLogicTimer::serialize(Archive& ar) 
{
	LogicTimer::serialize(ar);
	ar.serialize(durationInv_, "durationInv", 0);
}

void InterpolationGraphicsTimer::serialize(Archive& ar) 
{
	GraphicsTimer::serialize(ar);
	ar.serialize(durationInv_, "durationInv", 0);
}
