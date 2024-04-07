#pragma once

#include "xmath.h"

struct sBox6f
{
	Vect3f	min;
	Vect3f	max;

	sBox6f()												{  invalidate(); }
	sBox6f(const Vect3f& _min, const Vect3f& _max) : min(_min), max(_max) {}

	void invalidate() { min.set(FLT_INF, FLT_INF, FLT_INF); max.set(-FLT_INF, -FLT_INF, -FLT_INF); }

	bool valid() const { return min.x < max.x; }

	void addPoint(const Vect3f& v)
	{
		if(min.x>v.x) min.x=v.x; if(max.x<v.x) max.x=v.x;
		if(min.y>v.y) min.y=v.y; if(max.y<v.y) max.y=v.y;
		if(min.z>v.z) min.z=v.z; if(max.z<v.z) max.z=v.z; 
	}

	sBox6f& operator += (const Vect3f& v) { min+=v; max+=v; return *this; }
	sBox6f& operator -= (const Vect3f& v) { min-=v; max-=v; return *this; }

	float radius() const { return max.distance(min)*0.5f; }
	float radius2D() const { return Vect2f(max).distance(Vect2f(min))*.5f; }
	Vect3f center() const { return (min + max)*.5f; }
	Vect3f extent() const { return (max - min)*.5f; }

	bool isCrossOrInside(const Se3f& pose, const Vect3f& pointBegin, const Vect3f& pointEnd) const;

	inline float xmin() const					{ return min.x; }
	inline float xmax() const					{ return max.x; }
	inline float ymin() const					{ return min.y; }
	inline float ymax() const					{ return max.y; }
	inline float zmin() const					{ return min.z; }
	inline float zmax() const					{ return max.z; }
	inline float& xmin()						{ return min.x; }
	inline float& xmax()						{ return max.x; }
	inline float& ymin()						{ return min.y; }
	inline float& ymax()						{ return max.y; }
	inline float& zmin()						{ return min.z; }
	inline float& zmax()						{ return max.z; }

	void serialize(Archive& ar);
};
