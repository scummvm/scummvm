////////////////////////////////////////////////////////////////////////////////
//	XMath 1.0
//	Vector - Matrix library for 3D visualisation and
//	physics modeling.
//
//	(C) 1999 KDV Games
//	Author: Alexandre Kotliar
//  
//
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "XMath\xmath.h"
#include "XUtil.h"

#pragma warning(disable : 4073 )
#pragma init_seg(lib)

///////////////////////////////////////////////////////////////////////////////
//
//	Constants
//
///////////////////////////////////////////////////////////////////////////////

const Vect2f Vect2f::ZERO(0, 0);
const Vect2f Vect2f::ID(1, 1);

const Vect2i Vect2i::ZERO(0, 0);
const Vect2i Vect2i::ID(1, 1);

const Vect3d Vect3d::ZERO(0, 0, 0);
const Vect3d Vect3d::I   (1, 0, 0);
const Vect3d Vect3d::J   (0, 1, 0);
const Vect3d Vect3d::K   (0, 0, 1);
const Vect3d Vect3d::I_  (-1,  0,  0);
const Vect3d Vect3d::J_  ( 0, -1,  0);
const Vect3d Vect3d::K_  ( 0,  0, -1);
const Vect3d Vect3d::ID  ( 1,  1, 1);

const Vect3f Vect3f::ZERO(0, 0, 0);
const Vect3f Vect3f::I   (1, 0, 0);
const Vect3f Vect3f::J   (0, 1, 0);
const Vect3f Vect3f::K   (0, 0, 1);
const Vect3f Vect3f::I_  (-1,  0,  0);
const Vect3f Vect3f::J_  ( 0, -1,  0);
const Vect3f Vect3f::K_  ( 0,  0, -1);
const Vect3f Vect3f::ID  ( 1,  1, 1);

const Mat3d Mat3d::ZERO (Vect3d::ZERO, Vect3d::ZERO);
const Mat3d Mat3d::ID   (Vect3d(1, 1, 1), Vect3d::ZERO);

const Mat3f Mat3f::ZERO (Vect3f::ZERO, Vect3f::ZERO);
const Mat3f Mat3f::ID   (Vect3f(1, 1, 1), Vect3f::ZERO);

const MatXd MatXd::ID   (Mat3d::ID, Vect3d::ZERO);

const MatXf MatXf::ID   (Mat3f::ID, Vect3f::ZERO);

const QuatF QuatF::ID   (1, 0, 0, 0);
const QuatD QuatD::ID   (1, 0, 0, 0);

const Se3d  Se3d::ID    (QuatD::ID, Vect3d::ZERO);
const Se3f  Se3f::ID    (QuatF::ID, Vect3f::ZERO);

const Mat2f Mat2f::ID   (1, 0, 0, 1);
const MatX2f MatX2f::ID   (Mat2f::ID, Vect2f::ZERO);

const Vect4f Vect4f::ZERO(0, 0, 0, 0);
const Vect4f Vect4f::ID(1, 1, 1, 1);

RandomGenerator xm_random_generator;
int RandomGenerator::operator()(){ return ((value = value*214013L + 2531011L) >> 16) & 0x7fff; }

///////////////////////////////////////////////////////////////////////////////
//
//  class Vect2f
//
///////////////////////////////////////////////////////////////////////////////

float Vect2f::angle(const Vect2f& other) const
{
	return acosf(dot(other) * invSqrtFast(norm2() * other.norm2() * 1.0000001f));
}

///////////////////////////////////////////////////////////////////////////////
//
//  class Vect3f
//
///////////////////////////////////////////////////////////////////////////////

float Vect3f::angle(const Vect3f& other) const
{
	return acosf(dot(other) * invSqrtFast(norm2() * other.norm2() * 1.0000001f));
}

///////////////////////////////////////////////////////////////////////////////
//
//  class Mat3d
//
///////////////////////////////////////////////////////////////////////////////

Mat3d& Mat3d::set(const Vect3d& axis, double angle, int normalizeAxis)
{
  QuatD q;

  q.set(angle, axis, normalizeAxis);
  set(q);
  return *this;
}

Mat3d& Mat3d::set(const QuatD& q)
{
  xx = 2.0 * (q.s_ * q.s_ + q.x_ * q.x_ - 0.5);
  yy = 2.0 * (q.s_ * q.s_ + q.y_ * q.y_ - 0.5);
  zz = 2.0 * (q.s_ * q.s_ + q.z_ * q.z_ - 0.5);

  xy = 2.0 * (q.y_ * q.x_ - q.z_ * q.s_);
  yx = 2.0 * (q.x_ * q.y_ + q.z_ * q.s_);


  yz = 2.0 * (q.z_ * q.y_ - q.x_ * q.s_);
  zy = 2.0 * (q.y_ * q.z_ + q.x_ * q.s_);

  zx = 2.0 * (q.x_ * q.z_ - q.y_ * q.s_);
  xz = 2.0 * (q.z_ * q.x_ + q.y_ * q.s_);

  return *this;
}

Mat3d& Mat3d::set(const Vect3d& x_from, const Vect3d& y_from, const Vect3d& z_from, 
	const Vect3d& x_to, const Vect3d& y_to, const Vect3d& z_to)
{
	xx = dot(x_from, x_to);
	xy = dot(y_from, x_to);
	xz = dot(z_from, x_to);

	yx = dot(x_from, y_to);
	yy = dot(y_from, y_to);
	yz = dot(z_from, y_to);

	zx = dot(x_from, z_to);
	zy = dot(y_from, z_to);
	zz = dot(z_from, z_to);
	return *this;
}


int Mat3d::invert(const Mat3d& M)
{
  double D, oneOverDet;

  if (fabs(D = M.det()) < 1.0e-12) return 1; // not invertible
  oneOverDet = 1 / D;

  xx = (M.yy * M.zz - M.yz * M.zy) * oneOverDet;
  xy = (M.xz * M.zy - M.xy * M.zz) * oneOverDet;
  xz = (M.xy * M.yz - M.xz * M.yy) * oneOverDet;
  yx = (M.yz * M.zx - M.yx * M.zz) * oneOverDet;
  yy = (M.xx * M.zz - M.xz * M.zx) * oneOverDet;
  yz = (M.xz * M.yx - M.xx * M.yz) * oneOverDet;
  zx = (M.yx * M.zy - M.yy * M.zx) * oneOverDet;
  zy = (M.xy * M.zx - M.xx * M.zy) * oneOverDet;
  zz = (M.xx * M.yy - M.xy * M.yx) * oneOverDet;
  return 0;
}



int Mat3d::invert()
{
  double D, oneOverDet;
  double oxy, oyz, ozx, oyx, ozy, oxz, oxx, oyy;

  if (fabs(D = det()) < 1.0e-12) return 1; // not invertible
  oneOverDet = 1 / D;

  oxx = xx; oyy = yy;
  oxy = xy; oyx = yx;
  oyz = yz; ozy = zy;
  ozx = zx; oxz = xz;

  xy = (oxz * ozy - zz * oxy) * oneOverDet;
  yz = (oxz * oyx - xx * oyz) * oneOverDet;
  zx = (oyx * ozy - yy * ozx) * oneOverDet;
  yx = (oyz * ozx - oyx * zz) * oneOverDet;
  zy = (oxy * ozx - ozy * xx) * oneOverDet;
  xz = (oxy * oyz - oxz * yy) * oneOverDet;
  xx = (oyy * zz - oyz * ozy) * oneOverDet;
  yy = (oxx * zz - oxz * ozx) * oneOverDet;
  zz = (oxx * oyy - oxy * oyx) * oneOverDet;
  return 0;
}


Mat3d& Mat3d::mult(const Mat3d& M, const Mat3d& N)
{
  xx = M.xx * N.xx + M.xy * N.yx + M.xz * N.zx;
  xy = M.xx * N.xy + M.xy * N.yy + M.xz * N.zy;
  xz = M.xx * N.xz + M.xy * N.yz + M.xz * N.zz;
  yx = M.yx * N.xx + M.yy * N.yx + M.yz * N.zx;
  yy = M.yx * N.xy + M.yy * N.yy + M.yz * N.zy;
  yz = M.yx * N.xz + M.yy * N.yz + M.yz * N.zz;
  zx = M.zx * N.xx + M.zy * N.yx + M.zz * N.zx;
  zy = M.zx * N.xy + M.zy * N.yy + M.zz * N.zy;
  zz = M.zx * N.xz + M.zy * N.yz + M.zz * N.zz;

  return *this;
}


Mat3d& Mat3d::premult(const Mat3d& M)
{
  double oxy, oyz, ozx, oyx, ozy, oxz;

  oxy = xy; oyx = yx; oyz = yz; ozy = zy; ozx = zx; oxz = xz;

  xy = M.xx * oxy + M.xy * yy  + M.xz * ozy;
  xz = M.xx * oxz + M.xy * oyz + M.xz * zz;
  yx = M.yx * xx  + M.yy * oyx + M.yz * ozx;
  yz = M.yx * oxz + M.yy * oyz + M.yz * zz;
  zx = M.zx * xx  + M.zy * oyx + M.zz * ozx;
  zy = M.zx * oxy + M.zy * yy  + M.zz * ozy;

  xx = M.xx * xx  + M.xy * oyx + M.xz * ozx;
  yy = M.yx * oxy + M.yy * yy  + M.yz * ozy;
  zz = M.zx * oxz + M.zy * oyz + M.zz * zz;

  return *this;
}


Mat3d& Mat3d::postmult(const Mat3d& M)
{
  double oxy, oyz, ozx, oyx, ozy, oxz;

  oxy = xy; oyx = yx; oyz = yz; ozy = zy; ozx = zx; oxz = xz;

  xy = xx *  M.xy + oxy * M.yy + oxz * M.zy;
  xz = xx *  M.xz + oxy * M.yz + oxz * M.zz;
  yx = oyx * M.xx + yy	* M.yx + oyz * M.zx;
  yz = oyx * M.xz + yy	* M.yz + oyz * M.zz;
  zx = ozx * M.xx + ozy * M.yx + zz  * M.zx;
  zy = ozx * M.xy + ozy * M.yy + zz  * M.zy;

  xx = xx  * M.xx + oxy * M.yx + oxz * M.zx;
  yy = oyx * M.xy + yy	* M.yy + oyz * M.zy;
  zz = ozx * M.xz + ozy * M.yz + zz  * M.zz;
  
  return *this;
}



///////////////////////////////////////////////////////////////////////////////
//
//  class Mat3f
//
///////////////////////////////////////////////////////////////////////////////

Mat3f& Mat3f::set(const Vect3f& axis, float angle, int normalizeAxis)
{
  QuatF q;

  q.set(angle, axis, normalizeAxis);
  set(q);
  return *this;
}

Mat3f& Mat3f::set(const QuatF& q)
{
  xx = 2.f * (q.s_ * q.s_ + q.x_ * q.x_ - 0.5f);
  yy = 2.f * (q.s_ * q.s_ + q.y_ * q.y_ - 0.5f);
  zz = 2.f * (q.s_ * q.s_ + q.z_ * q.z_ - 0.5f);

  xy = 2.f * (q.y_ * q.x_ - q.z_ * q.s_);
  yx = 2.f * (q.x_ * q.y_ + q.z_ * q.s_);


  yz = 2.f * (q.z_ * q.y_ - q.x_ * q.s_);
  zy = 2.f * (q.y_ * q.z_ + q.x_ * q.s_);

  zx = 2.f * (q.x_ * q.z_ - q.y_ * q.s_);
  xz = 2.f * (q.z_ * q.x_ + q.y_ * q.s_);

  return *this;
}

Mat3f& Mat3f::set(const Vect3f& x_from, const Vect3f& y_from, const Vect3f& z_from, 
	const Vect3f& x_to, const Vect3f& y_to, const Vect3f& z_to)
{
	xx = dot(x_from, x_to);
	xy = dot(y_from, x_to);
	xz = dot(z_from, x_to);

	yx = dot(x_from, y_to);
	yy = dot(y_from, y_to);
	yz = dot(z_from, y_to);

	zx = dot(x_from, z_to);
	zy = dot(y_from, z_to);
	zz = dot(z_from, z_to);
	return *this;
}

int Mat3f::invert(const Mat3f& M)
{
  float D, oneOverDet;

  if (fabs(D = M.det()) < 1.0e-12) return 1; // not invertible
  oneOverDet = 1 / D;

  xx = (M.yy * M.zz - M.yz * M.zy) * oneOverDet;
  xy = (M.xz * M.zy - M.xy * M.zz) * oneOverDet;
  xz = (M.xy * M.yz - M.xz * M.yy) * oneOverDet;
  yx = (M.yz * M.zx - M.yx * M.zz) * oneOverDet;
  yy = (M.xx * M.zz - M.xz * M.zx) * oneOverDet;
  yz = (M.xz * M.yx - M.xx * M.yz) * oneOverDet;
  zx = (M.yx * M.zy - M.yy * M.zx) * oneOverDet;
  zy = (M.xy * M.zx - M.xx * M.zy) * oneOverDet;
  zz = (M.xx * M.yy - M.xy * M.yx) * oneOverDet;
  return 0;
}



int Mat3f::invert()
{
  float D, oneOverDet;
  float oxy, oyz, ozx, oyx, ozy, oxz, oxx, oyy;

  if (fabs(D = det()) < 1.0e-12) return 1; // not invertible
  oneOverDet = 1 / D;

  oxx = xx; oyy = yy;
  oxy = xy; oyx = yx;
  oyz = yz; ozy = zy;
  ozx = zx; oxz = xz;

  xy = (oxz * ozy - zz * oxy) * oneOverDet;
  yz = (oxz * oyx - xx * oyz) * oneOverDet;
  zx = (oyx * ozy - yy * ozx) * oneOverDet;
  yx = (oyz * ozx - oyx * zz) * oneOverDet;
  zy = (oxy * ozx - ozy * xx) * oneOverDet;
  xz = (oxy * oyz - oxz * yy) * oneOverDet;
  xx = (oyy * zz - oyz * ozy) * oneOverDet;
  yy = (oxx * zz - oxz * ozx) * oneOverDet;
  zz = (oxx * oyy - oxy * oyx) * oneOverDet;
  return 0;
}


Mat3f& Mat3f::mult(const Mat3f& M, const Mat3f& N)
{
  xx = M.xx * N.xx + M.xy * N.yx + M.xz * N.zx;
  xy = M.xx * N.xy + M.xy * N.yy + M.xz * N.zy;
  xz = M.xx * N.xz + M.xy * N.yz + M.xz * N.zz;
  yx = M.yx * N.xx + M.yy * N.yx + M.yz * N.zx;
  yy = M.yx * N.xy + M.yy * N.yy + M.yz * N.zy;
  yz = M.yx * N.xz + M.yy * N.yz + M.yz * N.zz;
  zx = M.zx * N.xx + M.zy * N.yx + M.zz * N.zx;
  zy = M.zx * N.xy + M.zy * N.yy + M.zz * N.zy;
  zz = M.zx * N.xz + M.zy * N.yz + M.zz * N.zz;

  return *this;
}


Mat3f& Mat3f::premult(const Mat3f& M)
{
  float oxy, oyz, ozx, oyx, ozy, oxz;

  oxy = xy; oyx = yx; oyz = yz; ozy = zy; ozx = zx; oxz = xz;

  xy = M.xx * oxy + M.xy * yy  + M.xz * ozy;
  xz = M.xx * oxz + M.xy * oyz + M.xz * zz;
  yx = M.yx * xx  + M.yy * oyx + M.yz * ozx;
  yz = M.yx * oxz + M.yy * oyz + M.yz * zz;
  zx = M.zx * xx  + M.zy * oyx + M.zz * ozx;
  zy = M.zx * oxy + M.zy * yy  + M.zz * ozy;

  xx = M.xx * xx  + M.xy * oyx + M.xz * ozx;
  yy = M.yx * oxy + M.yy * yy  + M.yz * ozy;
  zz = M.zx * oxz + M.zy * oyz + M.zz * zz;

  return *this;
}


Mat3f& Mat3f::postmult(const Mat3f& M)
{
  float oxy, oyz, ozx, oyx, ozy, oxz;

  oxy = xy; oyx = yx; oyz = yz; ozy = zy; ozx = zx; oxz = xz;

  xy = xx *  M.xy + oxy * M.yy + oxz * M.zy;
  xz = xx *  M.xz + oxy * M.yz + oxz * M.zz;
  yx = oyx * M.xx + yy	* M.yx + oyz * M.zx;
  yz = oyx * M.xz + yy	* M.yz + oyz * M.zz;
  zx = ozx * M.xx + ozy * M.yx + zz  * M.zx;
  zy = ozx * M.xy + ozy * M.yy + zz  * M.zy;

  xx = xx  * M.xx + oxy * M.yx + oxz * M.zx;
  yy = oyx * M.xy + yy	* M.yy + oyz * M.zy;
  zz = ozx * M.xz + ozy * M.yz + zz  * M.zz;
  
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
//
//  class MatXd
//
///////////////////////////////////////////////////////////////////////////////
MatXd& MatXd::mult(const MatXd& M, const MatXd& N)
{
  // multiply rotation matrices
  R.xx = M.R.xx * N.R.xx + M.R.xy * N.R.yx + M.R.xz * N.R.zx;
  R.xy = M.R.xx * N.R.xy + M.R.xy * N.R.yy + M.R.xz * N.R.zy;
  R.xz = M.R.xx * N.R.xz + M.R.xy * N.R.yz + M.R.xz * N.R.zz;
  R.yx = M.R.yx * N.R.xx + M.R.yy * N.R.yx + M.R.yz * N.R.zx;
  R.yy = M.R.yx * N.R.xy + M.R.yy * N.R.yy + M.R.yz * N.R.zy;
  R.yz = M.R.yx * N.R.xz + M.R.yy * N.R.yz + M.R.yz * N.R.zz;
  R.zx = M.R.zx * N.R.xx + M.R.zy * N.R.yx + M.R.zz * N.R.zx;
  R.zy = M.R.zx * N.R.xy + M.R.zy * N.R.yy + M.R.zz * N.R.zy;
  R.zz = M.R.zx * N.R.xz + M.R.zy * N.R.yz + M.R.zz * N.R.zz;

  // d = M.R * N.d + M.d
  d.x = M.R.xx * N.d.x + M.R.xy * N.d.y + M.R.xz * N.d.z + M.d.x;
  d.y = M.R.yx * N.d.x + M.R.yy * N.d.y + M.R.yz * N.d.z + M.d.y;
  d.z = M.R.zx * N.d.x + M.R.zy * N.d.y + M.R.zz * N.d.z + M.d.z;

  return *this;
}


MatXd& MatXd::premult(const MatXd& M)
{
  double oxy, oyz, ozx, oyx, ozy, oxz, odx, ody;


  // multiply rotation matrices
  oxy = R.xy; oyx = R.yx; oyz = R.yz; ozy = R.zy; ozx = R.zx; oxz = R.xz;

  R.xy = M.R.xx * oxy  + M.R.xy * R.yy + M.R.xz * ozy;
  R.xz = M.R.xx * oxz  + M.R.xy * oyz  + M.R.xz * R.zz;
  R.yx = M.R.yx * R.xx + M.R.yy * oyx  + M.R.yz * ozx;
  R.yz = M.R.yx * oxz  + M.R.yy * oyz  + M.R.yz * R.zz;
  R.zx = M.R.zx * R.xx + M.R.zy * oyx  + M.R.zz * ozx;
  R.zy = M.R.zx * oxy  + M.R.zy * R.yy + M.R.zz * ozy;

  R.xx = M.R.xx * R.xx + M.R.xy * oyx  + M.R.xz * ozx;
  R.yy = M.R.yx * oxy  + M.R.yy * R.yy + M.R.yz * ozy;
  R.zz = M.R.zx * oxz  + M.R.zy * oyz  + M.R.zz * R.zz;

  // d = M.R * d + M.d
  odx = d.x; ody = d.y;
  d.x = M.R.xx * odx + M.R.xy * ody + M.R.xz * d.z + M.d.x;
  d.y = M.R.yx * odx + M.R.yy * ody + M.R.yz * d.z + M.d.y;
  d.z = M.R.zx * odx + M.R.zy * ody + M.R.zz * d.z + M.d.z;

  return *this;
}


MatXd& MatXd::postmult(const MatXd& M)
{
  double oxy, oyz, ozx, oyx, ozy, oxz;
  Vect3d v;

  // d = R * M.d + d
  d.x += R.xx * M.d.x + R.xy * M.d.y + R.xz * M.d.z;
  d.y += R.yx * M.d.x + R.yy * M.d.y + R.yz * M.d.z;
  d.z += R.zx * M.d.x + R.zy * M.d.y + R.zz * M.d.z;

  // multiply rotation matrices
  oxy = R.xy; oyx = R.yx; oyz = R.yz; ozy = R.zy; ozx = R.zx; oxz = R.xz;
  R.xy = R.xx * M.R.xy + oxy  * M.R.yy + oxz  * M.R.zy;
  R.xz = R.xx * M.R.xz + oxy  * M.R.yz + oxz  * M.R.zz;
  R.yx = oyx  * M.R.xx + R.yy * M.R.yx + oyz  * M.R.zx;
  R.yz = oyx  * M.R.xz + R.yy * M.R.yz + oyz  * M.R.zz;
  R.zx = ozx  * M.R.xx + ozy  * M.R.yx + R.zz * M.R.zx;
  R.zy = ozx  * M.R.xy + ozy  * M.R.yy + R.zz * M.R.zy;

  R.xx = R.xx * M.R.xx + oxy  * M.R.yx + oxz  * M.R.zx;
  R.yy = oyx  * M.R.xy + R.yy * M.R.yy + oyz  * M.R.zy;
  R.zz = ozx  * M.R.xz + ozy  * M.R.yz + R.zz * M.R.zz;

  return *this;
}


MatXd& MatXd::invert(const MatXd& M)
{
  // invert the rotation part by transposing it
  R.xx = M.R.xx;
  R.xy = M.R.yx;
  R.xz = M.R.zx;
  R.yx = M.R.xy;
  R.yy = M.R.yy;
  R.yz = M.R.zy;
  R.zx = M.R.xz;
  R.zy = M.R.yz;
  R.zz = M.R.zz;

  // new displacement vector given by:	d' = -(R^-1) * d
  d.x = - (R.xx * M.d.x + R.xy * M.d.y + R.xz * M.d.z);
  d.y = - (R.yx * M.d.x + R.yy * M.d.y + R.yz * M.d.z);
  d.z = - (R.zx * M.d.x + R.zy * M.d.y + R.zz * M.d.z);

  return *this;
}



MatXd& MatXd::invert()
{
  double tmp, odx, ody;

  // invert the rotation part by transposing it
  tmp  = R.xy;
  R.xy = R.yx;
  R.yx = tmp;

  tmp  = R.yz;
  R.yz = R.zy;
  R.zy = tmp;

  tmp  = R.zx;
  R.zx = R.xz;
  R.xz = tmp;

  // new displacement vector given by:	d' = -(R^T) * d
  odx = d.x; ody = d.y;
  d.x = - (R.xx * odx + R.xy * ody + R.xz * d.z);
  d.y = - (R.yx * odx + R.yy * ody + R.yz * d.z);
  d.z = - (R.zx * odx + R.zy * ody + R.zz * d.z);

  return *this;
}


MatXd& MatXd::Invert(const MatXd& M)
{
	// invert the rotation part
	R.invert(M.R);
	
	// new displacement vector given by:	d' = -(R^-1) * d
	R.xform(M.d, d);
	d.negate();
	
	return *this;
}

MatXd& MatXd::Invert()
{
	// invert the rotation part
	R.invert();
	
	// new displacement vector given by:	d' = -(R^-1) * d
	R.xform(d);
	d.negate();
	
	return *this;
}



///////////////////////////////////////////////////////////////////////////////
//
//  class MatXf
//
///////////////////////////////////////////////////////////////////////////////

MatXf& MatXf::mult(const MatXf& M, const MatXf& N)
{
  // multiply rotation matrices
  R.xx = M.R.xx * N.R.xx + M.R.xy * N.R.yx + M.R.xz * N.R.zx;
  R.xy = M.R.xx * N.R.xy + M.R.xy * N.R.yy + M.R.xz * N.R.zy;
  R.xz = M.R.xx * N.R.xz + M.R.xy * N.R.yz + M.R.xz * N.R.zz;
  R.yx = M.R.yx * N.R.xx + M.R.yy * N.R.yx + M.R.yz * N.R.zx;
  R.yy = M.R.yx * N.R.xy + M.R.yy * N.R.yy + M.R.yz * N.R.zy;
  R.yz = M.R.yx * N.R.xz + M.R.yy * N.R.yz + M.R.yz * N.R.zz;
  R.zx = M.R.zx * N.R.xx + M.R.zy * N.R.yx + M.R.zz * N.R.zx;
  R.zy = M.R.zx * N.R.xy + M.R.zy * N.R.yy + M.R.zz * N.R.zy;
  R.zz = M.R.zx * N.R.xz + M.R.zy * N.R.yz + M.R.zz * N.R.zz;

  // d = M.R * N.d + M.d
  d.x = M.R.xx * N.d.x + M.R.xy * N.d.y + M.R.xz * N.d.z + M.d.x;
  d.y = M.R.yx * N.d.x + M.R.yy * N.d.y + M.R.yz * N.d.z + M.d.y;
  d.z = M.R.zx * N.d.x + M.R.zy * N.d.y + M.R.zz * N.d.z + M.d.z;

  return *this;
}


MatXf& MatXf::premult(const MatXf& M)
{
  float oxy, oyz, ozx, oyx, ozy, oxz, odx, ody;


  // multiply rotation matrices
  oxy = R.xy; oyx = R.yx; oyz = R.yz; ozy = R.zy; ozx = R.zx; oxz = R.xz;

  R.xy = M.R.xx * oxy  + M.R.xy * R.yy + M.R.xz * ozy;
  R.xz = M.R.xx * oxz  + M.R.xy * oyz  + M.R.xz * R.zz;
  R.yx = M.R.yx * R.xx + M.R.yy * oyx  + M.R.yz * ozx;
  R.yz = M.R.yx * oxz  + M.R.yy * oyz  + M.R.yz * R.zz;
  R.zx = M.R.zx * R.xx + M.R.zy * oyx  + M.R.zz * ozx;
  R.zy = M.R.zx * oxy  + M.R.zy * R.yy + M.R.zz * ozy;

  R.xx = M.R.xx * R.xx + M.R.xy * oyx  + M.R.xz * ozx;
  R.yy = M.R.yx * oxy  + M.R.yy * R.yy + M.R.yz * ozy;
  R.zz = M.R.zx * oxz  + M.R.zy * oyz  + M.R.zz * R.zz;

  // d = M.R * d + M.d
  odx = d.x; ody = d.y;
  d.x = M.R.xx * odx + M.R.xy * ody + M.R.xz * d.z + M.d.x;
  d.y = M.R.yx * odx + M.R.yy * ody + M.R.yz * d.z + M.d.y;
  d.z = M.R.zx * odx + M.R.zy * ody + M.R.zz * d.z + M.d.z;

  return *this;
}


MatXf& MatXf::postmult(const MatXf& M)
{
  float oxy, oyz, ozx, oyx, ozy, oxz;
  Vect3d v;

  // d = R * M.d + d
  d.x += R.xx * M.d.x + R.xy * M.d.y + R.xz * M.d.z;
  d.y += R.yx * M.d.x + R.yy * M.d.y + R.yz * M.d.z;
  d.z += R.zx * M.d.x + R.zy * M.d.y + R.zz * M.d.z;

  // multiply rotation matrices
  oxy = R.xy; oyx = R.yx; oyz = R.yz; ozy = R.zy; ozx = R.zx; oxz = R.xz;
  R.xy = R.xx * M.R.xy + oxy  * M.R.yy + oxz  * M.R.zy;
  R.xz = R.xx * M.R.xz + oxy  * M.R.yz + oxz  * M.R.zz;
  R.yx = oyx  * M.R.xx + R.yy * M.R.yx + oyz  * M.R.zx;
  R.yz = oyx  * M.R.xz + R.yy * M.R.yz + oyz  * M.R.zz;
  R.zx = ozx  * M.R.xx + ozy  * M.R.yx + R.zz * M.R.zx;
  R.zy = ozx  * M.R.xy + ozy  * M.R.yy + R.zz * M.R.zy;

  R.xx = R.xx * M.R.xx + oxy  * M.R.yx + oxz  * M.R.zx;
  R.yy = oyx  * M.R.xy + R.yy * M.R.yy + oyz  * M.R.zy;
  R.zz = ozx  * M.R.xz + ozy  * M.R.yz + R.zz * M.R.zz;

  return *this;
}


MatXf& MatXf::invert(const MatXf& M)
{
  // invert the rotation part by transposing it
  R.xx = M.R.xx;
  R.xy = M.R.yx;
  R.xz = M.R.zx;
  R.yx = M.R.xy;
  R.yy = M.R.yy;
  R.yz = M.R.zy;
  R.zx = M.R.xz;
  R.zy = M.R.yz;
  R.zz = M.R.zz;

  // new displacement vector given by:	d' = -(R^-1) * d
  d.x = - (R.xx * M.d.x + R.xy * M.d.y + R.xz * M.d.z);
  d.y = - (R.yx * M.d.x + R.yy * M.d.y + R.yz * M.d.z);
  d.z = - (R.zx * M.d.x + R.zy * M.d.y + R.zz * M.d.z);

  return *this;
}



MatXf& MatXf::invert()
{
  float tmp, odx, ody;

  // invert the rotation part by transposing it
  tmp  = R.xy;
  R.xy = R.yx;
  R.yx = tmp;

  tmp  = R.yz;
  R.yz = R.zy;
  R.zy = tmp;

  tmp  = R.zx;
  R.zx = R.xz;
  R.xz = tmp;

  // new displacement vector given by:	d' = -(R^T) * d
  odx = d.x; ody = d.y;
  d.x = - (R.xx * odx + R.xy * ody + R.xz * d.z);
  d.y = - (R.yx * odx + R.yy * ody + R.yz * d.z);
  d.z = - (R.zx * odx + R.zy * ody + R.zz * d.z);

  return *this;
}

MatXf& MatXf::Invert(const MatXf& M)
{
	// invert the rotation part
	R.invert(M.R);
	
	// new displacement vector given by:	d' = -(R^-1) * d
	R.xform(M.d, d);
	d.negate();
	
	return *this;
}

MatXf& MatXf::Invert()
{
	// invert the rotation part
	R.invert();
	
	// new displacement vector given by:	d' = -(R^-1) * d
	R.xform(d);
	d.negate();
	
	return *this;
}


///////////////////////////////////////////////////////////////////////////////
//
//  class QuatD
//
///////////////////////////////////////////////////////////////////////////////


QuatD& QuatD::set(double angle, const Vect3d& axis, int normalizeAxis)
{
	double theta = 0.5 * angle;
	double sine = sin(theta);
	s_ = cos(theta);

	if(normalizeAxis){
		double n2;
		if((n2 = axis.norm2()) < DBL_EPS){
			*this = QuatD::ID;
			return *this;
			}
		sine /= sqrt(n2);
		}

	x_ = axis.x * sine;
	y_ = axis.y * sine;
	z_ = axis.z * sine;

	return *this;
}


QuatD& QuatD::set(const Mat3d& R)
{
  double qs2, qx2, qy2, qz2;  // squared magniudes of quaternion components
  double tmp;
  int i;

  // first compute squared magnitudes of quaternion components - at least one
  // will be greater than 0 since quaternion is unit magnitude

  qs2 = 0.25 * (R.xx + R.yy + R.zz + 1);
  qx2 = qs2 - 0.5 * (R.yy + R.zz);
  qy2 = qs2 - 0.5 * (R.zz + R.xx);
  qz2 = qs2 - 0.5 * (R.xx + R.yy);


  // find maximum magnitude component
  i = (qs2 > qx2 ) ?
    ((qs2 > qy2) ? ((qs2 > qz2) ? 0 : 3) : ((qy2 > qz2) ? 2 : 3)) :
    ((qx2 > qy2) ? ((qx2 > qz2) ? 1 : 3) : ((qy2 > qz2) ? 2 : 3));

  // compute signed quaternion components using numerically stable method
  switch(i) {
  case 0:
    s_ = sqrt(qs2);
    tmp = 0.25 / s_;
    x_ = (R.zy - R.yz) * tmp;
    y_ = (R.xz - R.zx) * tmp;
    z_ = (R.yx - R.xy) * tmp;
    break;
  case 1:
    x_ = sqrt(qx2);
    tmp = 0.25 / x_;
    s_ = (R.zy - R.yz) * tmp;
    y_ = (R.xy + R.yx) * tmp;
    z_ = (R.xz + R.zx) * tmp;
    break;
  case 2:
    y_ = sqrt(qy2);
    tmp = 0.25 / y_;
    s_ = (R.xz - R.zx) * tmp;
    z_ = (R.yz + R.zy) * tmp;
    x_ = (R.yx + R.xy) * tmp;
    break;
  case 3:
    z_ = sqrt(qz2);
    tmp = 0.25 / z_;
    s_ = (R.yx - R.xy) * tmp;
    x_ = (R.zx + R.xz) * tmp;
    y_ = (R.zy + R.yz) * tmp;
    break;
  }
  // for consistency, force positive scalar component [ (s; v) = (-s; -v) ]
  if (s_ < 0) {
    s_ = -s_;
    x_ = -x_;
    y_ = -y_;
    z_ = -z_;
  }
  // normalize, just to be safe
  tmp = 1.0 / sqrt(s_*s_ + x_*x_ + y_*y_ + z_*z_);
  s_ *= tmp;
  x_ *= tmp;
  y_ *= tmp;
  z_ *= tmp;

  return *this;
}


QuatD& QuatD::mult(const QuatD& p, const QuatD& q)
{
  s_ = p.s_ * q.s_ - (p.x_ * q.x_ + p.y_ * q.y_ + p.z_ * q.z_);
  x_ = p.s_ * q.x_ +  q.s_ * p.x_ + p.y_ * q.z_ - p.z_ * q.y_;
  y_ = p.s_ * q.y_ +  q.s_ * p.y_ + p.z_ * q.x_ - p.x_ * q.z_;
  z_ = p.s_ * q.z_ +  q.s_ * p.z_ + p.x_ * q.y_ - p.y_ * q.x_;

  return *this;
}


QuatD& QuatD::premult(const QuatD& q)
{
  double ox, oy, oz;

  ox = x_; oy = y_; oz = z_;

  x_ = q.s_ * ox +  s_ * q.x_ + q.y_ * oz - q.z_ * oy;
  y_ = q.s_ * oy +  s_ * q.y_ + q.z_ * ox - q.x_ * oz;
  z_ = q.s_ * oz +  s_ * q.z_ + q.x_ * oy - q.y_ * ox;
  s_ = q.s_ * s_ - (q.x_ * ox + q.y_ * oy + q.z_ * oz);
  
  return *this;
}


QuatD& QuatD::postmult(const QuatD& q)
{
  double ox, oy, oz;

  ox = x_; oy = y_; oz = z_;

  x_ = s_ * q.x_ +  ox * q.s_ + oy * q.z_ - oz * q.y_;
  y_ = s_ * q.y_ +  oy * q.s_ + oz * q.x_ - ox * q.z_;
  z_ = s_ * q.z_ +  oz * q.s_ + ox * q.y_ - oy * q.x_;
  s_ = s_ * q.s_ - (ox * q.x_ + oy * q.y_ + oz * q.z_);

  return *this;
}


// The QuatD transformation routines use 19 multiplies and 12 adds
// (counting the multiplications by 2.0).  See Eqn (20) of "A
// Comparison of Transforms and Quaternions in Robotics," Funda and
// Paul, Proceedings of International Conference on Robotics and
// Automation, 1988, p. 886-991.

Vect3d& QuatD::xform(const Vect3d& v, Vect3d& xv) const
{
  Vect3d *u, uv, uuv;


  u = (Vect3d *) &x_;
  uv.cross(*u, v);
  uuv.cross(*u, uv);
  uv.scale(2.0 * s_);
  uuv.scale(2.0);
  xv.add(v, uv);
  xv.add(uuv);
  return xv;
}


Vect3d& QuatD::xform(Vect3d& v) const
{
  Vect3d *u, uv, uuv;

  u = (Vect3d *) &x_;
  uv.cross(*u, v);
  uuv.cross(*u, uv);
  uv.scale(2.0 * s_);
  uuv.scale(2.0);
  v.add(uv);
  v.add(uuv);
  return v;
}


Vect3d& QuatD::invXform(const Vect3d& v, Vect3d& xv) const
{
  Vect3d *u, uv, uuv;

  u = (Vect3d *) &x_;
  uv.cross(*u, v);
  uuv.cross(*u, uv);
  uv.scale(2.0 * -s_);
  uuv.scale(2.0);
  xv.add(v, uv);
  xv.add(uuv);
  return xv;
}


Vect3d& QuatD::invXform(Vect3d& v) const
{
  Vect3d *u, uv, uuv;

  u = (Vect3d *) &x_;
  uv.cross(*u, v);
  uuv.cross(*u, uv);
  uv.scale(2.0 * -s_);
  uuv.scale(2.0);
  v.add(uv);
  v.add(uuv);
  return v;
}


///////////////////////////////////////////////////////////////////////////////
//
//  class QuatF
//
///////////////////////////////////////////////////////////////////////////////


QuatF& QuatF::set(float angle, const Vect3f& axis, int normalizeAxis)
{
	float theta = 0.5f * angle;
	float sine = sinf(theta);
	s_ = cosf(theta);

	if(normalizeAxis){
		float n2;
		if((n2 = axis.norm2()) < FLT_EPS){
			*this = QuatD::ID;
			return *this;
			}
		sine *= invSqrtFast(n2);
		}

	x_ = axis.x * sine;
	y_ = axis.y * sine;
	z_ = axis.z * sine;

	return *this;
}

QuatF& QuatF::set(const Mat3f& R)
{
  float qs2, qx2, qy2, qz2;  // squared magniudes of quaternion components
  float tmp;
  int i;

  // first compute squared magnitudes of quaternion components - at least one
  // will be greater than 0 since quaternion is unit magnitude

  qs2 = 0.25f * (R.xx + R.yy + R.zz + 1);
  qx2 = qs2 - 0.5f * (R.yy + R.zz);
  qy2 = qs2 - 0.5f * (R.zz + R.xx);
  qz2 = qs2 - 0.5f * (R.xx + R.yy);


  // find maximum magnitude component
  i = (qs2 > qx2 ) ?
    ((qs2 > qy2) ? ((qs2 > qz2) ? 0 : 3) : ((qy2 > qz2) ? 2 : 3)) :
    ((qx2 > qy2) ? ((qx2 > qz2) ? 1 : 3) : ((qy2 > qz2) ? 2 : 3));

  // compute signed quaternion components using numerically stable method
  switch(i) {
  case 0:
    s_ = sqrtf(qs2);
    tmp = 0.25f / s_;
    x_ = (R.zy - R.yz) * tmp;
    y_ = (R.xz - R.zx) * tmp;
    z_ = (R.yx - R.xy) * tmp;
    break;
  case 1:
    x_ = sqrtf(qx2);
    tmp = 0.25f / x_;
    s_ = (R.zy - R.yz) * tmp;
    y_ = (R.xy + R.yx) * tmp;
    z_ = (R.xz + R.zx) * tmp;
    break;
  case 2:
    y_ = sqrtf(qy2);
    tmp = 0.25f / y_;
    s_ = (R.xz - R.zx) * tmp;
    z_ = (R.yz + R.zy) * tmp;
    x_ = (R.yx + R.xy) * tmp;
    break;
  case 3:
    z_ = sqrtf(qz2);
    tmp = 0.25f / z_;
    s_ = (R.yx - R.xy) * tmp;
    x_ = (R.zx + R.xz) * tmp;
    y_ = (R.zy + R.yz) * tmp;
    break;
  }
  // for consistency, force positive scalar component [ (s; v) = (-s; -v) ]
  if (s_ < 0) {
    s_ = -s_;
    x_ = -x_;
    y_ = -y_;
    z_ = -z_;
  }
  // normalize, just to be safe
  tmp = invSqrtFast(s_*s_ + x_*x_ + y_*y_ + z_*z_);
  s_ *= tmp;
  x_ *= tmp;
  y_ *= tmp;
  z_ *= tmp;

  return *this;
}

QuatF& QuatF::mult(const QuatF& p, const QuatF& q)
{
  s_ = p.s_ * q.s_ - (p.x_ * q.x_ + p.y_ * q.y_ + p.z_ * q.z_);
  x_ = p.s_ * q.x_ +  q.s_ * p.x_ + p.y_ * q.z_ - p.z_ * q.y_;
  y_ = p.s_ * q.y_ +  q.s_ * p.y_ + p.z_ * q.x_ - p.x_ * q.z_;
  z_ = p.s_ * q.z_ +  q.s_ * p.z_ + p.x_ * q.y_ - p.y_ * q.x_;

  return *this;
}


QuatF& QuatF::premult(const QuatF& q)
{
  float ox, oy, oz;

  ox = x_; oy = y_; oz = z_;

  x_ = q.s_ * ox +  s_ * q.x_ + q.y_ * oz - q.z_ * oy;
  y_ = q.s_ * oy +  s_ * q.y_ + q.z_ * ox - q.x_ * oz;
  z_ = q.s_ * oz +  s_ * q.z_ + q.x_ * oy - q.y_ * ox;
  s_ = q.s_ * s_ - (q.x_ * ox + q.y_ * oy + q.z_ * oz);
  
  return *this;
}


QuatF& QuatF::postmult(const QuatF& q)
{
  float ox, oy, oz;

  ox = x_; oy = y_; oz = z_;

  x_ = s_ * q.x_ +  ox * q.s_ + oy * q.z_ - oz * q.y_;
  y_ = s_ * q.y_ +  oy * q.s_ + oz * q.x_ - ox * q.z_;
  z_ = s_ * q.z_ +  oz * q.s_ + ox * q.y_ - oy * q.x_;
  s_ = s_ * q.s_ - (ox * q.x_ + oy * q.y_ + oz * q.z_);

  return *this;
}

// The QuatF transformation routines use 19 multiplies and 12 adds
// (counting the multiplications by 2.0).  See Eqn (20) of "A
// Comparison of Transforms and Quaternions in Robotics," Funda and
// Paul, Proceedings of International Conference on Robotics and
// Automation, 1988, p. 886-991.

Vect3f& QuatF::xform(const Vect3f& v, Vect3f& xv) const
{
  Vect3f *u, uv, uuv;

  u = (Vect3f *) &x_;
  uv.cross(*u, v);
  uuv.cross(*u, uv);
  uv.scale(2.f * s_);
  uuv.scale(2.f);
  xv.add(v, uv);
  xv.add(uuv);
  return xv;
}

Vect3f& QuatF::xform(Vect3f& v) const
{
  Vect3f *u, uv, uuv;

  u = (Vect3f *) &x_;
  uv.cross(*u, v);
  uuv.cross(*u, uv);
  uv.scale(2.f * s_);
  uuv.scale(2.f);
  v.add(uv);
  v.add(uuv);
  return v;
}

Vect3f& QuatF::invXform(const Vect3f& v, Vect3f& xv) const
{
  Vect3f *u, uv, uuv;

  u = (Vect3f *) &x_;
  uv.cross(*u, v);
  uuv.cross(*u, uv);
  uv.scale(2.f * -s_);
  uuv.scale(2.f);
  xv.add(v, uv);
  xv.add(uuv);
  return xv;
}


Vect3f& QuatF::invXform(Vect3f& v) const
{
  Vect3f *u, uv, uuv;

  u = (Vect3f *) &x_;
  uv.cross(*u, v);
  uuv.cross(*u, uv);
  uv.scale(2.f * -s_);
  uuv.scale(2.f);
  v.add(uv);
  v.add(uuv);
  return v;
}

//////////////////////////////////////////////////////////////////////////////////
//
//  	     XTool's I/O operations 
//
//////////////////////////////////////////////////////////////////////////////////
//  Vect2i I/O operations  //////////////
XStream& operator<= (XStream& s,const Vect2i& v)
{
	s <= v.x < "  " <= v.y;
	return s;
}
XStream& operator>= (XStream& s,Vect2i& v)
{
	s >= v.x >= v.y;
	return s;
}
XStream& operator< (XStream& s,const Vect2i& v)
{
	s.write(&v,sizeof(Vect2i));
	return s;
}
XStream& operator> (XStream& s,Vect2i& v)
{
	s.read(&v,sizeof(Vect2i));
	return s;
}

XBuffer& operator<= (XBuffer& b,const Vect2i& v)
{
	b <= v.x < "  " <= v.y;
	return b;
}
XBuffer& operator>= (XBuffer& b,Vect2i& v)
{
	b >= v.x >= v.y;
	return b;
}
XBuffer& operator< (XBuffer& b,const Vect2i& v)
{
	b < v.x < v.y;
	return b;
}
XBuffer& operator> (XBuffer& b,Vect2i& v)
{
	b > v.x > v.y;
	return b;
}

//  Vect2f I/O operations  //////////////
XStream& operator<= (XStream& s,const Vect2f& v)
{
	s <= v.x < "  " <= v.y;
	return s;
}
XStream& operator>= (XStream& s,Vect2f& v)
{
	s >= v.x >= v.y;
	return s;
}
XStream& operator< (XStream& s,const Vect2f& v)
{
	s.write(&v,sizeof(Vect2f));
	return s;
}
XStream& operator> (XStream& s,Vect2f& v)
{
	s.read(&v,sizeof(Vect2f));
	return s;
}

XBuffer& operator<= (XBuffer& b,const Vect2f& v)
{
	b <= v.x < "  " <= v.y;
	return b;
}
XBuffer& operator>= (XBuffer& b,Vect2f& v)
{
	b >= v.x >= v.y;
	return b;
}
XBuffer& operator< (XBuffer& b,const Vect2f& v)
{
	b < v.x < v.y;
	return b;
}
XBuffer& operator> (XBuffer& b,Vect2f& v)
{
	b > v.x > v.y;
	return b;
}

//  Vect2s I/O operations  //////////////
XStream& operator<= (XStream& s,const Vect2s& v)
{
	s <= v.x < "  " <= v.y;
	return s;
}
XStream& operator>= (XStream& s,Vect2s& v)
{
	s >= v.x >= v.y;
	return s;
}
XStream& operator< (XStream& s,const Vect2s& v)
{
	s.write(&v,sizeof(Vect2s));
	return s;
}
XStream& operator> (XStream& s,Vect2s& v)
{
	s.read(&v,sizeof(Vect2s));
	return s;
}

XBuffer& operator<= (XBuffer& b,const Vect2s& v)
{
	b <= v.x < "  " <= v.y;
	return b;
}
XBuffer& operator>= (XBuffer& b,Vect2s& v)
{
	b >= v.x >= v.y;
	return b;
}
XBuffer& operator< (XBuffer& b,const Vect2s& v)
{
	b < v.x < v.y;
	return b;
}
XBuffer& operator> (XBuffer& b,Vect2s& v)
{
	b > v.x > v.y;
	return b;
}

//  Vect3d I/O operations  //////////////
XStream& operator<= (XStream& s,const Vect3d& v)
{
	s <= v.x < "  " <= v.y < "  " <= v.z;
	return s;
}
XStream& operator>= (XStream& s,Vect3d& v)
{
	s >= v.x >= v.y >= v.z;
	return s;
}
XStream& operator< (XStream& s,const Vect3d& v)
{
	s.write(&v,sizeof(Vect3d));
	return s;
}
XStream& operator> (XStream& s,Vect3d& v)
{
	s.read(&v,sizeof(Vect3d));
	return s;
}

XBuffer& operator<= (XBuffer& b,const Vect3d& v)
{
	b <= v.x < "  " <= v.y < "  " <= v.z;
	return b;
}
XBuffer& operator>= (XBuffer& b,Vect3d& v)
{
	b >= v.x >= v.y >= v.z;
	return b;
}
XBuffer& operator< (XBuffer& b,const Vect3d& v)
{
	b < v.x < v.y < v.z;
	return b;
}
XBuffer& operator> (XBuffer& b,Vect3d& v)
{
	b > v.x > v.y > v.z;
	return b;
}

//  Vect3f I/O operations  //////////////
XStream& operator<= (XStream& s,const Vect3f& v)
{
	s <= v.x < "  " <= v.y < "  " <= v.z;
	return s;
}
XStream& operator>= (XStream& s,Vect3f& v)
{
	s >= v.x >= v.y >= v.z;
	return s;
}
XStream& operator< (XStream& s, const Vect3f& v)
{
	s.write(&v,sizeof(Vect3f));
	return s;
}
XStream& operator> (XStream& s,Vect3f& v)
{
	s.read(&v,sizeof(Vect3f));
	return s;
}

XBuffer& operator<= (XBuffer& b,const Vect3f& v)
{
	b <= v.x < "  " <= v.y < "  " <= v.z;
	return b;
}
XBuffer& operator>= (XBuffer& b,Vect3f& v)
{
	b >= v.x >= v.y >= v.z;
	return b;
}
XBuffer& operator< (XBuffer& b,const Vect3f& v)
{
	b < v.x < v.y < v.z;
	return b;
}
XBuffer& operator> (XBuffer& b,Vect3f& v)
{
	b > v.x > v.y > v.z;
	return b;
}

//   Mat3d I/O operations    /////////////
XStream& operator<= (XStream& s,const Mat3d& m)
{
	s <= m.xx < "  " <= m.xy < "  " <= m.xz < "\n";
	s <= m.yx < "  " <= m.yy < "  " <= m.yz < "\n";
	s <= m.zx < "  " <= m.zy < "  " <= m.zz < "\n";
	return s;
}
XStream& operator>= (XStream& s,Mat3d& m)
{
	s >= m.xx >= m.xy >= m.xz;
	s >= m.yx >= m.yy >= m.yz;
	s >= m.zx >= m.zy >= m.zz;
	return s;
}
XStream& operator< (XStream& s, const Mat3d& m)
{
	s.write(&m,sizeof(Mat3d));
	return s;
}
XStream& operator> (XStream& s,Mat3d& m)
{
	s.read(&m,sizeof(Mat3d));
	return s;
}

XBuffer& operator<= (XBuffer& b,const Mat3d& m)
{
	b <= m.xx < "  " <= m.xy < "  " <= m.xz < "\n";
	b <= m.yx < "  " <= m.yy < "  " <= m.yz < "\n";
	b <= m.zx < "  " <= m.zy < "  " <= m.zz < "\n";
	return b;
}
XBuffer& operator>= (XBuffer& b,Mat3d& m)
{
	b >= m.xx >= m.xy >= m.xz;
	b >= m.yx >= m.yy >= m.yz;
	b >= m.zx >= m.zy >= m.zz;
	return b;
}
XBuffer& operator< (XBuffer& b,const Mat3d& m)
{
	b < m.xx < m.xy < m.xz;
	b < m.yx < m.yy < m.yz;
	b < m.zx < m.zy < m.zz;
	return b;
}
XBuffer& operator> (XBuffer& b,Mat3d& m)
{
	b > m.xx > m.xy > m.xz;
	b > m.yx > m.yy > m.yz;
	b > m.zx > m.zy > m.zz;
	return b;
}

//   Mat3f I/O operations    /////////////
XStream& operator<= (XStream& s,const Mat3f& m)
{
	s <= m.xx < "  " <= m.xy < "  " <= m.xz < "\n";
	s <= m.yx < "  " <= m.yy < "  " <= m.yz < "\n";
	s <= m.zx < "  " <= m.zy < "  " <= m.zz < "\n";
	return s;
}
XStream& operator>= (XStream& s,Mat3f& m)
{
	s >= m.xx >= m.xy >= m.xz;
	s >= m.yx >= m.yy >= m.yz;
	s >= m.zx >= m.zy >= m.zz;
	return s;
}
XStream& operator< (XStream& s, const Mat3f& m)
{
	s.write(&m,sizeof(Mat3f));
	return s;
}
XStream& operator> (XStream& s,Mat3f& m)
{
	s.read(&m,sizeof(Mat3f));
	return s;
}

XBuffer& operator<= (XBuffer& b,const Mat3f& m)
{
	b <= m.xx < "  " <= m.xy < "  " <= m.xz < "\n";
	b <= m.yx < "  " <= m.yy < "  " <= m.yz < "\n";
	b <= m.zx < "  " <= m.zy < "  " <= m.zz < "\n";
	return b;
}
XBuffer& operator>= (XBuffer& b,Mat3f& m)
{
	b >= m.xx >= m.xy >= m.xz;
	b >= m.yx >= m.yy >= m.yz;
	b >= m.zx >= m.zy >= m.zz;
	return b;
}
XBuffer& operator< (XBuffer& b,const Mat3f& m)
{
	b < m.xx < m.xy < m.xz;
	b < m.yx < m.yy < m.yz;
	b < m.zx < m.zy < m.zz;
	return b;
}
XBuffer& operator> (XBuffer& b,Mat3f& m)
{
	b > m.xx > m.xy > m.xz;
	b > m.yx > m.yy > m.yz;
	b > m.zx > m.zy > m.zz;
	return b;
}


//  QuatD I/O operations  //////////////////
XStream& operator<= (XStream& s,const QuatD& q)
{
	s <= q.s_ < "   "  <= q.x_ < "  " <= q.y_ < "  " <= q.z_;
	return s;
}
XStream& operator>= (XStream& s,QuatD& q)
{
	s >= q.s_ >= q.x_ >= q.y_ >= q.z_;
	return s;
}
XStream& operator< (XStream& s, const QuatD& q)
{
	s.write(&q,sizeof(QuatD));
	return s;
}
XStream& operator> (XStream& s,QuatD& q)
{
	s.read(&q,sizeof(QuatD));
	return s;
}

XBuffer& operator<= (XBuffer& b,const QuatD& q)
{
	b <= q.s_ < "   "  <= q.x_ < "  " <= q.y_ < "  " <= q.z_;
	return b;
}
XBuffer& operator>= (XBuffer& b,QuatD& q)
{
	b >= q.s_ >= q.x_ >= q.y_ >= q.z_;
	return b;
}
XBuffer& operator< (XBuffer& b,const QuatD& q)
{
	b < q.s_ < q.x_ < q.y_ < q.z_;
	return b;
}
XBuffer& operator> (XBuffer& b,QuatD& q)
{
	b > q.s_ > q.x_ > q.y_ > q.z_;
	return b;
}

//  QuatF I/O operations  //////////////////
XStream& operator<= (XStream& s,const QuatF& q)
{
	s <= q.s_ < "   "  <= q.x_ < "  " <= q.y_ < "  " <= q.z_;
	return s;
}
XStream& operator>= (XStream& s,QuatF& q)
{
	s >= q.s_ >= q.x_ >= q.y_ >= q.z_;
	return s;
}
XStream& operator< (XStream& s, const QuatF& q)
{
	s.write(&q,sizeof(QuatF));
	return s;
}
XStream& operator> (XStream& s,QuatF& q)
{
	s.read(&q,sizeof(QuatF));
	return s;
}

XBuffer& operator<= (XBuffer& b,const QuatF& q)
{
	b <= q.s_ < "   "  <= q.x_ < "  " <= q.y_ < "  " <= q.z_;
	return b;
}
XBuffer& operator>= (XBuffer& b,QuatF& q)
{
	b >= q.s_ >= q.x_ >= q.y_ >= q.z_;
	return b;
}
XBuffer& operator< (XBuffer& b,const QuatF& q)
{
	b < q.s_ < q.x_ < q.y_ < q.z_;
	return b;
}
XBuffer& operator> (XBuffer& b,QuatF& q)
{
	b > q.s_ > q.x_ > q.y_ > q.z_;
	return b;
}


//  Se3f I/O operations  //////////////
XStream& operator<= (XStream& s,const Se3f& se3)
{
	s <= se3.q < "  " <= se3.d;
	return s;
}
XStream& operator>= (XStream& s,Se3f& se3)
{
	s >= se3.q >= se3.d;
	return s;
}
XStream& operator< (XStream& s,const Se3f& se3)
{
	s.write(&se3,sizeof(Vect2f));
	return s;
}
XStream& operator> (XStream& s,Se3f& se3)
{
	s.read(&se3,sizeof(Vect2f));
	return s;
}

XBuffer& operator<= (XBuffer& b,const Se3f& se3)
{
	b <= se3.q < "  " <= se3.d;
	return b;
}
XBuffer& operator>= (XBuffer& b,Se3f& se3)
{
	b >= se3.q >= se3.d;
	return b;
}
XBuffer& operator< (XBuffer& b,const Se3f& se3)
{
	b < se3.q < se3.d;
	return b;
}
XBuffer& operator> (XBuffer& b,Se3f& se3)
{
	b > se3.q > se3.d;
	return b;
}

//  Se3d I/O operations  //////////////
XStream& operator<= (XStream& s,const Se3d& se3)
{
	s <= se3.q < "  " <= se3.d;
	return s;
}
XStream& operator>= (XStream& s,Se3d& se3)
{
	s >= se3.q >= se3.d;
	return s;
}
XStream& operator< (XStream& s,const Se3d& se3)
{
	s.write(&se3,sizeof(Vect2f));
	return s;
}
XStream& operator> (XStream& s,Se3d& se3)
{
	s.read(&se3,sizeof(Vect2f));
	return s;
}

XBuffer& operator<= (XBuffer& b,const Se3d& se3)
{
	b <= se3.q < "  " <= se3.d;
	return b;
}
XBuffer& operator>= (XBuffer& b,Se3d& se3)
{
	b >= se3.q >= se3.d;
	return b;
}
XBuffer& operator< (XBuffer& b,const Se3d& se3)
{
	b < se3.q < se3.d;
	return b;
}
XBuffer& operator> (XBuffer& b,Se3d& se3)
{
	b > se3.q > se3.d;
	return b;
}

void Mat2f::Invert()
{
	Mat2f out;
	float det=xx*yy-xy*yx;
	xassert(fabs(det)>FLT_EPS);
	float inv_det=1/det;
	out.xx=yy*inv_det;
	out.xy=-xy*inv_det;
	out.yx=-yx*inv_det;
	out.yy=xx*inv_det;

	*this=out;
}

//////////////////////////////////////////////////////////////////////////////////
//  	     QuatD definitions
//////////////////////////////////////////////////////////////////////////////////
QuatD Slerp(const QuatD& A,const QuatD& B,double t)
{
	double theta = acos(dot(A,B));
	double sin_theta = sin(theta);
	if(fabs(sin_theta) < DBL_EPS)
		return A;
	sin_theta = 1/sin_theta;
	return A*(sin(theta*(1 - t))*sin_theta) + B*(sin(theta* t)*sin_theta);
}

//////////////////////////////////////////
void Vect2f::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
    ar.serialize(x, "x", "&x");
    ar.serialize(y, "y", "&y");
}

void Vect2i::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
    ar.serialize(x, "x", "&x");
    ar.serialize(y, "y", "&y");
}

void Vect2s::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
	ar.serialize(x, "x", "&x");
	ar.serialize(y, "y", "&y");
}

void Vect3f::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
    ar.serialize(x, "x", "&x");
    ar.serialize(y, "y", "&y");
    ar.serialize(z, "z", "&z");
}

void Vect3d::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
	ar.serialize(x, "x", "&x");
	ar.serialize(y, "y", "&y");
	ar.serialize(z, "z", "&z");
}

void Mat3f::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
	ar.serialize(xrow(), "xrow", "xrow");
	ar.serialize(yrow(), "yrow", "yrow");
	ar.serialize(zrow(), "zrow", "zrow");
}

void Mat3d::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
	ar.serialize(xrow(), "xrow", "xrow");
	ar.serialize(yrow(), "yrow", "yrow");
	ar.serialize(zrow(), "zrow", "zrow");
}

void MatXf::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
	ar.serialize(rot(), "rotation", "����������");
	ar.serialize(trans(), "position", "�������");
}

void MatXd::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
	ar.serialize(rot(), "rotation", "����������");
	ar.serialize(trans(), "position", "�������");
}

void QuatF::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
    ar.serialize(s_, "s", "&s");
    ar.serialize(x_, "x", "&x");
    ar.serialize(y_, "y", "&y");
    ar.serialize(z_, "z", "&z");
}

void QuatD::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
	ar.serialize(s_, "s", "&s");
	ar.serialize(x_, "x", "&x");
	ar.serialize(y_, "y", "&y");
	ar.serialize(z_, "z", "&z");
}

void Se3f::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
	ar.serialize(rot(), "rotation", "����������");
	ar.serialize(trans(), "position", "�������");
}

void Se3d::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
	ar.serialize(rot(), "rotation", "����������");
	ar.serialize(trans(), "position", "�������");
}

void Vect4f::serialize(Archive& ar) 
{
	MergeBlocksAuto mergeBlocks(ar);
	ar.serialize(x, "x", "&x");
	ar.serialize(y, "y", "&y");
	ar.serialize(z, "z", "&z");
	ar.serialize(z, "w", "&w");
}

void Mat3f::makeRotationZ()
{
	float cos = 0.0f;
	float sin;
	if(yy > FLT_EPS){
		float tan = xy / yy;
		cos = invSqrtFast(tan * tan + 1.0f);
		sin = cos * tan;
	}
	else
		sin = xy > 0.0f ? 1.0f : -1.0f;
	xx = cos; xy = -sin; xz = 0.0f;
	yx = sin; yy = cos; yz = 0.0f;
	zx = 0.0f; zy = 0.0f; zz = 1.0f;
}

