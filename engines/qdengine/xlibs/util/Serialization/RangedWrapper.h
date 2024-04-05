#ifndef __RANGED_WRAPPER_H_INCLUDED__
#define __RANGED_WRAPPER_H_INCLUDED__

#include "Range.h"

class Archive;

class RangedWrapperf{
public:
	RangedWrapperf(float value = 0.0f)
	: value_(value)
	, valuePointer_(0)
	, range_(Rangef(0.f, 1.f))
	, step_(1.f)
	{}

	RangedWrapperf(const RangedWrapperf& original)
	: value_(original.value_)
	, valuePointer_(0)
	, range_(original.range_)
	, step_(original.step_)
	{
	}

	RangedWrapperf(float& value, float _range_min, float _range_max, float _step = 0.f)
	: value_(value)
	, valuePointer_(&value)
	, range_(Rangef(_range_min, _range_max))
	, step_(_step)
	{}
	~RangedWrapperf(){
		if(valuePointer_)
			*valuePointer_ = value_;
	}

	operator float() const{
		return value_;
	}
	RangedWrapperf& operator=(const RangedWrapperf& rhs){
		value_ = rhs.value_;
		return *this;
	}
	RangedWrapperf& operator=(float value){
		value_ = value;
		return *this;
	}

	float& value() { return value_; }
	const float& value() const { return value_; }

	const Rangef& range() const { return range_; }
	float step() const { return step_; }
	void clip();

	bool serialize(Archive& ar, const char* name, const char* nameAlt);

private:
	Rangef range_;
	float step_;
	float* valuePointer_;
	float value_;
};


class RangedWrapperi{
public:
	RangedWrapperi(int value = 0.0f)
	: value_(value)
	, valuePointer_(0)
	, range_(Rangei(0.f, 1.f))
	, step_(1.f)
	{}

	RangedWrapperi(const RangedWrapperi& original)
	: value_(original.value_)
	, valuePointer_(0)
	, range_(original.range_)
	, step_(original.step_)
	{
	}

	RangedWrapperi(int& value, int _range_min, int _range_max, int _step = 1)
	: value_(value)
	, valuePointer_(&value)
	, range_(Rangei(_range_min, _range_max))
	, step_(_step)
	{}
	~RangedWrapperi(){
		if(valuePointer_)
			*valuePointer_ = value_;
	}

	operator int() const{
		return value_;
	}
	RangedWrapperi& operator=(const RangedWrapperi& rhs){
		value_ = rhs.value_;
		return *this;
	}
	RangedWrapperi& operator=(int value){
		value_ = value;
		return *this;
	}

	int& value() { return value_; }
	const int& value() const { return value_; }

	const Rangei& range() const { return range_; }
	int step() const { return step_; }
	void clip();

	bool serialize(Archive& ar, const char* name, const char* nameAlt);

private:
	Rangei range_;
	int step_;
	int* valuePointer_;
	int value_;
};

#endif
