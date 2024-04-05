#ifndef __UNICODE_CONVERTER_H__
#define __UNICODE_CONVERTER_H__
#include <string>

namespace serialization_helpers
{
	template<class unicode_string>
	struct t_ansi_to_unicode
	{
		typedef unicode_string                                    string_type;
		typedef typename string_type::value_type                  char_type;

		static bool convert(string_type& result, LPCSTR src, size_t len = -1, UINT codepage = CP_ACP);
	};

	// --------------------------------------------------------------------------

	template<class ansi_string>
	struct t_unicode_to_ansi
	{
		typedef ansi_string                                       string_type;
		typedef typename string_type::value_type                  char_type;

		static bool convert(string_type& result, LPCWSTR src, size_t len = -1, UINT codepage = CP_ACP);
	};
}; //namespace serialization_helpers

typedef serialization_helpers::t_ansi_to_unicode<std::wstring> ansi_to_utf16;
typedef serialization_helpers::t_unicode_to_ansi<std::string> utf16_to_ansi;

// --------------------------------------------------------------------------

inline bool a2w(std::wstring& result, LPCSTR src, size_t len, UINT codepage){
	return ansi_to_utf16::convert(result, src, len, codepage);
}

inline bool a2w(std::wstring& result, LPCSTR src, UINT codepage){
	return ansi_to_utf16::convert(result, src, -1, codepage);
}

inline bool a2w(std::wstring& result, LPCSTR src){
	return ansi_to_utf16::convert(result, src);
}

inline bool a2w(std::wstring& result, const std::string& src, UINT codepage){
	return ansi_to_utf16::convert(result, src.c_str(), src.size(), codepage);
}

inline bool a2w(std::wstring& result, const std::string& src){
	return ansi_to_utf16::convert(result, src.c_str(), src.size());
}

std::wstring a2w(LPCSTR src, size_t len, UINT codepage);
std::wstring a2w(LPCSTR src, UINT codepage);
std::wstring a2w(LPCSTR src);

std::wstring a2w(const std::string& src, UINT codepage);
std::wstring a2w(const std::string& src);

// --------------------------------------------------------------------------

inline bool w2a(std::string& result, LPCWSTR src, size_t len, UINT codepage){
	return utf16_to_ansi::convert(result, src, len, codepage);
}

inline bool w2a(std::string& result, LPCWSTR src, UINT codepage){
	return utf16_to_ansi::convert(result, src, -1, codepage);
}

inline bool w2a(std::string& result, LPCWSTR src){
	return utf16_to_ansi::convert(result, src);
}

inline bool w2a(std::string& result, const std::wstring& src, UINT codepage){
	return utf16_to_ansi::convert(result, src.c_str(), src.size(), codepage);
}

inline bool w2a(std::string& result, const std::wstring& src){
	return utf16_to_ansi::convert(result, src.c_str(), src.size());
}

std::string w2a(LPCWSTR src, size_t len, UINT codepage);
std::string w2a(LPCWSTR src, UINT codepage);
std::string w2a(LPCWSTR src);

std::string w2a(const std::wstring& src, UINT codepage);
std::string w2a(const std::wstring& src);

// --------------------------------------------------------------------------
/// Microsoft based conversion, good for wstring
void ucs2_to_utf8(const wchar_t* ucs2, int size16, char* utf8, int size8);

/// ISO Standart conversion
void utf16_to_utf8(const wchar_t* utf16, int size16, char* utf8, int size8);

void utf32_to_utf8(const unsigned int* utf32, int size32, char* utf8, int size8);

void utf8_to_utf16(const char* utf8, int size8, wchar_t* utf16, int size16);

void utf8_to_utf32(const char* utf8, int size8, unsigned int* utf32, int size16);

class XBuffer;
class WBuffer;

// количесво байт, достаточное, для представления исходного utf16 текста в кодировке utf8
size_t getUTF8size(const wchar_t* ucs2);
const char* toUTF8(XBuffer& buf, const std::wstring& ucs2);
const char* toUTF8(XBuffer& buf, const wchar_t* ucs2);

// количесво utf16 символов, достаточное, для представления исходного utf8 текста
size_t getUTF16size(const char* utf8);
const wchar_t* fromUTF8(WBuffer& buf, const std::string& utf8);
const wchar_t* fromUTF8(WBuffer& buf, const char* utf8);

#endif //__UNICODE_CONVERTER_H__
