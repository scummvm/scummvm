#ifndef __SYNCRO_TIMER__
#define __SYNCRO_TIMER__

typedef unsigned int time_type;
	
class SyncroTimer
{
public: 
	SyncroTimer();
	void set(int syncro_by_clock_, int  time_per_frame_, int max_time_interval_ = 0);

	time_type operator()() const { return time; } // время
	operator time_type() const { return time; } // время
	time_type delta() const { return delta_; } // последнее изменение 
	
	void adjust();
	void next_frame();
	void skip();
	void revert();

	void setTime(time_type t);
	void setSpeed(float speed) { time_speed = speed; }

private:	
	time_type time;
	time_type time_prev;
	int clock_prev;
	int max_time_interval;
	int time_per_frame;
	time_type delta_;
	time_type delta_prev;
	int syncro_by_clock;
	float time_speed;
	bool impossibleRevert;

	time_type int_clock();
	void calcDelta();
};

#endif // __SYNCRO_TIMER__
