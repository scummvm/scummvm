#include "StdAfx.h"
#include "mats.h"

#pragma warning(disable : 4073 )
#pragma init_seg(lib)

Mats Mats::ID(Se3f(QuatF(1, 0, 0, 0), Vect3f(0, 0, 0)), 1);

void Mats::mult(const Mats& t,const Mats& u)
{
	s=t.s*u.s;
	q.mult(t.q, u.q);
	t.q.xform(u.d, d);
	d*=t.s;
	d+=t.d;
}

void Mats::copy_right(MatXf& mat) const
{
	float s2=s*2;
	mat.R.xx = s2 * (q.s_ * q.s_ + q.x_ * q.x_ - 0.5f);
	mat.R.yy = s2 * (q.s_ * q.s_ + q.y_ * q.y_ - 0.5f);
	mat.R.zz = s2 * (q.s_ * q.s_ + q.z_ * q.z_ - 0.5f);

	mat.R.xy = s2 * (q.y_ * q.x_ - q.z_ * q.s_);
	mat.R.yx = s2 * (q.x_ * q.y_ + q.z_ * q.s_);


	mat.R.yz = s2 * (q.z_ * q.y_ - q.x_ * q.s_);
	mat.R.zy = s2 * (q.y_ * q.z_ + q.x_ * q.s_);

	mat.R.zx = s2 * (q.x_ * q.z_ - q.y_ * q.s_);
	mat.R.xz = s2 * (q.z_ * q.x_ + q.y_ * q.s_);

	mat.d=d;
}


void Mats::operator=(MatXf mat)
{
	s=mat.rot().xrow().norm();
	float invs=1/s;
	mat.rot()*=invs;
	se().set(mat);
	scale()=s;
}
