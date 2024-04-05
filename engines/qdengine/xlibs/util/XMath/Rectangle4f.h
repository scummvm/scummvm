#pragma once

#include "xmath.h"

struct sRectangle4f
{
	Vect2f	min;
	Vect2f	max;

	sRectangle4f()									{ }
	sRectangle4f(float xmin,float ymin,float xmax,float ymax)	{ min.x=xmin; min.y=ymin; max.x=xmax; max.y=ymax; }
	sRectangle4f(const sRectangle4f &rect)				{ min=rect.min; max=rect.max; }
	inline void set(float xmin,float ymin,float xmax,float ymax){ min.x=xmin; min.y=ymin; max.x=xmax; max.y=ymax; }
	inline float xmin() const						{ return min.x; }
	inline float ymin() const						{ return min.y; }
	inline float xmax() const						{ return max.x; }
	inline float ymax() const						{ return max.y; }
	inline float& xmin()							{ return min.x; }
	inline float& ymin()							{ return min.y; }
	inline float& xmax()							{ return max.x; }
	inline float& ymax()							{ return max.y; }

	void serialize(Archive& ar);

	static const sRectangle4f ID;
};
