#include "StdAfx.h"
#include "KeysBase.h"

float KeyBase::time_delta=0.05f;//в секундах
KeyColor::value KeyColor::none(0,0,0,0);

void KeyBase::serialize(Archive& ar)
{
	ar.serialize(time, "time", "time");
}

void KeyColor::serialize(Archive& ar)
{
	KeyBase::serialize(ar);
	Color4f color (r, g, b, a);
	ar.serialize(color, "color", "Цвет");
	r = color.r;
	g = color.g;
	b = color.b;
	a = color.a;
}

void KeysColor::serialize(Archive& ar)
{
	ar.serialize((vector<KeyColor>&)*this, "color", "color");
}

void KeysColor::MulToColor(Color4f color)
{
	for(iterator it=begin();it!=end();++it)
	{
		Color4f& c=*it;
		c*=color;
	}
}
