#include "stdafx.h"
#include "Colors.h"
#include "Serialization\Serialization.h"
#include "Windows.h"

#pragma warning(disable : 4073 )
#pragma init_seg(lib)

const Color4f Color4f::WHITE(1, 1, 1, 1);
const Color4f Color4f::BLACK(0, 0, 0, 1);
const Color4f Color4f::RED(1, 0, 0, 1);
const Color4f Color4f::GREEN(0, 1, 0, 1);
const Color4f Color4f::BLUE(0, 0, 1, 1);
const Color4f Color4f::YELLOW(1, 1, 0, 1);
const Color4f Color4f::MAGENTA(1, 0, 1, 1);
const Color4f Color4f::CYAN(0, 1, 1, 1);
const Color4f Color4f::ZERO(0, 0, 0, 0);

const Color4c Color4c::WHITE(255, 255, 255);
const Color4c Color4c::BLACK(0, 0, 0);
const Color4c Color4c::RED(255, 0, 0);
const Color4c Color4c::GREEN(0, 255, 0);
const Color4c Color4c::BLUE(0, 0, 255);
const Color4c Color4c::YELLOW(255, 255, 0);
const Color4c Color4c::MAGENTA(255, 0, 255);
const Color4c Color4c::CYAN(0, 255, 255);
const Color4c Color4c::ZERO(0, 0, 0, 0);

Color4c& Color4c::setGDI(unsigned long color)
{
	if(color){
		r = GetRValue(color);
		g = GetGValue(color);
		b = GetBValue(color);
		a = 255;
	}
	else
		*this = WHITE;
	return *this;
}


void Color4c::serialize(Archive& ar) 
{
	ar.serialize(r, "r", "&r");
	ar.serialize(g, "g", "&g");
	ar.serialize(b, "b", "&b");
	ar.serialize(a, "a", "&a");
}

void Color4f::serialize(Archive& ar) 
{
	ar.serialize(r, "r", "&r");
	ar.serialize(g, "g", "&g");
	ar.serialize(b, "b", "&b");
	ar.serialize(a, "a", "&a");
}

void Color3c::serialize(Archive& ar) 
{
	ar.serialize(r, "r", "&r");
	ar.serialize(g, "g", "&g");
	ar.serialize(b, "b", "&b");
}


// HSV
//Y = 0.30*R + 0.59*G + 0.11*B перевод цветного в чёрно-белый
//h=0..360,s=0..1,v=0..1

inline void HSVtoRGB(float h,float s,float v,
					 float& r,float& g,float& b)
{
	const float min=1e-5f;
	int i;
	float f,m,n,k;

	if(s<min){
		r=g=b=v;
	}
	else {
		if(h>=360)
			h=0;
		else
			h=h/60.;

		i=round(floor(h));
		f=h-i;
		m=v*(1-s);
		n=v*(1-s*f);
		k=v*(1-s*(1-f));

		switch(i){
		case 0:
			r=v; g=k; b=m;
			break;
		case 1:
			r=n; g=v; b=m;
			break;
		case 2:
			r=m; g=v; b=k;
			break;
		case 3:
			r=m; g=n; b=v;
			break;
		case 4:
			r=k; g=m; b=v;
			break;
		case 5:
			r=v; g=m; b=n;
			break;
		default:
			xassert(0);
		}
	}

	xassert(r>=0 && r<=1);
	xassert(g>=0 && g<=1);
	xassert(b>=0 && b<=1);
}

void Color4f::setHSV(float h,float s,float v, float alpha)
{
	HSVtoRGB(h,s,v, r,g,b);
	a = alpha;
}

void Color4c::setHSV(float h,float s,float v, unsigned char alpha)
{
	float rf,gf,bf;
	HSVtoRGB(h,s,v, rf,gf,bf);
	r = round(rf*255);
	g = round(gf*255);
	b = round(bf*255);
	a = alpha;
}

void Color4c::HSV(float& h,float& s,float& v)
{
	float rf = r/255.f;
	float gf = g/255.f;
	float bf = b/255.f;
	v = max(max(rf,gf),bf);
	float temp=min(min(rf,gf),bf);
	if(v==0)
		s=0;
	else 
		s=(v-temp)/v;

	if(s==0)
		h=0;
	else {
		float Cr=(v-rf)/(v-temp);
		float Cg=(v-gf)/(v-temp);
		float Cb=(v-bf)/(v-temp);

		if(rf==v) {
			h=Cb-Cg;
		}
		else if(gf==v) {
			h=2+Cr-Cb;
		} 
		else if(bf==v) {
			h=4+Cg-Cr;
		}

		h=60*h;
		if(h<0)h+=360;
	}
}
