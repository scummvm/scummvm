#pragma once

#include "XMath\xmath.h"

///////////////////////////////////////////////
// Плоскость, заданная нормальным уравнением
///////////////////////////////////////////////

class Plane
{
public:
	Plane()												{ }
	Plane(float a,float b,float c,float d)				{ A=a,B=b,C=c,D=d; }
	Plane(const Vect3f& a,const Vect3f& b,const Vect3f& c) { set(a,b,c); }
	void set(const Vect3f& a,const Vect3f& b,const Vect3f& c); 	// инициализация плоскости по трем точкам

	const Vect3f& normal() const { return *(Vect3f*)&A; }
	float distance(const Vect3f& a) const { return A*a.x+B*a.y+C*a.z+D; }

	void xform(MatXf& mat);

	float calcCross(const Vect3f& a,const Vect3f& b) const; // поиск пересечения данной плоскости с прямой заданной двумя точками a и b
	void reflectionMatrix(const MatXf &in,MatXf &out) const;
	void reflectionVector(const Vect3f& in,Vect3f& out) const;

private:
	float A,B,C,D;

	Vect3f& normal() { return *(Vect3f*)&A; }
};
