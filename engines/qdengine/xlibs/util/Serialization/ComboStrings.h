#pragma once

typedef vector<string> ComboStrings;
typedef vector<wstring> ComboWStrings;

/// Вспомогательная функция для работы с комбо-листами
string cutTokenFromComboList(string& comboList);
void joinComboList(string& outComboList, const ComboStrings& strings, char delimeter = '|');
void splitComboList(ComboStrings& outComboStrings, const char* comboList, char delimeter = '|');
// TODO: переписать
int indexInComboListString(const char* comboList, const char* value);

void splitComboListW(ComboWStrings& combo_array, const wchar_t* ptr, wchar_t delimeter);
wstring getStringTokenByIndexW(const wchar_t* worker, int number);
int indexInComboListStringW(const wchar_t* comboList, const wchar_t* value);
