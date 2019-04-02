#include <cxxtest/TestSuite.h>
#include "common/util.h"

/**
 * Test suite for the functions in common/util.h
 */
class UtilTestSuite : public CxxTest::TestSuite {
	public:
	void test_parsebool_yesno() {

		// First test for the parseBool function.
		// These are the mixed case yes/no cases that must work

		bool valasbool;
		bool success;

		Common::String string_1("Yes");
		success = Common::parseBool(string_1, valasbool);
		TS_ASSERT_EQUALS(success, 1);
		TS_ASSERT_EQUALS(valasbool, 1);

		Common::String string_2("nO");
		success = Common::parseBool(string_2, valasbool);
		TS_ASSERT_EQUALS(success, 1);
		TS_ASSERT_EQUALS(valasbool, 0);
	}

	void test_parsebool_truefalse() {

		// First test for the parseBool function.
		// These are the mixed case true/false cases that must work

		bool valasbool;
		bool success;

		Common::String string_3("tRuE");
		success = Common::parseBool(string_3, valasbool);
		TS_ASSERT_EQUALS(success, 1);
		TS_ASSERT_EQUALS(valasbool, 1);

		Common::String string_4("fAlSe");
		success = Common::parseBool(string_4, valasbool);
		TS_ASSERT_EQUALS(success, 1);
		TS_ASSERT_EQUALS(valasbool, 0);
	}

	void test_parsebool_onezero() {

		// Third test for the parseBool function.
		// These are the 1/0 cases that must work.
		// Note that while 'a-z'+0x20 must work just fine,
		// '0-1'+0x20 should NOT; this is addressed in
		// parsebool_bad

		bool valasbool;
		bool success;

		Common::String string_5("1");
		success = Common::parseBool(string_5, valasbool);
		TS_ASSERT_EQUALS(success, 1);
		TS_ASSERT_EQUALS(valasbool, 1);

		Common::String string_6("0");
		success = Common::parseBool(string_6, valasbool);
		TS_ASSERT_EQUALS(success, 1);
		TS_ASSERT_EQUALS(valasbool, 0);

	}

	void test_parsebool_bad() {

		bool valasbool;
		bool success;

		// Bad cases that should not return success #1:
		// Random string
		Common::String string_1("u_f1ght_l1k3_a_c0w");
		success = Common::parseBool(string_1, valasbool);
		TS_ASSERT_EQUALS(success, 0);

		// Bad cases that should not return success #2, #3:
		// The function should NOT accept trailing whitespaces:
		Common::String string_2(" yes");
		success = Common::parseBool(string_2, valasbool);
		TS_ASSERT_EQUALS(success, 0);

		Common::String string_3("yes ");
		success = Common::parseBool(string_3, valasbool);
		TS_ASSERT_EQUALS(success, 0);

		// While 'a-z'+0x20 must work just fine,
		// '0-1'+0x20 should NOT. '2' is not good either.

		Common::String string_4("\x50");
		success = Common::parseBool(string_4, valasbool);
		TS_ASSERT_EQUALS(success, 0);

		Common::String string_5("\x51");
		success = Common::parseBool(string_5, valasbool);
		TS_ASSERT_EQUALS(success, 0);

		Common::String string_6("2");
		success = Common::parseBool(string_6, valasbool);
		TS_ASSERT_EQUALS(success, 0);
	}

	void test_is_al_num() {

		// Test the isAlnum function
		// Should return true if and only if the input is an alphanumeric char

		TS_ASSERT_EQUALS(Common::isAlnum('a'), 1);
		TS_ASSERT_EQUALS(Common::isAlnum('A'), 1);
		TS_ASSERT_EQUALS(Common::isAlnum('z'), 1);
		TS_ASSERT_EQUALS(Common::isAlnum('Z'), 1);
		TS_ASSERT_EQUALS(Common::isAlnum('1'), 1);
		TS_ASSERT_EQUALS(Common::isAlnum('0'), 1);
		TS_ASSERT_EQUALS(Common::isAlnum('\xA7'), 0);
		TS_ASSERT_EQUALS(Common::isAlnum('$'), 0);
		TS_ASSERT_EQUALS(Common::isAlnum(' '), 0);
		TS_ASSERT_EQUALS(Common::isAlnum('\n'), 0);
		TS_ASSERT_EQUALS(Common::isAlnum('\b'), 0);
		TS_ASSERT_EQUALS(Common::isAlnum(0), 0);
		TS_ASSERT_EQUALS(Common::isAlnum(255), 0);

	}

	void test_is_alpha() {

		// Test the isAlpha function
		// Should return true if and only if the input is an alphanumeric char

		TS_ASSERT_EQUALS(Common::isAlpha('a'), 1);
		TS_ASSERT_EQUALS(Common::isAlpha('A'), 1);
		TS_ASSERT_EQUALS(Common::isAlpha('z'), 1);
		TS_ASSERT_EQUALS(Common::isAlpha('Z'), 1);
		TS_ASSERT_EQUALS(Common::isAlpha('1'), 0);
		TS_ASSERT_EQUALS(Common::isAlpha('0'), 0);
		TS_ASSERT_EQUALS(Common::isAlpha('\xA7'), 0);
		TS_ASSERT_EQUALS(Common::isAlpha('$'), 0);
		TS_ASSERT_EQUALS(Common::isAlpha(' '), 0);
		TS_ASSERT_EQUALS(Common::isAlpha('\n'), 0);
		TS_ASSERT_EQUALS(Common::isAlpha('\b'), 0);
		TS_ASSERT_EQUALS(Common::isAlpha(0), 0);
		TS_ASSERT_EQUALS(Common::isAlpha(255), 0);

	}

	void test_is_digit() {

		// Test the isDigit function
		// Should return true if and only if the input is a single digit

		TS_ASSERT_EQUALS(Common::isDigit('a'), 0);
		TS_ASSERT_EQUALS(Common::isDigit('A'), 0);
		TS_ASSERT_EQUALS(Common::isDigit('z'), 0);
		TS_ASSERT_EQUALS(Common::isDigit('Z'), 0);
		TS_ASSERT_EQUALS(Common::isDigit('1'), 1);
		TS_ASSERT_EQUALS(Common::isDigit('0'), 1);
		TS_ASSERT_EQUALS(Common::isDigit('\xA7'), 0);
		TS_ASSERT_EQUALS(Common::isDigit('$'), 0);
		TS_ASSERT_EQUALS(Common::isDigit(' '), 0);
		TS_ASSERT_EQUALS(Common::isDigit('\n'), 0);
		TS_ASSERT_EQUALS(Common::isDigit('\b'), 0);
		TS_ASSERT_EQUALS(Common::isDigit(0), 0);
		TS_ASSERT_EQUALS(Common::isDigit(255), 0);

	}

	void test_is_lower() {

		// Test the isLower function
		// Should return true if and only if the input is a lowercase char

		TS_ASSERT_EQUALS(Common::isLower('a'), 1);
		TS_ASSERT_EQUALS(Common::isLower('A'), 0);
		TS_ASSERT_EQUALS(Common::isLower('z'), 1);
		TS_ASSERT_EQUALS(Common::isLower('Z'), 0);
		TS_ASSERT_EQUALS(Common::isLower('1'), 0);
		TS_ASSERT_EQUALS(Common::isLower('0'), 0);
		TS_ASSERT_EQUALS(Common::isLower('\xA7'), 0);
		TS_ASSERT_EQUALS(Common::isLower('$'), 0);
		TS_ASSERT_EQUALS(Common::isLower(' '), 0);
		TS_ASSERT_EQUALS(Common::isLower('\n'), 0);
		TS_ASSERT_EQUALS(Common::isLower('\b'), 0);
		TS_ASSERT_EQUALS(Common::isLower(0), 0);
		TS_ASSERT_EQUALS(Common::isLower(255), 0);

	}


	void test_is_upper() {

		// Test the isUpper function
		// Should return true if and only if the input is an uppercase char

		TS_ASSERT_EQUALS(Common::isUpper('a'), 0);
		TS_ASSERT_EQUALS(Common::isUpper('A'), 1);
		TS_ASSERT_EQUALS(Common::isUpper('z'), 0);
		TS_ASSERT_EQUALS(Common::isUpper('Z'), 1);
		TS_ASSERT_EQUALS(Common::isUpper('1'), 0);
		TS_ASSERT_EQUALS(Common::isUpper('0'), 0);
		TS_ASSERT_EQUALS(Common::isUpper('\xA7'), 0);
		TS_ASSERT_EQUALS(Common::isUpper('$'), 0);
		TS_ASSERT_EQUALS(Common::isUpper(' '), 0);
		TS_ASSERT_EQUALS(Common::isUpper('\n'), 0);
		TS_ASSERT_EQUALS(Common::isUpper('\b'), 0);
		TS_ASSERT_EQUALS(Common::isUpper(0), 0);
		TS_ASSERT_EQUALS(Common::isUpper(255), 0);

	}
	void test_is_space() {
		// isSpace should return true iff the character is some kind of whitespace
		// or tab character
		for (int c = 0; c < 255; c++) {
			 if (c == ' '  || c == '\t' ||
				 c == '\r' || c == '\n' ||
				 c == '\v' || c == '\f') {
				 TS_ASSERT_EQUALS(Common::isSpace(c), 1);
			 } else {
				 TS_ASSERT_EQUALS(Common::isSpace(c), 0);
			 }
		}
	}

	void test_is_print() {
		// isPrint should return true iff the input is a printable ascii char.
		// That is to say, 0x20 to 0x7E.
		for (int c = 0; c < 255; c++) {
			 if (c >= 0x20 && c <= 0x7E) {
				 TS_ASSERT_EQUALS(Common::isPrint(c), 1);
			 } else {
				 TS_ASSERT_EQUALS(Common::isPrint(c), 0);
			 }
		}
	}
	void test_is_punct() {
		// isPunct should return true if the input is a punctation ascii char.
		for (int c = 0; c < 255; c++) {
			 if (c >= 33 && c <= 47) {
				 TS_ASSERT_EQUALS(Common::isPunct(c), 1);
			 } else if (c >= 58 && c <= 64) {
				 TS_ASSERT_EQUALS(Common::isPunct(c), 1);
			 } else if (c >= 91 && c <= 96) {
				 TS_ASSERT_EQUALS(Common::isPunct(c), 1);
			 } else if (c >= 123 && c <= 126) {
				 TS_ASSERT_EQUALS(Common::isPunct(c), 1);
			 } else {
				 TS_ASSERT_EQUALS(Common::isPunct(c), 0);
			 }
		}
	}
};
