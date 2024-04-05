#include "Stdafx.h"
#include "TextDB.h"
#include "Serialization\Serialization.h"
#include "Serialization\XPrmArchive.h"
#include "Serialization\MultiArchive.h"
#include "Game\GameOptions.h"
#include "UnicodeConverter.h"

TextDB::TextDB()
{
	loadLanguage(GameOptions::instance().getLanguage());
}

TextDB::TextDB(const char* language)
{
	loadLanguage(language);
}

bool TextDB::loadLanguage(const char* language)
{
	if(language_ != language){
		language_ = language;
		string fileName = string("Resource\\LocData\\") + language_ + "\\Text\\Texts.tdb";
		clear();
		XPrmIArchive ia;
		if(ia.open(fileName.c_str())){
			if(!serialize(ia, "TextDB_unicode", 0)){ /// CONVERSION 2008-1-18
				StaticMap<string, string> old_db;
				old_db.serialize(ia, "TextDB", 0);
				clear();
				int codepage = _stricmp(language_.c_str(), "russian") == 0 ? 1251 : 1252;
				StaticMap<string, string>::const_iterator it;
				FOR_EACH(old_db, it)
					insert(make_pair(it->first, a2w(it->second, codepage)));
			}
		}
		else
			return false;
	}
	return true;
}

void TextDB::saveLanguage()
{
	string fileName = string("Resource\\LocData\\") + language_ + "\\Text\\Texts.tdb";
	XPrmOArchive oa(fileName.c_str());
	serialize(oa, "TextDB_unicode", "");
}

bool TextDB::serialize(Archive& ar, const char* name, const char* nameAlt)
{
	return ar.serialize(static_cast<StaticMap<string, wstring>&>(*this), name, nameAlt);
}

const wchar_t* TextDB::getText(const char* text_id) 
{
	TextDB::const_iterator it = (*this).find(text_id);
	if(it != (*this).end())
		return it->second.c_str();

	a2w((*this)[text_id], text_id);

	return (*this)[text_id].c_str();
}


