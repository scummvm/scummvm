#include "stdafx.h"
#include <objbase.h>
#include "FileUtils\XGUID.h"
#include "Serialization\Serialization.h"

static const GUID gz = {0, 0, 0, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}};
const XGUID XGUID::ZERO = gz;

XGUID::XGUID(const GUID& gd) 
{ 
	static_cast<GUID&>(*this) = gd;
}

void XGUID::generate()
{
	HRESULT result = CoCreateGuid(this);
	xassert(result == S_OK);
}

bool XGUID::serialize(Archive& ar, const char* name, const char* nameAlt)
{
	static char buf[80];
	
	if(ar.isOutput()){
		int size = sprintf(buf,
			"{0x%08lX, 0x%04hX, 0x%04hX, {0x%02wX, 0x%02wX, 0x%02wX, 0x%02wX, 0x%02wX, 0x%02wX, 0x%02wX, 0x%02wX}}",
			Data1, Data2, Data3, Data4[0],
			Data4[1], Data4[2], Data4[3], Data4[4], Data4[5], Data4[6], Data4[7]);
		xassert(size < sizeof(buf));
	}
	else
		*buf = 0;
	
	string data(buf);
	bool res = ar.serialize(data, name, nameAlt);

	if(res && ar.isInput()){
		int rb[8];
		sscanf(data.c_str(),
			"{%lx, %hx, %hx, {%hx, %hx, %hx, %hx, %hx, %hx, %hx, %hx}}",
			&Data1, &Data2, &Data3,
			&rb[0], &rb[1], &rb[2], &rb[3], &rb[4], &rb[5], &rb[6], &rb[7]);
		for(int idx = 0; idx < 8; ++idx)
			Data4[idx] = (rb[idx] & 0xFF);
	}

	return res;
}


