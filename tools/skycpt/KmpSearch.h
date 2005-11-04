#ifndef __KmpSearch__
#define __KmpSearch__

#ifndef _MSC_VER
#define __fastcall
#endif

class KmpSearch {
public:
	void __fastcall init(const char *subStr);
	char * __fastcall search(const char *str);
//private:
	char _retarget[256], _subStr[256];
	unsigned long _strLen;
};

#endif //__KmpSearch__


