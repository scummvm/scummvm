#ifndef __UTF8_WPAPPER_
#define __UTF8_WPAPPER_

class UTF8Wpapper
{
public:
	UTF8Wpapper(string& utf8string) : utf8_(&utf8string)	{
		utf8c_ = 0;
		utf8Size_ = 0;
	}

	UTF8Wpapper(char* utf8string, size_t size) : utf8c_(utf8string), utf8Size_(size) {
		utf8_ = 0;
	}

	bool serialize(Archive& ar, const char* name, const char* nameAlt);
private:
	string* utf8_;
	char* utf8c_;
	size_t utf8Size_;
};



#endif //__UTF8_WPAPPER_