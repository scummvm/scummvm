#pragma once
/*

Для MatXf, Mat3f, QuatF:

v' = Mn*...*M3*M2*M1*v

Для Mat4f или D3DXMATRIX:

v' = v*M1*M2*M3*...*Mn

*/

#include "XMath\xmath.h"

struct D3DXMATRIX;

class Mat4f 
{
public:
	float _11,_12,_13,_14;
	float _21,_22,_23,_24; 
	float _31,_32,_33,_34;
	float _41,_42,_43,_44;

	Mat4f(){}
	Mat4f(float m11,float m12,float m13,float m14,
			float m21,float m22,float m23,float m24,
			float m31,float m32,float m33,float m34,
			float m41,float m42,float m43,float m44);

	Mat4f(const MatXf& X);

	void set(	
			float m11,float m12,float m13,float m14,
			float m21,float m22,float m23,float m24,
			float m31,float m32,float m33,float m34,
			float m41,float m42,float m43,float m44);

	void setTranslation(const Vect3f& trans); // Матрица трансляции
	void setOrthoOffCenterLH(float l, float r, float b, float t, float zn, float zf);

	Mat4f operator*(const Mat4f &m) const;
	
	void transpose();

	void xformVect(const Vect3f& in,Vect3f& out) const; // [in, 0]*this
	void invXformVect(const Vect3f& in,Vect3f& out) const;

	void xformPoint(const Vect3f& in,Vect3f& out) const; // [in, 1]*this
	void invXformPoint(const Vect3f& in,Vect3f& out) const;
	void xformPoint(const Vect4f& in,Vect4f& out) const; // [in]*this

	void xformCoord(const Vect3f& in,Vect3f& pv,Vect3f& pe) const; // [in, 1]*this, /= w 
	void xformCoord(const Vect3f& in,Vect3f& out) const { Vect3f pv; xformCoord(in, pv, out); }
	void xformCoord(Vect3f& v) const { Vect3f in = v; Vect3f pv; xformCoord(in, pv, v); }

	void xformCoord(const Vect3f& in,Vect4f &out) const; // [in]*this, /= w 
	
	Vect3f&  trans()						{ return *(Vect3f*)&_41; }
	Vect3f&  xrow()							{ return *(Vect3f*)&_11; }
	Vect3f&  yrow()							{ return *(Vect3f*)&_21; }
	Vect3f&  zrow()							{ return *(Vect3f*)&_31; }
	Vect4f&  row(int n)						{ return ((Vect4f*)this)[n]; }

	Vect4f col(int axis) const { return Vect4f(*(&_11 + axis), *(&_21 + axis), *(&_31 + axis), *(&_41 + axis)); }
	void setCol(int axis, const Vect4f& column);

	void serialize(Archive& ar);

	// index-based access:  0=xrow, 1=yrow, 2=zrow, 3=wrow.
	const Vect4f& operator[](int i) const {return *(((Vect4f *)this) + i);}
	Vect4f& operator[](int i)       {return *(((Vect4f *)this) + i);}

	operator D3DXMATRIX*() { return (D3DXMATRIX*)this; }
	operator const float*() const { return (const float*)this; }

	static Mat4f ID;
	static Mat4f ZERO;
};
