#ifndef __GUID_SERIALIZATION_H_INCLUDED__
#define __GUID_SERIALIZATION_H_INCLUDED__

class Archive;

class XGUID : public GUID
{
public:
	XGUID() {}
	XGUID(const GUID& gd);
	void generate();

	bool serialize(Archive& ar, const char* name, const char* nameAlt);

	static const XGUID ZERO;
};

typedef vector<XGUID> GUIDcontainer;

#endif //__GUID_SERIALIZATION_H_INCLUDED__
