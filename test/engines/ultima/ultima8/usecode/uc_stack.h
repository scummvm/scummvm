#include <cxxtest/TestSuite.h>
#include "engines/ultima/ultima8/usecode/uc_stack.h"
/**
 * Test suite for the functions in engines/ultima/ultima8/usecode/uc_stack.h
 */

class U8UCStackTestSuite : public CxxTest::TestSuite {
	public:
	U8UCStackTestSuite() {
	}

	void test_static_stack() {
		Ultima::Ultima8::UCStack stack;
		test_for_stack(stack);
	}

	void test_dynamic_stack() {
		Ultima::Ultima8::DynamicUCStack stack;
		Ultima::Ultima8::DynamicUCStack stack2(32);
		TS_ASSERT_EQUALS(stack2.getSize(), 32u);
		test_for_stack(stack);
		test_for_stack(stack2);
	}

	private:
	void test_for_stack(Ultima::Ultima8::BaseUCStack &s) {
		TS_ASSERT_EQUALS(s.stacksize(), 0u);
		s.push4(0xDEADBEEF);
		TS_ASSERT_EQUALS(s.stacksize(), 4u);
		TS_ASSERT_EQUALS(s.pop2(), 0xBEEFu);
		TS_ASSERT_EQUALS(s.pop2(), 0xDEADu);
		s.push1(0xFE);
		TS_ASSERT_EQUALS(s.stacksize(), 1u);
		s.push1(0xED);
		s.push2(0xC0DE);
		TS_ASSERT_EQUALS(s.pop2(), 0xC0DEu);
		TS_ASSERT_EQUALS(s.pop2(), 0xFEEDu);
		TS_ASSERT_EQUALS(s.stacksize(), 0u);
		TS_ASSERT_EQUALS(s.getSP(), s.getSize());
		s.push4(0xCAFEF00D);
		TS_ASSERT_EQUALS(s.getSP(), s.getSize()-4);
	}

};
