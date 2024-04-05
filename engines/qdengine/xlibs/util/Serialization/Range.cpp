#include "stdafx.h"
#include "Range.h"

#include "Serialization\Serialization.h"

void Rangef::set(float _min, float _max)
{
	min_ = _min;
	max_ = _max;
}

Rangef Rangef::intersection (const Rangef& _range) const
{
	float begin;
	float end;
	if(maximum() < _range.minimum() || minimum() > _range.maximum())
		return Rangef(0.f, 0.f);

	if(include(_range.minimum()))
		begin = _range.minimum();
	else
		begin = minimum();

	if(include(_range.maximum()))
		end = _range.maximum();
	else
		end = maximum();
	return Rangef(begin, end);
}

Rangef Rangef::merge (const Rangef& _range) const
{
	return Rangef(min(minimum(), _range.minimum()), max(maximum(), _range.maximum()));
}

float Rangef::clip(float& _value) const
{
	if(include(_value))
		return _value;
	else{
		if(_value < minimum())
			return minimum();
		else
			return maximum();
	}
}

void Rangef::serialize(Archive& ar)
{
	ar.serialize(min_, "min_", "Минимум");
	ar.serialize(max_, "max_", "Максимум");
}


// --------------------- Rangei

void Rangei::set(int _min, int _max)
{
	min_ = _min;
	max_ = _max;
}

Rangei Rangei::intersection (const Rangei& _range) const
{
	int begin;
	int end;
	if(maximum() < _range.minimum() || minimum() > _range.maximum())
		return Rangei(0, 0);

	if(include(_range.minimum()))
		begin = _range.minimum();
	else
		begin = minimum();

	if(include(_range.maximum()))
		end = _range.maximum();
	else
		end = maximum();
	return Rangei(begin, end);
}


int Rangei::clip(int& _value)
{
	if(include(_value))
		return _value;
	else{
		if(_value < minimum())
			return minimum();
		else
			return maximum();
	}
}

void Rangei::serialize(Archive& ar)
{
	ar.serialize(min_, "min_", "Минимум");
	ar.serialize(max_, "max_", "Максимум");
}


Rangei Rangei::merge (const Rangei& _range) const
{
	return Rangei(min(minimum(), _range.minimum()), max(maximum(), _range.maximum()));
}
