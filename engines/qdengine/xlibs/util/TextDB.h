#ifndef __TEXTDB_H__
#define __TEXTDB_H__

#include "XTL\StaticMap.h"
#include "XTL\Handle.h"

class TextDB : public StaticMap<std::string, std::wstring>
{
public:
	TextDB();
	TextDB(const char* language);
	
	bool loadLanguage(const char* language);
	void saveLanguage();

	const wchar_t* getText(const char* text_id);

	static TextDB& instance() { return Singleton<TextDB>::instance(); }

private:
	typedef StaticMap<std::string, std::string> Map;
	string language_;

	bool serialize(Archive& ar, const char* name, const char* nameAlt);

	bool load(const char* filename);
};

#endif //__TEXTDB_H__
