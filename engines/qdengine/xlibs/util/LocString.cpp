#include "StdAfx.h"
#include "LocString.h"
#include "Serialization\Serialization.h"
#include "TextDB.h"

void LocString::serialize(Archive& ar)
{
	ar.serialize(key_, "key", "^");
	if(ar.inPlace())
		ar.serialize(text_, "text", 0);

	if(ar.isInput())
		update();
}

void LocString::update()
{
	text_ = TextDB::instance().getText(key_.c_str());
}