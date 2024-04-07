#ifndef __MATS_H_INCLUDED__
#define __MATS_H_INCLUDED__

#include "XMath\XMath.h"

class Mats
{
	QuatF q;
	Vect3f d;
	float s;

public:
	const QuatF&  rot()   const {return q;}
	const Vect3f& trans() const {return d;}
	const float& scale()  const {return s;}
	QuatF&  rot()	    {return q;}
	Vect3f& trans()	    {return d;}
	float& scale()		{return s;}
	Se3f& se(){return *(Se3f*)&q;}
	const Se3f& se()const {return *(const Se3f*)&q;}

	void mult(const Mats& t,const Mats& u);
	void copy_right(MatXf& mat) const;
	
	Mats() {}
	Mats(const Se3f& _se, float _scale) { se() = _se; s = _scale; }
	void operator=(MatXf mat);//Предполагается что в матрице изотропный скэйлинг.

	void interpolate(const Mats& u, const Mats& v, float t) { se().interpolateExact(u.se(), v.se(), t); s = u.s + (v.s - u.s)*t; }

	static Mats ID;
};

#endif
