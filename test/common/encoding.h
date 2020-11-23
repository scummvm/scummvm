#include <cxxtest/TestSuite.h>

#include "common/str.h"
#include "common/ustr.h"
#include "../null_osystem.h"

// We support CJK on all the platforms but it relies on OSystem to read
// file which *in test environments* is available only on some platforms
#if NULL_OSYSTEM_IS_AVAILABLE
#define TEST_CJK 1
#else
#define TEST_CJK 0
#endif

class EncodingTestSuite : public CxxTest::TestSuite
{
public:
	void test_korean() {
#if TEST_CJK
		Common::install_null_g_system();
		const byte utf8[] = {
			0xea, 0xb2, 0x8c, 0xec, 0x9e, 0x84, 0xec, 0xa4,
			0x91, 0xec, 0xa7, 0x80, 0x20, 0xea, 0xb3, 0x84,
			0xec, 0x86, 0x8d, 0xed, 0x95, 0x98, 0xeb, 0xa0,
			0xa4, 0xeb, 0xa9, 0xb4, 0x20, 0xec, 0x8a, 0xa4,
			0xed, 0x8e, 0x98, 0xec, 0x9d, 0xb4, 0xec, 0x8a,
			0xa4, 0xed, 0x82, 0xa4, 0xeb, 0xa5, 0xbc, 0x20,
			0xec, 0xb9, 0x98, 0xec, 0x8b, 0x9c, 0xec, 0x98,
			0xa4, 0x2e, 0x00, 
		};

		const uint32 utf32[] = {
			0xac8c, 0xc784, 0xc911, 0xc9c0, 0x0020, 0xacc4,
			0xc18d, 0xd558, 0xb824, 0xba74, 0x0020, 0xc2a4,
			0xd398, 0xc774, 0xc2a4, 0xd0a4, 0xb97c, 0x0020,
			0xce58, 0xc2dc, 0xc624, 0x002e, 0
		};

		const byte uhc[] = {
			0xb0, 0xd4, 0xc0, 0xd3, 0xc1, 0xdf, 0xc1, 0xf6,
			0x20, 0xb0, 0xe8, 0xbc, 0xd3, 0xc7, 0xcf, 0xb7,
			0xc1, 0xb8, 0xe9, 0x20, 0xbd, 0xba, 0xc6, 0xe4,
			0xc0, 0xcc, 0xbd, 0xba, 0xc5, 0xb0, 0xb8, 0xa6,
			0x20, 0xc4, 0xa1, 0xbd, 0xc3, 0xbf, 0xc0, 0x2e,
			0x00
		};
		Common::U32String ustr_from_utf8((const char *) utf8, Common::kUtf8);
		Common::U32String ustr_from_uhc((const char *) uhc, Common::kWindows949);
		Common::U32String ustr(utf32);
		Common::String utf8_to_uhc = ustr_from_utf8.encode(Common::kWindows949);
		Common::String uhc_to_utf8 = ustr_from_uhc.encode(Common::kUtf8);
		TS_ASSERT_EQUALS(ustr_from_utf8, ustr);
		TS_ASSERT_EQUALS(ustr_from_uhc, ustr);
		TS_ASSERT(strcmp((const char *) utf8, uhc_to_utf8.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) uhc, utf8_to_uhc.c_str()) == 0);
#endif
	}

	void test_chinese() {
#if TEST_CJK
		Common::install_null_g_system();
		const byte utf8[] = {
			0xe9, 0x81, 0x8a, 0xe6, 0x88, 0xb2, 0xe6, 0x9a,
			0xab, 0xe5, 0x81, 0x9c, 0xe3, 0x80, 0x82, 0xe6,
			0x8c, 0x89, 0xe4, 0xb8, 0x8b, 0xe7, 0xa9, 0xba,
			0x21, 0xe7, 0x99, 0xbd, 0xe9, 0x8d, 0xb5, 0xe7,
			0xb9, 0xbc, 0xe7, 0xba, 0x8c, 0xe9, 0x81, 0x8a,
			0xe6, 0x88, 0xb2, 0xe3, 0x80, 0x82, 0x00, 
		};

		const uint32 utf32[] = {
			0x904a, 0x6232, 0x66ab, 0x505c, 0x3002, 0x6309,
			0x4e0b, 0x7a7a, 0x0021, 0x767d, 0x9375, 0x7e7c,
			0x7e8c, 0x904a, 0x6232, 0x3002, 0
		};

		const byte big5[] = {
			0xb9, 0x43, 0xc0, 0xb8, 0xbc, 0xc8, 0xb0, 0xb1,
			0xa1, 0x43, 0xab, 0xf6, 0xa4, 0x55, 0xaa, 0xc5,
			0x21, 0xa5, 0xd5, 0xc1, 0xe4, 0xc4, 0x7e, 0xc4,
			0xf2, 0xb9, 0x43, 0xc0, 0xb8, 0xa1, 0x43, 0x00
		};
		Common::U32String ustr_from_utf8((const char *) utf8, Common::kUtf8);
		Common::U32String ustr_from_big5((const char *) big5, Common::kWindows950);
		Common::U32String ustr(utf32);
		Common::String utf8_to_big5 = ustr_from_utf8.encode(Common::kWindows950);
		Common::String big5_to_utf8 = ustr_from_big5.encode(Common::kUtf8);
		TS_ASSERT_EQUALS(ustr_from_utf8, ustr);
		TS_ASSERT_EQUALS(ustr_from_big5, ustr);
		TS_ASSERT(strcmp((const char *) utf8, big5_to_utf8.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) big5, utf8_to_big5.c_str()) == 0);
#endif
	}

	void test_japanese() {
#if TEST_CJK
		Common::install_null_g_system();
		const byte utf8[] = {
			0xe4, 0xb8, 0x80, 0xe6, 0x99, 0x82, 0xe5, 0x81,
			0x9c, 0xe6, 0xad, 0xa2, 0xe3, 0x80, 0x82, 0xe7,
			0xb6, 0x9a, 0xe3, 0x81, 0x91, 0xe3, 0x82, 0x8b,
			0xe5, 0xa0, 0xb4, 0xe5, 0x90, 0x88, 0xe3, 0x81,
			0xaf, 0xe3, 0x82, 0xb9, 0xe3, 0x83, 0x9a, 0xe3,
			0x83, 0xbc, 0xe3, 0x82, 0xb9, 0xe3, 0x83, 0x90,
			0xe3, 0x83, 0xbc, 0xe3, 0x82, 0x92, 0xe6, 0x8a,
			0xbc, 0xe3, 0x81, 0x97, 0xe3, 0x81, 0xa6, 0xe3,
			0x81, 0x8f, 0xe3, 0x81, 0xa0, 0xe3, 0x81, 0x95,
			0xe3, 0x81, 0x84, 0xe3, 0x80, 0x82, 0
		};

		const uint32 utf32[] = {
			0x4e00, 0x6642, 0x505c, 0x6b62, 0x3002, 0x7d9a,
			0x3051, 0x308b, 0x5834, 0x5408, 0x306f, 0x30b9,
			0x30da, 0x30fc, 0x30b9, 0x30d0, 0x30fc, 0x3092,
			0x62bc, 0x3057, 0x3066, 0x304f, 0x3060, 0x3055,
			0x3044, 0x3002, 0
		};

		const byte cp932[] = {
			0x88, 0xea, 0x8e, 0x9e, 0x92, 0xe2, 0x8e, 0x7e,
			0x81, 0x42, 0x91, 0xb1, 0x82, 0xaf, 0x82, 0xe9,
			0x8f, 0xea, 0x8d, 0x87, 0x82, 0xcd, 0x83, 0x58,
			0x83, 0x79, 0x81, 0x5b, 0x83, 0x58, 0x83, 0x6f,
			0x81, 0x5b, 0x82, 0xf0, 0x89, 0x9f, 0x82, 0xb5,
			0x82, 0xc4, 0x82, 0xad, 0x82, 0xbe, 0x82, 0xb3,
			0x82, 0xa2, 0x81, 0x42, 0
		};
		Common::U32String ustr_from_utf8((const char *) utf8, Common::kUtf8);
		Common::U32String ustr_from_cp932((const char *) cp932, Common::kWindows932);
		Common::U32String ustr(utf32);
		Common::String utf8_to_cp932 = ustr_from_utf8.encode(Common::kWindows932);
		Common::String cp932_to_utf8 = ustr_from_cp932.encode(Common::kUtf8);
		TS_ASSERT_EQUALS(ustr_from_utf8, ustr);
		TS_ASSERT_EQUALS(ustr_from_cp932, ustr);
		TS_ASSERT(strcmp((const char *) utf8, cp932_to_utf8.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) cp932, utf8_to_cp932.c_str()) == 0);
#endif
	}

	void test_conversion_unicode_machine_endian() {
		//  |dolar|   cent    |     euro       |
		unsigned char utf8[] = {0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0};
#ifdef SCUMM_BIG_ENDIAN
		//| dolar |  cent  |    euro   |
		unsigned char utf16be[] = {0, 0x24, 0, 0xA2, 0x20, 0xAC, 0, 0};
		//| dolar       |  cent        |    euro  
		unsigned char utf32be[] = {0, 0, 0, 0x24, 0, 0, 0, 0xA2, 0, 0, 0x20, 0xAC, 0, 0, 0, 0};

		unsigned char *utf16 = utf16be;
		unsigned char *utf32 = utf32be;
#else
		//| dolar |  cent  |    euro   |
		unsigned char utf16le[] = {0x24, 0, 0xA2, 0, 0xAC, 0x20, 0, 0};
		//| dolar       |  cent        |    euro
		unsigned char utf32le[] = {0x24, 0, 0, 0, 0xA2, 0, 0, 0, 0xAC, 0x20, 0, 0, 0, 0, 0, 0};

		unsigned char *utf16 = utf16le;
		unsigned char *utf32 = utf32le;
#endif

		// UTF16 to UTF8
		Common::String resultstr8 = Common::U32String::decodeUTF16Native((uint16 *) utf16, 3).encode(Common::kUtf8);
		TS_ASSERT(resultstr8.c_str() != NULL);
		TS_ASSERT_EQUALS(memcmp(resultstr8.c_str(), utf8, 7), 0)

		// UTF32 to UTF8

		resultstr8 = Common::U32String((uint32 *) utf32, 3).encode(Common::kUtf8);
		TS_ASSERT(resultstr8.c_str() != NULL);
		TS_ASSERT_EQUALS(memcmp(resultstr8.c_str(), utf8, 7), 0);

		// UTF32 to UTF16
		uint16 *result16 = Common::U32String((uint32 *) utf32, 3).encodeUTF16Native(NULL);
		TS_ASSERT(result16 != NULL);
		TS_ASSERT_EQUALS(memcmp(result16, utf16, 8), 0);
		free(result16);

		// UTF8 to UTF16

		result16 = Common::U32String((char *) utf8, 6, Common::kUtf8).encodeUTF16Native(NULL);
		TS_ASSERT(result16 != NULL);
		TS_ASSERT_EQUALS(memcmp(result16, utf16, 8), 0);
		free(result16);

		// UTF8 to UTF32
		Common::U32String resultustr = Common::String((const char *) utf8, 6).decode(Common::kUtf8);
		TS_ASSERT_EQUALS(memcmp(resultustr.c_str(), utf32, 16), 0);

		// UTF16 to UTF32
		resultustr = Common::U32String::decodeUTF16Native((uint16 *) utf16, 3);
		TS_ASSERT_EQUALS(memcmp(resultustr.c_str(), utf32, 16), 0);
	}

	void test_conversion_unicode_big_endian() {
		//  |dolar|   cent    |     euro       |
		unsigned char utf8[] = {0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0};
		//| dolar |  cent  |    euro   |
		unsigned char utf16be[] = {0, 0x24, 0, 0xA2, 0x20, 0xAC, 0, 0};

		// UTF16 to UTF8
		Common::String resultstr8 = Common::U32String::decodeUTF16BE((uint16 *) utf16be, 3).encode(Common::kUtf8);
		TS_ASSERT(resultstr8.c_str() != NULL);
		TS_ASSERT_EQUALS(memcmp(resultstr8.c_str(), utf8, 7), 0);


		// UTF8 to UTF16
		uint16 *result16 = Common::U32String((char *) utf8, 6, Common::kUtf8).encodeUTF16BE(NULL);
		TS_ASSERT(result16 != NULL);
		TS_ASSERT_EQUALS(memcmp(result16, utf16be, 8), 0);
		free(result16);

	}

	void test_conversion_unicode_little_endian() {
		//  |dolar|   cent    |     euro       |
		unsigned char utf8[] = {0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0};
		//| dolar |  cent  |    euro   |
		unsigned char utf16le[] = {0x24, 0, 0xA2, 0, 0xAC, 0x20, 0, 0};

		// UTF16 to UTF8
		Common::String resultstr8 = Common::U32String::decodeUTF16LE((uint16 *) utf16le, 3).encode(Common::kUtf8);
		TS_ASSERT(resultstr8.c_str() != NULL);
		TS_ASSERT_EQUALS(memcmp(resultstr8.c_str(), utf8, 7), 0);

		// UTF8 to UTF16
		uint16 *result16 = Common::U32String((char *) utf8, 6, Common::kUtf8).encodeUTF16LE(NULL);
		TS_ASSERT(result16 != NULL);
		TS_ASSERT_EQUALS(memcmp(result16, utf16le, 8), 0);
		free(result16);
	
	}

	void test_cyrillic_transliteration() {
		unsigned char utf8[] = {/* Z */0xD0, 0x97, /* d */ 0xD0, 0xB4, /* r */ 0xD1, 0x80, /* a */ 0xD0, 0xB0, /* v */ 0xD0, 0xB2, /* s */ 0xD1, 0x81, /* t */ 0xD1, 0x82, /* v */ 0xD0, 0xB2, /* u */ 0xD1, 0x83, /* j */ 0xD0, 0xB9, /* t */ 0xD1, 0x82, /* e */ 0xD0, 0xB5, 0};
		unsigned char iso_8859_5[] = {0xB7, 0xD4, 0xE0, 0xD0, 0xD2, 0xE1, 0xE2, 0xD2, 0xE3, 0xD9, 0xE2, 0xD5, 0};
		unsigned char ascii[] = "Zdravstvujte";

		Common::String result = Common::U32String((const char *) utf8, 24, Common::kUtf8).encode(Common::kASCII);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), ascii, 13), 0);

		result = Common::U32String((const char *) iso_8859_5, 12, Common::kISO8859_5).encode(Common::kASCII);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), ascii, 13), 0);

		result = Common::U32String((const char *) iso_8859_5, 12, Common::kISO8859_5).encode(Common::kUtf8);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), utf8, 25), 0);

		result = Common::U32String((const char *) utf8, 24, Common::kUtf8).encode(Common::kISO8859_5);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), iso_8859_5, 13), 0);

		// this should stay the same
		result = Common::U32String((const char *) ascii, 12, Common::kASCII).encode(Common::kISO8859_5);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), ascii, 13), 0);
	}

	void test_other_conversions() {
		unsigned char cp850[] = {0x99, 0xE0, 0xEA, 0x41, 0x64, 0};
		unsigned char utf8_1[] = {0xC3, 0x96, 0xC3, 0x93, 0xC3, 0x9B, 0x41, 0x64, 0};

		unsigned char iso_8859_2[] = {0xA9, 0xE1, 0x6C, 0x65, 0xE8, 0x65, 0x6B, 0};
		unsigned char utf8_2[] = {0xC5, 0xA0, 0xC3, 0xA1, 0x6C, 0x65, 0xC4, 0x8D, 0x65, 0x6B, 0};

		Common::String result = Common::U32String((const char *) cp850, 5, Common::kDos850).encode(Common::kUtf8);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), utf8_1, 9), 0);

		result = Common::U32String((const char *) utf8_1, 8, Common::kUtf8).encode(Common::kDos850);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), cp850, 6), 0);

		result = Common::U32String((const char *) iso_8859_2, 7, Common::kISO8859_2).encode(Common::kUtf8);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), utf8_2, 11), 0);

		result = Common::U32String((const char *) utf8_2, 11, Common::kUtf8).encode(Common::kISO8859_2);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), iso_8859_2, 8), 0);
	}
};
