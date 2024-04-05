#ifndef __COMBOLISTVECTOR_H_INCLUDED__
#define __COMBOLISTVECTOR_H_INCLUDED__

#include "Serialization\SerializationTypes.h"
#include <vector>

// выбор строки из вектора строк с возвратом индекса
struct ComboVectorString : public ComboListString
{
	ComboVectorString(vector<string> strings, int value = 0, bool zeroValue = false);
	
	ComboVectorString(const char* lst, int value = 0);

	int value() const;

	bool serialize(Archive& ar, const char* name, const char* nameAlt);
};

struct ComboVectorWString
{
	ComboVectorWString(vector<wstring> strings, int value = 0, bool zeroValue = false);
	ComboVectorWString(const wchar_t* lst, int value = 0);

	ComboVectorWString& operator=(const wchar_t* val) { value_ = val ? val : L""; return *this; }
	ComboVectorWString& operator=(const wstring& val) { value_ = val; return *this; }

	operator const wchar_t*() const { return value_.c_str(); }
	const wchar_t* comboList() const { return comboList_.c_str(); }
	void setComboList(const wchar_t* comboList) { comboList_ = comboList; }

	wstring& getValue() { return value_; }
	const wstring& getValue() const { return value_; }
	int value() const;

	bool serialize(Archive& ar, const char* name, const char* nameAlt);

private:
	wstring value_;
	wstring comboList_;
};

#endif
