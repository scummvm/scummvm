///////////////////////////////////////////////////////////////////////////////
//
//	All 3D-functionality
//
//	Configuration:
//	Define _XMATH_NO_IOSTREAM to disable iostream using
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __XMATH_H__
#define __XMATH_H__

#include <math.h>
#include "XMath/fastMath.h"

#ifdef _XMATH_USE_IOSTREAM
#include <iostream>
using std::istream;
using std::ostream;
//#if (_MSC_VER >= 1300)
//using std::endl;
//#endif
#endif

class Archive;

///////////////////////////////////////////////////////////////////////////////
//	Structures predefenition
///////////////////////////////////////////////////////////////////////////////

class Vect2f;
class Vect2i;
class Vect2s;
class Vect3f;
class Vect3d;
class Mat3f;
class Mat3d;
class MatXf;
class MatXd;
class QuatF; 
class QuatD;
class Se3f; 
class Se3d;

class Vect4f;

class XStream;
class XBuffer;

///////////////////////////////////////////////////////////////////////////////
//		Axes 
///////////////////////////////////////////////////////////////////////////////
enum eAxis 
{ 
	X_AXIS=0, 
	Y_AXIS=1,
	Z_AXIS=2,
	W_AXIS=3
};


///////////////////////////////////////////////////////////////////////////////
//  		Constants
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
#define xm_inline __forceinline
#endif //_MSC_VER

///////////////////////////////////////////////////////////////////////////////
//
//  		Scalar Functions
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __ROUND__
#define __ROUND__

xm_inline int round(double x)
{
	int a;
	_asm {
		fld x
		fistp dword ptr a
	}
	return a;
}

xm_inline int round(float x)
{
	int a;
	_asm {
		fld x
		fistp dword ptr a
	}
	return a;
}

template <class T> 
xm_inline T sqr(const T& x){ return x*x; }

#endif // __ROUND__

xm_inline double SIGN(double x) { return x < 0.0 ? -1.0 : 1.0; }
xm_inline float SIGN(float x) { return x < 0.0f ? -1.0f : 1.0f; }
xm_inline int SIGN(int x) { return x != 0 ? (x > 0 ? 1 : -1 ) : 0; }

xm_inline double SIGN(double a, double b) { return b >= 0.0 ? fabs(a) : -fabs(a); }
xm_inline float SIGN(float a, float b) { return b >= 0.0f ? fabsf(a) : -fabsf(a); }

class RandomGenerator 
{
	enum { max_value = 0x7fff };
	int value;
public:
	RandomGenerator(int val = 1) { set(val); }
	void set(int val) { value = val; }
	int get() const { return value; }
	int operator()(); // Generates random value [0..max_value), non-inline due to some bugs with optimization
	xm_inline int operator()(int m) { return m ? (*this)() % m : 0; } // May by used in random_shuffle
	xm_inline int operator()(int min, int max) { return min + (*this)(max - min); }
	xm_inline float frnd(float x = 1.f){ return (float)((*this)()*2 - max_value)*x/(float)max_value; }
	xm_inline float fabsRnd(float x = 1.f){ return (float)((*this)())*x/(float)max_value; }
	xm_inline float fabsRnd(float min, float max){ return min + fabsRnd(max - min); }
	xm_inline float frand(){ return (*this)()/(float)max_value; }
};

#undef random
extern RandomGenerator xm_random_generator;
xm_inline unsigned random(unsigned m){ return xm_random_generator(m); }
xm_inline float frnd(float x){ return xm_random_generator.frnd(x); }
xm_inline float fabsRnd(float x){ return xm_random_generator.fabsRnd(x); }


#define NOMINMAX
#undef min
#undef max

xm_inline int min(int x,int y){ return x < y ? x : y; }
xm_inline float min(float x,float y){ return x < y ? x : y; }
xm_inline double min(double x,double y){ return x < y ? x : y; }

xm_inline int max(int x,int y){ return x > y ? x : y; }
xm_inline float max(float x,float y){ return x > y ? x : y; }
xm_inline double max(double x,double y){ return x > y ? x : y; }


template <class T> xm_inline T min(const T& a, const T& b, const T& c) { return min(min(a, b), c); }
template <class T> xm_inline T max(const T& a, const T& b, const T& c) { return max(max(a, b), c); }

xm_inline int mina(int x,int y){ return abs(x) < abs(y) ? x : y; }
xm_inline int mina(int x,int y,int z){ return mina(mina(x,y),z); }

xm_inline int maxa(int x,int y){ return abs(x) > abs(y) ? x : y; }
xm_inline int maxa(int x,int y,int z){ return maxa(maxa(x,y),z); }

xm_inline double mina(double x,double y){ return fabs(x) < fabs(y) ? x : y; }
xm_inline double mina(double x,double y,double z){ return mina(mina(x,y),z); }

xm_inline double maxa(double x,double y){ return fabs(x) > fabs(y) ? x : y; }
xm_inline double maxa(double x,double y,double z){ return maxa(maxa(x,y),z); }

xm_inline int minAbs(int x,int y){ return abs(x) < abs(y) ? abs(x) : abs(y); }
xm_inline int minAbs(int x,int y,int z){ return minAbs(minAbs(x,y),z); }

xm_inline int maxAbs(int x,int y){ return abs(x) > abs(y) ? abs(x) : abs(y); }
xm_inline int maxAbs(int x,int y,int z){ return maxAbs(maxAbs(x,y),z); }

xm_inline double minAbs(double x,double y){ return fabs(x) < fabs(y) ? fabs(x) : fabs(y); }
xm_inline double minAbs(double x,double y,double z){ return minAbs(minAbs(x,y),z); }

xm_inline double maxAbs(double x,double y){ return fabs(x) > fabs(y) ? fabs(x) : fabs(y); }
xm_inline double maxAbs(double x,double y,double z){ return maxAbs(maxAbs(x,y),z); }


xm_inline void average(double& x_avr, double x, double tau){ x_avr = x_avr*(1. - tau) + tau*x; }
xm_inline void average(double& x_avr, double x, double tau, double factor){ tau = pow(tau, factor); x_avr = x_avr*(1. - tau) + tau*x; }
xm_inline void average(float& x_avr, float x, float tau){ x_avr = x_avr*(1.f - tau) + tau*x; }
xm_inline void average(float& x_avr, float x, float tau, float factor){ tau = (float)pow(tau, factor); x_avr = x_avr*(1.f - tau) + tau*x; }

#define G2R(x) ((x)*M_PI/180.f)  
#define R2G(x) ((x)*180.f/M_PI)

xm_inline float Acos(float  x){ return x > 1.f ? 0 : (x < -1.f ? M_PI : acosf(x)); }
xm_inline double Acos(double  x){ return x > 1. ? 0 : (x < -1. ? M_PI : acos(x)); }

template<class T, class T1, class T2> 
xm_inline T clamp(const T& x, const T1& xmin, const T2& xmax) { if(x < xmin) return xmin; if(x > xmax) return xmax; return x; }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Vect2f
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Vect2f
{
public:	
	float x,y;

	xm_inline Vect2f()								{ }
	xm_inline Vect2f(float x_,float y_)					{ x = x_; y = y_; }
	
	typedef float float2[2];
	xm_inline Vect2f(const float2& v) { x = v[0]; y = v[1]; }

	xm_inline Vect2f(const Vect2i& v);
	xm_inline Vect2f(const Vect2s& v);

	xm_inline Vect2f& set(float x_, float y_)			{ x = x_; y = y_; return *this; }
	xm_inline Vect2f operator - () const				{ return Vect2f(-x,-y); }

	xm_inline int xi() const { return round(x); }
	xm_inline int yi() const { return round(y); }

	xm_inline const float& operator[](int i) const			{ return *(&x + i); }
	xm_inline float& operator[](int i)						{ return *(&x + i); }
	
	xm_inline Vect2f& operator += (const Vect2f &v)	{ x += v.x; y += v.y; return *this; }
	xm_inline Vect2f& operator -= (const Vect2f &v)	{ x -= v.x; y -= v.y; return *this; }
	xm_inline Vect2f& operator *= (const Vect2f &v)	{ x *= v.x; y *= v.y; return *this; }
	xm_inline Vect2f& operator /= (const Vect2f &v)	{ x /= v.x; y /= v.y; return *this; }
	xm_inline Vect2f& operator *= (float f)			{ x *= f; y *= f; return *this; }
	xm_inline Vect2f& operator /= (float f)			{ if(f != 0.f) f = 1/f; else f = 0.0001f; x *= f; y *= f; return *this; }

	xm_inline Vect2f operator + (const Vect2f &v) const		{ return Vect2f(*this) += v; }
	xm_inline Vect2f operator - (const Vect2f &v) const		{ return Vect2f(*this) -= v; }
	xm_inline Vect2f operator * (const Vect2f &v) const		{ return Vect2f(*this) *= v; }
	xm_inline Vect2f operator / (const Vect2f &v) const		{ return Vect2f(*this) /= v; }
	xm_inline Vect2f operator * (float f)	const		{ return Vect2f(*this) *= f; }
	xm_inline Vect2f operator / (float f)	const		{ return Vect2f(*this) /= f; }

	xm_inline bool eq(const Vect2f &v, float delta = FLT_COMPARE_TOLERANCE) const { return fabsf(v.x - x) < delta && fabsf(v.y - y) < delta; }

	xm_inline float dot(const Vect2f& v) const { return x*v.x + y*v.y; }
	xm_inline friend float dot(const Vect2f& u, const Vect2f& v) { return u.dot(v); }

	float angle(const Vect2f& other) const;

	xm_inline float operator % (const Vect2f &v) const { return x*v.y - y*v.x; }

	xm_inline Vect2f& scaleAdd(const Vect2f& u, float lambda) { x += lambda * u.x; y += lambda * u.y; return *this; }

	xm_inline Vect2f& interpolate(const Vect2f& u, const Vect2f& v, float lambda); // (1-lambda)*u + lambda*v

	xm_inline float norm()	const						{ return sqrtf(x*x + y*y); }
	xm_inline float norm2() const						{ return x*x + y*y; }
	xm_inline Vect2f& normalize(float norma)				{ float f = norma*invSqrtFast(x*x + y*y); x *= f; y *= f; return *this; }
	xm_inline float distance(const Vect2f &v) const	{ return sqrtf(distance2(v)); }
	xm_inline float distance2(const Vect2f &v) const	{ float dx = x - v.x, dy = y - v.y; return dx*dx + dy*dy; }

	xm_inline void swap(Vect2f &v)					{ Vect2f tmp = v; v = *this; *this = tmp; }

	//	I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
	friend ostream& operator<< (ostream& os, const Vect2f& v);
	friend istream& operator>> (istream& is, Vect2f& v);
#endif
	
	friend XStream& operator<= (XStream& s,const Vect2f& v);
	friend XStream& operator>= (XStream& s,Vect2f& v);
	friend XStream& operator< (XStream& s,const Vect2f& v);
	friend XStream& operator> (XStream& s,Vect2f& v);
	
	friend XBuffer& operator<= (XBuffer& b,const Vect2f& v);
	friend XBuffer& operator>= (XBuffer& b,Vect2f& v);
	friend XBuffer& operator< (XBuffer& b,const Vect2f& v);
	friend XBuffer& operator> (XBuffer& b,Vect2f& v);

    void serialize(Archive& ar);

	static const Vect2f ZERO;
	static const Vect2f ID;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Vect2i
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Vect2i
{
public:
	int x,y;

	xm_inline Vect2i()								{ }
	xm_inline Vect2i(int x_, int y_)						{ x = x_; y = y_; }
	xm_inline Vect2i(float x_, float y_)					{ x = round(x_); y = round(y_); }
	
	xm_inline Vect2i(const Vect2f& v)			{ x = round(v.x); y = round(v.y); }
	xm_inline Vect2i(const Vect2s& v);

	xm_inline void set(int x_, int y_)					{ x = x_; y=y_; }
	xm_inline void set(float x_, float y_)				{ x = round(x_); y = round(y_); }
	xm_inline Vect2i operator - () const				{ return Vect2i(-x, -y); }

	xm_inline const int& operator[](int i) const			{ return *(&x + i); }
	xm_inline int& operator[](int i)						{ return *(&x + i); }

	xm_inline Vect2i& operator += (const Vect2i& v)	{ x += v.x; y += v.y; return *this; }
	xm_inline Vect2i& operator -= (const Vect2i& v)	{ x -= v.x; y -= v.y; return *this; }
	xm_inline Vect2i& operator *= (const Vect2i& v)	{ x *= v.x; y *= v.y; return *this; }
	xm_inline Vect2i& operator /= (const Vect2i& v)	{ x /= v.x; y /= v.y; return *this; }

	xm_inline Vect2i operator + (const Vect2i& v) const 	{ return Vect2i(*this) += v; }
	xm_inline Vect2i operator - (const Vect2i& v) const 	{ return Vect2i(*this) -= v; }
	xm_inline Vect2i operator * (const Vect2i& v) const 	{ return Vect2i(*this) *= v; }

	xm_inline Vect2i& operator *= (int f)				{ x *= f; y *= f; return *this; }
	xm_inline Vect2i operator * (int f) const 		{ return Vect2i(*this) *= f; }
	
	xm_inline Vect2i& operator >>= (int n)				{ x >>= n; y >>= n; return *this; }
	xm_inline Vect2i operator >> (int n) const 		{ return Vect2i(*this) >>= n; }

	xm_inline Vect2i& operator *= (float f)				{ x = round(x*f); y = round(y*f); return *this; }
	xm_inline Vect2i& operator /= (float f)				{  return *this *= 1.f/f; }
	xm_inline Vect2i operator * (float f) const 		{ return Vect2i(*this) *= f; }
	xm_inline Vect2i operator / (float f) const 		{  return Vect2i(*this) /= f; }

	xm_inline int dot(const Vect2i& v) const { return x*v.x + y*v.y; }
	xm_inline friend int dot(const Vect2i& u, const Vect2i& v) { return u.dot(v); }
	
	xm_inline int operator % (const Vect2i &v) const { return x*v.y - y*v.x; }

	xm_inline int norm() const 						{ return round(sqrtf(float(x*x+y*y))); }
	xm_inline int norm2() const						{ return x*x+y*y; }
	
	xm_inline void normalize(int norma)				{ float f=(float)norma*invSqrtFast((float)(x*x + y*y)); x=round(x*f); y=round(y*f); }
	xm_inline int distance2(const Vect2i& v) const	{ return sqr(x - v.x) + sqr(y - v.y); }

	xm_inline int operator == (const Vect2i& v)	const	{ return x == v.x && y == v.y; }
	xm_inline int operator != (const Vect2i& v)	const	{ return x != v.x || y != v.y; }

	xm_inline void swap(Vect2i &v)					{ Vect2i tmp = v; v = *this; *this = tmp; }

	//	I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
	friend ostream& operator<< (ostream& os, const Vect2i& v);
	friend istream& operator>> (istream& is, Vect2i& v);
#endif
	
	friend XStream& operator<= (XStream& s,const Vect2i& v);
	friend XStream& operator>= (XStream& s,Vect2i& v);
	friend XStream& operator< (XStream& s,const Vect2i& v);
	friend XStream& operator> (XStream& s,Vect2i& v);
	
	friend XBuffer& operator<= (XBuffer& b,const Vect2i& v);
	friend XBuffer& operator>= (XBuffer& b,Vect2i& v);
	friend XBuffer& operator< (XBuffer& b,const Vect2i& v);
	friend XBuffer& operator> (XBuffer& b,Vect2i& v);

    void serialize(Archive& ar);

	static const Vect2i ZERO;
	static const Vect2i ID;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Vect2s
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Vect2s
{
public:
	short x,y;

	xm_inline Vect2s()										{ }
	xm_inline Vect2s(int x_,int y_)							{ x = x_; y = y_; }

	xm_inline Vect2s(const Vect2f& v)			{ x = round(v.x); y = round(v.y); }
	xm_inline Vect2s(const Vect2i& v)			{ x = v.x; y = v.y; }

	xm_inline void set(int x_, int y_)					{ x = x_; y = y_; }
	xm_inline Vect2s operator - () const				{ return Vect2s(-x,-y); }

	xm_inline const short& operator[](int i) const			{ return *(&x + i); }
	xm_inline short& operator[](int i)						{ return *(&x + i); }

	xm_inline Vect2s& operator += (const Vect2s& v)	{ x += v.x; y += v.y; return *this; }
	xm_inline Vect2s& operator -= (const Vect2s& v)	{ x -= v.x; y -= v.y; return *this; }
	xm_inline Vect2s& operator *= (const Vect2s& v)	{ x *= v.x; y *= v.y; return *this; }
	xm_inline Vect2s& operator *= (float f)			{ x = round(x*f); y = round(y*f); return *this; }
	xm_inline Vect2s& operator /= (float f)			{ if(f!=0.f) f=1/f; else f=0.0001f; x=round(x*f); y=round(y*f); return *this; }
	xm_inline Vect2s operator - (const Vect2s& v) const	{ return Vect2s(x - v.x, y - v.y); }
	xm_inline Vect2s operator + (const Vect2s& v) const	{ return Vect2s(x + v.x, y + v.y); }
	xm_inline Vect2s operator * (const Vect2s& v) const	{ return Vect2s(x*v.x, y*v.y); }
	xm_inline Vect2s operator * (float f) const				{ Vect2s tmp(round(x*f),round(y*f)); return tmp; }
	xm_inline Vect2s operator / (float f) const				{ if(f!=0.f) f=1/f; else f=0.0001f; Vect2s tmp(round(x*f),round(y*f)); return tmp; }

	xm_inline int operator == (const Vect2s& v)	const	{ return x == v.x && y == v.y; }

	xm_inline int norm() const								{ return round(sqrtf((float)(x*x+y*y))); }
	xm_inline int norm2() const								{ return x*x+y*y; }
	xm_inline int distance(const Vect2s& v) const			{ int dx=v.x-x,dy=v.y-y; return round(sqrtf((float)(dx*dx+dy*dy))); }
	xm_inline void normalize(int norma)				{ float f=(float)norma*invSqrtFast((float)((int)x*x + (int)y*y)); x=round(x*f); y=round(y*f); }

	xm_inline void swap(Vect2s &v)					{ Vect2s tmp = v; v = *this; *this = tmp; }
	
	//	I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
	friend ostream& operator<< (ostream& os, const Vect2s& v);
	friend istream& operator>> (istream& is, Vect2s& v);
#endif

	friend XStream& operator<= (XStream& s,const Vect2s& v);
	friend XStream& operator>= (XStream& s,Vect2s& v);
	friend XStream& operator< (XStream& s,const Vect2s& v);
	friend XStream& operator> (XStream& s,Vect2s& v);

	friend XBuffer& operator<= (XBuffer& b,const Vect2s& v);
	friend XBuffer& operator>= (XBuffer& b,Vect2s& v);
	friend XBuffer& operator< (XBuffer& b,const Vect2s& v);
	friend XBuffer& operator> (XBuffer& b,Vect2s& v);

	void serialize(Archive& ar);
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Mat2f
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Mat2f
{
  float xx, xy,
       yx, yy;
public:
	Mat2f(){}
	explicit Mat2f(float angle) { set(angle); }
	explicit Mat2f(const Vect2f& xAxis) { set(xAxis.x, xAxis.y, xAxis.y, -xAxis.x); } 
	Mat2f(float xx_, float xy_, float yx_, float yy_) { set(xx_, xy_, yx_, yy_); }
	void set(float angle){ xx = yy = cosf(angle); yx = sinf(angle); xy = -yx; }
	void set(float xx_, float xy_, float yx_, float yy_) { xx = xx_; xy = xy_; yx = yx_; yy = yy_; }
	
	// Rows
	xm_inline const Vect2f& operator[](int i) const	{ return ((Vect2f*)&xx)[i]; } 
	xm_inline Vect2f& operator[](int i)		{ return ((Vect2f*)&xx)[i]; }

	// Columns
	xm_inline Vect2f xcol() const { return Vect2f(xx, yx); }
	xm_inline Vect2f ycol() const { return Vect2f(xy, yy); }
	xm_inline Vect2f col(int axis) const { return axis == X_AXIS ? Vect2f(xx, yx) : Vect2f(xy, yy); }

	void invert(){ float t = xy; xy = yx; yx = t; }
	void Invert();

	xm_inline Mat2f& operator*= (const Mat2f& m) { return (*this) = Mat2f(xx*m.xx+xy*m.yx, xx*m.xy+xy*m.yy, yx*m.xx+yy*m.yx, yx*m.xy+yy*m.yy); }
	xm_inline const Mat2f operator* (const Mat2f& m) const { return Mat2f(*this) *= m; }

	// forward transform
	xm_inline friend Vect2f& operator*= (Vect2f& v, const Mat2f& m) { float x = v.x*m.xx + v.y*m.xy; v.y = v.x*m.yx + v.y*m.yy; v.x = x; return v; }
	// backward transform
	xm_inline Vect2f invXform(const Vect2f& v) const { return Vect2f(v.x*xx + v.y*yx, v.x*xy + v.y*yy); }

	static const Mat2f ID;
};
// forward transform
xm_inline const Vect2f operator* (const Mat2f& m, const Vect2f& v) { return Vect2f(v) *= m; }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class MatX2f
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MatX2f
{
public:
	Mat2f rot;
	Vect2f trans;

	MatX2f(){}
	MatX2f(const Mat2f& r, const Vect2f& t) : rot(r), trans(t) {}
	void set(const Mat2f& r, const Vect2f& t) { rot = r; trans = t; }

	void invert() { rot.invert(); trans = -rot.invXform(trans); }

	// forward transform
	friend Vect2f& operator *=(Vect2f& v, const MatX2f& m) { v *= m.rot; v += m.trans; return v; }
	// backward transform
	Vect2f invXform(const Vect2f& v) const { return rot.invXform(v - trans); }

	static const MatX2f ID;
};
// forward transform
xm_inline const Vect2f operator* (const MatX2f& m, const Vect2f& v) { return Vect2f(v) *= m; }



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Vect3f
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Vect3f 
{

public:
  typedef float float3[3];

  union {
	struct { float x, y, z; };
	struct { float3 array;  };
  };

  // constructors //////////////////////////////////////////////////////////////

  xm_inline Vect3f() {}
  xm_inline Vect3f(float x_, float y_, float z_) {x = x_; y = y_; z = z_;}
  explicit xm_inline Vect3f(const Vect2f& v, float z_ = 0) { x = v.x; y = v.y; z = z_;}

  xm_inline Vect3f(const float3& v) {x = v[0]; y = v[1]; z = v[2];}

  xm_inline operator const float3& () const { return array; }
  xm_inline operator float3& () { return array; }

  xm_inline operator Vect3d () const;
  xm_inline operator const Vect2f& () const { return *reinterpret_cast<const Vect2f*>(this); }

  // setters / accessors / translators /////////////////////////////////////////

  xm_inline Vect3f& set(float x_, float y_, float z_) { x = x_; y = y_; z = z_; return *this; }
  //xm_inline Vect3f& set(const float3& v) {x = v[0]; y = v[1]; z = v[2]; return *this; }

  xm_inline Vect3f& setSpherical(float psi,float theta,float radius);

  // index-based access:  0=x, 1=y, 2=z.
  xm_inline const float& operator[](int i) const {return *(&x + i);}
  xm_inline float& operator[](int i)       {return *(&x + i);}

  // Fortran index-based access:  1=x, 2=y, 3=z.
  xm_inline const float& operator()(int i) const {return *(&x + i - 1);}
  xm_inline float& operator()(int i)       {return *(&x + i - 1);}

  // Convertion to int ///////
  xm_inline int xi() const { return round(x); }
  xm_inline int yi() const { return round(y); }
  xm_inline int zi() const { return round(z); }

  //  Negate  ////////////////////////////////////
  xm_inline Vect3f operator- () const;
  xm_inline Vect3f& negate(const Vect3f& v);		    
  xm_inline Vect3f& negate(); 			    

  //  Logical operations  ////////////////////////////////
  xm_inline bool eq(const Vect3f &v, float delta = FLT_COMPARE_TOLERANCE) const;

  //  Addition and substruction  ////////////////////
  xm_inline Vect3f& add(const Vect3f& u, const Vect3f& v);  
  xm_inline Vect3f& add(const Vect3f& v);		   
  xm_inline Vect3f& sub(const Vect3f& u, const Vect3f& v);  
  xm_inline Vect3f& sub(const Vect3f& v);		    
  xm_inline Vect3f& operator+= (const Vect3f& v) { return add(v); }
  xm_inline Vect3f& operator-= (const Vect3f& v) { return sub(v); }
  xm_inline Vect3f operator+ (const Vect3f& v) const { Vect3f u; return u.add(*this,v); }
  xm_inline Vect3f operator- (const Vect3f& v) const { Vect3f u; return u.sub(*this,v); }

  // Component-wise multiplication and division  ////////////////
  xm_inline Vect3f& mult(const Vect3f& u, const Vect3f& v); 
  xm_inline Vect3f& mult(const Vect3f& v);		    
  xm_inline Vect3f& div(const Vect3f& u, const Vect3f& v); 
  xm_inline Vect3f& div(const Vect3f& v);		    
  xm_inline Vect3f& operator*= (const Vect3f& v) { return mult(v); }
  xm_inline Vect3f& operator/= (const Vect3f& v) { return div(v); }
  xm_inline Vect3f operator* (const Vect3f& v) const { Vect3f u; return u.mult(*this, v); }
  xm_inline Vect3f operator/ (const Vect3f& v) const { Vect3f u; return u.div(*this, v); }

  //  Cross product  //////////////////////
  xm_inline Vect3f& cross(const Vect3f& u, const Vect3f& v);// u x v  [!]
  xm_inline Vect3f& precross(const Vect3f& v); 	    // v x this  [!]
  xm_inline Vect3f& postcross(const Vect3f& v);	    // this x v  [!]
  xm_inline Vect3f& operator%= (const Vect3f& v) { return postcross(v); } // this x v  [!]
  xm_inline Vect3f operator% (const Vect3f& v) const { Vect3f u; return u.cross(*this, v); }

  //  Dot product  //////////////////////
  xm_inline float dot(const Vect3f& other) const;
  xm_inline friend float dot(const Vect3f& u, const Vect3f& v) { return u.dot(v); }

  // Angle between two vectors //////////
  float angle(const Vect3f& other) const;
  
  // Multiplication & division by scalar ///////////
  xm_inline Vect3f& scale(const Vect3f& v, float s);	   
  xm_inline Vect3f& scale(float s);			   

  xm_inline Vect3f& operator*= (float s) { return scale(s); }
  xm_inline Vect3f& operator/= (float s) { return scale(1/s); }
  xm_inline Vect3f operator* (float s) const { Vect3f u; return u.scale(*this, s); }
  xm_inline Vect3f operator/ (float s) const { Vect3f u; return u.scale(*this, 1/s); }
  xm_inline friend Vect3f operator* (float s,const Vect3f& v) { Vect3f u; return u.scale(v, s); }

  //  Normalize  ///////////////////////////
  xm_inline Vect3f& normalize(float r = 1.0f);
  xm_inline Vect3f& normalize(const Vect3f& v, float r = 1.0f);	   

  //  Operation returning scalar  ////////////
  xm_inline float norm()  const;
  xm_inline float norm2() const;  // norm^2
  xm_inline float distance(const Vect3f& other) const;
  xm_inline float distance2(const Vect3f& other) const;  // distance^2

  xm_inline float psi() const;
  xm_inline float theta() const;

  xm_inline float min() const;
  xm_inline float max() const;
  xm_inline float minAbs() const;
  xm_inline float maxAbs() const;
  xm_inline float sumAbs() const;  // |x| + |y| + |z|


  //  Composite functions  ////////////////////////////////
  xm_inline Vect3f& crossAdd(const Vect3f& u, const Vect3f& v, const Vect3f& w); // u x v + w [!]  this must be distinct from u and v, but not necessarily from w.
  xm_inline Vect3f& crossAdd(const Vect3f& u, const Vect3f& v); // u x v + this [!]
  xm_inline Vect3f& scaleAdd(const Vect3f& v, const Vect3f& u, float lambda); // v + lambda * u
  xm_inline Vect3f& scaleAdd(const Vect3f& u, float lambda);// this + lambda * u
  xm_inline Vect3f& interpolate(const Vect3f& u, const Vect3f& v, float lambda); // (1-lambda)*u + lambda*v

  //	I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
  friend ostream& operator<< (ostream& os, const Vect3f& v);
  friend istream& operator>> (istream& is, Vect3f& v);
#endif

  friend XStream& operator<= (XStream& s,const Vect3f& v);
  friend XStream& operator>= (XStream& s,Vect3f& v);
  friend XStream& operator< (XStream& s,const Vect3f& v);
  friend XStream& operator> (XStream& s,Vect3f& v);

  friend XBuffer& operator<= (XBuffer& b,const Vect3f& v);
  friend XBuffer& operator>= (XBuffer& b,Vect3f& v);
  friend XBuffer& operator< (XBuffer& b,const Vect3f& v);
  friend XBuffer& operator> (XBuffer& b,Vect3f& v);

  void serialize(Archive& ar);

  //  Swap  /////////////////////////
  xm_inline void swap(Vect3f& other);
  xm_inline friend void swap(Vect3f& u, Vect3f& v) { u.swap(v);}


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
//			class Vect3d
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Vect3d 
{

public:

  double x, y, z;

  // constructors //////////////////////////////////////////////////////////////

  xm_inline Vect3d() {}
  xm_inline Vect3d(double x_, double y_, double z_) { x = x_; y = y_; z = z_; }
  
  typedef float double3[3];
  xm_inline Vect3d(const double3& v) {x = v[0]; y = v[1]; z = v[2];}

  xm_inline operator Vect3f () const;

  xm_inline operator const double* () const { return &x; }
  xm_inline operator double* () { return &x; }

  // setters / accessors / translators /////////////////////////////////////////

  xm_inline Vect3d& set(double x_, double y_, double z_) { x = x_; y = y_; z = z_; return *this; }
  xm_inline Vect3d& set(const double v[3]) {x = v[0]; y = v[1]; z = v[2]; return *this; }

  xm_inline Vect3d& setSpherical(double psi,double theta,double radius);

  // index-based access:  0=x, 1=y, 2=z.
  xm_inline const double& operator[](int i) const {return *(&x + i);}
  xm_inline double& operator[](int i)       {return *(&x + i);}

  // Convertion to int ///////
  xm_inline int xi() const { return round(x); }
  xm_inline int yi() const { return round(y); }
  xm_inline int zi() const { return round(z); }

  //  Negate  ////////////////////////////////////
  xm_inline Vect3d operator- () const;
  xm_inline Vect3d& negate(const Vect3d& v);		    
  xm_inline Vect3d& negate(); 			    

  //  Logical operations  ////////////////////////////////
  xm_inline bool eq(const Vect3d &v, double delta = DBL_COMPARE_TOLERANCE) const;

  //  Addition and substruction  ////////////////////
  xm_inline Vect3d& add(const Vect3d& u, const Vect3d& v);  
  xm_inline Vect3d& add(const Vect3d& v);		   
  xm_inline Vect3d& sub(const Vect3d& u, const Vect3d& v);  
  xm_inline Vect3d& sub(const Vect3d& v);		    
  xm_inline Vect3d& operator+= (const Vect3d& v) { return add(v); }
  xm_inline Vect3d& operator-= (const Vect3d& v) { return sub(v); }
  xm_inline Vect3d operator+ (const Vect3d& v) const { Vect3d u; return u.add(*this,v); }
  xm_inline Vect3d operator- (const Vect3d& v) const { Vect3d u; return u.sub(*this,v); }

  // Component-wise multiplication and division  ////////////////
  xm_inline Vect3d& mult(const Vect3d& u, const Vect3d& v); 
  xm_inline Vect3d& mult(const Vect3d& v);		    
  xm_inline Vect3d& div(const Vect3d& u, const Vect3d& v); 
  xm_inline Vect3d& div(const Vect3d& v);		    
  xm_inline Vect3d& operator*= (const Vect3d& v) { return mult(v); }
  xm_inline Vect3d& operator/= (const Vect3d& v) { return div(v); }
  xm_inline Vect3d operator* (const Vect3d& v) const { Vect3d u; return u.mult(*this, v); }
  xm_inline Vect3d operator/ (const Vect3d& v) const { Vect3d u; return u.div(*this, v); }

  //  Cross product  //////////////////////
  xm_inline Vect3d& cross(const Vect3d& u, const Vect3d& v);// u x v  [!]
  xm_inline Vect3d& precross(const Vect3d& v); 	    // v x this  [!]
  xm_inline Vect3d& postcross(const Vect3d& v);	    // this x v  [!]
  xm_inline Vect3d& operator%= (const Vect3d& v) { return postcross(v); } // this x v  [!]
  xm_inline Vect3d operator% (const Vect3d& v) const { Vect3d u; return u.cross(*this, v); }

  //  Dot product  //////////////////////
  xm_inline double dot(const Vect3d& other) const;
  xm_inline friend double dot(const Vect3d& u, const Vect3d& v) { return u.dot(v); }

  // Multiplication & division by scalar ///////////
  xm_inline Vect3d& scale(const Vect3d& v, double s);	   
  xm_inline Vect3d& scale(double s);			   
  xm_inline Vect3d& operator*= (double s) { return scale(s); }
  xm_inline Vect3d& operator/= (double s) { return scale(1/s); }
  xm_inline Vect3d operator* (double s) const { Vect3d u; return u.scale(*this, s); }
  xm_inline Vect3d operator/ (double s) const { Vect3d u; return u.scale(*this, 1/s); }
  xm_inline friend Vect3d operator* (double s,const Vect3d& v) { Vect3d u; return u.scale(v, s); }

  //  Normalize  ///////////////////////////
  xm_inline Vect3d& normalize(double r = 1.0);
  xm_inline Vect3d& normalize(const Vect3d& v, double r = 1.0);	   

  //  Operation returning scalar  ////////////
  xm_inline double norm()  const;
  xm_inline double norm2() const;  // norm^2
  xm_inline double distance(const Vect3d& other) const;
  xm_inline double distance2(const Vect3d& other) const;  // distance^2

  xm_inline double psi() const;
  xm_inline double theta() const;

  xm_inline double min() const;
  xm_inline double max() const;
  xm_inline double minAbs() const;
  xm_inline double maxAbs() const;
  xm_inline double sumAbs() const;  // |x| + |y| + |z|


  //  Composite functions  ////////////////////////////////
  xm_inline Vect3d& crossAdd(const Vect3d& u, const Vect3d& v, const Vect3d& w); // u x v + w [!]  this must be distinct from u and v, but not necessarily from w.
  xm_inline Vect3d& crossAdd(const Vect3d& u, const Vect3d& v); // u x v + this [!]
  xm_inline Vect3d& scaleAdd(const Vect3d& v, const Vect3d& u, double lambda); // v + lambda * u
  xm_inline Vect3d& scaleAdd(const Vect3d& u, double lambda);// this + lambda * u
  xm_inline Vect3d& interpolate(const Vect3d& u, const Vect3d& v, double lambda); // (1-lambda)*u + lambda*v

  //	I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
  friend ostream& operator<< (ostream& os, const Vect3d& v);
  friend istream& operator>> (istream& is, Vect3d& v);
#endif

  friend XStream& operator<= (XStream& s,const Vect3d& v);
  friend XStream& operator>= (XStream& s,Vect3d& v);
  friend XStream& operator< (XStream& s,const Vect3d& v);
  friend XStream& operator> (XStream& s,Vect3d& v);

  friend XBuffer& operator<= (XBuffer& b,const Vect3d& v);
  friend XBuffer& operator>= (XBuffer& b,Vect3d& v);
  friend XBuffer& operator< (XBuffer& b,const Vect3d& v);
  friend XBuffer& operator> (XBuffer& b,Vect3d& v);

  void serialize(Archive& ar);

  //  Swap  /////////////////////////
  xm_inline void swap(Vect3d& other);
  xm_inline friend void swap(Vect3d& u, Vect3d& v) { u.swap(v);}


  // Vect3d constants ///////////////////////////////////////////////////////////

  static const Vect3d ZERO;
  static const Vect3d ID;
  static const Vect3d I;     // unit vector along +x axis
  static const Vect3d J;     // unit vector along +y axis
  static const Vect3d K;     // unit vector along +z axis
  static const Vect3d I_;    // unit vector along -x axis
  static const Vect3d J_;    // unit vector along -y axis
  static const Vect3d K_;    // unit vector along -z axis

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Mat3f
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Mat3f 
{

  friend class MatXf;
  friend class QuatF;

public:

  // (stored in row-major order)
  float xx, xy, xz,
       yx, yy, yz,
       zx, zy, zz;

public:

  // constructors //////////////////////////////////////////////////////////////

  Mat3f() {}
  
  xm_inline Mat3f(float xx,float xy,float xz,
		float yx,float yy,float yz,
		float zx,float zy,float zz);

  Mat3f(float angle, eAxis axis) { set(angle, axis); }

  Mat3f(const Vect3f& diag, const Vect3f& sym) {set(diag, sym);}

  Mat3f(const Vect3f& axis, float angle, int normalizeAxis = 1)
    {set(axis, angle, normalizeAxis);}

  Mat3f(const QuatF& q) {set(q);}

  Mat3f(const Vect3f& x_from, const Vect3f& y_from, const Vect3f& z_from, 
	const Vect3f& x_to = Vect3f::I, const Vect3f& y_to = Vect3f::J, const Vect3f& z_to = Vect3f::K)
	  { set(x_from, y_from, z_from, x_to, y_to, z_to); }

  xm_inline operator Mat3d () const;


  // setters / accessors ///////////////////////////////////////////////////////
  
  // Rotation around 'axis'  by radians-angle
  xm_inline Mat3f& set(float angle, eAxis axis); 

  // make a symmetric matrix, given the diagonal and symmetric
  // (off-diagonal) elements in canonical order
  xm_inline Mat3f& set(const Vect3f& diag, const Vect3f& sym);
  
  // set Mat3f as a rotation of 'angle' radians about 'axis'
  // axis is automatically normalized unless normalizeAxis = 0
  Mat3f& set(const Vect3f& axis, float angle, int normalizeAxis = 1);
  
  Mat3f& set(const QuatF& q);

  // Convertion "from"-basis -> "to"-basis
  Mat3f& set(const Vect3f& x_from, const Vect3f& y_from, const Vect3f& z_from, 
	const Vect3f& x_to = Vect3f::I, const Vect3f& y_to = Vect3f::J, const Vect3f& z_to = Vect3f::K);

  // index-based access:  0=xrow, 1=yrow, 2=zrow.
  xm_inline const Vect3f& operator[](int i) const {return *(((Vect3f *) &xx) + i);}
  xm_inline Vect3f& operator[](int i)       {return *(((Vect3f *) &xx) + i);}

  // set matrix to the skew symmetric matrix corresponding to 'v X'
  xm_inline Mat3f& setSkew(const Vect3f& v);

  // for reading rows
  xm_inline const Vect3f& xrow() const {return *((Vect3f *) &xx);}
  xm_inline const Vect3f& yrow() const {return *((Vect3f *) &yx);}
  xm_inline const Vect3f& zrow() const {return *((Vect3f *) &zx);}
  // for writing to rows
  xm_inline Vect3f& xrow()  {return *((Vect3f *) &xx);}
  xm_inline Vect3f& yrow()  {return *((Vect3f *) &yx);}
  xm_inline Vect3f& zrow()  {return *((Vect3f *) &zx);}

  // for reading columns
  xm_inline const Vect3f xcol() const {return Vect3f(xx, yx, zx);}
  xm_inline const Vect3f ycol() const {return Vect3f(xy, yy, zy);}
  xm_inline const Vect3f zcol() const {return Vect3f(xz, yz, zz);}
  xm_inline const Vect3f col(int axis) const { return axis == X_AXIS ? Vect3f(xx, yx, zx) : (axis == Y_AXIS ? Vect3f(xy, yy, zy) : Vect3f(xz, yz, zz) ); }
  // for writing to columns
  xm_inline Mat3f& setXcol(const Vect3f& v);
  xm_inline Mat3f& setYcol(const Vect3f& v);
  xm_inline Mat3f& setZcol(const Vect3f& v);
  xm_inline Mat3f& setCol(int axis, const Vect3f& v) { if(axis == X_AXIS) setXcol(v); else if(axis == Y_AXIS) setYcol(v); else setZcol(v); return *this; }


  // for reading a symmetric matrix
  xm_inline Vect3f diag() const {return Vect3f(xx, yy, zz);}
  xm_inline Vect3f sym()  const {return Vect3f(yz, zx, xy);}


  //	Access to elements for Mapple-like notation (numerating from 1):
  //	1,1   1,2    1,3
  //	2,1   2,2    2,3
  //	3,1   3,2    3,3
  xm_inline const float& operator ()(int i,int j) const { return (&xx)[(i - 1)*3 + j - 1]; }
  xm_inline float& operator ()(int i,int j){ return (&xx)[(i - 1)*3 + j - 1]; }

  //  Determinant  of matrix  /////////
  xm_inline float det() const;

  //  Negate  ///////////////////
  xm_inline Mat3f operator- () const;
  xm_inline Mat3f& negate(); 			   // -this
  xm_inline Mat3f& negate(const Mat3f& M);     // -M

  //  Addition & substruction  /////////////////////////
  xm_inline Mat3f& add(const Mat3f& M, const Mat3f& N);      // M + N
  xm_inline Mat3f& add(const Mat3f& M);		      // this + M
  xm_inline Mat3f& sub(const Mat3f& M, const Mat3f& N);      // M - N
  xm_inline Mat3f& sub(const Mat3f& M);		      // this - M
  xm_inline Mat3f& operator+= (const Mat3f& M) { return add(M); }
  xm_inline Mat3f& operator-= (const Mat3f& M) { return sub(M); }
  xm_inline Mat3f operator+ (const Mat3f& M) const { Mat3f N; return N.add(*this,M); }
  xm_inline Mat3f operator- (const Mat3f& M) const { Mat3f N; return N.sub(*this,M); }

  //  Mat3f - Mat3f multiplication  ///////////
  Mat3f& mult(const Mat3f& M, const Mat3f& N);     // M * N	   [!]
  Mat3f& premult(const Mat3f& M);		      // M * this  [!]
  Mat3f& postmult(const Mat3f& M);		      // this * M  [!]
  xm_inline Mat3f& operator*= (const Mat3f& M) { return postmult(M); }
  xm_inline Mat3f operator* (const Mat3f& M) const { Mat3f N; return N.mult(*this, M); }

  //  Scalar multiplication  /////////////
  xm_inline Mat3f& scale(const Mat3f& M, float s);	      // s * M
  xm_inline Mat3f& scale(const Vect3f &s);			      // s * this
  xm_inline Mat3f& scale(float s);			      // s * this
  xm_inline Mat3f& operator*= (float s) { return scale(s); }
  xm_inline Mat3f& operator/= (float s) { return scale(1/s); }
  xm_inline Mat3f operator* (float s) const { Mat3f N; return N.scale(*this, s); }
  xm_inline Mat3f operator/ (float s) const { Mat3f N; return N.scale(*this, 1/s); }
  xm_inline friend Mat3f operator* (float s,const Mat3f& M) { Mat3f N; return N.scale(M, s); }

  //  Multiplication  by Vect3f as diagonal matrix  /////////////
  xm_inline Mat3f& preScale(const Mat3f& M, const Vect3f& v);	      // Mat3f(v) * M
  xm_inline Mat3f& preScale(const Vect3f& v);			      // Mat3f(v) * this
  xm_inline Mat3f& postScale(const Mat3f& M, const Vect3f& v);	      // M * Mat3f(v)
  xm_inline Mat3f& postScale(const Vect3f& v);			      // this * Mat3f(v)

  //  Transposition  ////////////////
  xm_inline Mat3f& xpose();				     // this^T
  xm_inline Mat3f& xpose(const Mat3f& M);     // M^T	   [!]
  xm_inline friend Mat3f xpose(const Mat3f& M) { Mat3f N; return N.xpose(M); }

  //  Invertion  ////////////////////
  int  invert();  // this^-1, returns one if the matrix was not invertible, otherwise zero.
  int  invert(const Mat3f& M);    // M^-1	   [!]
  xm_inline friend Mat3f invert(const Mat3f& M) { Mat3f N; N.invert(M); return N; }

  //  Simmetrize  /////////////
  xm_inline Mat3f& symmetrize(const Mat3f& M);	      // M + M^T
  xm_inline Mat3f& symmetrize();			      // this + this^T

  void makeRotationZ();	

  // Transforming Vect3d ///////////////////////////////////////////////////////
  // return reference to converted vector	
  xm_inline Vect3d& xform(const Vect3d& v, Vect3d& xv) const; // (this)(v) => xv [!]
  xm_inline Vect3d& xform(Vect3d& v) const;		      // (this)(v) => v

  // These are exactly like the above methods, except the inverse
  // transform this^-1 (= this^T) is used.  This can be thought of as
  // a row vector transformation, e.g.: (v^T)(this) => xv^T
  xm_inline Vect3d& invXform(const Vect3d& v, Vect3d& xv) const;  // [!]
  xm_inline Vect3d& invXform(Vect3d& v) const;

  //  Transforming operators  ///////////////
  xm_inline friend Vect3d& operator*= (Vect3d& v, const Mat3f& M) { return M.xform(v); }
  xm_inline friend Vect3d operator* (const Vect3d& v, const Mat3f& M) { Vect3d xv; return M.xform(v, xv); }
  xm_inline friend Vect3d operator* (const Mat3f& M, const Vect3d& v) { Vect3d xv; return M.xform(v, xv); }
				     

  // Transforming Vect3f ///////////////////////////////////////////////////////
  
  // return reference to converted vector	
  xm_inline Vect3f& xform(const Vect3f& v, Vect3f& xv) const; // (this)(v) => xv [!]
  xm_inline Vect3f& xform(Vect3f& v) const;		      // (this)(v) => v

  // These are exactly like the above methods, except the inverse
  // transform this^-1 (= this^T) is used.  This can be thought of as
  // a row vector transformation, e.g.: (v^T)(this) => xv^T
  xm_inline Vect3f& invXform(const Vect3f& v, Vect3f& xv) const;  // [!]
  xm_inline Vect3f& invXform(Vect3f& v) const;

  //  Transforming operators  ///////////////
  xm_inline friend Vect3f& operator*= (Vect3f& v, const Mat3f& M) { return M.xform(v); }
  xm_inline friend Vect3f operator* (const Vect3f& v, const Mat3f& M) { Vect3f xv; return M.xform(v, xv); }
  xm_inline friend Vect3f operator* (const Mat3f& M, const Vect3f& v) { Vect3f xv; return M.xform(v, xv); }



  //	I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
  friend ostream& operator<< (ostream& os, const Mat3f& M);
  friend istream& operator>> (istream& is, Mat3f& M);
#endif

  friend XStream& operator<= (XStream& s,const Mat3f& M);
  friend XStream& operator>= (XStream& s,Mat3f& M);
  friend XStream& operator< (XStream& s,const Mat3f& M);
  friend XStream& operator> (XStream& s,Mat3f& M);

  friend XBuffer& operator<= (XBuffer& b,const Mat3f& M);
  friend XBuffer& operator>= (XBuffer& b,Mat3f& M);
  friend XBuffer& operator< (XBuffer& b,const Mat3f& M);
  friend XBuffer& operator> (XBuffer& b,Mat3f& M);

  void serialize(Archive& ar);

  // Mat3f constants ////////////////////////////////////////////////////////////

  static const Mat3f ZERO;    // zero matrix
  static const Mat3f ID;      // identity matrix

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Mat3d
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Mat3d 
{

  friend class QuatD;
  friend class MatXd;

private:

  // (stored in row-major order)
  double xx, xy, xz,
       yx, yy, yz,
       zx, zy, zz;

public:

  // constructors //////////////////////////////////////////////////////////////

  Mat3d() {}
  
  xm_inline Mat3d(double xx,double xy,double xz,
		double yx,double yy,double yz,
		double zx,double zy,double zz);

  Mat3d(double angle, eAxis axis) { set(angle, axis); }

  Mat3d(const Vect3d& diag, const Vect3d& sym) {set(diag, sym);}

  Mat3d(const Vect3d& axis, double angle, int normalizeAxis = 1)
    {set(axis, angle, normalizeAxis);}

  Mat3d(const QuatD& q) {set(q);}

  Mat3d(const Vect3d& x_from, const Vect3d& y_from, const Vect3d& z_from, 
	const Vect3d& x_to = Vect3d::I, const Vect3d& y_to = Vect3d::J, const Vect3d& z_to = Vect3d::K)
	  { set(x_from, y_from, z_from, x_to, y_to, z_to); }

  xm_inline operator Mat3f () const;

  // setters / accessors ///////////////////////////////////////////////////////
  
  // Rotation around 'axis'  by radians-angle
  xm_inline Mat3d& set(double angle, eAxis axis); 

  // make a symmetric matrix, given the diagonal and symmetric
  // (off-diagonal) elements in canonical order
  xm_inline Mat3d& set(const Vect3d& diag, const Vect3d& sym);
  
  // set Mat3d as a rotation of 'angle' radians about 'axis'
  // axis is automatically normalized unless normalizeAxis = 0
  Mat3d& set(const Vect3d& axis, double angle, int normalizeAxis = 1);
  
  Mat3d& set(const QuatD& q);

  // Convertion "from"-basis -> "to"-basis
  Mat3d& set(const Vect3d& x_from, const Vect3d& y_from, const Vect3d& z_from, 
	const Vect3d& x_to = Vect3d::I, const Vect3d& y_to = Vect3d::J, const Vect3d& z_to = Vect3d::K);

  // index-based access:  0=xrow, 1=yrow, 2=zrow.
  const Vect3d& operator[](int i) const {return *(((Vect3d *) &xx) + i);}
	Vect3d& operator[](int i)       {return *(((Vect3d *) &xx) + i);}

  // set matrix to the skew symmetric matrix corresponding to 'v X'
  xm_inline Mat3d& setSkew(const Vect3d& v);

  // for reading rows
  const Vect3d& xrow() const {return *((Vect3d *) &xx);}
  const Vect3d& yrow() const {return *((Vect3d *) &yx);}
  const Vect3d& zrow() const {return *((Vect3d *) &zx);}
  // for writing to rows
  Vect3d& xrow()  {return *((Vect3d *) &xx);}
  Vect3d& yrow()  {return *((Vect3d *) &yx);}
  Vect3d& zrow()  {return *((Vect3d *) &zx);}

  // for reading columns
  Vect3d xcol() const {return Vect3d(xx, yx, zx);}
  Vect3d ycol() const {return Vect3d(xy, yy, zy);}
  Vect3d zcol() const {return Vect3d(xz, yz, zz);}
  Vect3d col(int axis) const { return axis == X_AXIS ? Vect3d(xx, yx, zx) : (axis == Y_AXIS ? Vect3d(xy, yy, zy) : Vect3d(xz, yz, zz) ); }
  // for writing to columns
  xm_inline Mat3d& setXcol(const Vect3d& v);
  xm_inline Mat3d& setYcol(const Vect3d& v);
  xm_inline Mat3d& setZcol(const Vect3d& v);
  Mat3d& setCol(int axis, const Vect3d& v) { if(axis == X_AXIS) setXcol(v); else if(axis == Y_AXIS) setYcol(v); else setZcol(v); return *this; }

  // for reading a symmetric matrix
  Vect3d diag() const {return Vect3d(xx, yy, zz);}
  Vect3d sym()  const {return Vect3d(yz, zx, xy);}


  //	Access to elements for Mapple-like notation (numerating from 1):
  //	1,1   1,2    1,3
  //	2,1   2,2    2,3
  //	3,1   3,2    3,3
  const double& operator ()(int i,int j) const { return (&xx)[(i - 1)*3 + j - 1]; }
  double& operator ()(int i,int j){ return (&xx)[(i - 1)*3 + j - 1]; }

  //  Determinant  of matrix  /////////
  xm_inline double det() const;

  //  Negate  ///////////////////
  xm_inline Mat3d operator- () const;
  xm_inline Mat3d& negate(); 			   // -this
  xm_inline Mat3d& negate(const Mat3d& M);     // -M

  //  Addition & substruction  /////////////////////////
  xm_inline Mat3d& add(const Mat3d& M, const Mat3d& N);      // M + N
  xm_inline Mat3d& add(const Mat3d& M);		      // this + M
  xm_inline Mat3d& sub(const Mat3d& M, const Mat3d& N);      // M - N
  xm_inline Mat3d& sub(const Mat3d& M);		      // this - M
  Mat3d& operator+= (const Mat3d& M) { return add(M); }
  Mat3d& operator-= (const Mat3d& M) { return sub(M); }
  Mat3d operator+ (const Mat3d& M) const { Mat3d N; return N.add(*this,M); }
  Mat3d operator- (const Mat3d& M) const { Mat3d N; return N.sub(*this,M); }

  //  Mat3d - Mat3d multiplication  ///////////
  Mat3d& mult(const Mat3d& M, const Mat3d& N);     // M * N	   [!]
  Mat3d& premult(const Mat3d& M);		      // M * this  [!]
  Mat3d& postmult(const Mat3d& M);		      // this * M  [!]
  Mat3d& operator*= (const Mat3d& M) { return postmult(M); }
  Mat3d operator* (const Mat3d& M) const { Mat3d N; return N.mult(*this, M); }

  //  Scalar multiplication  /////////////
  xm_inline Mat3d& scale(const Mat3d& M, double s);	      // s * M
  xm_inline Mat3d& scale(double s);			      // s * this
  Mat3d& operator*= (double s) { return scale(s); }
  Mat3d& operator/= (double s) { return scale(1/s); }
  Mat3d operator* (double s) const { Mat3d N; return N.scale(*this, s); }
  Mat3d operator/ (double s) const { Mat3d N; return N.scale(*this, 1/s); }
  friend Mat3d operator* (double s,const Mat3d& M) { Mat3d N; return N.scale(M, s); }

  //  Multiplication  by Vect3d as diagonal matrix  /////////////
  xm_inline Mat3d& preScale(const Mat3d& M, const Vect3d& v);	      // Mat3d(v) * M
  xm_inline Mat3d& preScale(const Vect3d& v);			      // Mat3d(v) * this
  xm_inline Mat3d& postScale(const Mat3d& M, const Vect3d& v);	      // M * Mat3d(v)
  xm_inline Mat3d& postScale(const Vect3d& v);			      // this * Mat3d(v)

  //  Transposition  ////////////////
  xm_inline Mat3d& xpose();				     // this^T
  xm_inline Mat3d& xpose(const Mat3d& M);     // M^T	   [!]
  friend Mat3d xpose(const Mat3d& M) { Mat3d N; return N.xpose(M); }

  //  Invertion  ////////////////////
  int  invert();  // this^-1, returns one if the matrix was not invertible, otherwise zero.
  int  invert(const Mat3d& M);    // M^-1	   [!]
  friend Mat3d invert(const Mat3d& M) { Mat3d N; N.invert(M); return N; }

  //  Simmetrize  /////////////
  xm_inline Mat3d& symmetrize(const Mat3d& M);	      // M + M^T
  xm_inline Mat3d& symmetrize();			      // this + this^T



  // Transforming Vect3d ///////////////////////////////////////////////////////
  
  // return reference to converted vector	
  xm_inline Vect3d& xform(const Vect3d& v, Vect3d& xv) const; // (this)(v) => xv [!]
  xm_inline Vect3d& xform(Vect3d& v) const;		      // (this)(v) => v

  // These are exactly like the above methods, except the inverse
  // transform this^-1 (= this^T) is used.  This can be thought of as
  // a row vector transformation, e.g.: (v^T)(this) => xv^T
  xm_inline Vect3d& invXform(const Vect3d& v, Vect3d& xv) const;  // [!]
  xm_inline Vect3d& invXform(Vect3d& v) const;

  //  Transforming operators  ///////////////
  friend Vect3d& operator*= (Vect3d& v, const Mat3d& M) { return M.xform(v); }
  friend Vect3d operator* (const Vect3d& v, const Mat3d& M) { Vect3d xv; return M.xform(v, xv); }
  friend Vect3d operator* (const Mat3d& M, const Vect3d& v) { Vect3d xv; return M.xform(v, xv); }

				     

  // Transforming Vect3f ///////////////////////////////////////////////////////
  // return reference to converted vector	
  xm_inline Vect3f& xform(const Vect3f& v, Vect3f& xv) const; // (this)(v) => xv [!]
  xm_inline Vect3f& xform(Vect3f& v) const;		      // (this)(v) => v

  // These are exactly like the above methods, except the inverse
  // transform this^-1 (= this^T) is used.  This can be thought of as
  // a row vector transformation, e.g.: (v^T)(this) => xv^T
  xm_inline Vect3f& invXform(const Vect3f& v, Vect3f& xv) const;  // [!]
  xm_inline Vect3f& invXform(Vect3f& v) const;

  //  Transforming operators  ///////////////
  friend Vect3f& operator*= (Vect3f& v, const Mat3d& M) { return M.xform(v); }
  friend Vect3f operator* (const Vect3f& v, const Mat3d& M) { Vect3f xv; return M.xform(v, xv); }
  friend Vect3f operator* (const Mat3d& M, const Vect3f& v) { Vect3f xv; return M.xform(v, xv); }


  //	I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
  friend ostream& operator<< (ostream& os, const Mat3d& M);
  friend istream& operator>> (istream& is, Mat3d& M);
#endif

  friend XStream& operator<= (XStream& s,const Mat3d& M);
  friend XStream& operator>= (XStream& s,Mat3d& M);
  friend XStream& operator< (XStream& s,const Mat3d& M);
  friend XStream& operator> (XStream& s,Mat3d& M);

  friend XBuffer& operator<= (XBuffer& b,const Mat3d& M);
  friend XBuffer& operator>= (XBuffer& b,Mat3d& M);
  friend XBuffer& operator< (XBuffer& b,const Mat3d& M);
  friend XBuffer& operator> (XBuffer& b,Mat3d& M);

  void serialize(Archive& ar);
 
  // Mat3d constants ////////////////////////////////////////////////////////////

  static const Mat3d ZERO;    // zero matrix
  static const Mat3d ID;      // identity matrix

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class MatXf
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MatXf 
{
public:

  Mat3f R;
  Vect3f d;

public:

  // constructors //////////////////////////////////////////////////////////////

  xm_inline MatXf()				{}
  xm_inline MatXf(const Mat3f& R_, const Vect3f& d_) {set(R_, d_);}
  explicit xm_inline MatXf(const Se3f& T)			{set(T);}

  typedef float float16[16];
  xm_inline MatXf(const float16& T);

  xm_inline operator MatXd () const;

  // setters / accessors / translators /////////////////////////////////////////

  xm_inline MatXf& set(const Mat3f& R_, const Vect3f& d_) {R = R_; d = d_; return *this; }
  xm_inline MatXf& set(const Se3f& T);

  xm_inline const Mat3f&  rot()   const {return R;}
  xm_inline const Vect3f& trans() const {return d;}
  xm_inline Mat3f&  rot()	     {return R;}
  xm_inline Vect3f& trans()	     {return d;}


  //  MatXf - MatXf multiplication  ////////////
  MatXf& mult(const MatXf& M, const MatXf& N);    // M * N	   [!]
  MatXf& premult(const MatXf& M);		      // M * this  [!]
  MatXf& postmult(const MatXf& M); 	      // this * M  [!]
  xm_inline MatXf& operator*= (const MatXf& M) { return postmult(M); }
  xm_inline MatXf operator* (const MatXf& N) const { MatXf M; return M.mult(*this, N); }


  // Doesn't really invert the 3x3-matrix, but only transpose one.
  // That is, works for non-scaled matrix !!!
  MatXf& invert(const MatXf& M);		      // M^-1	   [!]
  MatXf& invert();			      // this^-1

  // Really inverts 3x3-matrix.
  MatXf& Invert(const MatXf& M);		      // M^-1	   [!]
  MatXf& Invert();			      // this^-1

  // Transforming Vect3d ///////////////////////////////////////////////////////

  // MatXs can transform elements of R^3 either as vectors or as
  // points.  The [!] indicates that the operands must be distinct.
  xm_inline Vect3d& xformVect(const Vect3d& v, Vect3d& xv) const; // this*(v 0)=>xv  [!]
  xm_inline Vect3d& xformVect(Vect3d& v) const;		  // this*(v 0)=>v
  xm_inline Vect3d& xformPoint(const Vect3d& p, Vect3d& xp) const;// this*(p 1)=>xp  [!]
  xm_inline Vect3d& xformPoint(Vect3d& p) const;		  // this*(p 1)=>p

  // These are exactly like the above methods, except the inverse
  // transform this^-1 is used.
  xm_inline Vect3d& invXformVect(const Vect3d& v, Vect3d& xv) const;
  xm_inline Vect3d& invXformVect(Vect3d& v) const;
  xm_inline Vect3d& invXformPoint(const Vect3d& p, Vect3d& xp) const;
  xm_inline Vect3d& invXformPoint(Vect3d& p) const;


  // Transforming Vect3f ///////////////////////////////////////////////////////

  // MatXs can transform elements of R^3 either as vectors or as
  // points.  The [!] indicates that the operands must be distinct.

  xm_inline Vect3f& xformVect(const Vect3f& v, Vect3f& xv) const; // this*(v 0)=>xv  [!]
  xm_inline Vect3f& xformVect(Vect3f& v) const;		  // this*(v 0)=>v
  xm_inline Vect3f& xformPoint(const Vect3f& p, Vect3f& xp) const;// this*(p 1)=>xp  [!]
  xm_inline Vect3f& xformPoint(Vect3f& p) const;		  // this*(p 1)=>p

  xm_inline Vect3f operator*(const Vect3f& p) const { Vect3f xp; xformPoint(p, xp); return xp; }

  // These are exactly like the above methods, except the inverse
  // transform this^-1 is used.
  xm_inline Vect3f& invXformVect(const Vect3f& v, Vect3f& xv) const;
  xm_inline Vect3f& invXformVect(Vect3f& v) const;
  xm_inline Vect3f& invXformPoint(const Vect3f& p, Vect3f& xp) const;
  xm_inline Vect3f& invXformPoint(Vect3f& p) const;



  //	I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
  friend ostream& operator<<(ostream& os, const MatXf& M);
  friend istream& operator>>(istream& is, MatXf& M);
#endif

  void serialize(Archive& ar);

  // MatXf constants ////////////////////////////////////////////////////////////

  static const MatXf ID;      // identity matrix

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class MatXd
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MatXd 
{
private:

  Mat3d R;
  Vect3d d;

public:

  // constructors //////////////////////////////////////////////////////////////

  xm_inline MatXd()				{}
  xm_inline MatXd(const Mat3d& R_, const Vect3d& d_) {set(R_, d_);}
  xm_inline MatXd(const Se3d& T)			{set(T);}
  
  typedef double double16[16];
  xm_inline MatXd(const double16& T);

  xm_inline operator MatXf () const;

  // setters / accessors / translators /////////////////////////////////////////

  xm_inline MatXd& set(const Mat3d& R_, const Vect3d& d_) {R = R_; d = d_; return *this; }
  xm_inline MatXd& set(const Se3d& T);

  xm_inline const Mat3d&  rot()   const {return R;}
  xm_inline const Vect3d& trans() const {return d;}
  xm_inline Mat3d&  rot()	     {return R;}
  xm_inline Vect3d& trans()	     {return d;}


  //  MatXd - MatXd multiplication  ////////////
  MatXd& mult(const MatXd& M, const MatXd& N);    // M * N	   [!]
  MatXd& premult(const MatXd& M);		      // M * this  [!]
  MatXd& postmult(const MatXd& M); 	      // this * M  [!]
  xm_inline MatXd& operator*= (const MatXd& M) { return postmult(M); }
  xm_inline MatXd operator* (const MatXd& N) const { MatXd M; return M.mult(*this, N); }


  // Doesn't really invert the 3x3-matrix, but only transpose one.
  // That is, works for non-scaled matrix !!!
  MatXd& invert(const MatXd& M);		      // M^-1	   [!]
  MatXd& invert();			      // this^-1

  // Really inverts 3x3-matrix.
  MatXd& Invert(const MatXd& M);		      // M^-1	   [!]
  MatXd& Invert();			      // this^-1


  // Transforming Vect3d ///////////////////////////////////////////////////////

  // MatXs can transform elements of R^3 either as vectors or as
  // points.  The [!] indicates that the operands must be distinct.

  xm_inline Vect3d& xformVect(const Vect3d& v, Vect3d& xv) const; // this*(v 0)=>xv  [!]
  xm_inline Vect3d& xformVect(Vect3d& v) const;		  // this*(v 0)=>v
  xm_inline Vect3d& xformPoint(const Vect3d& p, Vect3d& xp) const;// this*(p 1)=>xp  [!]
  xm_inline Vect3d& xformPoint(Vect3d& p) const;		  // this*(p 1)=>p

  // These are exactly like the above methods, except the inverse
  // transform this^-1 is used.
  xm_inline Vect3d& invXformVect(const Vect3d& v, Vect3d& xv) const;
  xm_inline Vect3d& invXformVect(Vect3d& v) const;
  xm_inline Vect3d& invXformPoint(const Vect3d& p, Vect3d& xp) const;
  xm_inline Vect3d& invXformPoint(Vect3d& p) const;



  // Transforming Vect3f ///////////////////////////////////////////////////////

  // MatXs can transform elements of R^3 either as vectors or as
  // points.  The [!] indicates that the operands must be distinct.
  xm_inline Vect3f& xformVect(const Vect3f& v, Vect3f& xv) const; // this*(v 0)=>xv  [!]
  xm_inline Vect3f& xformVect(Vect3f& v) const;		  // this*(v 0)=>v
  xm_inline Vect3f& xformPoint(const Vect3f& p, Vect3f& xp) const;// this*(p 1)=>xp  [!]
  xm_inline Vect3f& xformPoint(Vect3f& p) const;		  // this*(p 1)=>p

  // These are exactly like the above methods, except the inverse
  // transform this^-1 is used.
  xm_inline Vect3f& invXformVect(const Vect3f& v, Vect3f& xv) const;
  xm_inline Vect3f& invXformVect(Vect3f& v) const;
  xm_inline Vect3f& invXformPoint(const Vect3f& p, Vect3f& xp) const;
  xm_inline Vect3f& invXformPoint(Vect3f& p) const;


  //	I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
  friend ostream& operator<<(ostream& os, const MatXd& M);
  friend istream& operator>>(istream& is, MatXd& M);
#endif

  void serialize(Archive& ar);

  // MatXd constants ////////////////////////////////////////////////////////////

  static const MatXd ID;      // identity matrix

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class QuatF
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class QuatF 
{
  friend class Mat3f;
  friend class Se3f;

public:

  float s_, x_, y_, z_;


public:

  // constructors //////////////////////////////////////////////////////////////

  xm_inline QuatF() {}

  xm_inline QuatF(float s, float x, float y, float z) {set(s, x, y, z);}

  xm_inline QuatF(float angle, const Vect3f& axis, int normalizeAxis = 1) {set(angle, axis, normalizeAxis);}

  xm_inline QuatF(const Mat3f& R) {set(R);}

  xm_inline operator QuatD () const;

  // setters / accessors / translators /////////////////////////////////////////

  xm_inline QuatF& set(float s, float x, float y, float z) { s_=s; x_=x; y_=y; z_=z; return *this; }

  // set a quaternion to a rotation of 'angle' radians about 'axis'
  // the axis passed is automatically normalized unless normalizeAxis = 0
  QuatF& set(float angle, const Vect3f& axis, int normalizeAxis = 1);

  QuatF& set(const Mat3f& R);

  xm_inline operator const float* () const { return &s_; }
  xm_inline operator float* () { return &s_; }

  xm_inline const float& operator[](int i) const {return *(&s_ + i);}
  xm_inline float& operator[](int i)       {return *(&s_ + i);}

  float s() const {return s_;}
  float x() const {return x_;}
  float y() const {return y_;}
  float z() const {return z_;}

  float& s() {return s_;}
  float& x() {return x_;}
  float& y() {return y_;}
  float& z() {return z_;}

  xm_inline Vect3f axis() const;  // normalized axis of rotation
  xm_inline float angle() const;  // angle of rotation in radians, in range [0, 2pi)


  //  Logical operations  ///////////////
  xm_inline bool eq(const QuatF &v, float delta = FLT_COMPARE_TOLERANCE) const;

  //  Negate  ////////////////////////////////////
  xm_inline QuatF operator- () const { return QuatF(-s_, -x_, -y_, -z_); }
  xm_inline QuatF& negate(const QuatF& q){ s_ = -q.s_; x_ = -q.x_; y_ = -q.y_; z_ = -q.z_; return *this; }
  xm_inline QuatF& negate(){ s_ = -s_; x_ = -x_; y_ = -y_; z_ = -z_; return *this; }

  //  Normalization  ///////////////////////
  xm_inline QuatF& normalize(const QuatF& q); 	      // q/|q|
  xm_inline QuatF& normalize();			      // this/|this|
  xm_inline float norm() const { return sqrtf(x_*x_ + y_*y_ + z_*z_ + s_*s_); }
  xm_inline float norm2() const { return x_*x_ + y_*y_ + z_*z_ + s_*s_; }

  //  Invertion  /////////////////////////
  xm_inline QuatF& invert(const QuatF& q);		      // q^-1
  xm_inline QuatF& invert(); 			      // this^-1


  //  QuatF - QuatF operations  ///////////////
  xm_inline QuatF& operator+= (const QuatF& q);
  xm_inline QuatF& operator-= (const QuatF& q);
  xm_inline QuatF operator+ (const QuatF& q) const;
  xm_inline QuatF operator- (const QuatF& q) const;

  //   Cross product   /////////////////
  QuatF& mult(const QuatF& p, const QuatF& q);	      // p * q	   [!]
  QuatF& premult(const QuatF& q);			      // q * this  [!]
  QuatF& postmult(const QuatF& q); 		      // this * q  [!]
  xm_inline QuatF& operator%= (const QuatF& q){ return postmult(q); }
  xm_inline QuatF operator% (const QuatF& q) const { QuatF u; return u.mult(*this, q); }

  //  Dot product  ////////////////////
  xm_inline float dot(const QuatF& other) const;
  xm_inline friend float dot(const QuatF& u, const QuatF& v) { return u.dot(v); }

  //  Scalar multiplication & division  //////////
  xm_inline QuatF& operator*= (float s);
  xm_inline QuatF& operator/= (float s);
  xm_inline QuatF operator* (float s) const;
  xm_inline QuatF operator/ (float s) const;
  xm_inline friend QuatF operator* (float s,const QuatF& q);


  // Transforming Vect3f ///////////////////////////////////////////////////////

  Vect3f& xform(const Vect3f& u, Vect3f& v) const;	 // this (v 0) this^-1 => xv
  Vect3f& xform(Vect3f& v) const;			 // this (v 0) this^-1 => v

  // These are exactly like the above methods, except the inverse
  // transform is used (i.e. the factors this and this^-1 are swapped).
  Vect3f& invXform(const Vect3f& v, Vect3f& xv) const;
  Vect3f& invXform(Vect3f& v) const;


  //	I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
  friend ostream& operator<< (ostream& os, const QuatF& q);
  friend istream& operator>> (istream& is, QuatF& q);
#endif

  friend XStream& operator<= (XStream& s,const QuatF& q);
  friend XStream& operator>= (XStream& s,QuatF& q);
  friend XStream& operator< (XStream& s,const QuatF& q);
  friend XStream& operator> (XStream& s,QuatF& q);

  friend XBuffer& operator<= (XBuffer& b,const QuatF& q);
  friend XBuffer& operator>= (XBuffer& b,QuatF& q);
  friend XBuffer& operator< (XBuffer& b,const QuatF& q);
  friend XBuffer& operator> (XBuffer& b,QuatF& q);

  void serialize(Archive& ar);

  // miscellaneous /////////////////////////////////////////////////////////////
  xm_inline void slerp(const QuatF &a,const QuatF &b,float t);

  xm_inline void slerpExact(const QuatF &a,const QuatF &b,float t);

  // QuatF constants ////////////////////////////////////////////////////////////

  static const QuatF ID;   // identity quaternion

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class QuatD
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class QuatD 
{
  friend class Mat3d;
  friend class Se3d;

private:

  double s_, x_, y_, z_;


public:

  // constructors //////////////////////////////////////////////////////////////

  QuatD() {}

  QuatD(double s, double x, double y, double z) {set(s, x, y, z);}

  QuatD(double angle, const Vect3d& axis, int normalizeAxis = 1)
    {set(angle, axis, normalizeAxis);}

  QuatD(const Mat3d& R) {set(R);}

  xm_inline operator QuatF () const;

  // setters / accessors / translators /////////////////////////////////////////

  QuatD& set(double s, double x, double y, double z) { s_=s; x_=x; y_=y; z_=z; return *this; }

  // set a quaternion to a rotation of 'angle' radians about 'axis'
  // the axis passed is automatically normalized unless normalizeAxis = 0
  QuatD& set(double angle, const Vect3d& axis, int normalizeAxis = 1);

  QuatD& set(const Mat3d& R);

  xm_inline operator const double* () const { return &s_; }
  xm_inline operator double* () { return &s_; }

  xm_inline const double& operator[](int i) const {return *(&s_ + i);}
  xm_inline double& operator[](int i)       {return *(&s_ + i);}

  double s() const {return s_;}
  double x() const {return x_;}
  double y() const {return y_;}
  double z() const {return z_;}

  double& s() {return s_;}
  double& x() {return x_;}
  double& y() {return y_;}
  double& z() {return z_;}

  xm_inline Vect3d axis() const;  // normalized axis of rotation
  xm_inline double angle() const;  // angle of rotation in radians, in range [0, 2pi)


  //  Logical operations  ///////////////
  xm_inline bool eq(const QuatD& other, double delta = DBL_COMPARE_TOLERANCE) const;

    //  Negate  ////////////////////////////////////
  xm_inline QuatD operator- () const { return QuatD(-s_, -x_, -y_, -z_); }
  xm_inline QuatD& negate(const QuatD& q){ s_ = -q.s_; x_ = -q.x_; y_ = -q.y_; z_ = -q.z_; return *this; }
  xm_inline QuatD& negate(){ s_ = -s_; x_ = -x_; y_ = -y_; z_ = -z_; return *this; }

  //  Normalization  ///////////////////////
  xm_inline QuatD& normalize(const QuatD& q); 	      // q/|q|
  xm_inline QuatD& normalize();			      // this/|this|
  xm_inline double norm() const { return sqrt(x_*x_ + y_*y_ + z_*z_ + s_*s_); }

  //  Invertion  /////////////////////////
  xm_inline QuatD& invert(const QuatD& q);		      // q^-1
  xm_inline QuatD& invert(); 			      // this^-1


  //  QuatD - QuatD operations  ///////////////
  xm_inline QuatD& operator+= (const QuatD& q);
  xm_inline QuatD& operator-= (const QuatD& q);
  xm_inline QuatD operator+ (const QuatD& q) const;
  xm_inline QuatD operator- (const QuatD& q) const;

  //   Cross product   /////////////////
  QuatD& mult(const QuatD& p, const QuatD& q);	      // p * q	   [!]
  QuatD& premult(const QuatD& q);			      // q * this  [!]
  QuatD& postmult(const QuatD& q); 		      // this * q  [!]
  xm_inline QuatD& operator%= (const QuatD& q){ return postmult(q); }
  xm_inline QuatD operator% (const QuatD& q) const { QuatD u; return u.mult(*this, q); }

  //  Dot product  ////////////////////
  xm_inline double dot(const QuatD& other) const;
  friend double dot(const QuatD& u, const QuatD& v) { return u.dot(v); }

  //  Scalar multiplication & division  //////////
  xm_inline QuatD& operator*= (double s);
  xm_inline QuatD& operator/= (double s);
  xm_inline QuatD operator* (double s) const;
  xm_inline QuatD operator/ (double s) const;
  xm_inline friend QuatD operator* (double s,const QuatD& q);



  // Transforming Vect3d ///////////////////////////////////////////////////////

  Vect3d& xform(const Vect3d& u, Vect3d& v) const;	 // this (v 0) this^-1 => xv
  Vect3d& xform(Vect3d& v) const;			 // this (v 0) this^-1 => v

  // These are exactly like the above methods, except the inverse
  // transform is used (i.e. the factors this and this^-1 are swapped).
  Vect3d& invXform(const Vect3d& v, Vect3d& xv) const;
  Vect3d& invXform(Vect3d& v) const;


  //	I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
  friend ostream& operator<< (ostream& os, const QuatD& q);
  friend istream& operator>> (istream& is, QuatD& q);
#endif

  friend XStream& operator<= (XStream& s,const QuatD& q);
  friend XStream& operator>= (XStream& s,QuatD& q);
  friend XStream& operator< (XStream& s,const QuatD& q);
  friend XStream& operator> (XStream& s,QuatD& q);

  friend XBuffer& operator<= (XBuffer& b,const QuatD& q);
  friend XBuffer& operator>= (XBuffer& b,QuatD& q);
  friend XBuffer& operator< (XBuffer& b,const QuatD& q);
  friend XBuffer& operator> (XBuffer& b,QuatD& q);

  void serialize(Archive& ar);

  // miscellaneous /////////////////////////////////////////////////////////////
  xm_inline void slerp(const QuatD &a,const QuatD &b, double t);

  // QuatD constants ////////////////////////////////////////////////////////////

  static const QuatD ID;   // identity quaternion
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Se3f
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class Se3f 
{
private:

  QuatF q;     // rotation component
  Vect3f d;    // translation component

public:

  // constructors //////////////////////////////////////////////////////////////


  Se3f() {}
  Se3f(const QuatF& q_, const Vect3f& d_) { set(q_, d_); }
  explicit Se3f(const MatXf& X) { set(X); }
  
  xm_inline operator Se3d () const;

  // setters / accessors / translators /////////////////////////////////////////

  Se3f& set(const QuatF& q_, const Vect3f& d_) { q = q_; d = d_; return *this; }
  Se3f& set(const MatXf& X) { q.set(X.rot()); d = X.trans(); return *this; }

  const QuatF&  rot()   const {return q;}
  const Vect3f& trans() const {return d;}
	QuatF&  rot()	     {return q;}
	Vect3f& trans()	     {return d;}

  
  //  Se3f - Se3f multiplication  ////////////
  xm_inline Se3f& mult(const Se3f& T, const Se3f& U);    // T * U	[!]
  xm_inline Se3f& premult(const Se3f& T);		   // T * this	[!]
  xm_inline Se3f& postmult(const Se3f& T);		   // this * T	[!]
  xm_inline Se3f& operator*= (const Se3f& T) { return postmult(T); }
  xm_inline Se3f operator* (const Se3f& U) const { Se3f T; return T.mult(*this, U); }

  xm_inline interpolate(const Se3f& u, const Se3f& v, float t) { q.slerp(u.q, v.q, t); d.interpolate(u.d, v.d, t); }

  xm_inline interpolateExact(const Se3f& u, const Se3f& v, float t) { q.slerpExact(u.q, v.q, t); d.interpolate(u.d, v.d, t); }

  //  Invertion  ///////////////////
  xm_inline Se3f& invert(const Se3f& T);		   // T^-1
  xm_inline Se3f& invert(); 			   // this^-1


  // Transforming Vect3d ///////////////////////////////////////////////////////

  // Se3s can transform elements of R^3 either as vectors or as
  // points.  Multiple operands need not be distinct.

  xm_inline Vect3f& xformVect(const Vect3f& v, Vect3f& xv) const;  // this * (v 0) => xv
  xm_inline Vect3f& xformVect(Vect3f& v) const;		   // this * (v 0) => v
  xm_inline Vect3f& xformPoint(const Vect3f& p, Vect3f& xp) const; // this * (p 1) => xp
  xm_inline Vect3f& xformPoint(Vect3f& p) const;		   // this * (p 1) => p

  // These are exactly like the above methods, except the inverse
  // transform this^-1 is used.
  xm_inline Vect3f& invXformVect(const Vect3f& v, Vect3f& xv) const;
  xm_inline Vect3f& invXformVect(Vect3f& v) const;
  xm_inline Vect3f& invXformPoint(const Vect3f& p, Vect3f& xp) const;
  xm_inline Vect3f& invXformPoint(Vect3f& p) const;

  xm_inline bool eq(const Se3f& other, float transDelta, float rotDelta) const;

  //	I/O operations    //////////////////////////////////////
  
#ifdef _XMATH_USE_IOSTREAM
  friend ostream& operator<<(ostream& os, const Se3f& se3);
  friend istream& operator>>(istream& is, Se3f& se3);
#endif

  friend XStream& operator<= (XStream& s,const Se3f& v);
  friend XStream& operator>= (XStream& s,Se3f& v);
  friend XStream& operator< (XStream& s,const Se3f& v);
  friend XStream& operator> (XStream& s,Se3f& v);
	
  friend XBuffer& operator<= (XBuffer& b,const Se3f& v);
  friend XBuffer& operator>= (XBuffer& b,Se3f& v);
  friend XBuffer& operator< (XBuffer& b,const Se3f& v);
  friend XBuffer& operator> (XBuffer& b,Se3f& v);

  void serialize(Archive& ar);

  // Se3f constants /////////////////////////////////////////////////////////////

  static const Se3f ID;	    // identity Se3f
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Se3d
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Se3d 
{
private:

  QuatD q;     // rotation component
  Vect3d d;    // translation component

public:

  // constructors //////////////////////////////////////////////////////////////


  Se3d() {}
  Se3d(const QuatD& q_, const Vect3d& d_) { set(q_, d_); }
  Se3d(const MatXd& X) { set(X); }

  xm_inline operator Se3f () const;

  // setters / accessors / translators /////////////////////////////////////////

  Se3d& set(const QuatD& q_, const Vect3d& d_) { q = q_; d = d_; return *this; }
  Se3d& set(const MatXd& X) { q.set(X.rot()); d = X.trans(); return *this; }

  const QuatD&  rot()   const {return q;}
  const Vect3d& trans() const {return d;}
	QuatD&  rot()	     {return q;}
	Vect3d& trans()	     {return d;}

  
  //  Se3d - Se3d multiplication  ////////////
  xm_inline Se3d& mult(const Se3d& T, const Se3d& U);    // T * U	[!]
  xm_inline Se3d& premult(const Se3d& T);		   // T * this	[!]
  xm_inline Se3d& postmult(const Se3d& T);		   // this * T	[!]
  Se3d& operator*= (const Se3d& T) { return postmult(T); }
  Se3d operator* (const Se3d& U) const { Se3d T; return T.mult(*this, U); }

  //  Invertion  ///////////////////
  xm_inline Se3d& invert(const Se3d& T);		   // T^-1
  xm_inline Se3d& invert(); 			   // this^-1


  // Transforming Vect3d ///////////////////////////////////////////////////////

  // Se3s can transform elements of R^3 either as vectors or as
  // points.  Multiple operands need not be distinct.

  xm_inline Vect3d& xformVect(const Vect3d& v, Vect3d& xv) const;  // this * (v 0) => xv
  xm_inline Vect3d& xformVect(Vect3d& v) const;		   // this * (v 0) => v
  xm_inline Vect3d& xformPoint(const Vect3d& p, Vect3d& xp) const; // this * (p 1) => xp
  xm_inline Vect3d& xformPoint(Vect3d& p) const;		   // this * (p 1) => p

  // These are exactly like the above methods, except the inverse
  // transform this^-1 is used.
  xm_inline Vect3d& invXformVect(const Vect3d& v, Vect3d& xv) const;
  xm_inline Vect3d& invXformVect(Vect3d& v) const;
  xm_inline Vect3d& invXformPoint(const Vect3d& p, Vect3d& xp) const;
  xm_inline Vect3d& invXformPoint(Vect3d& p) const;

  xm_inline bool eq(const Se3d& other, double transDelta, double rotDelta) const;

  //	I/O operations    //////////////////////////////////////
  
#ifdef _XMATH_USE_IOSTREAM
  friend ostream& operator<<(ostream& os, const Se3d& se3);
  friend istream& operator>>(istream& is, Se3d& se3);
#endif

  friend XStream& operator<= (XStream& s,const Se3d& v);
  friend XStream& operator>= (XStream& s,Se3d& v);
  friend XStream& operator< (XStream& s,const Se3d& v);
  friend XStream& operator> (XStream& s,Se3d& v);
	
  friend XBuffer& operator<= (XBuffer& b,const Se3d& v);
  friend XBuffer& operator>= (XBuffer& b,Se3d& v);
  friend XBuffer& operator< (XBuffer& b,const Se3d& v);
  friend XBuffer& operator> (XBuffer& b,Se3d& v);

  void serialize(Archive& ar);

  // Se3d constants /////////////////////////////////////////////////////////////

  static const Se3d ID;	    // identity Se3d
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			class Vect4f
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Vect4f 
{

public:

  float x, y, z, w;

  // constructors //////////////////////////////////////////////////////////////

  xm_inline Vect4f() {}
  xm_inline Vect4f(float x_, float y_, float z_, float w_ = 1.0f) { x = x_; y = y_; z = z_; w = w_; }
  xm_inline explicit Vect4f(const Vect3f& v3, float w_) { x = v3.x; y = v3.y; z = v3.z; w = w_; }

  xm_inline operator Vect3f () const { return Vect3f(x, y, z); }

  // setters / accessors / translators /////////////////////////////////////////

  Vect4f& set(float x_, float y_, float z_, float w_ = 1.0f) { x = x_; y = y_; z = z_; w = w_; return *this; }

  // index-based access:  0=x, 1=y, 2=z, 3 = w.
  xm_inline const float& operator[](int i) const {return *(&x + i);}
  xm_inline float& operator[](int i)       {return *(&x + i);}

  // Convertion to int ///////
  xm_inline int xi() const { return round(x); }
  xm_inline int yi() const { return round(y); }
  xm_inline int zi() const { return round(z); }
  xm_inline int wi() const { return round(w); }

  //  Logical operations  ////////////////////////////////
  xm_inline bool eq(const Vect4f& v, float delta = FLT_COMPARE_TOLERANCE) const;

  //  Addition and substruction  ////////////////////
  xm_inline Vect4f& add(const Vect4f& u, const Vect4f& v);  
  xm_inline Vect4f& add(const Vect4f& v);		   
  xm_inline Vect4f& sub(const Vect4f& u, const Vect4f& v);  
  xm_inline Vect4f& sub(const Vect4f& v);		    
  xm_inline Vect4f& operator+= (const Vect4f& v) { return add(v); }
  xm_inline Vect4f& operator-= (const Vect4f& v) { return sub(v); }
  xm_inline Vect4f operator+ (const Vect4f& v) const { Vect4f u; return u.add(*this,v); }
  xm_inline Vect4f operator- (const Vect4f& v) const { Vect4f u; return u.sub(*this,v); }

  // Component-wise multiplication and division  ////////////////
  xm_inline Vect4f& mult(const Vect4f& u, const Vect4f& v); 
  xm_inline Vect4f& mult(const Vect4f& v);		    
  xm_inline Vect4f& div(const Vect4f& u, const Vect4f& v); 
  xm_inline Vect4f& div(const Vect4f& v);		    
  xm_inline Vect4f& operator*= (const Vect4f& v) { return mult(v); }
  xm_inline Vect4f& operator/= (const Vect4f& v) { return div(v); }
  xm_inline Vect4f operator* (const Vect4f& v) const { Vect4f u; return u.mult(*this, v); }
  xm_inline Vect4f operator/ (const Vect4f& v) const { Vect4f u; return u.div(*this, v); }

  // Multiplication & division by scalar ///////////
  xm_inline Vect4f& scale(const Vect4f& v, float s);	   
  xm_inline Vect4f& scale(float s);			   
  xm_inline Vect4f& operator*= (float s) { return scale(s); }
  xm_inline Vect4f& operator/= (float s) { return scale(1/s); }
  xm_inline Vect4f operator* (float s) const { Vect4f u; return u.scale(*this, s); }
  xm_inline Vect4f operator/ (float s) const { Vect4f u; return u.scale(*this, 1/s); }
  xm_inline friend Vect4f operator* (float s,const Vect4f& v) { Vect4f u; return u.scale(v, s); }

  //	I/O operations    //////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM
  friend ostream& operator<< (ostream& os, const Vect4f& v);
  friend istream& operator>> (istream& is, Vect4f& v);
#endif

  friend XStream& operator<= (XStream& s,const Vect4f& v);
  friend XStream& operator>= (XStream& s,Vect4f& v);
  friend XStream& operator< (XStream& s,const Vect4f& v);
  friend XStream& operator> (XStream& s,Vect4f& v);

  friend XBuffer& operator<= (XBuffer& b,const Vect4f& v);
  friend XBuffer& operator>= (XBuffer& b,Vect4f& v);
  friend XBuffer& operator< (XBuffer& b,const Vect4f& v);
  friend XBuffer& operator> (XBuffer& b,Vect4f& v);

  void serialize(Archive& ar);

  //  Swap  /////////////////////////
  xm_inline void swap(Vect4f& other);
  xm_inline friend void swap(Vect4f& u, Vect4f& v) { u.swap(v);}


  // Vect4f constants ///////////////////////////////////////////////////////////

  static const Vect4f ZERO;
  static const Vect4f ID;

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//			Miscellaneous functions
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Decomposition  ////////////////////////////////
xm_inline void decomposition(const Vect3f& axis, const Vect3f& v, Vect3f& v_normal, Vect3f& v_tangent)
{
	// axis - axis of decomposition, v_normal - collinear to axis, v_tangent - perpendicular to axis
	v_normal.scale(axis, dot(axis, v)/((axis).norm2()));
	v_tangent.sub(v,v_normal);
}
xm_inline void decomposition(const Vect3d& axis, const Vect3d& v, Vect3d& v_normal, Vect3d& v_tangent)
{
	// axis - axis of decomposition, v_normal - collinear to axis, v_tangent - perpendicular to axis
	v_normal.scale(axis, dot(axis, v)/((axis).norm2()));
	v_tangent.sub(v,v_normal);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////						DEFINITIONS
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//		Vect2 definitions
//
///////////////////////////////////////////////////////////////////////////////
xm_inline Vect2i::Vect2i(const Vect2s& v)	{ x = v.x; y = v.y; }
xm_inline Vect2f::Vect2f(const Vect2i& v)	{ x = float(v.x); y = float(v.y); }
xm_inline Vect2f::Vect2f(const Vect2s& v)	{ x = v.x; y = v.y; }

Vect2f& Vect2f::interpolate(const Vect2f& u, const Vect2f& v, float lambda)
{
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
Vect3d::operator Vect3f () const 
{ 
  return Vect3f((float)x, (float)y, (float)z); 
}
Vect3f::operator Vect3d () const 
{ 
  return Vect3d(x, y, z); 
}
//  Dot product  //////////////////////
//xm_inline double dot(const Vect3d& u, const Vect3f& v) { return u.dot(v); }
//xm_inline float dot(const Vect3f& u, const Vect3d& v) { return u.dot(v); }

bool Vect3f::eq(const Vect3f& other, float delta) const
{
  return fabs(x - other.x) < delta && 
	    fabs(y - other.y) < delta && 
	    fabs(z - other.z) < delta;
}

Vect3f Vect3f::operator- () const
{
	return Vect3f(-x,-y,-z);
}

//  Norm operations /////////
float Vect3f::sumAbs() const
{
	return (float)(fabs(x) + fabs(y) + fabs(z));
}


//  Descart - spherical function  //////////////
float Vect3f::psi() const
{
	return (float)atan2(y,x);
}
float Vect3f::theta() const
{
	return (float)acos(z/(norm() + FLT_EPS));
}
Vect3f& Vect3f::setSpherical(float psi,float theta,float radius)
{
	x = radius*(float)sin(theta);
	y = x*(float)sin(psi);
	x = x*(float)cos(psi);
	z = radius*(float)cos(theta);
	return *this;
}

float Vect3f::dot (const Vect3f& other) const
{
  return x * other.x + y * other.y + z * other.z;
}


float Vect3f::norm() const
{
  return (float)sqrt(x * x + y * y + z * z);
}


float Vect3f::norm2() const
{
  return (x * x + y * y + z * z);
}


float Vect3f::distance(const Vect3f& other) const
{
  Vect3f w;

  w.sub(other, *this);
  return w.norm();
}


float Vect3f::distance2(const Vect3f& other) const
{
  Vect3f w;

  w.sub(other, *this);
  return w.norm2();
}


float Vect3f::min() const
{
  return (x <= y) ? ((x <= z) ? x : z) : ((y <= z) ? y : z);
}


float Vect3f::max() const
{
  return (x >= y) ? ((x >= z) ? x : z) : ((y >= z) ? y : z);
}


float Vect3f::minAbs() const
{
  float ax, ay, az;

  ax = (float)fabs(x);
  ay = (float)fabs(y);
  az = (float)fabs(z);
  return (ax <= ay) ? ((ax <= az) ? ax : az) : ((ay <= az) ? ay : az);
}


float Vect3f::maxAbs() const
{
  float ax, ay, az;

  ax = (float)fabs(x);
  ay = (float)fabs(y);
  az = (float)fabs(z);
  return (ax >= ay) ? ((ax >= az) ? ax : az) : ((ay >= az) ? ay : az);
}


void Vect3f::swap(Vect3f& other)
{
  Vect3f tmp;

  tmp = *this;
  *this = other;
  other = tmp;
}


Vect3f& Vect3f::normalize(const Vect3f& v, float r)
{
  float s = r*invSqrtFast(v.x * v.x + v.y * v.y + v.z * v.z);
  x = s * v.x;
  y = s * v.y;
  z = s * v.z;
  return *this;
}


Vect3f& Vect3f::normalize(float r)
{
  float s = r*invSqrtFast(x * x + y * y + z * z);
  x *= s;
  y *= s;
  z *= s;
  return *this;
}

Vect3f& Vect3f::negate(const Vect3f& v)
{
  x = - v.x;
  y = - v.y;
  z = - v.z;
  return *this;
}


Vect3f& Vect3f::negate()
{
  x = - x;
  y = - y;
  z = - z;
  return *this;
}


Vect3f& Vect3f::add(const Vect3f& u, const Vect3f& v)
{
  x = u.x + v.x;
  y = u.y + v.y;
  z = u.z + v.z;
  return *this;
}


Vect3f& Vect3f::add(const Vect3f& v)
{
  x += v.x;
  y += v.y;
  z += v.z;
  return *this;
}


Vect3f& Vect3f::sub(const Vect3f& u, const Vect3f& v)
{
  x = u.x - v.x;
  y = u.y - v.y;
  z = u.z - v.z;
  return *this;
}


Vect3f& Vect3f::sub(const Vect3f& v)
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
  return *this;
}


Vect3f& Vect3f::mult(const Vect3f& u, const Vect3f& v)
{
  x = u.x * v.x;
  y = u.y * v.y;
  z = u.z * v.z;
  return *this;
}


Vect3f& Vect3f::mult(const Vect3f& v)
{
  x *= v.x;
  y *= v.y;
  z *= v.z;
  return *this;
}

Vect3f& Vect3f::div(const Vect3f& u, const Vect3f& v)
{
  x = u.x / v.x;
  y = u.y / v.y;
  z = u.z / v.z;
  return *this;
}


Vect3f& Vect3f::div(const Vect3f& v)
{
  x /= v.x;
  y /= v.y;
  z /= v.z;
  return *this;
}



Vect3f& Vect3f::scale(const Vect3f& v, float s)
{
  x = s * v.x;
  y = s * v.y;
  z = s * v.z;
  return *this;
}


Vect3f& Vect3f::scale(float s)
{
  x *= s;
  y *= s;
  z *= s;
  return *this;
}



Vect3f& Vect3f::cross(const Vect3f& u, const Vect3f& v)
{
  x = u.y * v.z - u.z * v.y;
  y = u.z * v.x - u.x * v.z;
  z = u.x * v.y - u.y * v.x;
  return *this;
}


Vect3f& Vect3f::precross(const Vect3f& v)
{
  float ox, oy;

  ox = x;
  oy = y;
  x = v.y * z - v.z * oy;
  y = v.z * ox - v.x * z;
  z = v.x * oy - v.y * ox;
  return *this;
}


Vect3f& Vect3f::postcross(const Vect3f& v)
{
  float ox, oy;

  ox = x;
  oy = y;
  x = oy * v.z - z * v.y;
  y = z * v.x - ox * v.z;
  z = ox * v.y - oy * v.x;
  return *this;
}


Vect3f& Vect3f::crossAdd(const Vect3f& u, const Vect3f& v, const Vect3f& w)
{
  x = u.y * v.z - u.z * v.y + w.x;
  y = u.z * v.x - u.x * v.z + w.y;
  z = u.x * v.y - u.y * v.x + w.z;
  return *this;
}


Vect3f& Vect3f::crossAdd(const Vect3f& u, const Vect3f& v)
{
  x += u.y * v.z - u.z * v.y;
  y += u.z * v.x - u.x * v.z;
  z += u.x * v.y - u.y * v.x;
  return *this;
}


Vect3f& Vect3f::scaleAdd(const Vect3f& v, const Vect3f& u, float lambda)
{
  x = v.x + lambda * u.x;
  y = v.y + lambda * u.y;
  z = v.z + lambda * u.z;
  return *this;
}


Vect3f& Vect3f::scaleAdd(const Vect3f& u, float lambda)
{
  x += lambda * u.x;
  y += lambda * u.y;
  z += lambda * u.z;
  return *this;
}


Vect3f& Vect3f::interpolate(const Vect3f& u, const Vect3f& v, float lambda)
{
  float lambda2 = 1.0f - lambda;

  x = lambda2 * u.x + lambda * v.x;
  y = lambda2 * u.y + lambda * v.y;
  z = lambda2 * u.z + lambda * v.z;
  return *this;
}


///////////////////////////////////////////////////////////////////////////////
//
//		Vect3d xm_inline definitions
//
///////////////////////////////////////////////////////////////////////////////

bool Vect3d::eq(const Vect3d& other, double delta) const
{
  return fabs(x - other.x) < delta && 
	    fabs(y - other.y) < delta && 
	    fabs(z - other.z) < delta;
}

Vect3d Vect3d::operator- () const
{
	return Vect3d(-x,-y,-z);
}

//  Norm operations /////////
double Vect3d::sumAbs() const
{
	return fabs(x) + fabs(y) + fabs(z);
}


//  Descart - spherical function  //////////////
double Vect3d::psi() const
{
	return atan2(y,x);
}
double Vect3d::theta() const
{
	return acos(z/(norm() + DBL_EPS));
}
Vect3d& Vect3d::setSpherical(double psi,double theta,double radius)
{
	x = radius*sin(theta);
	y = x*sin(psi);
	x = x*cos(psi);
	z = radius*cos(theta);
	return *this;
}

double Vect3d::dot (const Vect3d& other) const
{
  return x * other.x + y * other.y + z * other.z;
}


double Vect3d::norm() const
{
  return sqrt(x * x + y * y + z * z);
}


double Vect3d::norm2() const
{
  return (x * x + y * y + z * z);
}


double Vect3d::distance(const Vect3d& other) const
{
  Vect3d w;

  w.sub(other, *this);
  return w.norm();
}


double Vect3d::distance2(const Vect3d& other) const
{
  Vect3d w;

  w.sub(other, *this);
  return w.norm2();
}


double Vect3d::min() const
{
  return (x <= y) ? ((x <= z) ? x : z) : ((y <= z) ? y : z);
}


double Vect3d::max() const
{
  return (x >= y) ? ((x >= z) ? x : z) : ((y >= z) ? y : z);
}


double Vect3d::minAbs() const
{
  double ax, ay, az;

  ax = fabs(x);
  ay = fabs(y);
  az = fabs(z);
  return (ax <= ay) ? ((ax <= az) ? ax : az) : ((ay <= az) ? ay : az);
}


double Vect3d::maxAbs() const
{
  double ax, ay, az;

  ax = fabs(x);
  ay = fabs(y);
  az = fabs(z);
  return (ax >= ay) ? ((ax >= az) ? ax : az) : ((ay >= az) ? ay : az);
}


void Vect3d::swap(Vect3d& other)
{
  Vect3d tmp;

  tmp = *this;
  *this = other;
  other = tmp;
}


Vect3d& Vect3d::normalize(const Vect3d& v, double r)
{
  double s = r / (sqrt(v.x * v.x + v.y * v.y + v.z * v.z) + DBL_EPS);
  x = s * v.x;
  y = s * v.y;
  z = s * v.z;
  return *this;
}


Vect3d& Vect3d::normalize(double r)
{
  double s = r / (sqrt(x * x + y * y + z * z) + DBL_EPS);
  x *= s;
  y *= s;
  z *= s;
  return *this;
}

Vect3d& Vect3d::negate(const Vect3d& v)
{
  x = - v.x;
  y = - v.y;
  z = - v.z;
  return *this;
}


Vect3d& Vect3d::negate()
{
  x = - x;
  y = - y;
  z = - z;
  return *this;
}


Vect3d& Vect3d::add(const Vect3d& u, const Vect3d& v)
{
  x = u.x + v.x;
  y = u.y + v.y;
  z = u.z + v.z;
  return *this;
}


Vect3d& Vect3d::add(const Vect3d& v)
{
  x += v.x;
  y += v.y;
  z += v.z;
  return *this;
}


Vect3d& Vect3d::sub(const Vect3d& u, const Vect3d& v)
{
  x = u.x - v.x;
  y = u.y - v.y;
  z = u.z - v.z;
  return *this;
}


Vect3d& Vect3d::sub(const Vect3d& v)
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
  return *this;
}


Vect3d& Vect3d::mult(const Vect3d& u, const Vect3d& v)
{
  x = u.x * v.x;
  y = u.y * v.y;
  z = u.z * v.z;
  return *this;
}


Vect3d& Vect3d::mult(const Vect3d& v)
{
  x *= v.x;
  y *= v.y;
  z *= v.z;
  return *this;
}

Vect3d& Vect3d::div(const Vect3d& u, const Vect3d& v)
{
  x = u.x / v.x;
  y = u.y / v.y;
  z = u.z / v.z;
  return *this;
}


Vect3d& Vect3d::div(const Vect3d& v)
{
  x /= v.x;
  y /= v.y;
  z /= v.z;
  return *this;
}



Vect3d& Vect3d::scale(const Vect3d& v, double s)
{
  x = s * v.x;
  y = s * v.y;
  z = s * v.z;
  return *this;
}


Vect3d& Vect3d::scale(double s)
{
  x *= s;
  y *= s;
  z *= s;
  return *this;
}



Vect3d& Vect3d::cross(const Vect3d& u, const Vect3d& v)
{
  x = u.y * v.z - u.z * v.y;
  y = u.z * v.x - u.x * v.z;
  z = u.x * v.y - u.y * v.x;
  return *this;
}


Vect3d& Vect3d::precross(const Vect3d& v)
{
  double ox, oy;

  ox = x;
  oy = y;
  x = v.y * z - v.z * oy;
  y = v.z * ox - v.x * z;
  z = v.x * oy - v.y * ox;
  return *this;
}


Vect3d& Vect3d::postcross(const Vect3d& v)
{
  double ox, oy;

  ox = x;
  oy = y;
  x = oy * v.z - z * v.y;
  y = z * v.x - ox * v.z;
  z = ox * v.y - oy * v.x;
  return *this;
}


Vect3d& Vect3d::crossAdd(const Vect3d& u, const Vect3d& v, const Vect3d& w)
{
  x = u.y * v.z - u.z * v.y + w.x;
  y = u.z * v.x - u.x * v.z + w.y;
  z = u.x * v.y - u.y * v.x + w.z;
  return *this;
}


Vect3d& Vect3d::crossAdd(const Vect3d& u, const Vect3d& v)
{
  x += u.y * v.z - u.z * v.y;
  y += u.z * v.x - u.x * v.z;
  z += u.x * v.y - u.y * v.x;
  return *this;
}


Vect3d& Vect3d::scaleAdd(const Vect3d& v, const Vect3d& u, double lambda)
{
  x = v.x + lambda * u.x;
  y = v.y + lambda * u.y;
  z = v.z + lambda * u.z;
  return *this;
}


Vect3d& Vect3d::scaleAdd(const Vect3d& u, double lambda)
{
  x += lambda * u.x;
  y += lambda * u.y;
  z += lambda * u.z;
  return *this;
}


Vect3d& Vect3d::interpolate(const Vect3d& u, const Vect3d& v, double lambda)
{
  double lambda2 = 1.0 - lambda;

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

Mat3f::Mat3f(float xx_,float xy_,float xz_,
	 float yx_,float yy_,float yz_,
	 float zx_,float zy_,float zz_)
{
	xx = xx_; xy = xy_; xz = xz_;
	yx = yx_; yy = yy_; yz = yz_;
	zx = zx_; zy = zy_; zz = zz_;
}

Mat3f& Mat3f::set(float angle, eAxis axis)
{
//	------ Calculate Matrix for ROTATE point an angle ------
	float calpha = (float)cos(angle);
	float salpha = (float)sin(angle);
	switch(axis){
		case Z_AXIS:
			xx   =	calpha; xy	 = -salpha; xz	 = 0;
			yx   =	salpha; yy	 = calpha;  yz	 = 0;
			zx   = 0;	  zy	 = 0;	    zz	 = 1;
			break;
		case X_AXIS:
			xx   =	1;	  xy	 =  0;	    xz	 = 0;
			yx   =	0;	  yy	 =  calpha; yz	 = -salpha;
			zx   =	0;	  zy	 =  salpha; zz	 = calpha;
			break;
		case Y_AXIS:
			xx   = calpha;	xy	 =  0;	    xz	 = salpha;
			yx   = 0;	  yy	 =  1;	    yz	 = 0;
			zx   = -salpha; zy	 =  0;	    zz	 = calpha;
			break;
		}
	return *this;
}

Mat3f& Mat3f::set(const Vect3f& diag, const Vect3f& sym)
{
  xx = diag.x;
  yy = diag.y;
  zz = diag.z;
  yz = zy = sym.x;
  zx = xz = sym.y;
  xy = yx = sym.z;
  return *this;
}


Mat3f& Mat3f::setXcol(const Vect3f& v)
{
  xx = v.x;
  yx = v.y;
  zx = v.z;
  return *this;
}


Mat3f& Mat3f::setYcol(const Vect3f& v)
{
  xy = v.x;
  yy = v.y;
  zy = v.z;
  return *this;
}


Mat3f& Mat3f::setZcol(const Vect3f& v)
{
  xz = v.x;
  yz = v.y;
  zz = v.z;
  return *this;
}


Mat3f& Mat3f::setSkew(const Vect3f& v)
{
  xx = yy = zz = 0.0;
  zy =	v.x;
  yz = -v.x;
  xz =	v.y;
  zx = -v.y;
  yx =	v.z;
  xy = -v.z;
  return *this;
}


float Mat3f::det() const
{
  return  xx * (yy * zz - yz * zy)
	+ xy * (yz * zx - yx * zz)
	+ xz * (yx * zy - yy * zx);
}


Mat3f& Mat3f::xpose(const Mat3f& M)
{
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


Mat3f& Mat3f::xpose()
{
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


Mat3f& Mat3f::symmetrize(const Mat3f& M)
{
  xx = 2 * M.xx;
  yy = 2 * M.yy;
  zz = 2 * M.zz;
  xy = yx = M.xy + M.yx;
  yz = zy = M.yz + M.zy;
  zx = xz = M.zx + M.xz;
  return *this;
}


Mat3f& Mat3f::symmetrize()
{
  xx = 2 * xx;
  yy = 2 * yy;
  zz = 2 * zz;
  xy = yx = xy + yx;
  yz = zy = yz + zy;
  zx = xz = zx + xz;
  return *this;
}

Mat3f Mat3f::operator- () const
{
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

Mat3f& Mat3f::negate(const Mat3f& M)
{
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


Mat3f& Mat3f::negate()
{
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


Mat3f& Mat3f::add(const Mat3f& M, const Mat3f& N)
{
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


Mat3f& Mat3f::add(const Mat3f& M)
{
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


Mat3f& Mat3f::sub(const Mat3f& M, const Mat3f& N)
{
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


Mat3f& Mat3f::sub(const Mat3f& M)
{
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


Mat3f& Mat3f::scale(const Mat3f& M, float s)
{
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


Mat3f& Mat3f::scale(const Vect3f &s)
{
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

Mat3f& Mat3f::scale(float s)
{
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

Mat3f& Mat3f::preScale(const Mat3f& M, const Vect3f& v)
{	// Mat3f(v) * M
	xx = M.xx*v.x;
	xy = M.xy*v.x;
	xz = M.xz*v.x;

	yx = M.yx*v.y;
	yy = M.yy*v.y;
	yz = M.yz*v.y;

	zx = M.zx*v.z;
	zy = M.zy*v.z;
	zz = M.zz*v.z;
	return *this;
}

Mat3f& Mat3f::preScale(const Vect3f& v)
{	// Mat3f(s) * this
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

Mat3f& Mat3f::postScale(const Mat3f& M, const Vect3f& v)
{	// M * Mat3f(v)
	xx = M.xx*v.x;
	xy = M.xy*v.y;
	xz = M.xz*v.z;

	yx = M.yx*v.x;
	yy = M.yy*v.y;
	yz = M.yz*v.z;

	zx = M.zx*v.x;
	zy = M.zy*v.y;
	zz = M.zz*v.z;
	return *this;
}

Mat3f& Mat3f::postScale(const Vect3f& v)
{	// this * Mat3f(v)
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



//  Vect3d transforming  /////////////////////
Vect3d& Mat3f::xform(const Vect3d& v, Vect3d& xv) const
{
  xv.x = xx * v.x + xy * v.y + xz * v.z;
  xv.y = yx * v.x + yy * v.y + yz * v.z;
  xv.z = zx * v.x + zy * v.y + zz * v.z;
  return xv;
}


Vect3d& Mat3f::xform(Vect3d& v) const
{
  double ox, oy;

  ox = v.x; oy= v.y;
  v.x = xx * ox + xy * oy + xz * v.z;
  v.y = yx * ox + yy * oy + yz * v.z;
  v.z = zx * ox + zy * oy + zz * v.z;
  return v;
}


Vect3d& Mat3f::invXform(const Vect3d& v, Vect3d& xv) const
{
  xv.x = xx * v.x + yx * v.y + zx * v.z;
  xv.y = xy * v.x + yy * v.y + zy * v.z;
  xv.z = xz * v.x + yz * v.y + zz * v.z;
  return xv;
}


Vect3d& Mat3f::invXform(Vect3d& v) const
{
  double ox, oy;

  ox = v.x; oy= v.y;
  v.x = xx * ox + yx * oy + zx * v.z;
  v.y = xy * ox + yy * oy + zy * v.z;
  v.z = xz * ox + yz * oy + zz * v.z;
  return v;
}

//  Vect3f transforming  /////////////////////
Vect3f& Mat3f::xform(const Vect3f& v, Vect3f& xv) const
{
  xv.x = (float)(xx * v.x + xy * v.y + xz * v.z);
  xv.y = (float)(yx * v.x + yy * v.y + yz * v.z);
  xv.z = (float)(zx * v.x + zy * v.y + zz * v.z);
  return xv;
}


Vect3f& Mat3f::xform(Vect3f& v) const
{
  float ox, oy;

  ox = v.x; oy= v.y;
  v.x = (float)(xx * ox + xy * oy + xz * v.z);
  v.y = (float)(yx * ox + yy * oy + yz * v.z);
  v.z = (float)(zx * ox + zy * oy + zz * v.z);
  return v;
}


Vect3f& Mat3f::invXform(const Vect3f& v, Vect3f& xv) const
{
  xv.x = (float)(xx * v.x + yx * v.y + zx * v.z);
  xv.y = (float)(xy * v.x + yy * v.y + zy * v.z);
  xv.z = (float)(xz * v.x + yz * v.y + zz * v.z);
  return xv;
}


Vect3f& Mat3f::invXform(Vect3f& v) const
{
  float ox, oy;

  ox = v.x; oy= v.y;
  v.x = (float)(xx * ox + yx * oy + zx * v.z);
  v.y = (float)(xy * ox + yy * oy + zy * v.z);
  v.z = (float)(xz * ox + yz * oy + zz * v.z);
  return v;
}

Mat3f::operator Mat3d () const 
{ 
	return Mat3d( xx, xy, xz,   yx, yy, yz,  zx, zy, zz ); 
}

Mat3d::operator Mat3f () const 
{
	return Mat3f( (float)xx, (float)xy, (float)xz,  
				(float)yx, (float)yy, (float)yz, 
				(float)zx, (float)zy, (float)zz );
}



///////////////////////////////////////////////////////////////////////////////
//
//		Mat3d xm_inline definitions
//
///////////////////////////////////////////////////////////////////////////////
Mat3d::Mat3d(double xx_,double xy_,double xz_,
	 double yx_,double yy_,double yz_,
	 double zx_,double zy_,double zz_)
{
	xx = xx_; xy = xy_; xz = xz_;
	yx = yx_; yy = yy_; yz = yz_;
	zx = zx_; zy = zy_; zz = zz_;
}

Mat3d& Mat3d::set(double angle, eAxis axis)
{
//	------ Calculate Matrix for ROTATE point an angle ------
	double calpha = cos(angle);
	double salpha = sin(angle);
	switch(axis){
		case Z_AXIS:
			xx   =	calpha; xy	 = -salpha; xz	 = 0;
			yx   =	salpha; yy	 = calpha;  yz	 = 0;
			zx   = 0;	  zy	 = 0;	    zz	 = 1;
			break;
		case X_AXIS:
			xx   =	1;	  xy	 =  0;	    xz	 = 0;
			yx   =	0;	  yy	 =  calpha; yz	 = -salpha;
			zx   =	0;	  zy	 =  salpha; zz	 = calpha;
			break;
		case Y_AXIS:
			xx   = calpha;	xy	 =  0;	    xz	 = salpha;
			yx   = 0;	  yy	 =  1;	    yz	 = 0;
			zx   = -salpha; zy	 =  0;	    zz	 = calpha;
			break;
		}
	return *this;
}

Mat3d& Mat3d::set(const Vect3d& diag, const Vect3d& sym)
{
  xx = diag.x;
  yy = diag.y;
  zz = diag.z;
  yz = zy = sym.x;
  zx = xz = sym.y;
  xy = yx = sym.z;
  return *this;
}

Mat3d& Mat3d::setXcol(const Vect3d& v)
{
  xx = v.x;
  yx = v.y;
  zx = v.z;
  return *this;
}


Mat3d& Mat3d::setYcol(const Vect3d& v)
{
  xy = v.x;
  yy = v.y;
  zy = v.z;
  return *this;
}


Mat3d& Mat3d::setZcol(const Vect3d& v)
{
  xz = v.x;
  yz = v.y;
  zz = v.z;
  return *this;
}


Mat3d& Mat3d::setSkew(const Vect3d& v)
{
  xx = yy = zz = 0.0;
  zy =	v.x;
  yz = -v.x;
  xz =	v.y;
  zx = -v.y;
  yx =	v.z;
  xy = -v.z;
  return *this;
}


double Mat3d::det() const
{
  return  xx * (yy * zz - yz * zy)
	+ xy * (yz * zx - yx * zz)
	+ xz * (yx * zy - yy * zx);
}


Mat3d& Mat3d::xpose(const Mat3d& M)
{
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


Mat3d& Mat3d::xpose()
{
  double tmp;

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


Mat3d& Mat3d::symmetrize(const Mat3d& M)
{
  xx = 2 * M.xx;
  yy = 2 * M.yy;
  zz = 2 * M.zz;
  xy = yx = M.xy + M.yx;
  yz = zy = M.yz + M.zy;
  zx = xz = M.zx + M.xz;
  return *this;
}


Mat3d& Mat3d::symmetrize()
{
  xx = 2 * xx;
  yy = 2 * yy;
  zz = 2 * zz;
  xy = yx = xy + yx;
  yz = zy = yz + zy;
  zx = xz = zx + xz;
  return *this;
}

Mat3d Mat3d::operator- () const
{
   Mat3d M;
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

Mat3d& Mat3d::negate(const Mat3d& M)
{
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


Mat3d& Mat3d::negate()
{
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


Mat3d& Mat3d::add(const Mat3d& M, const Mat3d& N)
{
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


Mat3d& Mat3d::add(const Mat3d& M)
{
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


Mat3d& Mat3d::sub(const Mat3d& M, const Mat3d& N)
{
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


Mat3d& Mat3d::sub(const Mat3d& M)
{
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


Mat3d& Mat3d::scale(const Mat3d& M, double s)
{
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


Mat3d& Mat3d::scale(double s)
{
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

Mat3d& Mat3d::preScale(const Mat3d& M, const Vect3d& v)
{	// Mat3d(v) * M
	xx = M.xx*v.x;
	xy = M.xy*v.x;
	xz = M.xz*v.x;

	yx = M.yx*v.y;
	yy = M.yy*v.y;
	yz = M.yz*v.y;

	zx = M.zx*v.z;
	zy = M.zy*v.z;
	zz = M.zz*v.z;
	return *this;
}

Mat3d& Mat3d::preScale(const Vect3d& v)
{	// Mat3d(s) * this
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

Mat3d& Mat3d::postScale(const Mat3d& M, const Vect3d& v)
{	// M * Mat3d(v)
	xx = M.xx*v.x;
	xy = M.xy*v.y;
	xz = M.xz*v.z;

	yx = M.yx*v.x;
	yy = M.yy*v.y;
	yz = M.yz*v.z;

	zx = M.zx*v.x;
	zy = M.zy*v.y;
	zz = M.zz*v.z;
	return *this;
}

Mat3d& Mat3d::postScale(const Vect3d& v)
{	// this * Mat3d(v)
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



//  Vect3d transforming  /////////////////////
Vect3d& Mat3d::xform(const Vect3d& v, Vect3d& xv) const
{
  xv.x = xx * v.x + xy * v.y + xz * v.z;
  xv.y = yx * v.x + yy * v.y + yz * v.z;
  xv.z = zx * v.x + zy * v.y + zz * v.z;
  return xv;
}


Vect3d& Mat3d::xform(Vect3d& v) const
{
  double ox, oy;

  ox = v.x; oy= v.y;
  v.x = xx * ox + xy * oy + xz * v.z;
  v.y = yx * ox + yy * oy + yz * v.z;
  v.z = zx * ox + zy * oy + zz * v.z;
  return v;
}


Vect3d& Mat3d::invXform(const Vect3d& v, Vect3d& xv) const
{
  xv.x = xx * v.x + yx * v.y + zx * v.z;
  xv.y = xy * v.x + yy * v.y + zy * v.z;
  xv.z = xz * v.x + yz * v.y + zz * v.z;
  return xv;
}


Vect3d& Mat3d::invXform(Vect3d& v) const
{
  double ox, oy;

  ox = v.x; oy= v.y;
  v.x = xx * ox + yx * oy + zx * v.z;
  v.y = xy * ox + yy * oy + zy * v.z;
  v.z = xz * ox + yz * oy + zz * v.z;
  return v;
}


//  Vect3f transforming  /////////////////////
Vect3f& Mat3d::xform(const Vect3f& v, Vect3f& xv) const
{
  xv.x = (float)(xx * v.x + xy * v.y + xz * v.z);
  xv.y = (float)(yx * v.x + yy * v.y + yz * v.z);
  xv.z = (float)(zx * v.x + zy * v.y + zz * v.z);
  return xv;
}


Vect3f& Mat3d::xform(Vect3f& v) const
{
  float ox, oy;

  ox = v.x; oy= v.y;
  v.x = (float)(xx * ox + xy * oy + xz * v.z);
  v.y = (float)(yx * ox + yy * oy + yz * v.z);
  v.z = (float)(zx * ox + zy * oy + zz * v.z);
  return v;
}


Vect3f& Mat3d::invXform(const Vect3f& v, Vect3f& xv) const
{
  xv.x = (float)(xx * v.x + yx * v.y + zx * v.z);
  xv.y = (float)(xy * v.x + yy * v.y + zy * v.z);
  xv.z = (float)(xz * v.x + yz * v.y + zz * v.z);
  return xv;
}


Vect3f& Mat3d::invXform(Vect3f& v) const
{
  float ox, oy;

  ox = v.x; oy= v.y;
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
MatXf::MatXf(const float16& T)
{
	R[0][0] = T[0];	R[1][0] = T[1];	R[2][0] = T[2]; 
	R[0][1] = T[4]; R[1][1] = T[5];	R[2][1] = T[6]; 
	R[0][2] = T[8]; R[1][2] = T[9];	R[2][2] = T[10];
	d[0] = T[12]; d[1] = T[13]; d[2] = T[14];
}

MatXf::operator MatXd () const 
{ 
	return MatXd(R, d); 
}

MatXf& MatXf::set(const Se3f& T)
{
  R.set(T.rot());
  d = T.trans();
  return *this;
}

//  Vect3d transforming  /////////////////////
Vect3d& MatXf::xformVect(const Vect3d& v, Vect3d& xv) const
{
  return R.xform(v, xv);
}


Vect3d& MatXf::xformVect(Vect3d& v) const
{
  return R.xform(v);
}


Vect3d& MatXf::xformPoint(const Vect3d& p, Vect3d& xp) const
{
  R.xform(p, xp);
  xp.add(d);
  return xp;
}


Vect3d& MatXf::xformPoint(Vect3d& p) const
{
  R.xform(p);
  p.add(d);
  return p;
}


Vect3d& MatXf::invXformVect(const Vect3d& v, Vect3d& xv) const
{
  return R.invXform(v, xv);
}


Vect3d& MatXf::invXformVect(Vect3d& v) const
{
  return R.invXform(v);
}


Vect3d& MatXf::invXformPoint(const Vect3d& p, Vect3d& xp) const
{
  xp.sub(p, d);
  R.invXform(xp);
  return xp;
}


Vect3d& MatXf::invXformPoint(Vect3d& p) const
{
  p.sub(d);
  R.invXform(p);
  return p;
}


//  Vect3f transforming  /////////////////////
Vect3f& MatXf::xformVect(const Vect3f& v, Vect3f& xv) const
{
  return R.xform(v, xv);
}


Vect3f& MatXf::xformVect(Vect3f& v) const
{
  return R.xform(v);
}


Vect3f& MatXf::xformPoint(const Vect3f& p, Vect3f& xp) const
{
  R.xform(p, xp);
  xp.add(d);
  return xp;
}


Vect3f& MatXf::xformPoint(Vect3f& p) const
{
  R.xform(p);
  p.add(d);
  return p;
}


Vect3f& MatXf::invXformVect(const Vect3f& v, Vect3f& xv) const
{
  return R.invXform(v, xv);
}


Vect3f& MatXf::invXformVect(Vect3f& v) const
{
  return R.invXform(v);
}


Vect3f& MatXf::invXformPoint(const Vect3f& p, Vect3f& xp) const
{
  xp.sub(p, d);
  R.invXform(xp);
  return xp;
}


Vect3f& MatXf::invXformPoint(Vect3f& p) const
{
  p.sub(d);
  R.invXform(p);
  return p;
}




///////////////////////////////////////////////////////////////////////////////
//
//		MatXd xm_inline definitions
//
///////////////////////////////////////////////////////////////////////////////
MatXd::MatXd(const double16& T)
{
	R[0][0] = T[0];	R[1][0] = T[1];	R[2][0] = T[2]; 
	R[0][1] = T[4]; R[1][1] = T[5];	R[2][1] = T[6]; 
	R[0][2] = T[8]; R[1][2] = T[9];	R[2][2] = T[10];
	d[0] = T[12]; d[1] = T[13]; d[2] = T[14];
}

MatXd::operator MatXf () const 
{ 
	return MatXf(R, d); 
}

MatXd& MatXd::set(const Se3d& T)
{
  R.set(T.rot());
  d = T.trans();
  return *this;
}


//  Vect3d transforming  /////////////////////
Vect3d& MatXd::xformVect(const Vect3d& v, Vect3d& xv) const
{
  return R.xform(v, xv);
}


Vect3d& MatXd::xformVect(Vect3d& v) const
{
  return R.xform(v);
}


Vect3d& MatXd::xformPoint(const Vect3d& p, Vect3d& xp) const
{
  R.xform(p, xp);
  xp.add(d);
  return xp;
}


Vect3d& MatXd::xformPoint(Vect3d& p) const
{
  R.xform(p);
  p.add(d);
  return p;
}


Vect3d& MatXd::invXformVect(const Vect3d& v, Vect3d& xv) const
{
  return R.invXform(v, xv);
}


Vect3d& MatXd::invXformVect(Vect3d& v) const
{
  return R.invXform(v);
}


Vect3d& MatXd::invXformPoint(const Vect3d& p, Vect3d& xp) const
{
  xp.sub(p, d);
  R.invXform(xp);
  return xp;
}


Vect3d& MatXd::invXformPoint(Vect3d& p) const
{
  p.sub(d);
  R.invXform(p);
  return p;
}




//  Vect3f transforming  /////////////////////
Vect3f& MatXd::xformVect(const Vect3f& v, Vect3f& xv) const
{
  return R.xform(v, xv);
}


Vect3f& MatXd::xformVect(Vect3f& v) const
{
  return R.xform(v);
}


Vect3f& MatXd::xformPoint(const Vect3f& p, Vect3f& xp) const
{
  R.xform(p, xp);
  xp.add(d);
  return xp;
}


Vect3f& MatXd::xformPoint(Vect3f& p) const
{
  R.xform(p);
  p.add(d);
  return p;
}


Vect3f& MatXd::invXformVect(const Vect3f& v, Vect3f& xv) const
{
  return R.invXform(v, xv);
}


Vect3f& MatXd::invXformVect(Vect3f& v) const
{
  return R.invXform(v);
}


Vect3f& MatXd::invXformPoint(const Vect3f& p, Vect3f& xp) const
{
  xp.sub(p, d);
  R.invXform(xp);
  return xp;
}


Vect3f& MatXd::invXformPoint(Vect3f& p) const
{
  p.sub(d);
  R.invXform(p);
  return p;
}




///////////////////////////////////////////////////////////////////////////////
//		QuatF xm_inline definitions
///////////////////////////////////////////////////////////////////////////////
QuatF::operator QuatD () const 
{ 
	return QuatD((float)s_,(float)x_,(float)y_,(float)z_); 
}

bool QuatF::eq(const QuatF& other, float delta) const
{
  return fabs(s_ - other.s_) < delta && 
	    fabs(x_ - other.x_) < delta && 
	    fabs(y_ - other.y_) < delta && 
	    fabs(z_ - other.z_) < delta;
}

Vect3f QuatF::axis() const
{
  Vect3f v(x_, y_, z_);
  if (v.norm() == 0.0) v = Vect3f::I;  // axis is arbitrary here
  else v.normalize();
  return v;
}


float QuatF::angle() const
{
  return 2 * acosf(s_);
}


QuatF& QuatF::normalize(const QuatF& q)
{
  float scale = 1.f*invSqrtFast(q.s_*q.s_ + q.x_*q.x_ + q.y_*q.y_ + q.z_*q.z_);
  s_ = scale * q.s_;
  x_ = scale * q.x_;
  y_ = scale * q.y_;
  z_ = scale * q.z_;
  return *this;
}

QuatF& QuatF::normalize()
{
  float scale = 1.f*invSqrtFast(s_*s_ + x_*x_ + y_*y_ + z_*z_);
  s_ *= scale;
  x_ *= scale;
  y_ *= scale;
  z_ *= scale;
  return *this;
}


QuatF& QuatF::invert(const QuatF& q)
{
  s_ = -q.s_;
  x_ =	q.x_;
  y_ =	q.y_;
  z_ =	q.z_;
  return *this;
}


QuatF& QuatF::invert()
{
  s_ = -s_;
  return *this;
}

QuatF& QuatF::operator+= (const QuatF& q)
{
	s_ += q.s_;
	x_ += q.x_;
	y_ += q.y_;
	z_ += q.z_;
	return *this;
}
QuatF& QuatF::operator-= (const QuatF& q)
{
	s_ -= q.s_;
	x_ -= q.x_;
	y_ -= q.y_;
	z_ -= q.z_;
	return *this;
}
QuatF QuatF::operator+ (const QuatF& q) const
{
	return QuatF(s_ + q.s_,x_ + q.x_,y_ + q.y_,z_ + q.z_);
}
QuatF QuatF::operator- (const QuatF& q) const
{
	return QuatF(s_ - q.s_,x_ - q.x_,y_ - q.y_,z_ - q.z_);
}

float QuatF::dot (const QuatF& q) const
{
	return s_*q.s_ + x_*q.x_ + y_*q.y_ + z_*q.z_;
}


//  Scalar operations  /////////////////
QuatF& QuatF::operator*= (float w)
{
	s_ *= w;
	x_ *= w;
	y_ *= w;
	z_ *= w;
	return *this;
}
QuatF& QuatF::operator/= (float w)
{
	w = 1/w;
	s_ *= w;
	x_ *= w;
	y_ *= w;
	z_ *= w;
	return *this;
}
QuatF QuatF::operator* (float w) const
{
	return QuatF(s_*w,x_*w,y_*w,z_*w);
}
QuatF QuatF::operator/ (float w) const
{
	w = 1/w;
	return QuatF(s_*w,x_*w,y_*w,z_*w);
}
QuatF operator* (float w,const QuatF& q)
{
	return QuatF(q.s_*w,q.x_*w,q.y_*w,q.z_*w);
}

xm_inline void QuatF::slerp(const QuatF& a,const QuatF& b,float t)
{ 
	// Slerp(q1,q2,t) = (sin((1-t)*A)/sin(A))*q1+(sin(t*A)/sin(A))*q2 
	// Но делаем линейную интерполяцию и нормализуем

    float cosom = a.dot(b);
    if(cosom < 0.0){ 
		cosom = -cosom; 
		negate(b);
	}
	else
		*this = b;

    float scale0 = 1.0f - t;
	float scale1 = t;
	
	*this *= scale1;
	*this += a*scale0;
	normalize();
}

xm_inline void QuatF::slerpExact(const QuatF& a,const QuatF& b,float t)
{ 
	// Slerp(q1,q2,t) = (sin((1-t)*A)/sin(A))*q1+(sin(t*A)/sin(A))*q2 
	float scale0, scale1;

	// calc cosine
	float cosom = a.dot(b);

	// adjust signs (if necessary)
	if(cosom < 0.0){ 
		cosom = -cosom; 
		negate(b);
	}
	else
		*this = b;

	// calculate coefficients
	if ((1.0 - cosom) > 1e-5f ) {
		// standard case (slerp)
		float omega = acosf(cosom);
		float sinom = sinf(omega);
		scale0 = sinf((1.0f - t) * omega) / sinom;
		scale1 = sinf(t * omega) / sinom;
	}
	else {	// "from" and "to" quaternions are very close, so we can do a linear interpolation
		scale0 = 1.0f - t;
		scale1 = t;
	}

	// calculate final values
	*this *= scale1;
	*this += a*scale0;
}

///////////////////////////////////////////////////////////////////////////////
//		QuatD xm_inline definitions
///////////////////////////////////////////////////////////////////////////////
QuatD::operator QuatF () const 
{ 
	return QuatF((float)s_,(float)x_,(float)y_,(float)z_); 
}

bool QuatD::eq(const QuatD& other, double delta) const
{
  return fabs(s_ - other.s_) < delta && 
	    fabs(x_ - other.x_) < delta && 
	    fabs(y_ - other.y_) < delta && 
	    fabs(z_ - other.z_) < delta;
}

Vect3d QuatD::axis() const
{
  Vect3d v(x_, y_, z_);
  if (v.norm() == 0.0) v = Vect3d::I;  // axis is arbitrary here
  else v.normalize();
  return v;
}


double QuatD::angle() const
{
  return 2 * acos(s_);
}


QuatD& QuatD::normalize(const QuatD& q)
{
  double scale;

  scale = 1.0 / sqrt(q.s_*q.s_ + q.x_*q.x_ + q.y_*q.y_ + q.z_*q.z_);
  s_ = scale * q.s_;
  x_ = scale * q.x_;
  y_ = scale * q.y_;
  z_ = scale * q.z_;
  return *this;
}

QuatD& QuatD::normalize()
{
  double scale;

  scale = 1.0 / sqrt(s_*s_ + x_*x_ + y_*y_ + z_*z_);
  s_ *= scale;
  x_ *= scale;
  y_ *= scale;
  z_ *= scale;
  return *this;
}


QuatD& QuatD::invert(const QuatD& q)
{
  s_ = -q.s_;
  x_ =	q.x_;
  y_ =	q.y_;
  z_ =	q.z_;
  return *this;
}


QuatD& QuatD::invert()
{
  s_ = -s_;
  return *this;
}

QuatD& QuatD::operator+= (const QuatD& q)
{
	s_ += q.s_;
	x_ += q.x_;
	y_ += q.y_;
	z_ += q.z_;
	return *this;
}
QuatD& QuatD::operator-= (const QuatD& q)
{
	s_ -= q.s_;
	x_ -= q.x_;
	y_ -= q.y_;
	z_ -= q.z_;
	return *this;
}
QuatD QuatD::operator+ (const QuatD& q) const
{
	return QuatD(s_ + q.s_,x_ + q.x_,y_ + q.y_,z_ + q.z_);
}
QuatD QuatD::operator- (const QuatD& q) const
{
	return QuatD(s_ - q.s_,x_ - q.x_,y_ - q.y_,z_ - q.z_);
}

double QuatD::dot (const QuatD& q) const
{
	return s_*q.s_ + x_*q.x_ + y_*q.y_ + z_*q.z_;
}


//  Scalar operations  /////////////////
QuatD& QuatD::operator*= (double w)
{
	s_ *= w;
	x_ *= w;
	y_ *= w;
	z_ *= w;
	return *this;
}
QuatD& QuatD::operator/= (double w)
{
	w = 1/w;
	s_ *= w;
	x_ *= w;
	y_ *= w;
	z_ *= w;
	return *this;
}
QuatD QuatD::operator* (double w) const
{
	return QuatD(s_*w,x_*w,y_*w,z_*w);
}
QuatD QuatD::operator/ (double w) const
{
	w = 1/w;
	return QuatD(s_*w,x_*w,y_*w,z_*w);
}
QuatD operator* (double w,const QuatD& q)
{
	return QuatD(q.s_*w,q.x_*w,q.y_*w,q.z_*w);
}

xm_inline void QuatD::slerp(const QuatD& a,const QuatD& b, double t)
{ 
	// Slerp(q1,q2,t) = (sin((1-t)*A)/sin(A))*q1+(sin(t*A)/sin(A))*q2 
        double scale0, scale1;

        // calc cosine
        double cosom = a.dot(b);

        // adjust signs (if necessary)
        if(cosom < 0.0){ 
		cosom = -cosom; 
		negate(b);
		}
	else
		*this = b;

        // calculate coefficients
       if ((1.0 - cosom) > 1e-5f ) {
                // standard case (slerp)
                double omega = acos(cosom);
                double sinom = sin(omega);
                scale0 = sin((1.0 - t) * omega) / sinom;
                scale1 = sin(t * omega) / sinom;
		}
        else {	// "from" and "to" quaternions are very close, so we can do a linear interpolation
                scale0 = 1.0 - t;
                scale1 = t;
		}

        // calculate final values
	*this *= scale1;
	*this += a*scale0;
}



///////////////////////////////////////////////////////////////////////////////
//		Se3f xm_inline definitions
///////////////////////////////////////////////////////////////////////////////
Se3f::operator Se3d () const 
{ 
	return Se3d(q, d); 
}

Se3f& Se3f::mult(const Se3f& T, const Se3f& U)
{
  q.mult(T.q, U.q);
  T.q.xform(U.d, d);
  d.add(d, T.d);
  return *this;
}


Se3f& Se3f::premult(const Se3f& T)
{
  q.premult(T.q);
  T.q.xform(d);
  d.add(T.d);
  return *this;
}


Se3f& Se3f::postmult(const Se3f& T)
{
  Vect3f v;

  q.xform(T.d, v);
  d.add(v);
  q.postmult(T.q);
  return *this;
}


Se3f& Se3f::invert(const Se3f& T)
{
  q.s_ = -T.q.s_;
  q.x_ =  T.q.x_;
  q.y_ =  T.q.y_;
  q.z_ =  T.q.z_;
  q.xform(T.d, d);
  d.negate(d);
  return *this;
}


Se3f& Se3f::invert()
{
  q.s_ = -q.s_;
  q.xform(d);
  d.negate();
  return *this;
}


Vect3f& Se3f::xformVect(const Vect3f& v, Vect3f& xv) const
{
  q.xform(v, xv);
  return xv;
}


Vect3f& Se3f::xformVect(Vect3f& v) const
{
  q.xform(v);
  return v;
}


Vect3f& Se3f::xformPoint(const Vect3f& p, Vect3f& xp) const
{
  q.xform(p, xp);
  xp.add(d);
  return xp;
}


Vect3f& Se3f::xformPoint(Vect3f& p) const
{
  q.xform(p);
  p.add(d);
  return p;
}


Vect3f& Se3f::invXformVect(const Vect3f& v, Vect3f& xv) const
{
  q.invXform(v, xv);
  return xv;
}


Vect3f& Se3f::invXformVect(Vect3f& v) const
{
  q.invXform(v);
  return v;
}


Vect3f& Se3f::invXformPoint(const Vect3f& p, Vect3f& xp) const
{
  xp.sub(p, d);
  q.invXform(xp);
  return xp;
}


Vect3f& Se3f::invXformPoint(Vect3f& p) const
{
  p.sub(d);
  q.invXform(p);
  return p;
}

bool Se3f::eq(const Se3f& other, float transDelta, float rotDelta) const
{
	return trans().eq(other.trans(), transDelta) && rot().eq(other.rot(), rotDelta);
}


///////////////////////////////////////////////////////////////////////////////
//		Se3d xm_inline definitions
///////////////////////////////////////////////////////////////////////////////
Se3d::operator Se3f () const 
{ 
	return Se3f(q, d); 
}

Se3d& Se3d::mult(const Se3d& T, const Se3d& U)
{
  q.mult(T.q, U.q);
  T.q.xform(U.d, d);
  d.add(d, T.d);
  return *this;
}


Se3d& Se3d::premult(const Se3d& T)
{
  q.premult(T.q);
  T.q.xform(d);
  d.add(T.d);
  return *this;
}


Se3d& Se3d::postmult(const Se3d& T)
{
  Vect3d v;

  q.xform(T.d, v);
  d.add(v);
  q.postmult(T.q);
  return *this;
}


Se3d& Se3d::invert(const Se3d& T)
{
  q.s_ = -T.q.s_;
  q.x_ =  T.q.x_;
  q.y_ =  T.q.y_;
  q.z_ =  T.q.z_;
  q.xform(T.d, d);
  d.negate(d);
  return *this;
}


Se3d& Se3d::invert()
{
  q.s_ = -q.s_;
  q.xform(d);
  d.negate();
  return *this;
}


Vect3d& Se3d::xformVect(const Vect3d& v, Vect3d& xv) const
{
  q.xform(v, xv);
  return xv;
}


Vect3d& Se3d::xformVect(Vect3d& v) const
{
  q.xform(v);
  return v;
}


Vect3d& Se3d::xformPoint(const Vect3d& p, Vect3d& xp) const
{
  q.xform(p, xp);
  xp.add(d);
  return xp;
}


Vect3d& Se3d::xformPoint(Vect3d& p) const
{
  q.xform(p);
  p.add(d);
  return p;
}


Vect3d& Se3d::invXformVect(const Vect3d& v, Vect3d& xv) const
{
  q.invXform(v, xv);
  return xv;
}


Vect3d& Se3d::invXformVect(Vect3d& v) const
{
  q.invXform(v);
  return v;
}


Vect3d& Se3d::invXformPoint(const Vect3d& p, Vect3d& xp) const
{
  xp.sub(p, d);
  q.invXform(xp);
  return xp;
}


Vect3d& Se3d::invXformPoint(Vect3d& p) const
{
  p.sub(d);
  q.invXform(p);
  return p;
}

bool Se3d::eq(const Se3d& other, double transDelta, double rotDelta) const
{
	return trans().eq(other.trans(), transDelta) && rot().eq(other.rot(), rotDelta);
}


///////////////////////////////////////////////////////////////////////////////
//
//		Vect4f xm_inline definitions
//
///////////////////////////////////////////////////////////////////////////////

bool Vect4f::eq(const Vect4f& other, float delta) const
{
  return fabs(x - other.x) < delta && 
	    fabs(y - other.y) < delta && 
	    fabs(z - other.z) < delta && 
	    fabs(w - other.w) < delta;
}

void Vect4f::swap(Vect4f& other)
{
  Vect4f tmp;

  tmp = *this;
  *this = other;
  other = tmp;
}

Vect4f& Vect4f::add(const Vect4f& u, const Vect4f& v)
{
  x = u.x + v.x;
  y = u.y + v.y;
  z = u.z + v.z;
  w = u.w + v.w;
  return *this;
}


Vect4f& Vect4f::add(const Vect4f& v)
{
  x += v.x;
  y += v.y;
  z += v.z;
  w += v.w;
  return *this;
}


Vect4f& Vect4f::sub(const Vect4f& u, const Vect4f& v)
{
  x = u.x - v.x;
  y = u.y - v.y;
  z = u.z - v.z;
  w = u.w - v.w;
  return *this;
}


Vect4f& Vect4f::sub(const Vect4f& v)
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
  w -= v.w;
  return *this;
}


Vect4f& Vect4f::mult(const Vect4f& u, const Vect4f& v)
{
  x = u.x * v.x;
  y = u.y * v.y;
  z = u.z * v.z;
  w = u.w * v.w;
  return *this;
}


Vect4f& Vect4f::mult(const Vect4f& v)
{
  x *= v.x;
  y *= v.y;
  z *= v.z;
  w *= v.w;
  return *this;
}

Vect4f& Vect4f::div(const Vect4f& u, const Vect4f& v)
{
  x = u.x / v.x;
  y = u.y / v.y;
  z = u.z / v.z;
  w = u.w / v.w;
  return *this;
}


Vect4f& Vect4f::div(const Vect4f& v)
{
  x /= v.x;
  y /= v.y;
  z /= v.z;
  w /= v.w;
  return *this;
}



Vect4f& Vect4f::scale(const Vect4f& v, float s)
{
  x = s * v.x;
  y = s * v.y;
  z = s * v.z;
  w = s * v.w;
  return *this;
}


Vect4f& Vect4f::scale(float s)
{
  x *= s;
  y *= s;
  z *= s;
  w *= s;
  return *this;
}


//////////////////////////////////////////////////////////////////////////////////
//
//  	     Stream's I/O operations 
//
//////////////////////////////////////////////////////////////////////////////////
#ifdef _XMATH_USE_IOSTREAM

//  Vect2s  I/O //////////////////////////
inline ostream& operator<<(ostream &os, const Vect2i& v)
{
	os << v.x << "  " << v.y;
	return os;
}

inline istream& operator>>(istream &is, Vect2i& v)
{
	is >> v.x >> v.y;
	return is;
}

//  Vect2s  I/O //////////////////////////
inline ostream& operator<<(ostream &os, const Vect2f& v)
{
	os << v.x << "  " << v.y;
	return os;
}

inline istream& operator>>(istream &is, Vect2f& v)
{
	is >> v.x >> v.y;
	return is;
}


//  Vect2s  I/O //////////////////////////
inline ostream& operator<<(ostream &os, const Vect2s& v)
{
	os << v.x << "  " << v.y;
	return os;
}

inline istream& operator>>(istream &is, Vect2s& v)
{
	is >> v.x >> v.y;
	return is;
}


//  Vect3d  I/O //////////////////////////
inline ostream& operator<<(ostream &os, const Vect3d& v)
{
	os << v.x << "  " << v.y << "  " << v.z;
	return os;
}

inline istream& operator>>(istream &is, Vect3d& v)
{
	is >> v.x >> v.y >> v.z;
	return is;
}


//  Vect3f  I/O //////////////////////////
inline ostream& operator<<(ostream &os, const Vect3f& v)
{
	os << v.x << "  " << v.y << "  " << v.z;
	return os;
}

inline istream& operator>>(istream &is, Vect3f& v)
{
	is >> v.x >> v.y >> v.z;
	return is;
}

//  Vect4f  I/O //////////////////////////
inline ostream& operator<<(ostream &os, const Vect4f& v)
{
	os << v.x << "  " << v.y << "  " << v.z << "  " << v.w;
	return os;
}

inline istream& operator>>(istream &is, Vect4f& v)
{
	is >> v.x >> v.y >> v.z >> v.w;
	return is;
}


//  Mat3d I/O  ////////////////////////
inline ostream& operator<<(ostream& os, const Mat3d& m)
{
  os << m.xx << " \t" << m.xy << " \t" << m.xz << std::endl;
  os << m.yx << " \t" << m.yy << " \t" << m.yz << std::endl;
  os << m.zx << " \t" << m.zy << " \t" << m.zz;
  return os;
}

inline istream& operator>>(istream &is, Mat3d& m)
{
  is >> m.xx >> m.xy >> m.xz;
  is >> m.yx >> m.yy >> m.yz;
  is >> m.zx >> m.zy >> m.zz;
  return is;
}

//  Mat3f I/O  ////////////////////////
inline ostream& operator<<(ostream& os, const Mat3f& m)
{
  os << m.xx << " \t" << m.xy << " \t" << m.xz << std::endl;
  os << m.yx << " \t" << m.yy << " \t" << m.yz << std::endl;
  os << m.zx << " \t" << m.zy << " \t" << m.zz;
  return os;
}

inline istream& operator>>(istream &is, Mat3f& m)
{
  is >> m.xx >> m.xy >> m.xz;
  is >> m.yx >> m.yy >> m.yz;
  is >> m.zx >> m.zy >> m.zz;
  return is;
}


//  MatXd  I/O   ///////////////////
inline ostream& operator<<(ostream& os, const MatXd& m)
{
  return os << m.R << "  " << m.d;
}

inline istream& operator>>(istream& is, MatXd& m)
{
  Se3d T;
  is >> T;
  m.set(T);
  return is;
}


//  MatXf  I/O   ///////////////////
inline ostream& operator<<(ostream& os, const MatXf& m)
{
  return os << m.R << "  " << m.d;
}

inline istream& operator>>(istream& is, MatXf& m)
{
  Se3d T;
  is >> T;
  m.set(T);
  return is;
}


//  QuatD  I/O   ///////////////////
inline istream& operator>>(istream& is, QuatD& q)
{
  is >> q.s_ >> q.x_ >> q.y_ >> q.z_;
  return is;
}

inline ostream& operator<<(ostream& os, const QuatD& q)
{
  os << q.s_ << "  " << q.x_ << "  " << q.y_ << "  " << q.z_;
  return os;
}

//  QuatF  I/O   ///////////////////
inline istream& operator>>(istream& is, QuatF& q)
{
  is >> q.s_ >> q.x_ >> q.y_ >> q.z_;
  return is;
}

inline ostream& operator<<(ostream& os, const QuatF& q)
{
  os << q.s_ << "  " << q.x_ << "  " << q.y_ << "  " << q.z_;
  return os;
}


//  Se3d I/O  ///////////////////
inline ostream& operator<<(ostream& os, const Se3d& se3)
{
  return os << se3.q << "  " << se3.d;
}

inline istream& operator>>(istream& is, Se3d& se3)
{
	return is >> se3.q >> se3.d;
}

//  Se3f I/O  ///////////////////
inline ostream& operator<<(ostream& os, const Se3f& se3)
{
  return os << se3.q << "  " << se3.d;
}

inline istream& operator>>(istream& is, Se3f& se3)
{
	return is >> se3.q >> se3.d;
}


#endif  // _XMATH_NO_IOSTREAM

//#ifdef _DEBUG
//#pragma comment(lib,"XMathD.lib")
//#else
//#pragma comment(lib,"XMath.lib")
//#endif

#endif // __XMATH_H__
