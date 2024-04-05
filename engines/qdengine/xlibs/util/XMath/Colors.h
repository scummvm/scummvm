#pragma once

#include "XMath.h"

class Archive;
struct Color3c;

typedef unsigned long DWORD;

struct Color4f
{
	float r,g,b,a;
	
	Color4f() 											{ }
	Color4f(float r_,float g_,float b_,float a_=1.f)		{ r = r_; g = g_; b = b_; a = a_; }
	explicit Color4f(const struct Color4c& color);

	void set(float r_,float g_,float b_,float a_)	{ r = r_; g = g_; b = b_; a = a_; }

	void setHSV(float h,float s,float v, float alpha = 1.f);

	Color4f& operator += (const Color4f &color)	{ r+=color.r; g+=color.g; b+=color.b; a+=color.a; return *this; }
	Color4f& operator -= (const Color4f &color)	{ r-=color.r; g-=color.g; b-=color.b; a-=color.a; return *this; }
	Color4f& operator *= (const Color4f &color)	{ r*=color.r; g*=color.g; b*=color.b; a*=color.a; return *this; }
	Color4f& operator *= (float f)			{ r*=f; g*=f; b*=f; a*=f; return *this; }
	Color4f& operator /= (float f)			{ if(f!=0) f=1/f; else f=0.001f; r*=f; g*=f; b*=f; a*=f; return *this; }
	Color4f	operator + (const Color4f &color) const	{ Color4f tmp(r+color.r,g+color.g,b+color.b,a+color.a); return tmp; }
	Color4f	operator - (const Color4f &color) const	{ Color4f tmp(r-color.r,g-color.g,b-color.b,a-color.a); return tmp; }
	Color4f	operator * (const Color4f &color) const	{ Color4f tmp(r*color.r,g*color.g,b*color.b,a*color.a); return tmp; }
	Color4f	operator * (float f) const 		{ Color4f tmp(r*f,g*f,b*f,a*f); return tmp; }
	Color4f	operator / (float f) const 		{ if(f!=0.f) f=1/f; else f=0.001f; Color4f tmp(r*f,g*f,b*f,a*f); return tmp; }
	void mul3(const Color4f& x,const Color4f& y){r=x.r*y.r;g=x.g*y.g;b=x.b*y.b;}

	int GetR() const 						{ return round(255*r); }
	int GetG() const 						{ return round(255*g); }
	int GetB() const 						{ return round(255*b); }
	int GetA() const 						{ return round(255*a); }
	DWORD RGBA() const 						{ return (round(255*r) << 16) | (round(255*g) << 8) | round(255*b) | (round(255*a) << 24); }
	DWORD GetRGB() const 					{ return (round(255*r) << 16) | (round(255*g) << 8) | round(255*b); }
	DWORD RGBGDI() const 					{ return round(255*r) | (round(255*g) << 8) | (round(255*b) << 16); }
	void interpolate(const Color4f &u,const Color4f &v,float f) { r=u.r+(v.r-u.r)*f; g=u.g+(v.g-u.g)*f; b=u.b+(v.b-u.b)*f; a=u.a+(v.a-u.a)*f; }
	void interpolate3(const Color4f &u,const Color4f &v,float f) { r=u.r+(v.r-u.r)*f; g=u.g+(v.g-u.g)*f; b=u.b+(v.b-u.b)*f; }
	bool operator == (const Color4f &color) const { return fabs(r - color.r) < FLT_EPS && fabs(g - color.g) < FLT_EPS && fabs(b - color.b) < FLT_EPS && fabs(a - color.a) < FLT_EPS; }
	bool operator != (const Color4f &color) const { return !(*this == color); }

	void serialize(Archive& ar);

	operator const Vect4f& () const { return *((Vect4f*)this); }

	static const Color4f WHITE;
	static const Color4f BLACK;
	static const Color4f RED;
	static const Color4f GREEN;
	static const Color4f BLUE;
	static const Color4f YELLOW;
	static const Color4f MAGENTA;
	static const Color4f CYAN;
	static const Color4f ZERO;
};

struct Color4c
{
	union{
		struct{ unsigned char b,g,r,a; };
		struct{ unsigned long argb; };
	};
	
	Color4c()										{ }
	Color4c(const Color4f& color)					{ set(color.GetR(),color.GetG(),color.GetB(),color.GetA()); }
	Color4c(const Color3c& color);
	Color4c(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a=255) { r=_r; g=_g; b=_b; a=_a; }
	explicit Color4c(unsigned long _argb) { argb=_argb; }
	void set(int rc,int gc,int bc,int ac=255)	{ r=rc; g=gc; b=bc; a=ac; }
	void set(const Color4f& color)			{ set(color.GetR(),color.GetG(),color.GetB(),color.GetA()); }
	
	Color4c& setSafe(const Color4f& color)		{ set(clamp(color.GetR(),0,255), clamp(color.GetG(),0,255), clamp(color.GetB(),0,255), clamp(color.GetA(),0,255)); return *this; }
	Color4c& setSafe1(const Color4f& color)		{ set(clamp(color.GetR(),0,255), clamp(color.GetG(),0,255), clamp(color.GetB(),0,255), clamp(color.GetA(),0,255)); return *this; }

	Color4c& setGDI(unsigned long color);
	void setHSV(float h,float s,float v, unsigned char alpha = 255);

	Color4c& operator *= (float f)			{ r=round(r*f); g=round(g*f); b=round(b*f); a=round(a*f); return *this; }
	Color4c& operator += (Color4c &p)		{ r+=p.r; g+=p.g; b+=p.b; a+=p.a; return *this; }
	Color4c& operator -= (Color4c &p)		{ r-=p.r; g-=p.g; b-=p.b; a-=p.a; return *this; }
	Color4c operator + (Color4c &p)		{ return Color4c(r+p.r,g+p.g,b+p.b,a+p.a); }
	Color4c operator - (Color4c &p)		{ return Color4c(r-p.r,g-p.g,b-p.b,a-p.a); }
	Color4c operator * (int f) const 		{ return Color4c(r*f,g*f,b*f,a*f); }
	Color4c operator / (int f) const 		{ if(f!=0) f=(1<<16)/f; else f=1<<16; return Color4c((r*f)>>16,(g*f)>>16,(b*f)>>16,(a*f)>>16); }
	
	bool operator==(const Color4c& rhs) const{ return argb == rhs.argb; }
	bool operator!=(const Color4c& rhs) const{ return argb != rhs.argb; }
	
	DWORD  RGBA() const 						{ return ((const DWORD*)this)[0]; }
	DWORD& RGBA()							{ return ((DWORD*)this)[0]; }
	DWORD RGBGDI() const 					{ return r | g << 8 | b << 16; }
	void HSV(float& h,float& s,float& v);
	unsigned char& operator[](int i)				{ return ((unsigned char*)this)[i];}
	void interpolate(const Color4c &u,const Color4c &v,float f) { r=round(u.r+int(v.r-u.r)*f); g=round(u.g+int(v.g-u.g)*f); b=round(u.b+int(v.b-u.b)*f); a=round(u.a+(v.a-u.a)*f); }
	void serialize(Archive& ar);

	static const Color4c WHITE;
	static const Color4c BLACK;
	static const Color4c RED;
	static const Color4c GREEN;
	static const Color4c BLUE;
	static const Color4c YELLOW;
	static const Color4c MAGENTA;
	static const Color4c CYAN;
	static const Color4c ZERO;
};

struct Color3c
{
	unsigned char b,g,r;
	
	Color3c()										{ }
	Color3c(int rc,int gc,int bc)                  { r=rc; g=gc; b=bc; }
	explicit Color3c(const Color4f& color)					{ set(color.GetR(),color.GetG(),color.GetB()); }
	explicit Color3c(const Color4c& color)					{ set(color.r, color.g, color.b); }

	inline void set(int rc,int gc,int bc)	{ r=rc; g=gc; b=bc; }
	inline void set(const Color4f& color)			{ set(color.GetR(),color.GetG(),color.GetB()); }
	inline Color3c& operator *= (float f)			{ r=round(r*f); g=round(g*f); b=round(b*f); return *this; }
	inline Color3c& operator += (Color3c &p)		{ r+=p.r; g+=p.g; b+=p.b; return *this; }
	inline Color3c& operator -= (Color3c &p)		{ r-=p.r; g-=p.g; b-=p.b; return *this; }
	inline Color3c operator + (Color3c &p)		{ return Color3c(r+p.r,g+p.g,b+p.b); }
	inline Color3c operator - (Color3c &p)		{ return Color3c(r-p.r,g-p.g,b-p.b); }
	inline Color3c operator * (int f) const 		{ return Color3c(r*f,g*f,b*f); }
	inline Color3c operator / (int f) const 		{ if(f!=0) f=(1<<16)/f; else f=1<<16; return Color3c((r*f)>>16,(g*f)>>16,(b*f)>>16); }
	unsigned char& operator[](int i)				{ return ((unsigned char*)this)[i];}
	__forceinline void interpolate(const Color3c &u,const Color3c &v,float f) { r=round(u.r+int(v.r-u.r)*f); g=round(u.g+int(v.g-u.g)*f); b=round(u.b+int(v.b-u.b)*f); }
	void serialize(Archive& ar);
};


inline Color4f::Color4f(const Color4c& color) 
{ 
	r = color.r/255.f; 
	g = color.g/255.f; 
	b = color.b/255.f; 
	a = color.a/255.f; 
}

inline Color4c::Color4c(const Color3c& color)
{
	set(color.r,color.g,color.b,255);
}
