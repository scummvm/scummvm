#ifndef __FPS_COUNTER_H__
#define __FPS_COUNTER_H__

class fpsCounter
{
public:
	fpsCounter(int period = 3000);
	~fpsCounter(){ }

	bool quant();
	
	float fps_value() const { return value_; }
	float fps_value_min() const { return value_min_; }
	float fps_value_max() const { return value_max_; }

	void reset();

	int period() const { return period_; }
	void set_period(int p){ period_ = p; }

private:
	
	float start_time_;
	float prev_time_;

	int frame_count_;

	int period_;

	float value_;
	float value_min_;
	float value_max_;

	float min_frame_time_;
	float max_frame_time_;
};

#endif /* __FPS_COUNTER_H__ */

