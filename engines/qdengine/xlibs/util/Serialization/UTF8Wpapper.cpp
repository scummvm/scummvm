#include "StdAfx.h"
#include "UTF8Wrapper.h"
#include "Serialization\Serialization.h"
#include "WBuffer.h"
#include "UnicodeConverter.h"

size_t getUTF16size(const char* utf8);

bool UTF8Wpapper::serialize(Archive& ar, const char* name, const char* nameAlt)
{
	wstring ucs2;

	if(ar.isOutput()){
		WBuffer buffer;
		if(utf8_)
			fromUTF8(buffer, *utf8_);
		else {
			utf8c_[utf8Size_ - 1] = 0;
			fromUTF8(buffer, utf8c_);
		}

		ucs2 = buffer.c_str();
	}

	bool ret = ar.serialize(ucs2, name, nameAlt);

	if(ar.isInput()){
		if(ret){
			XBuffer utf8;
			toUTF8(utf8, ucs2);

			if(utf8_)
				*utf8_ = utf8.c_str();
			else {
				strncpy(utf8c_, utf8.c_str(), utf8Size_ - 1);
				utf8c_[utf8Size_ - 1] = 0;
			}
			return true;
		}	
	}
	
	return ret;
}
