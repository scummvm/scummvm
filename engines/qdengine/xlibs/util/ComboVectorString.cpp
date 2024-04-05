#include "stdafx.h"
#include "ComboVectorString.h"
#include "Serialization\Serialization.h"
#include "UnicodeConverter.h"

string getStringTokenByIndex(const char* worker, int number);

ComboVectorString::ComboVectorString(vector<string> strings, int value, bool zeroValue)
: ComboListString()
{
	string comboList = zeroValue ? "|" : "";
	vector<string>::iterator it;
	FOR_EACH(strings, it){
		if(it != strings.begin())
			comboList += "|";
		comboList += it->c_str();
	}
	setComboList(comboList.c_str());
	(ComboListString&)(*this) = (value >= strings.size() ? "" : strings[value]);
}

ComboVectorString::ComboVectorString(const char* lst, int value)
: ComboListString(lst)
{
	if(value >= 0)
		(ComboListString&)(*this) = getStringTokenByIndex(lst, value);
}

int ComboVectorString::value() const
{
	return indexInComboListString(comboList(), (const char*)(*this));
}

bool ComboVectorString::serialize(Archive& ar, const char* name, const char* nameAlt)
{
	return ar.serialize((ComboListString&)(*this), name, nameAlt);
}

ComboVectorWString::ComboVectorWString(vector<wstring> strings, int value, bool zeroValue)
{
	wstring comboList;
	if(zeroValue)
		comboList = L"|";
	vector<wstring>::iterator it;
	FOR_EACH(strings, it){
		if(it != strings.begin())
			comboList += L"|";
		comboList += it->c_str();
	}
	setComboList(comboList.c_str());
	(*this) = (value >= strings.size() ? L"" : strings[value]);
}

ComboVectorWString::ComboVectorWString(const wchar_t* lst, int value)
{
	setComboList(lst);
	if(value >= 0)
		(*this) = getStringTokenByIndexW(lst, value);
}

int ComboVectorWString::value() const
{
	return indexInComboListStringW(comboList(), (const wchar_t*)(*this));
}

bool ComboVectorWString::serialize(Archive& ar, const char* name, const char* nameAlt)
{
	string buf(w2a(comboList_));
	ComboVectorString combo(buf.c_str(), value());
	if(ar.serialize(combo, name, nameAlt)){
		if(ar.isInput())
			(*this) = getStringTokenByIndexW(comboList_.c_str(), combo.value());
		return true;
	}
	return false;
}
