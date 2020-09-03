/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// This file does nothing functional!
// It test support for main C++11 features
// In the future, it might be extended to test also C++14, C++17, C++20 and any future standard
//
// In order to enable the tests, we have to `define ENABLE_TEST_CPP_11` (and of course, compile this file)
// Then it should print "Testing C++11" *during compilation*
// If the message is printed, and there are no compilation errors - great, C++11 is supported on this platform
// If there are errors, each one of the tests can be disabled, by defining the relevant DONT_TEST_*
// It's important to disable failing tests, because we might decide to support only specific subset of C++11
//
// Note: there are 3 warnings in my GCC run, they have no signficance

#if defined(NONSTANDARD_PORT)
#include "portdefs.h"
#endif

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#ifdef ENABLE_TEST_CPP_11
#pragma message("Testing C++11")
// The tests are based on https://blog.petrzemek.net/2014/12/07/improving-cpp98-code-with-cpp11/
// See there for further links and explanations
//
// We're not testing `nullptr` and `override`, since they're defined in common/c++11-compat.h

#include "common/array.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/rect.h"

#ifndef DONT_TEST_UNICODE_STRING_LITERAL
const char16_t *u16str = u"\u00DAnicode string";
const char32_t *u32str = U"\u00DAnicode string";
#endif

#ifndef DONT_TEST_INITIALIZIER_LIST1
#ifndef USE_INITIALIZIER_LIST_REPLACEMENT
#include <initializer_list>
#else
namespace std {
template<class T> class initializer_list {
public:
	typedef T value_type;
	typedef const T& reference;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef const T* iterator;
	typedef const T* const_iterator;

	constexpr initializer_list() noexcept = default;
	constexpr size_t size() const noexcept { return m_size; };
	constexpr const T* begin() const noexcept { return m_begin; };
	constexpr const T* end() const noexcept { return m_begin + m_size; }

private:
	// Note: begin has to be first or the compiler gets very upset
	const T* m_begin = { nullptr };
	size_t m_size = { 0 };

	// The compiler is allowed to call this constructor
	constexpr initializer_list(const T* t, size_t s) noexcept : m_begin(t) , m_size(s) {}
};

template<class T> constexpr const T* begin(initializer_list<T> il) noexcept {
	return il.begin();
}

template<class T> constexpr const T* end(initializer_list<T> il) noexcept {
	return il.end();
}
} // end namespace std
#endif
#endif

#ifndef DONT_TEST_CLASS_ENUM
// ----------------------------------
// Scoped/Strongly Typed Enumerations
// ----------------------------------
enum class MyEnum {
	VAL1,
	VAL2,
	VAL3
};
#endif

#ifndef DONT_TEST_FINAL_CLASS
// ----------------------------------
// Non-Inheritable Classes (final)
// ----------------------------------
// C++11
class TestNewStandards final {
#else
class TestNewStandards {
#endif
private:
	void do_nothing(const int &i) {
		// don't do anything with i
	};

#ifndef DONT_TEST_FINAL_FUNCTION
	// ----------------------------------
	// Non-Overridable Member Functions (final)
	// ----------------------------------
	virtual void f() final {}
#endif

#ifndef DONT_TEST_VARIADIC_TEMPLATES
	// ------------------------
	// Variadic Templates
	// ------------------------
	template <typename T>
	void variadic_function(const T &value) {
		do_nothing(value);
	}

	template <typename U, typename... T>
	void variadic_function(const U &head, const T &... tail) {
		do_nothing(head);
		variadic_function(tail...);
	}
#endif

#ifndef DONT_TEST_TYPE_ALIASES
	// ------------------------
	// Type Aliases
	// * note - this test has another bunch of code below
	// ------------------------
	// C++98
	template<typename T>
	struct Dictionary_98 {
		typedef Common::HashMap<Common::String, T, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> type;
	};
	// Usage:
	Dictionary_98<int>::type d98;

	// C++11
	template <typename T>
	using Dictionary_11 = Common::HashMap<Common::String, T, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>;
	// Usage:
	Dictionary_11<int> d11;
#endif

#ifndef DONT_TEST_INITIALIZIER_LIST1
	// Array with C++11 initialization list
	template<class T> class ArrayCpp11 : public Common::Array<T> {
	public:
		ArrayCpp11(std::initializer_list<T> list) {
			if (list.size()) {
				this->allocCapacity(list.size());
				Common::uninitialized_copy(list.begin(), list.end(), this->_storage);
			}
		}
	};
#endif

	void test_cpp11() {
#ifdef DONT_TEST_INITIALIZIER_LIST1
		// ------------------------
		// Initializer list
		// ------------------------
		// C++98
		Common::Array<int> arr;
		arr.push_back(1);
		arr.push_back(2);
		arr.push_back(3);
#else
		// C++11
		ArrayCpp11<int> arr = {1, 2, 3};
#endif

#ifndef DONT_TEST_INITIALIZIER_LIST2
		// C++11
		Common::Point arr3[] = {{0, 0}, {1, 1}};
#endif

#ifndef DONT_TEST_AUTO_TYPE_INFERENCE
		// ------------------------
		// Automatic Type Inference
		// ------------------------
		// C++98
		for (Common::Array<int>::iterator i = arr.begin(), e = arr.end(); i != e; ++i)
			;

		// C++11
		for (auto i = arr.begin(), e = arr.end(); i != e; ++i)
			;
#endif

#ifndef DONT_TEST_RANGE_BASED_FOR_LOOP
		// ------------------------
		// Range based for loop
		// ------------------------
		// C++98
		for (Common::Array<int>::iterator i = arr.begin(), e = arr.end(); i != e; ++i)
			do_nothing(*i);

		// C++11
		for (int &i : arr)
			do_nothing(i);
#endif

#ifndef DONT_TEST_LAMBDA_FUNCTIONS
		// ------------------------
		// Lambda functions
		// ------------------------
		// C++98
		// the following isn't working in VS, but it's not really important to debug...
		// Common::for_each(arr.begin(), arr.end(), do_nothing);

		// C++11
		Common::for_each(arr.begin(), arr.end(),
			[](int i) {
				// don't do anything with i
			}
		);
#endif

#ifndef DONT_TEST_VARIADIC_TEMPLATES
		variadic_function(1, 1, 2, 3, 5, 8, 13, 21, 34);
#endif

#ifndef DONT_TEST_GET_RID_OF_SPACE_IN_NESTED_TEMPLATES
		// ------------------------
		// No Need For an Extra Space In Nested Template Declarations
		// ------------------------
		// C++98
		Common::Array<Common::Array<int> > v_98;

		// C++11
		Common::Array<Common::Array<int>> v_11;
#endif

#ifndef DONT_TEST_TYPE_ALIASES
		// ------------------------
		// Type Aliases
		// * note - this test has another bunch of code above
		// ------------------------
		// C++98
		typedef void (*fp_98)(int, int);

		// C++11
		using fp_11 = void (*)(int, int);
#endif

	};

#ifndef DONT_TEST_ALT_FUNCTION_SYNTAX
	// ------------------------
	// Alternative Function Syntax
	// ------------------------
	// C++98
	int f_98(int x, int y) {return x;}

	// C++11
	auto f_11(int x, int y) -> int {return x;}
#endif

#ifndef DONT_TEST_NON_STATIC_INIT
	// ------------------------
	// Non-Static Data Member Initializers
	// ------------------------
	int j = 3;
	Common::String s = "non static init";
#endif

#ifndef DONT_TEST_EXPLICIT
	// ------------------------
	// Explicit Conversion Operators
	// ------------------------
	explicit operator bool() const {return true;}
#endif


public:
	TestNewStandards() {
		test_cpp11();
	}

#ifndef DONT_TEST_MOVE_SEMANTICS
	// ------------------------
	// Move semantics
	// Note: this test hasn't been taken from the aforementioned web page
	// ------------------------
	TestNewStandards(TestNewStandards&& t) {
		// I'm not convinced that it's a good example of move sematics, it's a complicated topic. But just checking the syntax.
	}
#endif


#ifndef DONT_TEST_DELETED_FUNCTIONS
	// ------------------------
	// Explicitly Deleted Functions
	// (useful for non copyable classes,
	//  particularly for our Singleton class)
	// ------------------------
	TestNewStandards &operator=(const TestNewStandards &) = delete;
#endif

};

static TestNewStandards test = TestNewStandards();

#endif
