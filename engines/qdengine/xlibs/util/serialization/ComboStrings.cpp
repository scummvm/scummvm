#include "StdAfx.h"
#include "ComboStrings.h"

string cutTokenFromComboList(string& comboList) {
	int pos = comboList.find("|");
	if(pos == string::npos){
		string name = comboList;
		comboList = "";
		return name;
	}
	else{
		string name(comboList, 0, pos);
		comboList.erase(0, pos + 1);
		return name;
	}
}

void joinComboList(string& out, const ComboStrings& strings, char delimeter)
{
	ComboStrings::const_iterator it;
	FOR_EACH(strings, it){
		out += *it;
		out.push_back(delimeter);
	}
	if(!out.empty())
		out.pop_back();
}

void joinComboListW(wstring& out, const ComboWStrings& strings, wchar_t delimeter)
{
	ComboWStrings::const_iterator it;
	FOR_EACH(strings, it){
		out += *it;
		out.push_back(delimeter);
	}
	if(!out.empty())
		out.pop_back();
}

void splitComboList(ComboStrings& combo_array, const char* ptr, char delimeter)
{
	combo_array.clear();

	const char* start = ptr;

	for(; *ptr; ++ptr)
		if(*ptr == delimeter){
			combo_array.push_back(string(start, ptr));
			start = ptr + 1;
		}
		combo_array.push_back(string(start, ptr));
}

void splitComboListW(ComboWStrings& combo_array, const wchar_t* ptr, wchar_t delimeter)
{
	combo_array.clear();

	const wchar_t* start = ptr;

	for(; *ptr; ++ptr)
		if(*ptr == delimeter){
			combo_array.push_back(wstring(start, ptr));
			start = ptr + 1;
		}
		combo_array.push_back(wstring(start, ptr));
}

int indexInComboListString(const char* combo_string, const char* value)
{
	if(!combo_string || !value)
		return -1;

	if(*combo_string == 0 && *value == 0)
		return 0;

	size_t value_len = strlen(value);
	const char* ptr = combo_string;

	const char* token_start = ptr;
	const char* token_end = ptr;

	int index = 0;
	while(*ptr != '\0') {
		if(*ptr != '|')
			token_end = ptr+1;
		if(*ptr == '|' || *(ptr + 1) == '\0') {
			size_t len = token_end - token_start;
			if(len == value_len && strncmp(token_start, value, len) == 0)
				return index;
			++index;
			token_start = ptr+1;
			token_end = ptr+1;
		}
		++ptr;
	}
	return -1;
}

int indexInComboListStringW(const wchar_t* combo_string, const wchar_t* value)
{
	if(!combo_string || !value)
		return -1;

	if(*combo_string == 0 && *value == 0)
		return 0;

	size_t value_len = wcslen(value);
	const wchar_t* ptr = combo_string;

	const wchar_t* token_start = ptr;
	const wchar_t* token_end = ptr;

	int index = 0;
	while(*ptr != L'\0') {
		if(*ptr != L'|')
			token_end = ptr+1;
		if(*ptr == L'|' || *(ptr + 1) == L'\0') {
			size_t len = token_end - token_start;
			if(len == value_len && wcsncmp(token_start, value, len) == 0)
				return index;
			++index;
			token_start = ptr+1;
			token_end = ptr+1;
		}
		++ptr;
	}
	return -1;
}

string getStringTokenByIndex(const char* worker, int number)
{
	int current = 0;
	const char* begin = worker;
	while(*worker){
		if(*worker == '|')
			if(current == number)
				break;
			else{
				++current;
				begin = worker + 1;
			}
			++worker;
	}
	if(current == number && worker > begin)
		return string(begin, worker);
	return string("");
}

wstring getStringTokenByIndexW(const wchar_t* worker, int number)
{
	int current = 0;
	const wchar_t* begin = worker;
	while(*worker){
		if(*worker == L'|')
			if(current == number)
				break;
			else{
				++current;
				begin = worker + 1;
			}
			++worker;
	}
	if(current == number && worker > begin)
		return wstring(begin, worker);
	return wstring(L"");
}
