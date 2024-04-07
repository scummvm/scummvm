/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "fps_counter.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

fpsCounter::fpsCounter(int period) : start_time_(0.0f),
	prev_time_(0.0f),
	period_(period),
	frame_count_(0),
	value_(-1.0f),
	value_min_(0.0f),
	value_max_(0.0f),
	min_frame_time_(1000.0f),
	max_frame_time_(0.0f)
{
}

bool fpsCounter::quant()
{
	float time = float(xclock());

	if(min_frame_time_ > time - prev_time_)
		min_frame_time_ = time - prev_time_;

	if(max_frame_time_ < time - prev_time_)
		max_frame_time_ = time - prev_time_;

	frame_count_++;
	prev_time_ = time;

	if(period_){
		if(prev_time_ - start_time_ >= period_){
			value_ = float(frame_count_)/(prev_time_ - start_time_)*1000.0f;

			value_min_ = 1000.0f/max_frame_time_;
			value_max_ = 1000.0f/min_frame_time_;

			frame_count_ = 0;
			start_time_ = prev_time_;

			min_frame_time_ = 10000.0f;
			max_frame_time_ = 0.0f;

			return true;
		}
		return false;
	}
	else {
		value_ = float(frame_count_)/(float(xclock()) - start_time_)*1000.0f;
		return true;
	}
}

void fpsCounter::reset()
{
	prev_time_ = start_time_ = float(xclock());
	frame_count_ = 0;
	value_ = -1.0f;
	value_min_ = value_max_ = 0.0f;

	min_frame_time_ = 10000.0f;
	max_frame_time_ = 0.0f;
}
