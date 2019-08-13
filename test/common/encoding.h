#include <cxxtest/TestSuite.h>

#include "common/encoding.h"
#include "backends/platform/sdl/posix/posix.h"

#ifdef USE_ICONV
class EncodingTestSuite : public CxxTest::TestSuite {
	public:
		void test_conversion_unicode_machine_endian() {
				   		       //  |dolar|   cent    |     euro       |
			unsigned char utf8[] = {0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0};
							        //| dolar |  cent  |    euro   |
			unsigned char utf16be[] = {0, 0x24, 0, 0xA2, 0x20, 0xAC, 0, 0};
								    //| dolar |  cent  |    euro   |
			unsigned char utf16le[] = {0x24, 0, 0xA2, 0, 0xAC, 0x20, 0, 0};
							        //| dolar       |  cent        |    euro  
			unsigned char utf32be[] = {0, 0, 0, 0x24, 0, 0, 0, 0xA2, 0, 0, 0x20, 0xAC, 0, 0, 0, 0};
						            //| dolar       |  cent        |    euro
			unsigned char utf32le[] = {0x24, 0, 0, 0, 0xA2, 0, 0, 0, 0xAC, 0x20, 0, 0, 0, 0, 0, 0};
#ifdef SCUMM_BIG_ENDIAN
			unsigned char *utf16 = utf16be;
			unsigned char *utf32 = utf32be;
#else
			unsigned char *utf16 = utf16le;
			unsigned char *utf32 = utf32le;
#endif

			// UTF16 to UTF8
			Common::Encoding converter("UTF-8", "UTF-16");

			char *result = converter.convert((char *) utf16, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8, 7);
			free(result);

			result = Common::Encoding::convert("UTF-8", "UTF-16", (char *) utf16, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8, 7);
			free(result);

			// UTF32 to UTF8
			converter.setFrom("UTF-32");

			result = converter.convert((char *) utf32, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8, 7);
			free(result);

			result = Common::Encoding::convert("UTF-8", "UTF-32", (char *) utf32, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8, 7);
			free(result);

			// UTF32 to UTF16
			converter.setTo("UTF-16");

			result = converter.convert((char *) utf32, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf16, 8);
			free(result);

			result = Common::Encoding::convert("UTF-16", "UTF-32", (char *) utf32, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf16, 8);
			free(result);

			// UTF8 to UTF16
			converter.setFrom("UTF-8");

			result = converter.convert((char *) utf8, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf16, 8);
			free(result);

			result = Common::Encoding::convert("UTF-16", "UTF-8", (char *) utf8, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf16, 8);
			free(result);

			// UTF8 to UTF32
			converter.setTo("UTF-32");

			result = converter.convert((char *) utf8, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf32, 16);
			free(result);

			result = Common::Encoding::convert("UTF-32", "UTF-8", (char *) utf8, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf32, 16);
			free(result);

			// UTF16 to UTF32
			converter.setFrom("UTF-16");

			result = converter.convert((char *) utf16, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf32, 16);
			free(result);

			result = Common::Encoding::convert("UTF-32", "UTF-16", (char *) utf16, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf32, 16);
			free(result);
		}

		void test_conversion_unicode_big_endian() {
				   		       //  |dolar|   cent    |     euro       |
			unsigned char utf8[] = {0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0};
							        //| dolar |  cent  |    euro   |
			unsigned char utf16be[] = {0, 0x24, 0, 0xA2, 0x20, 0xAC, 0, 0};
							        //| dolar       |  cent        |    euro  
			unsigned char utf32be[] = {0, 0, 0, 0x24, 0, 0, 0, 0xA2, 0, 0, 0x20, 0xAC, 0, 0, 0, 0};

			// UTF16 to UTF8
			Common::Encoding converter("UTF-8", "UTF-16BE");

			char *result = converter.convert((char *) utf16be, 6);

			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8, 7);
			free(result);

			result = Common::Encoding::convert("UTF-8", "UTF-16BE", (char *) utf16be, 6);

			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8, 7);
			free(result);

			// UTF32 to UTF8
			converter.setFrom("UTF-32BE");

			result = converter.convert((char *) utf32be, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8, 7);
			free(result);

			result = Common::Encoding::convert("UTF-8", "UTF-32BE", (char *) utf32be, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8, 7);
			free(result);

			// UTF32 to UTF16
			converter.setTo("UTF-16BE");

			result = converter.convert((char *) utf32be, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf16be, 8);
			free(result);

			result = Common::Encoding::convert("UTF-16BE", "UTF-32BE", (char *) utf32be, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf16be, 8);
			free(result);

			// UTF8 to UTF16
			converter.setFrom("UTF-8");

			result = converter.convert((char *) utf8, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf16be, 8);
			free(result);

			result = Common::Encoding::convert("UTF-16BE", "UTF-8", (char *) utf8, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf16be, 8);
			free(result);

			// UTF8 to UTF32
			converter.setTo("UTF-32BE");

			result = converter.convert((char *) utf8, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf32be, 16);
			free(result);

			result = Common::Encoding::convert("UTF-32BE", "UTF-8", (char *) utf8, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf32be, 16);
			free(result);

			// UTF16 to UTF32
			converter.setFrom("UTF-16BE");

			result = converter.convert((char *) utf16be, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf32be, 16);
			free(result);

			result = Common::Encoding::convert("UTF-32BE", "UTF-16BE", (char *) utf16be, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf32be, 16);
			free(result);
		}

		void test_conversion_unicode_little_endian() {
				   		       //  |dolar|   cent    |     euro       |
			unsigned char utf8[] = {0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0};
								    //| dolar |  cent  |    euro   |
			unsigned char utf16le[] = {0x24, 0, 0xA2, 0, 0xAC, 0x20, 0, 0};
						            //| dolar       |  cent        |    euro
			unsigned char utf32le[] = {0x24, 0, 0, 0, 0xA2, 0, 0, 0, 0xAC, 0x20, 0, 0, 0, 0, 0, 0};

			// UTF16 to UTF8
			Common::Encoding converter("UTF-8", "UTF-16LE");

			char *result = converter.convert((char *) utf16le, 6);

			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8, 7);
			free(result);

			result = Common::Encoding::convert("UTF-8", "UTF-16LE", (char *) utf16le, 6);

			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8, 7);
			free(result);

			// UTF32 to UTF8
			converter.setFrom("UTF-32LE");

			result = converter.convert((char *) utf32le, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8, 7);
			free(result);

			result = Common::Encoding::convert("UTF-8", "UTF-32LE", (char *) utf32le, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8, 7);
			free(result);

			// UTF32 to UTF16
			converter.setTo("UTF-16LE");

			result = converter.convert((char *) utf32le, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf16le, 8);
			free(result);

			result = Common::Encoding::convert("UTF-16LE", "UTF-32LE", (char *) utf32le, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf16le, 8);
			free(result);

			// UTF8 to UTF16
			converter.setFrom("UTF-8");

			result = converter.convert((char *) utf8, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf16le, 8);
			free(result);

			result = Common::Encoding::convert("UTF-16LE", "UTF-8", (char *) utf8, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf16le, 8);
			free(result);

			// UTF8 to UTF32
			converter.setTo("UTF-32LE");

			result = converter.convert((char *) utf8, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf32le, 16);
			free(result);

			result = Common::Encoding::convert("UTF-32LE", "UTF-8", (char *) utf8, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf32le, 16);
			free(result);

			// UTF16 to UTF32
			converter.setFrom("UTF-16LE");

			result = converter.convert((char *) utf16le, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf32le, 16);
			free(result);

			result = Common::Encoding::convert("UTF-32LE", "UTF-16LE", (char *) utf16le, 6);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf32le, 16);
			free(result);
		}

		void test_cyrilic_transliteration() {
			unsigned char utf8[] = {/* Z */0xD0, 0x97, /* d */ 0xD0, 0xB4, /* r */ 0xD1, 0x80, /* a */ 0xD0, 0xB0, /* v */ 0xD0, 0xB2, /* s */ 0xD1, 0x81, /* t */ 0xD1, 0x82, /* v */ 0xD0, 0xB2, /* u */ 0xD1, 0x83, /* j */ 0xD0, 0xB9, /* t */ 0xD1, 0x82, /* e */ 0xD0, 0xB5, 0};
			unsigned char iso_8859_5[] = {0xB7, 0xD4, 0xE0, 0xD0, 0xD2, 0xE1, 0xE2, 0xD2, 0xE3, 0xD9, 0xE2, 0xD5, 0};
			unsigned char ascii[] = "Zdravstvujte";

			Common::Encoding converter("ASCII", "UTF-8");
			char *result = converter.convert((char *)utf8, 24);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, ascii, 13);
			free(result);

			converter.setFrom("iso-8859-5");
			result = converter.convert((char *)iso_8859_5, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, ascii, 13);
			free(result);

			converter.setTo("UTF-8");
			result = converter.convert((char *)iso_8859_5, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8, 25);
			free(result);

			converter.setTo("iso-8859-5");
			converter.setFrom("UTF-8");
			result = converter.convert((char *)utf8, 24);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, iso_8859_5, 13);
			free(result);

			// this should stay the same
			converter.setFrom("ASCII");
			result = converter.convert((char *)ascii, 12);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, ascii, 13);
			free(result);
		}

		void test_other_conversions() {
			unsigned char cp850[] = {0x99, 0xE0, 0xEA, 0x41, 0x64, 0};
			unsigned char utf8_1[] = {0xC3, 0x96, 0xC3, 0x93, 0xC3, 0x9B, 0x41, 0x64, 0};

			unsigned char iso_8859_2[] = {0xA9, 0xE1, 0x6C, 0x65, 0xE8, 0x65, 0x6B, 0};
			unsigned char utf8_2[] = {0xC5, 0xA0, 0xC3, 0xA1, 0x6C, 0x65, 0xC4, 0x8D, 0x65, 0x6B, 0};

			char *result = Common::Encoding::convert("UTF-8", "CP850", (char *)cp850, 5);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8_1, 9);
			free(result);

			result = Common::Encoding::convert("CP850", "UTF-8", (char *)utf8_1, 8);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, cp850, 6);
			free(result);

			result = Common::Encoding::convert("UTF-8", "iso-8859-2", (char *)iso_8859_2, 7);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, utf8_2, 11);
			free(result);

			result = Common::Encoding::convert("iso-8859-2", "UTF-8", (char *)utf8_2, 11);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, iso_8859_2, 8);
			free(result);

			result = Common::Encoding::convert("ASCII", "UTF-8", (char *)utf8_2, 11);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, "Salecek", 8);
			free(result);

			result = Common::Encoding::convert("ASCII", "iso-8859-2", (char *)iso_8859_2, 7);
			TS_ASSERT(result != NULL);
			TS_ASSERT_SAME_DATA(result, "Salecek", 8);
			free(result);
		}
};
#endif
