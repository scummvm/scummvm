#include "qdengine/xlibs/xutil/xglobal.h"
#include "UnicodeConverter.h"

namespace serialization_helpers {
template<class unicode_string>
bool t_ansi_to_unicode<unicode_string>::convert(string_type &result, LPCSTR str, size_t str_len, UINT codepage) {
	if (str == NULL || str_len == 0) {
		result.erase();
		return true;
	}

	//åñëè str_len == -1, òî MBTWC ó÷èòûâàåò çàâåðøàþùèé íóëåâîé ñèìâîë
	int wlen = ::MultiByteToWideChar(codepage, 0, str, str_len, NULL, 0);

	xassert(wlen >= 0);
	if (wlen == 0 || (wlen == 1 && str_len == -1)) { //error èëè str==""
		result.erase();
		return ::GetLastError();
	}

	if (str_len != -1)
		wlen += 1; //ó÷èòûâàåì çàâåðøàþùèé UNICODE-ñèìâîë

	xassert(wlen > 1); //âåðíåì íå òîëüêî çàâåðøàþùèé ñèìâîë

	//ðåçåðâèðóåì ìåñòî ïîä âñå ñèìâîëû (âêëþ÷àÿ è òåðìèíàëüíûé)
	result.resize(wlen);

	::MultiByteToWideChar(codepage, 0, str, str_len, const_cast<LPWSTR>(result.c_str()), wlen);

	//óäàëÿåì òåðìèíàëüíûé UNICODE-ñèìâîë
	result.erase(result.size() - 1, 1);

	return true;
}

// --------------------------------------------------------------------------

template<class ansi_string>
bool t_unicode_to_ansi<ansi_string>::convert(string_type &result, LPCWSTR wstr, size_t wstr_len, UINT codepage) {
	if (wstr == NULL || wstr_len == 0) {
		result.erase();
		return true;
	}

	//åñëè wstr_len == -1, òî WCTMB ó÷èòûâàåò çàâåðøàþùèé íóëåâîé ñèìâîë
	int alen = ::WideCharToMultiByte(codepage, 0, wstr, wstr_len, NULL, 0, NULL, NULL);

	xassert(alen >= 0);
	if (alen == 0 || (alen == 1 && wstr_len == -1)) { //error ��� wstr==L""
		result.erase();
		return ::GetLastError();
	}

	//HINT: Çäåñü ïðåäïîëàãàåòñÿ, ÷òî äëÿ âñåõ ANSI êîäèðîâîê òåðìèíàëüíûé ñèìâîë
	//      çàíèìàåò 1 áàéò
	if (wstr_len != -1)
		alen += 1; //ó÷èòûâàåì çàâåðøàþùèé ñèìâîë

	xassert(alen > 1); //âåðíåì íå òîëüêî çàâåðøàþùèé ñèìâîë

	//ðåçåðâèðóåì ìåñòî ïîä âñå ñèìâîëû (âêëþ÷àÿ è òåðìèíàëüíûé)
	result.resize(alen);

	::WideCharToMultiByte(codepage, 0, wstr, wstr_len, const_cast<LPSTR>(result.c_str()), alen, NULL, NULL);

	//èñêëþ÷àåì òåðìèíàëüíûé ñèìâîë
	//HINT: Óâåðåííîñòü îòíîñèòåëüíî îäíîãî áàéòà?
	result.erase(result.size() - 1, 1);

	return true;
}

}; //namespace serialization_helpers

// --------------------------------------------------------------------------

std::wstring a2w(LPCSTR src, size_t len, UINT codepage) {
	std::wstring result;
	ansi_to_utf16::convert(result, src, len, codepage);
	return result;
}

std::wstring a2w(LPCSTR src, UINT codepage) {
	std::wstring result;
	ansi_to_utf16::convert(result, src, -1, codepage);
	return result;
}

std::wstring a2w(LPCSTR src) {
	std::wstring result;
	ansi_to_utf16::convert(result, src);
	return result;
}

std::wstring a2w(const std::string &src, UINT codepage) {
	std::wstring result;
	ansi_to_utf16::convert(result, src.c_str(), src.size(), codepage);
	return result;
}

std::wstring a2w(const std::string &src) {
	std::wstring result;
	ansi_to_utf16::convert(result, src.c_str(), src.size());
	return result;
}

// --------------------------------------------------------------------------

std::string w2a(LPCWSTR src, size_t len, UINT codepage) {
	std::string result;
	utf16_to_ansi::convert(result, src, len, codepage);
	return result;
}

std::string w2a(LPCWSTR src, UINT codepage) {
	std::string result;
	utf16_to_ansi::convert(result, src, -1, codepage);
	return result;
}

std::string w2a(LPCWSTR src) {
	std::string result;
	utf16_to_ansi::convert(result, src);
	return result;
}

std::string w2a(const std::wstring &src, UINT codepage) {
	std::string result;
	utf16_to_ansi::convert(result, src.c_str(), src.size(), codepage);
	return result;
}

std::string w2a(const std::wstring &src) {
	std::string result;
	utf16_to_ansi::convert(result, src.c_str(), src.size());
	return result;
}
