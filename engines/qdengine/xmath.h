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

///////////////////////////////////////////////////////////////////////////////
//
//	All 3D-functionality
//
//	Configuration:
//	Define _XMATH_NO_IOSTREAM to disable iostream using
//
///////////////////////////////////////////////////////////////////////////////
#ifndef QDENGINE_UTIL_XMATH_H
#define QDENGINE_UTIL_XMATH_H

#include "common/scummsys.h"

namespace QDEngine {

class Archive;

///////////////////////////////////////////////////////////////////////////////
//	Structures predefenition
///////////////////////////////////////////////////////////////////////////////

class Vect2f;
class Vect2i;
class Vect2s;
class Vect3f;
class Mat3f;
class MatXf;

///////////////////////////////////////////////////////////////////////////////
//		Axes
///////////////////////////////////////////////////////////////////////////////
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

#if _MSC_VER == 1100 /* if MSVisual C++ 5.0 */
#define xm_inline inline
#else
#define xm_inline inline
#endif //_MSC_VER

inline float invSqrtFast(float x) {
	x += 1e-7f; // Добавка, устраняющая деление на 0
	float xhalf = 0.5f * x;
	int i = *(int *)&x; // get bits for floating value
	i = 0x5f375a86 - (i >> 1); // gives initial guess y0
	x = *(float *)&i; // convert bits back to float
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
xm_inline T sqr(const T &x) {
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

	xm_inline Vect2f()                              { }
	xm_inline Vect2f(float x_, float y_)                 {
		x = x_;
		y = y_;
	}

	typedef float float2[2];
	xm_inline Vect2f(const float2 &v) {
		x = v[0];
		y = v[1];
	}

	xm_inline Vect2f(const Vect2i &v);
	xm_inline Vect2f(const Vect2s &v);

	xm_inline Vect2f &set(float x_, float y_)           {
		x = x_;
		y = y_;
		return *this;
	}
	xm_inline Vect2f operator - () const                {
		return Vect2f(-x, -y);
	}

	xm_inline int xi() const {
		return round(x);
	}
	xm_inline int yi() const {
		return round(y);
	}

	xm_inline const float &operator[](int i) const          {
		return *(&x + i);
	}
	xm_inline float &operator[](int i)                      {
		return *(&x + i);
	}

	xm_inline Vect2f &operator += (const Vect2f &v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	xm_inline Vect2f &operator -= (const Vect2f &v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
	xm_inline Vect2f &operator *= (const Vect2f &v) {
		x *= v.x;
		y *= v.y;
		return *this;
	}
	xm_inline Vect2f &operator /= (const Vect2f &v) {
		x /= v.x;
		y /= v.y;
		return *this;
	}
	xm_inline Vect2f &operator *= (float f)         {
		x *= f;
		y *= f;
		return *this;
	}
	xm_inline Vect2f &operator /= (float f)         {
		if (f != 0.f) f = 1 / f;
		else f = 0.0001f;
		x *= f;
		y *= f;
		return *this;
	}

	xm_inline Vect2f operator + (const Vect2f &v) const     {
		return Vect2f(*this) += v;
	}
	xm_inline Vect2f operator - (const Vect2f &v) const     {
		return Vect2f(*this) -= v;
	}
	xm_inline Vect2f operator * (const Vect2f &v) const     {
		return Vect2f(*this) *= v;
	}
	xm_inline Vect2f operator / (const Vect2f &v) const     {
		return Vect2f(*this) /= v;
	}
	xm_inline Vect2f operator * (float f)   const       {
		return Vect2f(*this) *= f;
	}
	xm_inline Vect2f operator / (float f)   const       {
		return Vect2f(*this) /= f;
	}

	xm_inline bool eq(const Vect2f &v, float delta = FLT_COMPARE_TOLERANCE) const {
		return fabsf(v.x - x) < delta && fabsf(v.y - y) < delta;
	}

	xm_inline float dot(const Vect2f &v) const {
		return x * v.x + y * v.y;
	}
	xm_inline friend float dot(const Vect2f &u, const Vect2f &v) {
		return u.dot(v);
	}

	float angle(const Vect2f &other) const;

	xm_inline float operator % (const Vect2f &v) const {
		return x * v.y - y * v.x;
	}

	xm_inline Vect2f &scaleAdd(const Vect2f &u, float lambda) {
		x += lambda * u.x;
		y += lambda * u.y;
		return *this;
	}

	xm_inline Vect2f &interpolate(const Vect2f &u, const Vect2f &v, float lambda); // (1-lambda)*u + lambda*v

	xm_inline float norm()  const                       {
		return sqrtf(x * x + y * y);
	}
	xm_inline float norm2() const                       {
		return x * x + y * y;
	}
	xm_inline Vect2f &normalize(float norma)                {
		float f = norma * invSqrtFast(x * x + y * y);
		x *= f;
		y *= f;
		return *this;
	}
	xm_inline float distance(const Vect2f &v) const {
		return sqrtf(distance2(v));
	}
	xm_inline float distance2(const Vect2f &v) const    {
		float dx = x - v.x, dy = y - v.y;
		return dx * dx + dy * dy;
	}

	xm_inline void swap(Vect2f &v)                  {
		Vect2f tmp = v;
		v = *this;
		*this = tmp;
	}

	//  I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
	friend ostream &operator<< (ostream &os, const Vect2f &v);
	friend istream &operator>> (istream &is, Vect2f &v);
#endif

	static const Vect2f ZERO;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Vect2i
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Vect2i {
public:
	int x, y;

	xm_inline Vect2i()                              { }
	xm_inline Vect2i(int x_, int y_)                        {
		x = x_;
		y = y_;
	}
	xm_inline Vect2i(float x_, float y_)                    {
		x = round(x_);
		y = round(y_);
	}

	xm_inline Vect2i(const Vect2f &v)           {
		x = round(v.x);
		y = round(v.y);
	}
	xm_inline Vect2i(const Vect2s &v);

	xm_inline void set(int x_, int y_)                  {
		x = x_;
		y = y_;
	}
	xm_inline void set(float x_, float y_)              {
		x = round(x_);
		y = round(y_);
	}
	xm_inline Vect2i operator - () const                {
		return Vect2i(-x, -y);
	}

	xm_inline const int &operator[](int i) const            {
		return *(&x + i);
	}
	xm_inline int &operator[](int i)                        {
		return *(&x + i);
	}

	xm_inline Vect2i &operator += (const Vect2i &v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	xm_inline Vect2i &operator -= (const Vect2i &v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
	xm_inline Vect2i &operator *= (const Vect2i &v) {
		x *= v.x;
		y *= v.y;
		return *this;
	}
	xm_inline Vect2i &operator /= (const Vect2i &v) {
		x /= v.x;
		y /= v.y;
		return *this;
	}

	xm_inline Vect2i operator + (const Vect2i &v) const     {
		return Vect2i(*this) += v;
	}
	xm_inline Vect2i operator - (const Vect2i &v) const     {
		return Vect2i(*this) -= v;
	}
	xm_inline Vect2i operator * (const Vect2i &v) const     {
		return Vect2i(*this) *= v;
	}

	xm_inline Vect2i &operator *= (int f)               {
		x *= f;
		y *= f;
		return *this;
	}
	xm_inline Vect2i operator * (int f) const       {
		return Vect2i(*this) *= f;
	}

	xm_inline Vect2i &operator >>= (int n)              {
		x >>= n;
		y >>= n;
		return *this;
	}
	xm_inline Vect2i operator >> (int n) const      {
		return Vect2i(*this) >>= n;
	}

	xm_inline Vect2i &operator *= (float f)             {
		x = round(x * f);
		y = round(y * f);
		return *this;
	}
	xm_inline Vect2i &operator /= (float f)             {
		return *this *= 1.f / f;
	}
	xm_inline Vect2i operator * (float f) const         {
		return Vect2i(*this) *= f;
	}
	xm_inline Vect2i operator / (float f) const         {
		return Vect2i(*this) /= f;
	}

	xm_inline int dot(const Vect2i &v) const {
		return x * v.x + y * v.y;
	}
	xm_inline friend int dot(const Vect2i &u, const Vect2i &v) {
		return u.dot(v);
	}

	xm_inline int operator % (const Vect2i &v) const {
		return x * v.y - y * v.x;
	}

	xm_inline int norm() const                      {
		return round(sqrtf(float(x * x + y * y)));
	}
	xm_inline int norm2() const                     {
		return x * x + y * y;
	}

	xm_inline void normalize(int norma)             {
		float f = (float)norma * invSqrtFast((float)(x * x + y * y));
		x = round(x * f);
		y = round(y * f);
	}
	xm_inline int distance2(const Vect2i &v) const  {
		return sqr(x - v.x) + sqr(y - v.y);
	}

	xm_inline int operator == (const Vect2i &v) const   {
		return x == v.x && y == v.y;
	}
	xm_inline int operator != (const Vect2i &v) const   {
		return x != v.x || y != v.y;
	}

	xm_inline void swap(Vect2i &v)                  {
		Vect2i tmp = v;
		v = *this;
		*this = tmp;
	}

	//  I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
	friend ostream &operator<< (ostream &os, const Vect2i &v);
	friend istream &operator>> (istream &is, Vect2i &v);
#endif

	static const Vect2i ZERO;
	static const Vect2i ID;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Vect2s
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Vect2s {
public:
	int16 x, y;

	xm_inline Vect2s()                                      { }
	xm_inline Vect2s(int x_, int y_)                         {
		x = x_;
		y = y_;
	}

	xm_inline Vect2s(const Vect2f &v)           {
		x = round(v.x);
		y = round(v.y);
	}
	xm_inline Vect2s(const Vect2i &v)           {
		x = v.x;
		y = v.y;
	}

	xm_inline void set(int x_, int y_)                  {
		x = x_;
		y = y_;
	}
	xm_inline Vect2s operator - () const                {
		return Vect2s(-x, -y);
	}

	xm_inline const int16 &operator[](int i) const          {
		return *(&x + i);
	}
	xm_inline int16 &operator[](int i)                      {
		return *(&x + i);
	}

	xm_inline Vect2s &operator += (const Vect2s &v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	xm_inline Vect2s &operator -= (const Vect2s &v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
	xm_inline Vect2s &operator *= (const Vect2s &v) {
		x *= v.x;
		y *= v.y;
		return *this;
	}
	xm_inline Vect2s &operator *= (float f)         {
		x = round(x * f);
		y = round(y * f);
		return *this;
	}
	xm_inline Vect2s &operator /= (float f)         {
		if (f != 0.f) f = 1 / f;
		else f = 0.0001f;
		x = round(x * f);
		y = round(y * f);
		return *this;
	}
	xm_inline Vect2s operator - (const Vect2s &v) const {
		return Vect2s(x - v.x, y - v.y);
	}
	xm_inline Vect2s operator + (const Vect2s &v) const {
		return Vect2s(x + v.x, y + v.y);
	}
	xm_inline Vect2s operator * (const Vect2s &v) const {
		return Vect2s(x * v.x, y * v.y);
	}
	xm_inline Vect2s operator * (float f) const             {
		Vect2s tmp(round(x * f), round(y * f));
		return tmp;
	}
	xm_inline Vect2s operator / (float f) const             {
		if (f != 0.f) f = 1 / f;
		else f = 0.0001f;
		Vect2s tmp(round(x * f), round(y * f));
		return tmp;
	}

	xm_inline int operator == (const Vect2s &v) const   {
		return x == v.x && y == v.y;
	}

	xm_inline int norm() const                              {
		return round(sqrtf((float)(x * x + y * y)));
	}
	xm_inline int norm2() const                             {
		return x * x + y * y;
	}
	xm_inline int distance(const Vect2s &v) const           {
		int dx = v.x - x, dy = v.y - y;
		return round(sqrtf((float)(dx * dx + dy * dy)));
	}
	xm_inline void normalize(int norma)             {
		float f = (float)norma * invSqrtFast((float)((int)x * x + (int)y * y));
		x = round(x * f);
		y = round(y * f);
	}

	xm_inline void swap(Vect2s &v)                  {
		Vect2s tmp = v;
		v = *this;
		*this = tmp;
	}

	//  I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
	friend ostream &operator<< (ostream &os, const Vect2s &v);
	friend istream &operator>> (istream &is, Vect2s &v);
#endif

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

	xm_inline Vect3f() {}
	xm_inline Vect3f(float x_, float y_, float z_) {
		x = x_;
		y = y_;
		z = z_;
	}
	explicit xm_inline Vect3f(const Vect2f &v, float z_ = 0) {
		x = v.x;
		y = v.y;
		z = z_;
	}

	xm_inline Vect3f(const float3 &v) {
		x = v[0];
		y = v[1];
		z = v[2];
	}

	xm_inline operator const Vect2f &() const {
		return *reinterpret_cast<const Vect2f *>(this);
	}

	// setters / accessors / translators /////////////////////////////////////////

	xm_inline Vect3f &set(float x_, float y_, float z_) {
		x = x_;
		y = y_;
		z = z_;
		return *this;
	}
	//xm_inline Vect3f& set(const float3& v) {x = v[0]; y = v[1]; z = v[2]; return *this; }

	xm_inline Vect3f &setSpherical(float psi, float theta, float radius);

	// index-based access:  0=x, 1=y, 2=z.
	xm_inline const float &operator[](int i) const {
		return *(&x + i);
	}
	xm_inline float &operator[](int i)       {
		return *(&x + i);
	}

	// Fortran index-based access:  1=x, 2=y, 3=z.
	xm_inline const float &operator()(int i) const {
		return *(&x + i - 1);
	}
	xm_inline float &operator()(int i)       {
		return *(&x + i - 1);
	}

	// Convertion to int ///////
	xm_inline int xi() const {
		return round(x);
	}
	xm_inline int yi() const {
		return round(y);
	}
	xm_inline int zi() const {
		return round(z);
	}

	//  Negate  ////////////////////////////////////
	xm_inline Vect3f operator- () const;
	xm_inline Vect3f &negate(const Vect3f &v);
	xm_inline Vect3f &negate();

	//  Logical operations  ////////////////////////////////
	xm_inline bool eq(const Vect3f &v, float delta = FLT_COMPARE_TOLERANCE) const;

	//  Addition and substruction  ////////////////////
	xm_inline Vect3f &add(const Vect3f &u, const Vect3f &v);
	xm_inline Vect3f &add(const Vect3f &v);
	xm_inline Vect3f &sub(const Vect3f &u, const Vect3f &v);
	xm_inline Vect3f &sub(const Vect3f &v);
	xm_inline Vect3f &operator+= (const Vect3f &v) {
		return add(v);
	}
	xm_inline Vect3f &operator-= (const Vect3f &v) {
		return sub(v);
	}
	xm_inline Vect3f operator+ (const Vect3f &v) const {
		Vect3f u;
		return u.add(*this, v);
	}
	xm_inline Vect3f operator- (const Vect3f &v) const {
		Vect3f u;
		return u.sub(*this, v);
	}

	// Component-wise multiplication and division  ////////////////
	xm_inline Vect3f &mult(const Vect3f &u, const Vect3f &v);
	xm_inline Vect3f &mult(const Vect3f &v);
	xm_inline Vect3f &div(const Vect3f &u, const Vect3f &v);
	xm_inline Vect3f &div(const Vect3f &v);
	xm_inline Vect3f &operator*= (const Vect3f &v) {
		return mult(v);
	}
	xm_inline Vect3f &operator/= (const Vect3f &v) {
		return div(v);
	}
	xm_inline Vect3f operator* (const Vect3f &v) const {
		Vect3f u;
		return u.mult(*this, v);
	}
	xm_inline Vect3f operator/ (const Vect3f &v) const {
		Vect3f u;
		return u.div(*this, v);
	}

	//  Cross product  //////////////////////
	xm_inline Vect3f &cross(const Vect3f &u, const Vect3f &v);// u x v  [!]
	xm_inline Vect3f &precross(const Vect3f &v);      // v x this  [!]
	xm_inline Vect3f &postcross(const Vect3f &v);     // this x v  [!]
	xm_inline Vect3f &operator%= (const Vect3f &v) {
		return postcross(v);    // this x v  [!]
	}
	xm_inline Vect3f operator% (const Vect3f &v) const {
		Vect3f u;
		return u.cross(*this, v);
	}

	//  Dot product  //////////////////////
	xm_inline float dot(const Vect3f &other) const;
	xm_inline friend float dot(const Vect3f &u, const Vect3f &v) {
		return u.dot(v);
	}

	// Angle between two vectors //////////
	float angle(const Vect3f &other) const;

	// Multiplication & division by scalar ///////////
	xm_inline Vect3f &scale(const Vect3f &v, float s);
	xm_inline Vect3f &scale(float s);

	xm_inline Vect3f &operator*= (float s) {
		return scale(s);
	}
	xm_inline Vect3f &operator/= (float s) {
		return scale(1 / s);
	}
	xm_inline Vect3f operator* (float s) const {
		Vect3f u;
		return u.scale(*this, s);
	}
	xm_inline Vect3f operator/ (float s) const {
		Vect3f u;
		return u.scale(*this, 1 / s);
	}
	xm_inline friend Vect3f operator* (float s, const Vect3f &v) {
		Vect3f u;
		return u.scale(v, s);
	}

	//  Normalize  ///////////////////////////
	xm_inline Vect3f &normalize(float r = 1.0f);
	xm_inline Vect3f &normalize(const Vect3f &v, float r = 1.0f);

	//  Operation returning scalar  ////////////
	xm_inline float norm()  const;
	xm_inline float norm2() const;  // norm^2
	xm_inline float distance(const Vect3f &other) const;
	xm_inline float distance2(const Vect3f &other) const;  // distance^2

	xm_inline float psi() const;
	xm_inline float theta() const;

	xm_inline float min() const;
	xm_inline float max() const;
	xm_inline float minAbs() const;
	xm_inline float maxAbs() const;
	xm_inline float sumAbs() const;  // |x| + |y| + |z|


	//  Composite functions  ////////////////////////////////
	xm_inline Vect3f &crossAdd(const Vect3f &u, const Vect3f &v, const Vect3f &w); // u x v + w [!]  this must be distinct from u and v, but not necessarily from w.
	xm_inline Vect3f &crossAdd(const Vect3f &u, const Vect3f &v); // u x v + this [!]
	xm_inline Vect3f &scaleAdd(const Vect3f &v, const Vect3f &u, float lambda); // v + lambda * u
	xm_inline Vect3f &scaleAdd(const Vect3f &u, float lambda);// this + lambda * u
	xm_inline Vect3f &interpolate(const Vect3f &u, const Vect3f &v, float lambda); // (1-lambda)*u + lambda*v

	//    I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
	friend ostream &operator<< (ostream &os, const Vect3f &v);
	friend istream &operator>> (istream &is, Vect3f &v);
#endif

	//  Swap  /////////////////////////
	xm_inline void swap(Vect3f &other);
	xm_inline friend void swap(Vect3f &u, Vect3f &v) {
		u.swap(v);
	}


	// Vect3f constants ///////////////////////////////////////////////////////////

	static const Vect3f ZERO;
	static const Vect3f ID;
	static const Vect3f I;     // unit vector along +x axis
	static const Vect3f J;     // unit vector along +y axis
	static const Vect3f K;     // unit vector along +z axis
	static const Vect3f I_;    // unit vector along -x axis
	static const Vect3f J_;    // unit vector along -y axis
	static const Vect3f K_;    // unit vector along -z axis

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Mat3f
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Mat3f {

	friend class MatXf;

public:

	// (stored in row-major order)
	float xx, xy, xz,
	      yx, yy, yz,
	      zx, zy, zz;

public:

	// constructors //////////////////////////////////////////////////////////////

	Mat3f() {}

	xm_inline Mat3f(float xx, float xy, float xz,
	                float yx, float yy, float yz,
	                float zx, float zy, float zz);

	Mat3f(float angle, eAxis axis) {
		set(angle, axis);
	}

	Mat3f(const Vect3f &diag, const Vect3f &sym) {
		set(diag, sym);
	}

	Mat3f(const Vect3f &axis, float angle, int normalizeAxis = 1) {
		set(axis, angle, normalizeAxis);
	}

	Mat3f(const Vect3f &x_from, const Vect3f &y_from, const Vect3f &z_from,
	      const Vect3f &x_to = Vect3f::I, const Vect3f &y_to = Vect3f::J, const Vect3f &z_to = Vect3f::K) {
		set(x_from, y_from, z_from, x_to, y_to, z_to);
	}


	// setters / accessors ///////////////////////////////////////////////////////

	// Rotation around 'axis'  by radians-angle
	xm_inline Mat3f &set(float angle, eAxis axis);

	// make a symmetric matrix, given the diagonal and symmetric
	// (off-diagonal) elements in canonical order
	xm_inline Mat3f &set(const Vect3f &diag, const Vect3f &sym);

	// set Mat3f as a rotation of 'angle' radians about 'axis'
	// axis is automatically normalized unless normalizeAxis = 0
	Mat3f &set(const Vect3f &axis, float angle, int normalizeAxis = 1);

	// Convertion "from"-basis->"to"-basis
	Mat3f &set(const Vect3f &x_from, const Vect3f &y_from, const Vect3f &z_from,
	           const Vect3f &x_to = Vect3f::I, const Vect3f &y_to = Vect3f::J, const Vect3f &z_to = Vect3f::K);

	// index-based access:  0=xrow, 1=yrow, 2=zrow.
	xm_inline const Vect3f &operator[](int i) const {
		return *(((const Vect3f *) &xx) + i);
	}
	xm_inline Vect3f &operator[](int i)       {
		return *(((Vect3f *) &xx) + i);
	}

	// set matrix to the skew symmetric matrix corresponding to 'v X'
	xm_inline Mat3f &setSkew(const Vect3f &v);

	// for reading rows
	xm_inline const Vect3f &xrow() const {
		return *((const Vect3f *) &xx);
	}
	xm_inline const Vect3f &yrow() const {
		return *((const Vect3f *) &yx);
	}
	xm_inline const Vect3f &zrow() const {
		return *((const Vect3f *) &zx);
	}
	// for writing to rows
	xm_inline Vect3f &xrow()  {
		return *((Vect3f *) &xx);
	}
	xm_inline Vect3f &yrow()  {
		return *((Vect3f *) &yx);
	}
	xm_inline Vect3f &zrow()  {
		return *((Vect3f *) &zx);
	}

	// for reading columns
	xm_inline const Vect3f xcol() const {
		return Vect3f(xx, yx, zx);
	}
	xm_inline const Vect3f ycol() const {
		return Vect3f(xy, yy, zy);
	}
	xm_inline const Vect3f zcol() const {
		return Vect3f(xz, yz, zz);
	}
	xm_inline const Vect3f col(int axis) const {
		return axis == X_AXIS ? Vect3f(xx, yx, zx) : (axis == Y_AXIS ? Vect3f(xy, yy, zy) : Vect3f(xz, yz, zz));
	}
	// for writing to columns
	xm_inline Mat3f &setXcol(const Vect3f &v);
	xm_inline Mat3f &setYcol(const Vect3f &v);
	xm_inline Mat3f &setZcol(const Vect3f &v);
	xm_inline Mat3f &setCol(int axis, const Vect3f &v) {
		if (axis == X_AXIS) setXcol(v);
		else if (axis == Y_AXIS) setYcol(v);
		else setZcol(v);
		return *this;
	}


	// for reading a symmetric matrix
	xm_inline Vect3f diag() const {
		return Vect3f(xx, yy, zz);
	}
	xm_inline Vect3f sym()  const {
		return Vect3f(yz, zx, xy);
	}


	//    Access to elements for Mapple-like notation (numerating from 1):
	//    1,1   1,2    1,3
	//    2,1   2,2    2,3
	//    3,1   3,2    3,3
	xm_inline const float &operator()(int i, int j) const {
		return (&xx)[(i - 1) * 3 + j - 1];
	}
	xm_inline float &operator()(int i, int j) {
		return (&xx)[(i - 1) * 3 + j - 1];
	}

	//  Determinant  of matrix  /////////
	xm_inline float det() const;

	//  Negate  ///////////////////
	xm_inline Mat3f operator- () const;
	xm_inline Mat3f &negate();               // -this
	xm_inline Mat3f &negate(const Mat3f &M);     // -M

	//  Addition & substruction  /////////////////////////
	xm_inline Mat3f &add(const Mat3f &M, const Mat3f &N);      // M + N
	xm_inline Mat3f &add(const Mat3f &M);           // this + M
	xm_inline Mat3f &sub(const Mat3f &M, const Mat3f &N);      // M - N
	xm_inline Mat3f &sub(const Mat3f &M);           // this - M
	xm_inline Mat3f &operator+= (const Mat3f &M) {
		return add(M);
	}
	xm_inline Mat3f &operator-= (const Mat3f &M) {
		return sub(M);
	}
	xm_inline Mat3f operator+ (const Mat3f &M) const {
		Mat3f N;
		return N.add(*this, M);
	}
	xm_inline Mat3f operator- (const Mat3f &M) const {
		Mat3f N;
		return N.sub(*this, M);
	}

	//  Mat3f - Mat3f multiplication  ///////////
	Mat3f &mult(const Mat3f &M, const Mat3f &N);     // M * N    [!]
	Mat3f &premult(const Mat3f &M);             // M * this  [!]
	Mat3f &postmult(const Mat3f &M);            // this * M  [!]
	xm_inline Mat3f &operator*= (const Mat3f &M) {
		return postmult(M);
	}
	xm_inline Mat3f operator* (const Mat3f &M) const {
		Mat3f N;
		return N.mult(*this, M);
	}

	//  Scalar multiplication  /////////////
	xm_inline Mat3f &scale(const Mat3f &M, float s);        // s * M
	xm_inline Mat3f &scale(const Vect3f &s);                // s * this
	xm_inline Mat3f &scale(float s);                // s * this
	xm_inline Mat3f &operator*= (float s) {
		return scale(s);
	}
	xm_inline Mat3f &operator/= (float s) {
		return scale(1 / s);
	}
	xm_inline Mat3f operator* (float s) const {
		Mat3f N;
		return N.scale(*this, s);
	}
	xm_inline Mat3f operator/ (float s) const {
		Mat3f N;
		return N.scale(*this, 1 / s);
	}
	xm_inline friend Mat3f operator* (float s, const Mat3f &M) {
		Mat3f N;
		return N.scale(M, s);
	}

	//  Multiplication  by Vect3f as diagonal matrix  /////////////
	xm_inline Mat3f &preScale(const Mat3f &M, const Vect3f &v);         // Mat3f(v) * M
	xm_inline Mat3f &preScale(const Vect3f &v);                 // Mat3f(v) * this
	xm_inline Mat3f &postScale(const Mat3f &M, const Vect3f &v);        // M * Mat3f(v)
	xm_inline Mat3f &postScale(const Vect3f &v);                // this * Mat3f(v)

	//  Transposition  ////////////////
	xm_inline Mat3f &xpose();                  // this^T
	xm_inline Mat3f &xpose(const Mat3f &M);     // M^T       [!]
	xm_inline friend Mat3f xpose(const Mat3f &M) {
		Mat3f N;
		return N.xpose(M);
	}

	//  Invertion  ////////////////////
	int  invert();  // this^-1, returns one if the matrix was not invertible, otherwise zero.
	int  invert(const Mat3f &M);    // M^-1      [!]
	xm_inline friend Mat3f invert(const Mat3f &M) {
		Mat3f N;
		N.invert(M);
		return N;
	}

	//  Simmetrize  /////////////
	xm_inline Mat3f &symmetrize(const Mat3f &M);        // M + M^T
	xm_inline Mat3f &symmetrize();                  // this + this^T

	void makeRotationZ();


	// Transforming Vect3f ///////////////////////////////////////////////////////

	// return reference to converted vector
	xm_inline Vect3f &xform(const Vect3f &v, Vect3f &xv) const; // (this)(v) => xv [!]
	xm_inline Vect3f &xform(Vect3f &v) const;           // (this)(v) => v

	// These are exactly like the above methods, except the inverse
	// transform this^-1 (= this^T) is used.  This can be thought of as
	// a row vector transformation, e.g.: (v^T)(this) => xv^T
	xm_inline Vect3f &invXform(const Vect3f &v, Vect3f &xv) const;  // [!]
	xm_inline Vect3f &invXform(Vect3f &v) const;

	//  Transforming operators  ///////////////
	xm_inline friend Vect3f &operator*= (Vect3f &v, const Mat3f &M) {
		return M.xform(v);
	}
	xm_inline friend Vect3f operator* (const Vect3f &v, const Mat3f &M) {
		Vect3f xv;
		return M.xform(v, xv);
	}
	xm_inline friend Vect3f operator* (const Mat3f &M, const Vect3f &v) {
		Vect3f xv;
		return M.xform(v, xv);
	}



	//    I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
	friend ostream &operator<< (ostream &os, const Mat3f &M);
	friend istream &operator>> (istream &is, Mat3f &M);
#endif

	// Mat3f constants ////////////////////////////////////////////////////////////

	static const Mat3f ZERO;    // zero matrix
	static const Mat3f ID;      // identity matrix

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class MatXf
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MatXf {
public:

	Mat3f R;
	Vect3f d;

public:

	// constructors //////////////////////////////////////////////////////////////

	xm_inline MatXf()             {}
	xm_inline MatXf(const Mat3f &R_, const Vect3f &d_) {
		set(R_, d_);
	}

	typedef float float16[16];
	xm_inline MatXf(const float16 &T);

	// setters / accessors / translators /////////////////////////////////////////

	xm_inline MatXf &set(const Mat3f &R_, const Vect3f &d_) {
		R = R_;
		d = d_;
		return *this;
	}

	xm_inline const Mat3f  &rot()   const {
		return R;
	}
	xm_inline const Vect3f &trans() const {
		return d;
	}
	xm_inline Mat3f  &rot()        {
		return R;
	}
	xm_inline Vect3f &trans()      {
		return d;
	}


	//  MatXf - MatXf multiplication  ////////////
	MatXf &mult(const MatXf &M, const MatXf &N);    // M * N     [!]
	MatXf &premult(const MatXf &M);             // M * this  [!]
	MatXf &postmult(const MatXf &M);        // this * M  [!]
	xm_inline MatXf &operator*= (const MatXf &M) {
		return postmult(M);
	}
	xm_inline MatXf operator* (const MatXf &N) const {
		MatXf M;
		return M.mult(*this, N);
	}


	// Doesn't really invert the 3x3-matrix, but only transpose one.
	// That is, works for non-scaled matrix !!!
	MatXf &invert(const MatXf &M);              // M^-1      [!]
	MatXf &invert();                // this^-1

	// Really inverts 3x3-matrix.
	MatXf &Invert(const MatXf &M);              // M^-1      [!]
	MatXf &Invert();                // this^-1

	// Transforming Vect3f ///////////////////////////////////////////////////////

	// MatXs can transform elements of R^3 either as vectors or as
	// points.  The [!] indicates that the operands must be distinct.

	xm_inline Vect3f &xformVect(const Vect3f &v, Vect3f &xv) const; // this*(v 0)=>xv  [!]
	xm_inline Vect3f &xformVect(Vect3f &v) const;       // this*(v 0)=>v
	xm_inline Vect3f &xformPoint(const Vect3f &p, Vect3f &xp) const;// this*(p 1)=>xp  [!]
	xm_inline Vect3f &xformPoint(Vect3f &p) const;          // this*(p 1)=>p

	xm_inline Vect3f operator*(const Vect3f &p) const {
		Vect3f xp;
		xformPoint(p, xp);
		return xp;
	}

	// These are exactly like the above methods, except the inverse
	// transform this^-1 is used.
	xm_inline Vect3f &invXformVect(const Vect3f &v, Vect3f &xv) const;
	xm_inline Vect3f &invXformVect(Vect3f &v) const;
	xm_inline Vect3f &invXformPoint(const Vect3f &p, Vect3f &xp) const;
	xm_inline Vect3f &invXformPoint(Vect3f &p) const;



	//    I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
	friend ostream &operator<<(ostream &os, const MatXf &M);
	friend istream &operator>>(istream &is, MatXf &M);
#endif

	// MatXf constants ////////////////////////////////////////////////////////////

	static const MatXf ID;      // identity matrix

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			Miscellaneous functions
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Decomposition  ////////////////////////////////
xm_inline void decomposition(const Vect3f &axis, const Vect3f &v, Vect3f &v_normal, Vect3f &v_tangent) {
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
xm_inline Vect2i::Vect2i(const Vect2s &v)   {
	x = v.x;
	y = v.y;
}
xm_inline Vect2f::Vect2f(const Vect2i &v)   {
	x = float(v.x);
	y = float(v.y);
}
xm_inline Vect2f::Vect2f(const Vect2s &v)   {
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
//		Vect3f xm_inline definitions
//
///////////////////////////////////////////////////////////////////////////////

//  Dot product  //////////////////////
//xm_inline double dot(const Vect3d& u, const Vect3f& v) { return u.dot(v); }
//xm_inline float dot(const Vect3f& u, const Vect3d& v) { return u.dot(v); }

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


///////////////////////////////////////////////////////////////////////////////
//
//		Mat3f xm_inline definitions
//
///////////////////////////////////////////////////////////////////////////////

Mat3f::Mat3f(float xx_, float xy_, float xz_,
             float yx_, float yy_, float yz_,
             float zx_, float zy_, float zz_) {
	xx = xx_;
	xy = xy_;
	xz = xz_;
	yx = yx_;
	yy = yy_;
	yz = yz_;
	zx = zx_;
	zy = zy_;
	zz = zz_;
}

Mat3f &Mat3f::set(float angle, eAxis axis) {
//	------ Calculate Matrix for ROTATE point an angle ------
	float calpha = (float)cos(angle);
	float salpha = (float)sin(angle);
	switch (axis) {
	case Z_AXIS:
		xx   =  calpha;
		xy   = -salpha;
		xz   = 0;
		yx   =  salpha;
		yy   = calpha;
		yz   = 0;
		zx   = 0;
		zy     = 0;
		zz   = 1;
		break;
	case X_AXIS:
		xx   =  1;
		xy     =  0;
		xz   = 0;
		yx   =  0;
		yy     =  calpha;
		yz   = -salpha;
		zx   =  0;
		zy     =  salpha;
		zz   = calpha;
		break;
	case Y_AXIS:
		xx   = calpha;
		xy   =  0;
		xz   = salpha;
		yx   = 0;
		yy     =  1;
		yz   = 0;
		zx   = -salpha;
		zy   =  0;
		zz   = calpha;
		break;
	default:
		break;
	}
	return *this;
}

Mat3f &Mat3f::set(const Vect3f &diag, const Vect3f &sym) {
	xx = diag.x;
	yy = diag.y;
	zz = diag.z;
	yz = zy = sym.x;
	zx = xz = sym.y;
	xy = yx = sym.z;
	return *this;
}


Mat3f &Mat3f::setXcol(const Vect3f &v) {
	xx = v.x;
	yx = v.y;
	zx = v.z;
	return *this;
}


Mat3f &Mat3f::setYcol(const Vect3f &v) {
	xy = v.x;
	yy = v.y;
	zy = v.z;
	return *this;
}


Mat3f &Mat3f::setZcol(const Vect3f &v) {
	xz = v.x;
	yz = v.y;
	zz = v.z;
	return *this;
}


Mat3f &Mat3f::setSkew(const Vect3f &v) {
	xx = yy = zz = 0.0;
	zy =  v.x;
	yz = -v.x;
	xz =  v.y;
	zx = -v.y;
	yx =  v.z;
	xy = -v.z;
	return *this;
}


float Mat3f::det() const {
	return  xx * (yy * zz - yz * zy)
	        + xy * (yz * zx - yx * zz)
	        + xz * (yx * zy - yy * zx);
}


Mat3f &Mat3f::xpose(const Mat3f &M) {
	xx = M.xx;
	xy = M.yx;
	xz = M.zx;

	yx = M.xy;
	yy = M.yy;
	yz = M.zy;

	zx = M.xz;
	zy = M.yz;
	zz = M.zz;
	return *this;
}


Mat3f &Mat3f::xpose() {
	float tmp;

	tmp = xy;
	xy = yx;
	yx = tmp;

	tmp = yz;
	yz = zy;
	zy = tmp;

	tmp = zx;
	zx = xz;
	xz = tmp;
	return *this;
}


Mat3f &Mat3f::symmetrize(const Mat3f &M) {
	xx = 2 * M.xx;
	yy = 2 * M.yy;
	zz = 2 * M.zz;
	xy = yx = M.xy + M.yx;
	yz = zy = M.yz + M.zy;
	zx = xz = M.zx + M.xz;
	return *this;
}


Mat3f &Mat3f::symmetrize() {
	xx = 2 * xx;
	yy = 2 * yy;
	zz = 2 * zz;
	xy = yx = xy + yx;
	yz = zy = yz + zy;
	zx = xz = zx + xz;
	return *this;
}

Mat3f Mat3f::operator- () const {
	Mat3f M;
	M.xx = - xx;
	M.xy = - xy;
	M.xz = - xz;

	M.yx = - yx;
	M.yy = - yy;
	M.yz = - yz;

	M.zx = - zx;
	M.zy = - zy;
	M.zz = - zz;
	return M;
}

Mat3f &Mat3f::negate(const Mat3f &M) {
	xx = - M.xx;
	xy = - M.xy;
	xz = - M.xz;

	yx = - M.yx;
	yy = - M.yy;
	yz = - M.yz;

	zx = - M.zx;
	zy = - M.zy;
	zz = - M.zz;

	return *this;
}


Mat3f &Mat3f::negate() {
	xx = - xx;
	xy = - xy;
	xz = - xz;

	yx = - yx;
	yy = - yy;
	yz = - yz;

	zx = - zx;
	zy = - zy;
	zz = - zz;

	return *this;
}


Mat3f &Mat3f::add(const Mat3f &M, const Mat3f &N) {
	xx = M.xx + N.xx;
	xy = M.xy + N.xy;
	xz = M.xz + N.xz;

	yx = M.yx + N.yx;
	yy = M.yy + N.yy;
	yz = M.yz + N.yz;

	zx = M.zx + N.zx;
	zy = M.zy + N.zy;
	zz = M.zz + N.zz;

	return *this;
}


Mat3f &Mat3f::add(const Mat3f &M) {
	xx += M.xx;
	xy += M.xy;
	xz += M.xz;

	yx += M.yx;
	yy += M.yy;
	yz += M.yz;

	zx += M.zx;
	zy += M.zy;
	zz += M.zz;

	return *this;
}


Mat3f &Mat3f::sub(const Mat3f &M, const Mat3f &N) {
	xx = M.xx - N.xx;
	xy = M.xy - N.xy;
	xz = M.xz - N.xz;

	yx = M.yx - N.yx;
	yy = M.yy - N.yy;
	yz = M.yz - N.yz;

	zx = M.zx - N.zx;
	zy = M.zy - N.zy;
	zz = M.zz - N.zz;

	return *this;
}


Mat3f &Mat3f::sub(const Mat3f &M) {
	xx -= M.xx;
	xy -= M.xy;
	xz -= M.xz;

	yx -= M.yx;
	yy -= M.yy;
	yz -= M.yz;

	zx -= M.zx;
	zy -= M.zy;
	zz -= M.zz;

	return *this;
}


Mat3f &Mat3f::scale(const Mat3f &M, float s) {
	xx = s * M.xx;
	xy = s * M.xy;
	xz = s * M.xz;
	yx = s * M.yx;
	yy = s * M.yy;
	yz = s * M.yz;
	zx = s * M.zx;
	zy = s * M.zy;
	zz = s * M.zz;

	return *this;
}


Mat3f &Mat3f::scale(const Vect3f &s) {
	xx *= s.x;
	xy *= s.y;
	xz *= s.z;
	yx *= s.x;
	yy *= s.y;
	yz *= s.z;
	zx *= s.x;
	zy *= s.y;
	zz *= s.z;

	return *this;
}

Mat3f &Mat3f::scale(float s) {
	xx *= s;
	xy *= s;
	xz *= s;
	yx *= s;
	yy *= s;
	yz *= s;
	zx *= s;
	zy *= s;
	zz *= s;

	return *this;
}

Mat3f &Mat3f::preScale(const Mat3f &M, const Vect3f &v) {
	// Mat3f(v) * M
	xx = M.xx * v.x;
	xy = M.xy * v.x;
	xz = M.xz * v.x;

	yx = M.yx * v.y;
	yy = M.yy * v.y;
	yz = M.yz * v.y;

	zx = M.zx * v.z;
	zy = M.zy * v.z;
	zz = M.zz * v.z;
	return *this;
}

Mat3f &Mat3f::preScale(const Vect3f &v) {
	// Mat3f(s) * this
	xx *= v.x;
	xy *= v.x;
	xz *= v.x;

	yx *= v.y;
	yy *= v.y;
	yz *= v.y;

	zx *= v.z;
	zy *= v.z;
	zz *= v.z;
	return *this;
}

Mat3f &Mat3f::postScale(const Mat3f &M, const Vect3f &v) {
	// M * Mat3f(v)
	xx = M.xx * v.x;
	xy = M.xy * v.y;
	xz = M.xz * v.z;

	yx = M.yx * v.x;
	yy = M.yy * v.y;
	yz = M.yz * v.z;

	zx = M.zx * v.x;
	zy = M.zy * v.y;
	zz = M.zz * v.z;
	return *this;
}

Mat3f &Mat3f::postScale(const Vect3f &v) {
	// this * Mat3f(v)
	xx *= v.x;
	xy *= v.y;
	xz *= v.z;

	yx *= v.x;
	yy *= v.y;
	yz *= v.z;

	zx *= v.x;
	zy *= v.y;
	zz *= v.z;
	return *this;
}


//  Vect3f transforming  /////////////////////
Vect3f &Mat3f::xform(const Vect3f &v, Vect3f &xv) const {
	xv.x = (float)(xx * v.x + xy * v.y + xz * v.z);
	xv.y = (float)(yx * v.x + yy * v.y + yz * v.z);
	xv.z = (float)(zx * v.x + zy * v.y + zz * v.z);
	return xv;
}


Vect3f &Mat3f::xform(Vect3f &v) const {
	float ox, oy;

	ox = v.x;
	oy = v.y;
	v.x = (float)(xx * ox + xy * oy + xz * v.z);
	v.y = (float)(yx * ox + yy * oy + yz * v.z);
	v.z = (float)(zx * ox + zy * oy + zz * v.z);
	return v;
}


Vect3f &Mat3f::invXform(const Vect3f &v, Vect3f &xv) const {
	xv.x = (float)(xx * v.x + yx * v.y + zx * v.z);
	xv.y = (float)(xy * v.x + yy * v.y + zy * v.z);
	xv.z = (float)(xz * v.x + yz * v.y + zz * v.z);
	return xv;
}


Vect3f &Mat3f::invXform(Vect3f &v) const {
	float ox, oy;

	ox = v.x;
	oy = v.y;
	v.x = (float)(xx * ox + yx * oy + zx * v.z);
	v.y = (float)(xy * ox + yy * oy + zy * v.z);
	v.z = (float)(xz * ox + yz * oy + zz * v.z);
	return v;
}


///////////////////////////////////////////////////////////////////////////////
//
//		MatXf xm_inline definitions
//
///////////////////////////////////////////////////////////////////////////////
MatXf::MatXf(const float16 &T) {
	R[0][0] = T[0];
	R[1][0] = T[1];
	R[2][0] = T[2];
	R[0][1] = T[4];
	R[1][1] = T[5];
	R[2][1] = T[6];
	R[0][2] = T[8];
	R[1][2] = T[9];
	R[2][2] = T[10];
	d[0] = T[12];
	d[1] = T[13];
	d[2] = T[14];
}

//  Vect3f transforming  /////////////////////
Vect3f &MatXf::xformVect(const Vect3f &v, Vect3f &xv) const {
	return R.xform(v, xv);
}


Vect3f &MatXf::xformVect(Vect3f &v) const {
	return R.xform(v);
}


Vect3f &MatXf::xformPoint(const Vect3f &p, Vect3f &xp) const {
	R.xform(p, xp);
	xp.add(d);
	return xp;
}


Vect3f &MatXf::xformPoint(Vect3f &p) const {
	R.xform(p);
	p.add(d);
	return p;
}


Vect3f &MatXf::invXformVect(const Vect3f &v, Vect3f &xv) const {
	return R.invXform(v, xv);
}


Vect3f &MatXf::invXformVect(Vect3f &v) const {
	return R.invXform(v);
}


Vect3f &MatXf::invXformPoint(const Vect3f &p, Vect3f &xp) const {
	xp.sub(p, d);
	R.invXform(xp);
	return xp;
}


Vect3f &MatXf::invXformPoint(Vect3f &p) const {
	p.sub(d);
	R.invXform(p);
	return p;
}


//////////////////////////////////////////////////////////////////////////////////
//
//           Stream's I/O operations
//
//////////////////////////////////////////////////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM

//  Vect2s  I/O //////////////////////////
inline ostream &operator<<(ostream &os, const Vect2i &v) {
	os << v.x << "  " << v.y;
	return os;
}

inline istream &operator>>(istream &is, Vect2i &v) {
	is >> v.x >> v.y;
	return is;
}

//  Vect2s  I/O //////////////////////////
inline ostream &operator<<(ostream &os, const Vect2f &v) {
	os << v.x << "  " << v.y;
	return os;
}

inline istream &operator>>(istream &is, Vect2f &v) {
	is >> v.x >> v.y;
	return is;
}


//  Vect2s  I/O //////////////////////////
inline ostream &operator<<(ostream &os, const Vect2s &v) {
	os << v.x << "  " << v.y;
	return os;
}

inline istream &operator>>(istream &is, Vect2s &v) {
	is >> v.x >> v.y;
	return is;
}


//  Vect3f  I/O //////////////////////////
inline ostream &operator<<(ostream &os, const Vect3f &v) {
	os << v.x << "  " << v.y << "  " << v.z;
	return os;
}

inline istream &operator>>(istream &is, Vect3f &v) {
	is >> v.x >> v.y >> v.z;
	return is;
}

//  Mat3f I/O  ////////////////////////
inline ostream &operator<<(ostream &os, const Mat3f &m) {
	os << m.xx << " \t" << m.xy << " \t" << m.xz << "\n";
	os << m.yx << " \t" << m.yy << " \t" << m.yz << "\n";
	os << m.zx << " \t" << m.zy << " \t" << m.zz;
	return os;
}

inline istream &operator>>(istream &is, Mat3f &m) {
	is >> m.xx >> m.xy >> m.xz;
	is >> m.yx >> m.yy >> m.yz;
	is >> m.zx >> m.zy >> m.zz;
	return is;
}


//  MatXf  I/O   ///////////////////
inline ostream &operator<<(ostream &os, const MatXf &m) {
	return os << m.R << "  " << m.d;
}

#endif  // _XMATH_NO_IOSTREAM

//#ifdef _DEBUG
//#pragma comment(lib,"XMathD.lib")
//#else
//#pragma comment(lib,"XMath.lib")
//#endif

} // namespace QDEngine

#endif // QDENGINE_UTIL_XMATH_H
