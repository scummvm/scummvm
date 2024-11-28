/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMMON_DBCS_STR_H
#define COMMON_DBCS_STR_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/str-enc.h"
#include "common/str-base.h"

namespace Common {

class String;

/**
 * A simple string class for DBCS strings in ScummVM.
 *
 * The presence of \0 characters in the string will cause undefined
 * behavior in some operations.
 */

class DBCSString : public BaseString<uint16> {
public:
	typedef uint32 unsigned_type; /*!< Unsigned version of the underlying type. */
public:
	/** Construct a new empty string. */
	constexpr DBCSString() : BaseString<uint16>() {}

	/** Construct a new string from the given null-terminated C string. */
	explicit DBCSString(const value_type *str) : BaseString<uint16>(str) {}

	/** Construct a new string containing exactly @p len characters read from address @p str. */
	DBCSString(const value_type *str, uint32 len) : BaseString<uint16>(str, len) {}

	/** Construct a new string containing the characters between @p beginP (including) and @p endP (excluding). */
	DBCSString(const value_type *beginP, const value_type *endP) : BaseString<uint16>(beginP, endP) {}

	/** Construct a copy of the given string. */
	DBCSString(const DBCSString &str) : BaseString<uint16>(str) {}

	/** Construct a string by moving an existing string. */
	DBCSString(DBCSString &&str) : BaseString<uint16>(static_cast<BaseString<uint16> &&>(str)) {}

	/** Construct a new string from the given null-terminated C string that uses the given @p page encoding. */
	explicit DBCSString(const char *str);

	/** Construct a new string containing exactly @p len characters read from address @p str. */
	DBCSString(const char *str, uint32 len);

	/** Construct a new string containing the characters between @p beginP (including) and @p endP (excluding). */
	DBCSString(const char *beginP, const char *endP);

	/** Construct a copy of the given string. */
	explicit DBCSString(const String &str);

	/** Construct a string consisting of the given character. */
	explicit constexpr DBCSString(value_type c) : BaseString<uint16>(c) {}

	/** Assign a given string to this string. */
	DBCSString &operator=(const DBCSString &str);

	/** Move a given string to this string. */
	DBCSString &operator=(DBCSString &&str);

	/** @overload */
	DBCSString &operator=(const String &str);

	/** @overload */
	DBCSString &operator=(const value_type *str);

	/** @overload */
	DBCSString &operator=(const char *str);

	/** @overload */
	DBCSString &operator=(value_type c);

	/** Append the given string to this string. */
	DBCSString &operator+=(const DBCSString &str);

	/** @overload */
	DBCSString &operator+=(const value_type *str);

	/** @overload */
	DBCSString &operator+=(value_type c);

	using BaseString<value_type>::operator==;
	using BaseString<value_type>::operator!=;

	/** Check whether this string is identical to string @p x. */
	bool operator==(const String &x) const;

	/** @overload */
	bool operator==(const char *x) const;

	/** Check whether this string is different than string @p x. */
	bool operator!=(const String &x) const;

	/** @overload */
	bool operator!=(const char *x) const;

	/** Convert the string to the standard String represantation. */
	String convertToString() const;

	/** Return a substring of this string */
	DBCSString substr(size_t pos = 0, size_t len = npos) const;

private:
	void decodeDBCS(const char *str, uint32 len);

	friend class String;
};

/** Concatenate strings @p x and @p y. */
DBCSString operator+(const DBCSString &x, const DBCSString &y);

/** Append the given @p y character to the given @p x string. */
DBCSString operator+(const DBCSString &x, DBCSString::value_type y);

/** @} */

} // End of namespace Common

#endif
