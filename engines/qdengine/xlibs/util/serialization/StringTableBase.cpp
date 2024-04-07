#include "stdafx.h"
#include "StringTableBase.h"
#include "Serialization\Serialization.h"

void StringTableBase::serialize(Archive& ar) 
{
	ar.serialize(name_, "|name|first", "&Èìÿ"); // CONVERSION 31.07.07
	if(ar.isEdit())
		ar.serialize(index_, "index", 0);
}

bool StringTableBaseSimple::serialize(Archive& ar, const char* name, const char* nameAlt) 
{
	if(ar.isEdit()){
		if(ar.openStruct((StringTableBase&)(*this), name, nameAlt)){
			ar.serialize(name_, "name", "&Èìÿ");
			ar.serialize(index_, "index", 0);
			ar.closeStruct(name);
		}
		return true;
	}
	else
		return ar.serialize(name_, name, nameAlt);
}

