/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef QDENGINE_XMATH_H
#define QDENGINE_XMATH_H

#include "common/scummsys.h"

namespace QDEngine {

class Archive;

class Vect2f;
class Vect2i;
class Vect2s;
class Vect3f;

enum eAxis {
	X_AXIS = 0,
	Y_AXIS = 1,
	Z_AXIS = 2,
	W_AXIS = 3
};


///////////////////////////////////////////////////////////////////////////////
//          Constants
///////////////////////////////////////////////////////////////////////////////
#undef M_PI
#define M_PI  3.14159265358979323846f
#undef M_PI_2
#define M_PI_2 1.57079632679489661923f
#undef M_PI_4
#define M_PI_4 0.785398163397448309616f

const double DBL_EPS = 1.e-15;
const double DBL_INF = 1.e+100;
const double DBL_COMPARE_TOLERANCE = 1.e-10;

const float FLT_EPS = 1.192092896e-07f; //1.e-7f;
const float FLT_INF = 1.e+30f;
const float FLT_COMPARE_TOLERANCE = 1.e-5f;

const int INT_INF = 0x7fffffff;

inline float invSqrtFast(float x) {
	x += 1e-7f; // Добавка, устраняющая деление на 0
	float xhalf = 0.5f * x;

	uint32 i;
	memcpy(&i, &x, 4);
	i = 0x5f375a86 - (i >> 1); // gives initial guess y0
	memcpy(&x, &i, 4);
	x = x * (1.5f - xhalf * x * x); // Newton step, repeating increases accuracy
	return x;
}

inline float cycle(float f, float size) {
	return fmod(fmod(f, size) + size, size);
}

inline float getDist(float v0, float v1, float size) {
	float d = fmod(v0 - v1, size);
	float ad = (float)fabs(d);
	float dd = size - ad;
	if (ad <= dd) return d;
	return d < 0 ? d + size : d - size;
}

inline float getDeltaAngle(float to, float from) {
	return getDist(to, from, 2 * M_PI);
}

inline float cycleAngle(float a) {
	return cycle(a, 2 * M_PI);
}


///////////////////////////////////////////////////////////////////////////////
//
//          Scalar Functions
//
///////////////////////////////////////////////////////////////////////////////

template <class T>
inline T sqr(const T &x) {
	return x*x;
}

#define G2R(x) ((x)*M_PI/180.f)
#define R2G(x) ((x)*180.f/M_PI)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Vect2f
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Vect2f {
public:
	float x, y;

	inline Vect2f()                              { x = y = 0.0; }
	inline Vect2f(float x_, float y_)                 {
		x = x_;
		y = y_;
	}

	typedef float float2[2];
	inline Vect2f(const float2 &v) {
		x = v[0];
		y = v[1];
	}

	inline Vect2f(const Vect2i &v);
	inline Vect2f(const Vect2s &v);

	inline Vect2f &set(float x_, float y_)           {
		x = x_;
		y = y_;
		return *this;
	}
	inline Vect2f operator - () const                {
		return Vect2f(-x, -y);
	}

	inline int xi() const {
		return round(x);
	}
	inline int yi() const {
		return round(y);
	}

	inline const float &operator[](int i) const          {
		return *(&x + i);
	}
	inline float &operator[](int i)                      {
		return *(&x + i);
	}

	inline Vect2f &operator += (const Vect2f &v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	inline Vect2f &operator -= (const Vect2f &v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
	inline Vect2f &operator *= (const Vect2f &v) {
		x *= v.x;
		y *= v.y;
		return *this;
	}
	inline Vect2f &operator /= (const Vect2f &v) {
		x /= v.x;
		y /= v.y;
		return *this;
	}
	inline Vect2f &operator *= (float f)         {
		x *= f;
		y *= f;
		return *this;
	}
	inline Vect2f &operator /= (float f)         {
		if (f != 0.f) f = 1 / f;
		else f = 0.0001f;
		x *= f;
		y *= f;
		return *this;
	}

	inline Vect2f operator + (const Vect2f &v) const     {
		return Vect2f(*this) += v;
	}
	inline Vect2f operator - (const Vect2f &v) const     {
		return Vect2f(*this) -= v;
	}
	inline Vect2f operator * (const Vect2f &v) const     {
		return Vect2f(*this) *= v;
	}
	inline Vect2f operator / (const Vect2f &v) const     {
		return Vect2f(*this) /= v;
	}
	inline Vect2f operator * (float f)   const       {
		return Vect2f(*this) *= f;
	}
	inline Vect2f operator / (float f)   const       {
		return Vect2f(*this) /= f;
	}

	inline bool eq(const Vect2f &v, float delta = FLT_COMPARE_TOLERANCE) const {
		return fabsf(v.x - x) < delta && fabsf(v.y - y) < delta;
	}

	inline float dot(const Vect2f &v) const {
		return x * v.x + y * v.y;
	}
	inline friend float dot(const Vect2f &u, const Vect2f &v) {
		return u.dot(v);
	}

	inline float operator % (const Vect2f &v) const {
		return x * v.y - y * v.x;
	}

	inline Vect2f &scaleAdd(const Vect2f &u, float lambda) {
		x += lambda * u.x;
		y += lambda * u.y;
		return *this;
	}

	inline Vect2f &interpolate(const Vect2f &u, const Vect2f &v, float lambda); // (1-lambda)*u + lambda*v

	inline float norm()  const                       {
		return sqrtf(x * x + y * y);
	}
	inline float norm2() const                       {
		return x * x + y * y;
	}
	inline Vect2f &normalize(float norma)                {
		float f = norma * invSqrtFast(x * x + y * y);
		x *= f;
		y *= f;
		return *this;
	}
	inline float distance(const Vect2f &v) const {
		return sqrtf(distance2(v));
	}
	inline float distance2(const Vect2f &v) const    {
		float dx = x - v.x, dy = y - v.y;
		return dx * dx + dy * dy;
	}

	inline void swap(Vect2f &v)                  {
		Vect2f tmp = v;
		v = *this;
		*this = tmp;
	}
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Vect2i
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Vect2i {
public:
	int x, y;

	inline Vect2i()                              { x = y = 0; }
	inline Vect2i(int x_, int y_)                        {
		x = x_;
		y = y_;
	}
	inline Vect2i(float x_, float y_)                    {
		x = round(x_);
		y = round(y_);
	}

	inline Vect2i(const Vect2f &v)           {
		x = round(v.x);
		y = round(v.y);
	}
	inline Vect2i(const Vect2s &v);

	inline void set(int x_, int y_)                  {
		x = x_;
		y = y_;
	}
	inline void set(float x_, float y_)              {
		x = round(x_);
		y = round(y_);
	}
	inline Vect2i operator - () const                {
		return Vect2i(-x, -y);
	}

	inline const int &operator[](int i) const            {
		return *(&x + i);
	}
	inline int &operator[](int i)                        {
		return *(&x + i);
	}

	inline Vect2i &operator += (const Vect2i &v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	inline Vect2i &operator -= (const Vect2i &v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
	inline Vect2i &operator *= (const Vect2i &v) {
		x *= v.x;
		y *= v.y;
		return *this;
	}
	inline Vect2i &operator /= (const Vect2i &v) {
		x /= v.x;
		y /= v.y;
		return *this;
	}

	inline Vect2i operator + (const Vect2i &v) const     {
		return Vect2i(*this) += v;
	}
	inline Vect2i operator - (const Vect2i &v) const     {
		return Vect2i(*this) -= v;
	}
	inline Vect2i operator * (const Vect2i &v) const     {
		return Vect2i(*this) *= v;
	}

	inline Vect2i &operator *= (int f)               {
		x *= f;
		y *= f;
		return *this;
	}
	inline Vect2i operator * (int f) const       {
		return Vect2i(*this) *= f;
	}

	inline Vect2i &operator >>= (int n)              {
		x >>= n;
		y >>= n;
		return *this;
	}
	inline Vect2i operator >> (int n) const      {
		return Vect2i(*this) >>= n;
	}

	inline Vect2i &operator *= (float f)             {
		x = round(x * f);
		y = round(y * f);
		return *this;
	}
	inline Vect2i &operator /= (float f)             {
		return *this *= 1.f / f;
	}
	inline Vect2i operator * (float f) const         {
		return Vect2i(*this) *= f;
	}
	inline Vect2i operator / (float f) const         {
		return Vect2i(*this) /= f;
	}

	inline int dot(const Vect2i &v) const {
		return x * v.x + y * v.y;
	}
	inline friend int dot(const Vect2i &u, const Vect2i &v) {
		return u.dot(v);
	}

	inline int operator % (const Vect2i &v) const {
		return x * v.y - y * v.x;
	}

	inline int norm() const                      {
		return round(sqrtf(float(x * x + y * y)));
	}
	inline int norm2() const                     {
		return x * x + y * y;
	}

	inline void normalize(int norma)             {
		float f = (float)norma * invSqrtFast((float)(x * x + y * y));
		x = round(x * f);
		y = round(y * f);
	}
	inline int distance2(const Vect2i &v) const  {
		return sqr(x - v.x) + sqr(y - v.y);
	}

	inline int operator == (const Vect2i &v) const   {
		return x == v.x && y == v.y;
	}
	inline int operator != (const Vect2i &v) const   {
		return x != v.x || y != v.y;
	}

	inline void swap(Vect2i &v)                  {
		Vect2i tmp = v;
		v = *this;
		*this = tmp;
	}
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Vect2s
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Vect2s {
public:
	int16 x, y;

	inline Vect2s()                                      { x = y = 0; }
	inline Vect2s(int x_, int y_)                         {
		x = x_;
		y = y_;
	}

	inline Vect2s(const Vect2f &v)           {
		x = round(v.x);
		y = round(v.y);
	}
	inline Vect2s(const Vect2i &v)           {
		x = v.x;
		y = v.y;
	}

	inline void set(int x_, int y_)                  {
		x = x_;
		y = y_;
	}
	inline Vect2s operator - () const                {
		return Vect2s(-x, -y);
	}

	inline const int16 &operator[](int i) const          {
		return *(&x + i);
	}
	inline int16 &operator[](int i)                      {
		return *(&x + i);
	}

	inline Vect2s &operator += (const Vect2s &v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	inline Vect2s &operator -= (const Vect2s &v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
	inline Vect2s &operator *= (const Vect2s &v) {
		x *= v.x;
		y *= v.y;
		return *this;
	}
	inline Vect2s &operator *= (float f)         {
		x = round(x * f);
		y = round(y * f);
		return *this;
	}
	inline Vect2s &operator /= (float f)         {
		if (f != 0.f) f = 1 / f;
		else f = 0.0001f;
		x = round(x * f);
		y = round(y * f);
		return *this;
	}
	inline Vect2s operator - (const Vect2s &v) const {
		return Vect2s(x - v.x, y - v.y);
	}
	inline Vect2s operator + (const Vect2s &v) const {
		return Vect2s(x + v.x, y + v.y);
	}
	inline Vect2s operator * (const Vect2s &v) const {
		return Vect2s(x * v.x, y * v.y);
	}
	inline Vect2s operator * (float f) const             {
		Vect2s tmp(round(x * f), round(y * f));
		return tmp;
	}
	inline Vect2s operator / (float f) const             {
		if (f != 0.f) f = 1 / f;
		else f = 0.0001f;
		Vect2s tmp(round(x * f), round(y * f));
		return tmp;
	}

	inline int operator == (const Vect2s &v) const   {
		return x == v.x && y == v.y;
	}

	inline int norm() const                              {
		return round(sqrtf((float)(x * x + y * y)));
	}
	inline int norm2() const                             {
		return x * x + y * y;
	}
	inline int distance(const Vect2s &v) const           {
		int dx = v.x - x, dy = v.y - y;
		return round(sqrtf((float)(dx * dx + dy * dy)));
	}
	inline void normalize(int norma)             {
		float f = (float)norma * invSqrtFast((float)((int)x * x + (int)y * y));
		x = round(x * f);
		y = round(y * f);
	}

	inline void swap(Vect2s &v)                  {
		Vect2s tmp = v;
		v = *this;
		*this = tmp;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Vect3f
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Vect3f {

public:
	typedef float float3[3];

	float x, y, z;

	// constructors //////////////////////////////////////////////////////////////

	inline Vect3f() { x = y = z = 0.0;}
	inline Vect3f(float x_, float y_, float z_) {
		x = x_;
		y = y_;
		z = z_;
	}
	explicit inline Vect3f(const Vect2f &v, float z_ = 0) {
		x = v.x;
		y = v.y;
		z = z_;
	}

	inline Vect3f(const float3 &v) {
		x = v[0];
		y = v[1];
		z = v[2];
	}

	inline operator const Vect2f &() const {
		return *reinterpret_cast<const Vect2f *>(this);
	}

	// setters / accessors / translators /////////////////////////////////////////

	inline Vect3f &set(float x_, float y_, float z_) {
		x = x_;
		y = y_;
		z = z_;
		return *this;
	}

	inline Vect3f &setSpherical(float psi, float theta, float radius);

	// index-based access:  0=x, 1=y, 2=z.
	inline const float &operator[](int i) const {
		return *(&x + i);
	}
	inline float &operator[](int i)       {
		return *(&x + i);
	}

	// Fortran index-based access:  1=x, 2=y, 3=z.
	inline const float &operator()(int i) const {
		return *(&x + i - 1);
	}
	inline float &operator()(int i)       {
		return *(&x + i - 1);
	}

	// Conversion to int ///////
	inline int xi() const {
		return round(x);
	}
	inline int yi() const {
		return round(y);
	}
	inline int zi() const {
		return round(z);
	}

	//  Negate  ////////////////////////////////////
	inline Vect3f operator- () const;
	inline Vect3f &negate(const Vect3f &v);
	inline Vect3f &negate();

	//  Logical operations  ////////////////////////////////
	inline bool eq(const Vect3f &v, float delta = FLT_COMPARE_TOLERANCE) const;

	//  Addition and substruction  ////////////////////
	inline Vect3f &add(const Vect3f &u, const Vect3f &v);
	inline Vect3f &add(const Vect3f &v);
	inline Vect3f &sub(const Vect3f &u, const Vect3f &v);
	inline Vect3f &sub(const Vect3f &v);
	inline Vect3f &operator+= (const Vect3f &v) {
		return add(v);
	}
	inline Vect3f &operator-= (const Vect3f &v) {
		return sub(v);
	}
	inline Vect3f operator+ (const Vect3f &v) const {
		Vect3f u;
		return u.add(*this, v);
	}
	inline Vect3f operator- (const Vect3f &v) const {
		Vect3f u;
		return u.sub(*this, v);
	}

	// Component-wise multiplication and division  ////////////////
	inline Vect3f &mult(const Vect3f &u, const Vect3f &v);
	inline Vect3f &mult(const Vect3f &v);
	inline Vect3f &div(const Vect3f &u, const Vect3f &v);
	inline Vect3f &div(const Vect3f &v);
	inline Vect3f &operator*= (const Vect3f &v) {
		return mult(v);
	}
	inline Vect3f &operator/= (const Vect3f &v) {
		return div(v);
	}
	inline Vect3f operator* (const Vect3f &v) const {
		Vect3f u;
		return u.mult(*this, v);
	}
	inline Vect3f operator/ (const Vect3f &v) const {
		Vect3f u;
		return u.div(*this, v);
	}

	//  Cross product  //////////////////////
	inline Vect3f &cross(const Vect3f &u, const Vect3f &v);// u x v  [!]
	inline Vect3f &precross(const Vect3f &v);      // v x this  [!]
	inline Vect3f &postcross(const Vect3f &v);     // this x v  [!]
	inline Vect3f &operator%= (const Vect3f &v) {
		return postcross(v);    // this x v  [!]
	}
	inline Vect3f operator% (const Vect3f &v) const {
		Vect3f u;
		return u.cross(*this, v);
	}

	//  Dot product  //////////////////////
	inline float dot(const Vect3f &other) const;
	inline friend float dot(const Vect3f &u, const Vect3f &v) {
		return u.dot(v);
	}

	// Multiplication & division by scalar ///////////
	inline Vect3f &scale(const Vect3f &v, float s);
	inline Vect3f &scale(float s);

	inline Vect3f &operator*= (float s) {
		return scale(s);
	}
	inline Vect3f &operator/= (float s) {
		return scale(1 / s);
	}
	inline Vect3f operator* (float s) const {
		Vect3f u;
		return u.scale(*this, s);
	}
	inline Vect3f operator/ (float s) const {
		Vect3f u;
		return u.scale(*this, 1 / s);
	}
	inline friend Vect3f operator* (float s, const Vect3f &v) {
		Vect3f u;
		return u.scale(v, s);
	}

	//  Normalize  ///////////////////////////
	inline Vect3f &normalize(float r = 1.0f);
	inline Vect3f &normalize(const Vect3f &v, float r = 1.0f);

	//  Operation returning scalar  ////////////
	inline float norm()  const;
	inline float norm2() const;  // norm^2
	inline float distance(const Vect3f &other) const;
	inline float distance2(const Vect3f &other) const;  // distance^2

	inline float psi() const;
	inline float theta() const;

	inline float min() const;
	inline float max() const;
	inline float minAbs() const;
	inline float maxAbs() const;
	inline float sumAbs() const;  // |x| + |y| + |z|


	//  Composite functions  ////////////////////////////////
	inline Vect3f &crossAdd(const Vect3f &u, const Vect3f &v, const Vect3f &w); // u x v + w [!]  this must be distinct from u and v, but not necessarily from w.
	inline Vect3f &crossAdd(const Vect3f &u, const Vect3f &v); // u x v + this [!]
	inline Vect3f &scaleAdd(const Vect3f &v, const Vect3f &u, float lambda); // v + lambda * u
	inline Vect3f &scaleAdd(const Vect3f &u, float lambda);// this + lambda * u
	inline Vect3f &interpolate(const Vect3f &u, const Vect3f &v, float lambda); // (1-lambda)*u + lambda*v

	//  Swap  /////////////////////////
	inline void swap(Vect3f &other);
	inline friend void swap(Vect3f &u, Vect3f &v) {
		u.swap(v);
	}
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			Miscellaneous functions
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Decomposition  ////////////////////////////////
inline void decomposition(const Vect3f &axis, const Vect3f &v, Vect3f &v_normal, Vect3f &v_tangent) {
	// axis - axis of decomposition, v_normal - collinear to axis, v_tangent - perpendicular to axis
	v_normal.scale(axis, dot(axis, v) / ((axis).norm2()));
	v_tangent.sub(v, v_normal);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////                        DEFINITIONS
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//		Vect2 definitions
//
///////////////////////////////////////////////////////////////////////////////
inline Vect2i::Vect2i(const Vect2s &v)   {
	x = v.x;
	y = v.y;
}
inline Vect2f::Vect2f(const Vect2i &v)   {
	x = float(v.x);
	y = float(v.y);
}
inline Vect2f::Vect2f(const Vect2s &v)   {
	x = v.x;
	y = v.y;
}

Vect2f &Vect2f::interpolate(const Vect2f &u, const Vect2f &v, float lambda) {
	float lambda2 = 1.0f - lambda;

	x = lambda2 * u.x + lambda * v.x;
	y = lambda2 * u.y + lambda * v.y;
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
//
//		Vect3f inline definitions
//
///////////////////////////////////////////////////////////////////////////////

//  Dot product  //////////////////////
//inline double dot(const Vect3d& u, const Vect3f& v) { return u.dot(v); }
//inline float dot(const Vect3f& u, const Vect3d& v) { return u.dot(v); }

bool Vect3f::eq(const Vect3f &other, float delta) const {
	return fabs(x - other.x) < delta &&
	       fabs(y - other.y) < delta &&
	       fabs(z - other.z) < delta;
}

Vect3f Vect3f::operator- () const {
	return Vect3f(-x, -y, -z);
}

//  Norm operations /////////
float Vect3f::sumAbs() const {
	return (float)(fabs(x) + fabs(y) + fabs(z));
}


//  Descart - spherical function  //////////////
float Vect3f::psi() const {
	return (float)atan2(y, x);
}
float Vect3f::theta() const {
	return (float)acos(z / (norm() + FLT_EPS));
}
Vect3f &Vect3f::setSpherical(float psi, float theta, float radius) {
	x = radius * (float)sin(theta);
	y = x * (float)sin(psi);
	x = x * (float)cos(psi);
	z = radius * (float)cos(theta);
	return *this;
}

float Vect3f::dot(const Vect3f &other) const {
	return x * other.x + y * other.y + z * other.z;
}


float Vect3f::norm() const {
	return (float)sqrt(x * x + y * y + z * z);
}


float Vect3f::norm2() const {
	return (x * x + y * y + z * z);
}


float Vect3f::distance(const Vect3f &other) const {
	Vect3f w;

	w.sub(other, *this);
	return w.norm();
}


float Vect3f::distance2(const Vect3f &other) const {
	Vect3f w;

	w.sub(other, *this);
	return w.norm2();
}


float Vect3f::min() const {
	return (x <= y) ? ((x <= z) ? x : z) : ((y <= z) ? y : z);
}


float Vect3f::max() const {
	return (x >= y) ? ((x >= z) ? x : z) : ((y >= z) ? y : z);
}


float Vect3f::minAbs() const {
	float ax, ay, az;

	ax = (float)fabs(x);
	ay = (float)fabs(y);
	az = (float)fabs(z);
	return (ax <= ay) ? ((ax <= az) ? ax : az) : ((ay <= az) ? ay : az);
}


float Vect3f::maxAbs() const {
	float ax, ay, az;

	ax = (float)fabs(x);
	ay = (float)fabs(y);
	az = (float)fabs(z);
	return (ax >= ay) ? ((ax >= az) ? ax : az) : ((ay >= az) ? ay : az);
}


void Vect3f::swap(Vect3f &other) {
	Vect3f tmp;

	tmp = *this;
	*this = other;
	other = tmp;
}


Vect3f &Vect3f::normalize(const Vect3f &v, float r) {
	float s = r * invSqrtFast(v.x * v.x + v.y * v.y + v.z * v.z);
	x = s * v.x;
	y = s * v.y;
	z = s * v.z;
	return *this;
}


Vect3f &Vect3f::normalize(float r) {
	float s = r * invSqrtFast(x * x + y * y + z * z);
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

Vect3f &Vect3f::negate(const Vect3f &v) {
	x = - v.x;
	y = - v.y;
	z = - v.z;
	return *this;
}


Vect3f &Vect3f::negate() {
	x = - x;
	y = - y;
	z = - z;
	return *this;
}


Vect3f &Vect3f::add(const Vect3f &u, const Vect3f &v) {
	x = u.x + v.x;
	y = u.y + v.y;
	z = u.z + v.z;
	return *this;
}


Vect3f &Vect3f::add(const Vect3f &v) {
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}


Vect3f &Vect3f::sub(const Vect3f &u, const Vect3f &v) {
	x = u.x - v.x;
	y = u.y - v.y;
	z = u.z - v.z;
	return *this;
}


Vect3f &Vect3f::sub(const Vect3f &v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}


Vect3f &Vect3f::mult(const Vect3f &u, const Vect3f &v) {
	x = u.x * v.x;
	y = u.y * v.y;
	z = u.z * v.z;
	return *this;
}


Vect3f &Vect3f::mult(const Vect3f &v) {
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

Vect3f &Vect3f::div(const Vect3f &u, const Vect3f &v) {
	x = u.x / v.x;
	y = u.y / v.y;
	z = u.z / v.z;
	return *this;
}


Vect3f &Vect3f::div(const Vect3f &v) {
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}



Vect3f &Vect3f::scale(const Vect3f &v, float s) {
	x = s * v.x;
	y = s * v.y;
	z = s * v.z;
	return *this;
}


Vect3f &Vect3f::scale(float s) {
	x *= s;
	y *= s;
	z *= s;
	return *this;
}



Vect3f &Vect3f::cross(const Vect3f &u, const Vect3f &v) {
	x = u.y * v.z - u.z * v.y;
	y = u.z * v.x - u.x * v.z;
	z = u.x * v.y - u.y * v.x;
	return *this;
}


Vect3f &Vect3f::precross(const Vect3f &v) {
	float ox, oy;

	ox = x;
	oy = y;
	x = v.y * z - v.z * oy;
	y = v.z * ox - v.x * z;
	z = v.x * oy - v.y * ox;
	return *this;
}


Vect3f &Vect3f::postcross(const Vect3f &v) {
	float ox, oy;

	ox = x;
	oy = y;
	x = oy * v.z - z * v.y;
	y = z * v.x - ox * v.z;
	z = ox * v.y - oy * v.x;
	return *this;
}


Vect3f &Vect3f::crossAdd(const Vect3f &u, const Vect3f &v, const Vect3f &w) {
	x = u.y * v.z - u.z * v.y + w.x;
	y = u.z * v.x - u.x * v.z + w.y;
	z = u.x * v.y - u.y * v.x + w.z;
	return *this;
}


Vect3f &Vect3f::crossAdd(const Vect3f &u, const Vect3f &v) {
	x += u.y * v.z - u.z * v.y;
	y += u.z * v.x - u.x * v.z;
	z += u.x * v.y - u.y * v.x;
	return *this;
}


Vect3f &Vect3f::scaleAdd(const Vect3f &v, const Vect3f &u, float lambda) {
	x = v.x + lambda * u.x;
	y = v.y + lambda * u.y;
	z = v.z + lambda * u.z;
	return *this;
}


Vect3f &Vect3f::scaleAdd(const Vect3f &u, float lambda) {
	x += lambda * u.x;
	y += lambda * u.y;
	z += lambda * u.z;
	return *this;
}


Vect3f &Vect3f::interpolate(const Vect3f &u, const Vect3f &v, float lambda) {
	float lambda2 = 1.0f - lambda;

	x = lambda2 * u.x + lambda * v.x;
	y = lambda2 * u.y + lambda * v.y;
	z = lambda2 * u.z + lambda * v.z;
	return *this;
}

} // namespace QDEngine

#endif // QDENGINE_XMATH_H
