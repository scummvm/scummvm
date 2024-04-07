#include "stdafx.h"
#include "Plane.h"


void Plane::set(const Vect3f& a,const Vect3f& b,const Vect3f& c) 
{
	A=(b.y-a.y)*(c.z-a.z)-(c.y-a.y)*(b.z-a.z);
	B=(b.z-a.z)*(c.x-a.x)-(c.z-a.z)*(b.x-a.x);
	C=(b.x-a.x)*(c.y-a.y)-(c.x-a.x)*(b.y-a.y);
	float s = sqrtf(A*A+B*B+C*C);
	if(s > FLT_EPS)
	{
		s=1/s;
		A*=s;
		B*=s;
		C*=s;
	}else
	{
		A=1;B=0;C=0;
	}

	D=-A*a.x-B*a.y-C*a.z;
}

float Plane::calcCross(const Vect3f& a,const Vect3f& b) const
{
	Vect3f v=a-b;
	float t=A*v.x+B*v.y+C*v.z;
	if(t==0) return 0;			// прямая и плоскость параллельны
	t=(A*a.x+B*a.y+C*a.z+D)/t;
	return t;
}

void Plane::xform(MatXf &mat) 
{
	Vect3f p;
	mat.xformPoint(normal()*D,p);
	normal()=mat.xformVect(normal());
	D=-A*p.x-B*p.y-C*p.z;
}

void Plane::reflectionMatrix(const MatXf &in,MatXf &out) const 
{
	MatXf RefSurface(	Mat3f(	1-2*A*A,	-2*A*B,		-2*A*C,
		-2*B*A,		1-2*B*B,	-2*B*C,
		-2*C*A,		-2*C*B,		1-2*C*C),
		Vect3f(	-2*D*A,		-2*D*B,		-2*D*C));
	out=in*RefSurface;
}

void Plane::reflectionVector(const Vect3f& in,Vect3f& out) const 
{
	out=in-2*dot(normal(),in)*normal();
}