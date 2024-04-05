#include "stdafx.h"
#include "Mat4f.h"
#include "XMath\SafeMath.h"

#pragma warning(disable : 4073 )
#pragma init_seg(lib)

Mat4f Mat4f::ID(1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1);

Mat4f Mat4f::ZERO(0, 0, 0, 0,
					  0, 0, 0, 0,
					  0, 0, 0, 0,
					  0, 0, 0, 0);

Mat4f::Mat4f(float m11,float m12,float m13,float m14,  float m21,float m22,float m23,float m24,  float m31,float m32,float m33,float m34,  float m41,float m42,float m43,float m44) 
{
	_11=m11,_12=m12,_13=m13,_14=m14; _21=m21,_22=m22,_23=m23,_24=m24; _31=m31,_32=m32,_33=m33,_34=m34; _41=m41,_42=m42,_43=m43,_44=m44;
}

Mat4f::Mat4f(const MatXf& X)
{
	const Mat3f& rot = X.rot();
	const Vect3f& trans = X.trans();
	_11 = rot.xx; _12 = rot.yx; _13 = rot.zx; _14 = 0;
	_21 = rot.xy; _22 = rot.yy; _23 = rot.zy; _24 = 0;
	_31 = rot.xz; _32 = rot.yz; _33 = rot.zz; _34 = 0;
	_41 = trans.x; _42 = trans.y; _43 = trans.z; _44 = 1.0f;
}


void Mat4f::set(float m11,float m12,float m13,float m14,  float m21,float m22,float m23,float m24,  float m31,float m32,float m33,float m34,  float m41,float m42,float m43,float m44) 
{
	_11=m11,_12=m12,_13=m13,_14=m14; _21=m21,_22=m22,_23=m23,_24=m24; _31=m31,_32=m32,_33=m33,_34=m34; _41=m41,_42=m42,_43=m43,_44=m44;
}

void Mat4f::setTranslation(const Vect3f& trans)
{
	_11 = 1; _12 = 0; _13 = 0; _14 = 0;
	_21 = 0; _22 = 1; _23 = 0; _24 = 0;
	_31 = 0; _32 = 0; _33 = 1; _34 = 0;
	_41 = trans.x; _42 = trans.y; _43 = trans.z; _44 = 1.0f;

}

void Mat4f::setOrthoOffCenterLH(float l, float r, float b, float t, float zn, float zf)
{
	set(
		2/(r-l),      0,            0,           0,
		0,            2/(t-b),      0,           0,
		0,            0,            1/(zf-zn),   0,
		(l+r)/(l-r),  (t+b)/(b-t),  zn/(zn-zf),  l
	);
}


Mat4f Mat4f::operator*(const Mat4f &m) const
{
	return Mat4f( 
		_11*m._11+_12*m._21+_13*m._31+_14*m._41, _11*m._12+_12*m._22+_13*m._32+_14*m._42, _11*m._13+_12*m._23+_13*m._33+_14*m._43, _11*m._14+_12*m._24+_13*m._34+_14*m._44,
		_21*m._11+_22*m._21+_23*m._31+_24*m._41, _21*m._12+_22*m._22+_23*m._32+_24*m._42, _21*m._13+_22*m._23+_23*m._33+_24*m._43, _21*m._14+_22*m._24+_23*m._34+_24*m._44,
		_31*m._11+_32*m._21+_33*m._31+_34*m._41, _31*m._12+_32*m._22+_33*m._32+_34*m._42, _31*m._13+_32*m._23+_33*m._33+_34*m._43, _31*m._14+_32*m._24+_33*m._34+_34*m._44,
		_41*m._11+_42*m._21+_43*m._31+_44*m._41, _41*m._12+_42*m._22+_43*m._32+_44*m._42, _41*m._13+_42*m._23+_43*m._33+_44*m._43, _41*m._14+_42*m._24+_43*m._34+_44*m._44);
}

void Mat4f::transpose() 
{
	swap(_12, _21);
	swap(_13, _31); 
	swap(_14, _41);
	swap(_23, _32); 
	swap(_24, _42);
	swap(_34, _43);
}

void Mat4f::xformPoint(const Vect3f& in,Vect3f& out) const
{
	out.x=_11*in.x+_21*in.y+_31*in.z+_41;
	out.y=_12*in.x+_22*in.y+_32*in.z+_42;
	out.z=_13*in.x+_23*in.y+_33*in.z+_43;
}

void Mat4f::xformPoint(const Vect4f& in,Vect4f& out) const
{
	out.x=_11*in.x+_21*in.y+_31*in.z+_41;
	out.y=_12*in.x+_22*in.y+_32*in.z+_42;
	out.z=_13*in.x+_23*in.y+_33*in.z+_43;
	out.w=_14*in.x+_24*in.y+_34*in.z+_44;
}

void Mat4f::xformVect(const Vect3f& in,Vect3f& out) const
{
	out.x=_11*in.x+_21*in.y+_31*in.z;
	out.y=_12*in.x+_22*in.y+_32*in.z;
	out.z=_13*in.x+_23*in.y+_33*in.z;
}

void Mat4f::invXformPoint(const Vect3f& in,Vect3f& out) const
{
	out.x=_11*(in.x-_41)+_12*(in.y-_42)+_13*(in.z-_43);
	out.y=_21*(in.x-_41)+_22*(in.y-_42)+_23*(in.z-_43);
	out.z=_31*(in.x-_41)+_32*(in.y-_42)+_33*(in.z-_43);
}

void Mat4f::invXformVect(const Vect3f& in,Vect3f& out) const
{
	out.x=_11*in.x+_12*in.y+_13*in.z;
	out.y=_21*in.x+_22*in.y+_23*in.z;
	out.z=_31*in.x+_32*in.y+_33*in.z;
}

void Mat4f::xformCoord(const Vect3f& in,Vect3f& pv,Vect3f& pe) const
{
	pv.x = _11*in.x+_21*in.y+_31*in.z+_41;
	pv.y = _12*in.x+_22*in.y+_32*in.z+_42;
	pv.z = _13*in.x+_23*in.y+_33*in.z+_43;
	float w = _14*in.x+_24*in.y+_34*in.z+_44;
	if(isEq(w, 1.f))
		pe = pv;
	else if(fabsf(w) > 1e-5f)
		pe.scale(pv, 1.f/w);
	else
		pe = Vect3f::ZERO;
}

void Mat4f::xformCoord(const Vect3f& in,Vect4f &out) const
{
	out.x=_11*in.x+_21*in.y+_31*in.z+_41;
	out.y=_12*in.x+_22*in.y+_32*in.z+_42;
	out.z=_13*in.x+_23*in.y+_33*in.z+_43;
	out.w=_14*in.x+_24*in.y+_34*in.z+_44;
	float d=out.w;
	if(fabsf(d)<1e-5f) 
		d=1e5f; 
	else 
		d=1/d;
	out.x=out.x*d;
	out.y=out.y*d;
	out.z=out.z*d;
	out.w=d;
}

void Mat4f::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
	ar.serialize(row(0), "xrow", "xrow");
	ar.serialize(row(1), "yrow", "yrow");
	ar.serialize(row(2), "zrow", "zrow");
	ar.serialize(row(3), "wrow", "wrow");
}

void Mat4f::setCol(int axis, const Vect4f& column)
{
	*(&_11 + axis) = column.x;
	*(&_21 + axis) = column.y;
	*(&_31 + axis) = column.z;
	*(&_41 + axis) = column.w;
}