#include <cxxtest/TestSuite.h>

#include "common/base64.h"
#include "common/memstream.h"

static const char *base64_test_string[] = {
	"",
	"a",
	"abc",
	"abcdefghijklmnopqrstuvwxyz",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
	("12345678901234567890123456789012345678901234567890123456789012"
		"345678901234567890")
};

static const char *base64_test_encoded[] = {
	"",
	"YQ==",
	"YWJj",
	"YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXo=",
	("QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVphYmNkZWZnaGlqa2xtbm9wcXJzdH"
		"V2d3h5ejAxMjM0NTY3ODk="),
	("MTIzNDU2Nzg5MDEyMzQ1Njc4OTAxMjM0NTY3ODkwMTIzNDU2Nzg5MDEyMzQ1Nj"
		"c4OTAxMjM0NTY3ODkwMTIzNDU2Nzg5MDEyMzQ1Njc4OTA=")
};

static const char *base64_validate_tests[] = {
	"YQ", // missing padding
	"!@#$", // characters not in encoding table
	"YQ==YWJj", // data after padding
};

#include <common/pack-start.h>	// START STRUCT PACKING

struct Base64TestStruct {
	uint32 x;
	byte y;
	uint16 z;
	uint32 a;
	byte b;
} PACKED_STRUCT;

#include <common/pack-end.h>	// END STRUCT PACKING

class Base64TestSuite : public CxxTest::TestSuite {
	public:
	void test_b64Validate() {
		for (int i = 0; i < 6; i++) {
			Common::String encoded = base64_test_encoded[i];
			// All of these should return true.
			TS_ASSERT_EQUALS(Common::b64Validate(encoded), true);
		}
		for (int i = 0; i < 3; i++) {
			Common::String encoded = base64_validate_tests[i];
			// All of these should return false.
			TS_ASSERT_EQUALS(Common::b64Validate(encoded), false);
		}

	}

	void test_b64EncodeString() {
		for (int i = 0; i < 6; i++) {
			Common::String string = base64_test_string[i];
			Common::String encoded = Common::b64EncodeString(string);
			TS_ASSERT_EQUALS(encoded, base64_test_encoded[i]);
		}
	}

	void test_b64EncodeStream() {
		for (int i = 0; i < 6; i++) {
			Common::MemoryReadStream stream((const byte *)base64_test_string[i], strlen(base64_test_string[i]));
			Common::String encoded = Common::b64EncodeStream(stream);
			TS_ASSERT_EQUALS(encoded, base64_test_encoded[i]);
		}
	}

	void test_b64EncodeData() {
		Base64TestStruct *test = new Base64TestStruct();
		test->x = 1;
		test->y = 2;
		test->z = 3;
		test->a = 4;
		test->b = 5;

		Common::String encoded = Common::b64EncodeData(test, sizeof(Base64TestStruct));
		TS_ASSERT_EQUALS(encoded, "AQAAAAIDAAQAAAAF");
		delete test;
	}

	void test_b64DecodeString() {
		for (int i = 0; i < 6; i++) {
			Common::String encoded = base64_test_encoded[i];
			Common::String string = Common::b64DecodeString(encoded);
			TS_ASSERT_EQUALS(string, base64_test_string[i]);
		}
	}

	void test_b64DecodeStream() {
		for (int i = 0; i < 6; i++) {
			Common::String encoded = base64_test_encoded[i];
			Common::MemoryReadStream *stream = Common::b64DecodeStream(encoded, strlen(base64_test_string[i]));
			TS_ASSERT_EQUALS(stream->size(), strlen(base64_test_string[i]));

			char *data = (char *)malloc(stream->size());
			stream->read(data, stream->size());
			delete stream;

			Common::String string(data, strlen(base64_test_string[i]));
			TS_ASSERT_EQUALS(string, base64_test_string[i]);
			free(data);
		}
	}

	void test_b64DecodeData() {
		Base64TestStruct *test = new Base64TestStruct();

		Common::String encoded = "AQAAAAIDAAQAAAAF";
		bool success = Common::b64DecodeData(encoded, test);
		TS_ASSERT_EQUALS(success, true);

		TS_ASSERT_EQUALS(test->x, 1);
		TS_ASSERT_EQUALS(test->y, 2);
		TS_ASSERT_EQUALS(test->z, 3);
		TS_ASSERT_EQUALS(test->a, 4);
		TS_ASSERT_EQUALS(test->b, 5);
		delete test;
	}
};
