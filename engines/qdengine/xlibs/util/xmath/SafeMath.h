#ifndef _PERIMETER_SAFE_MATH_
#define _PERIMETER_SAFE_MATH_

#include "XMath/fastMath.h"

inline float cycle(float f, float size) 
{ 
	return fmodFast(fmodFast(f, size) + size, size); 
}

inline float getDist(float v0, float v1, float size) 
{
	float d = fmodFast(v0 - v1, size);
	float ad = (float)fabs(d);
	float dd = size - ad;
	if(ad <= dd) return d;
	return d < 0 ? d + size : d - size;
}

inline float uncycle(float f1, float f0, float size) 
{	
	return f0 + getDist(f1, f0, size); 
}

inline float getDeltaAngle(float to,float from)
{
	return getDist(to, from, 2*M_PI);
}

inline float cycleAngle(float a)
{
	return cycle(a, 2*M_PI);
}


inline bool isEq(float x, float y, float eps = FLT_EPS)
{ 
	return fabsf(x - y) < eps; 
}

inline bool isLess(float x, float y, float eps = FLT_EPS)
{ 
	return x < y + eps; 
}

inline bool isGreater(float x, float y, float eps = FLT_EPS)
{ 
	return x > y - eps; 
}

#endif


