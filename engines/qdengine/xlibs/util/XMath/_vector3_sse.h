#ifndef _VECTOR3_SSE_H
#define _VECTOR3_SSE_H
//------------------------------------------------------------------------------
/**
    @class _vector3_sse
    @ingroup Math

    SSE based vector3 class.

    (C) 2002 RadonLabs GmbH
*/
#include <xmmintrin.h>

#ifdef _USE_MATH_
#include <math.h>
#endif _USE_MATH_

//------------------------------------------------------------------------------
class _vector3_sse
{
public:
    /// constructor 1
    _vector3_sse();
    /// constructor 2
    _vector3_sse(const float _x, const float _y, const float _z);
    /// constructor 3
    _vector3_sse(const _vector3_sse& vec);
    /// constructor 4
    _vector3_sse(const float* p);
    /// private constructor
    _vector3_sse(const __m128& m);
    /// set elements 1
    void set(const float _x, const float _y, const float _z);
    /// set elements 2 
    void set(const _vector3_sse& vec);
    /// set elements 3
    void set(const float* p);
    /// return length
    float len() const;
    /// return length squared
    float lensquared() const;
    /// normalize
    void norm();
    /// inplace add
    void operator +=(const _vector3_sse& v0);
    /// inplace sub
    void operator -=(const _vector3_sse& v0);
    /// inplace scalar multiplication
    void operator *=(float s);
    /// fuzzy compare
    bool isequal(const _vector3_sse& v, float tol) const;
    /// fuzzy compare, returns -1, 0, +1
    int compare(const _vector3_sse& v, float tol) const;
    /// rotate around axis
    void rotate(const _vector3_sse& axis, float angle);
    /// inplace linear interpolation
    void lerp(const _vector3_sse& v0, float lerpVal);
    /// returns a vector orthogonal to self, not normalized
    _vector3_sse findortho() const;

	operator  __m128() const	{ return m128; }
    union
    {
        __m128 m128;
        struct
        {
            float x, y, z, pad;
        };
    };
};

//------------------------------------------------------------------------------
/**
*/
inline
_vector3_sse::_vector3_sse()
{
    m128 = _mm_setzero_ps();
}

//------------------------------------------------------------------------------
/**
*/
inline
_vector3_sse::_vector3_sse(const float _x, const float _y, const float _z)
{
    m128 = _mm_set_ps(0.0f, _z, _y, _x);
}

//------------------------------------------------------------------------------
/**
*/
inline
_vector3_sse::_vector3_sse(const _vector3_sse& vec)
{
    m128 = vec.m128;
}

//------------------------------------------------------------------------------
/**
*/
inline
_vector3_sse::_vector3_sse(const __m128& m)
{
    m128 = m;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3_sse::set(const float _x, const float _y, const float _z)
{
    m128 = _mm_set_ps(0.0f, _z, _y, _x);
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3_sse::set(const _vector3_sse& vec)
{
    m128 = vec.m128;
}

//------------------------------------------------------------------------------
/**
*/
inline
float
_vector3_sse::len() const
{
    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;
    static const int W = 3;

    __m128 a = _mm_mul_ps(m128, m128);

    // horizontal add
    __m128 b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(X,X,X,X)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(Y,Y,Y,Y)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(Z,Z,Z,Z))));
    __m128 l = _mm_sqrt_ss(b);
    return *(float*)&l;
}

//------------------------------------------------------------------------------
/**
*/
inline
float 
_vector3_sse::lensquared() const
{
    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;
    static const int W = 3;

    __m128 a = _mm_mul_ps(m128, m128);
    __m128 b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(X,X,X,X)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(Y,Y,Y,Y)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(Z,Z,Z,Z))));
    return *(float*)&b;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3_sse::norm()
{
    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;
    static const int W = 3;

    __m128 a = _mm_mul_ps(m128, m128);

    // horizontal add
    __m128 b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(X,X,X,X)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(Y,Y,Y,Y)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(Z,Z,Z,Z))));

    // get reciprocal of square root of squared length
    __m128 f = _mm_rsqrt_ss(b);
    __m128 oneDivLen = _mm_shuffle_ps(f, f, _MM_SHUFFLE(X,X,X,X));
    
    m128 = _mm_mul_ps(m128, oneDivLen);
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3_sse::operator +=(const _vector3_sse& v)
{
    m128 = _mm_add_ps(m128, v.m128);
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3_sse::operator -=(const _vector3_sse& v)
{
    m128 = _mm_sub_ps(m128, v.m128);
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3_sse::operator *=(float s)
{
    __m128 packed = _mm_set1_ps(s);
    m128 = _mm_mul_ps(m128, packed);
}

//------------------------------------------------------------------------------
/**
*/
#ifdef _USE_MATH_
inline
bool
_vector3_sse::isequal(const _vector3_sse& v, float tol) const
{
    if (fabs(v.x - x) > tol)      return false;
    else if (fabs(v.y - y) > tol) return false;
    else if (fabs(v.z - z) > tol) return false;
    return true;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
_vector3_sse::compare(const _vector3_sse& v, float tol) const
{
    if (fabs(v.x - x) > tol)      return (v.x > x) ? +1 : -1; 
    else if (fabs(v.y - y) > tol) return (v.y > y) ? +1 : -1;
    else if (fabs(v.z - z) > tol) return (v.z > z) ? +1 : -1;
    else                          return 0;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3_sse::rotate(const _vector3_sse& axis, float angle)
{
    // rotates this one around given vector. We do
    // rotation with matrices, but these aren't defined yet!
    float rotM[9];
    float sa, ca;

    sa = (float) sin(angle);
    ca = (float) cos(angle);

    // build a rotation matrix
    rotM[0] = ca + (1 - ca) * axis.x * axis.x;
    rotM[1] = (1 - ca) * axis.x * axis.y - sa * axis.z;
    rotM[2] = (1 - ca) * axis.z * axis.x + sa * axis.y;
    rotM[3] = (1 - ca) * axis.x * axis.y + sa * axis.z;
    rotM[4] = ca + (1 - ca) * axis.y * axis.y;
    rotM[5] = (1 - ca) * axis.y * axis.z - sa * axis.x;
    rotM[6] = (1 - ca) * axis.z * axis.x - sa * axis.y;
    rotM[7] = (1 - ca) * axis.y * axis.z + sa * axis.x;
    rotM[8] = ca + (1 - ca) * axis.z * axis.z;

    // "handmade" multiplication
    _vector3_sse help(rotM[0] * this->x + rotM[1] * this->y + rotM[2] * this->z,
                      rotM[3] * this->x + rotM[4] * this->y + rotM[5] * this->z,
                      rotM[6] * this->x + rotM[7] * this->y + rotM[8] * this->z);
    *this = help;
}
#endif _USE_MATH_
//------------------------------------------------------------------------------
/**
*/
static 
inline 
_vector3_sse operator +(const _vector3_sse& v0, const _vector3_sse& v1) 
{
    return _vector3_sse(_mm_add_ps(v0.m128, v1.m128));
}

//------------------------------------------------------------------------------
/**
*/
static 
inline 
_vector3_sse operator -(const _vector3_sse& v0, const _vector3_sse& v1) 
{
    return _vector3_sse(_mm_sub_ps(v0.m128, v1.m128));
}

//------------------------------------------------------------------------------
/**
*/
static 
inline 
_vector3_sse operator *(const _vector3_sse& v0, const float s) 
{
    __m128 packed = _mm_set1_ps(s);
    return _vector3_sse(_mm_mul_ps(v0.m128, packed));
}

//------------------------------------------------------------------------------
/**
*/
static 
inline 
_vector3_sse operator -(const _vector3_sse& v) 
{
    __m128 zero = _mm_setzero_ps();
    return _vector3_sse(_mm_sub_ps(zero, v.m128));
}

//------------------------------------------------------------------------------
/**
    Dot product.
*/
static
inline
float dot(const _vector3_sse& v0, const _vector3_sse& v1)
{
    __m128 a = _mm_mul_ps(v0.m128, v1.m128);
    __m128 b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0,0,0,0)), _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(1,1,1,1)), _mm_shuffle_ps(a, a, _MM_SHUFFLE(2,2,2,2))));
    return *(float*)&b;
}

//------------------------------------------------------------------------------
/**
    Cross product.
*/
static 
inline 
_vector3_sse cross(const _vector3_sse& v0, const _vector3_sse& v1) 
{
    // x = v0.y * v1.z - v0.z * v1.y
    // y = v0.z * v1.x - v0.x * v1.z
    // z = v0.x * v1.y - v0.y * v1.x
    //
    // a = v0.y | v0.z | v0.x | xxx
    // b = v1.z | v1.x | v1.y | xxx
    // c = v0.z | v0.x | v0.y | xxx
    // d = v1.y | v1.z | v1.x | xxx
    //

    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;
    static const int W = 3;

    __m128 a = _mm_shuffle_ps(v0.m128, v0.m128, _MM_SHUFFLE(W, X, Z, Y));
    __m128 b = _mm_shuffle_ps(v1.m128, v1.m128, _MM_SHUFFLE(W, Y, X, Z));
    __m128 c = _mm_shuffle_ps(v0.m128, v0.m128, _MM_SHUFFLE(W, Y, X, Z));
    __m128 d = _mm_shuffle_ps(v1.m128, v1.m128, _MM_SHUFFLE(W, X, Z, Y));

    __m128 e = _mm_mul_ps(a, b);
    __m128 f = _mm_mul_ps(c, d);

    return _vector3_sse(_mm_sub_ps(e, f));
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3_sse::lerp(const _vector3_sse& v0, float lerpVal)
{
    x = v0.x + ((x - v0.x) * lerpVal);
    y = v0.y + ((y - v0.y) * lerpVal);
    z = v0.z + ((z - v0.z) * lerpVal);
}

//------------------------------------------------------------------------------
/**
    Find a vector that is orthogonal to self. Self should not be (0,0,0).
    Return value is not normalized.
*/
inline
_vector3_sse
_vector3_sse::findortho() const
{
    if (0.0 != x)
    {
        return _vector3_sse((-y - z) / x, 1.0, 1.0);
    } else
    if (0.0 != y)
    {
        return _vector3_sse(1.0, (-x - z) / y, 1.0);
    } else
    if (0.0 != z)
    {
        return _vector3_sse(1.0, 1.0, (-x - y) / z);
    } else
    {
        return _vector3_sse(0.0, 0.0, 0.0);
    }
}

//------------------------------------------------------------------------------
#endif
