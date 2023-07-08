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

#ifndef COMMON_PATH_H
#define COMMON_PATH_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/str-array.h"

namespace Common {

/**
 * @defgroup common_path Path
 * @ingroup common
 *
 * @brief API for working with paths.
 *
 * @{
 */

/**
 * Simple path class. Allows simple conversion to/from path strings with
 * arbitrary directory separators, providing a common representation.
 * 
 * Internally, this is just a simple wrapper around a String, using
 * "//" (unit separator) as a directory separator and "/+" as "/".
 */
class Path {
private:
	String _str;

	String getIdentifierString() const;
	size_t findLastSeparator(size_t last = String::npos) const;

public:
	/**
	 * Hash and comparator for Path with following changes:
	 * * case-insensitive
	 * * decoding of punycode
	 * * Matching ':' and '/' inside path components to
	 * This allows a path "Sound Manager 3.1 / SoundLib<separator>Sound"
	 * to match both "xn--Sound Manager 3.1  SoundLib-lba84k/Sound"
	 * and "Sound Manager 3.1 : SoundLib/Sound"
	 */
	struct IgnoreCaseAndMac_EqualsTo {
		bool operator()(const Path& x, const Path& y) const;
	};

	struct IgnoreCaseAndMac_Hash {
		uint operator()(const Path& x) const;
	};

	/** Construct a new empty path. */
	Path() {}

	/** Construct a copy of the given path. */
	Path(const Path &path);

	/**
	 * Construct a new path from the given NULL-terminated C string.
	 * 
	 * @param str       A NULL-terminated C string representing a path,
	 *                  e.g. "foo/bar/baz"
	 * @param separator The directory separator used in the path string.
	 *                  Defaults to '/'.
	 */
	Path(const char *str, char separator = '/');

	/**
	 * Construct a new path from the given String.
	 * 
	 * @param str       A String representing a path, e.g. "foo/bar/baz"
	 * @param separator The directory separator used in the path string.
	 *                  Defaults to '/'.
	 */
	Path(const String &str, char separator = '/');

	/**
	 * Converts a path to a string using the given directory separator.
	 * 
	 * @param separator The character used to separate directory names.
	 *                  Defaults to '/'.
	 */
	String toString(char separator = '/') const;

	/**
	 * Returns the Path for the parent directory of this path.
	 *
	 * Appending the getLastComponent() of a path to getParent() returns a path
	 * identical to the original path.
	 */
	Path getParent() const;

	/**
	 * Returns the last component of this path.
	 *
	 * Appending the getLastComponent() of a path to getParent() returns a path
	 * identical to the original path.
	 */
	Path getLastComponent() const;

	/** Check whether this path is identical to path @p x. */
	bool operator==(const Path &x) const;

	/** Check whether this path is different than path @p x. */
	bool operator!=(const Path &x) const;

	/** Return if this path is empty */
	bool empty() const;

	/** Assign a given path to this path. */
	Path &operator=(const Path &str);

	/** @overload */
	Path &operator=(const char *str);

	/** @overload */
	Path &operator=(const String &str);

	void set(const char *str, char separator = '/');

	/**
	 * Appends the given path to this path (in-place).
	 * Does not automatically add a directory separator.
	 */
	Path &appendInPlace(const Path &x);

	/** @overload */
	Path &appendInPlace(const String &str, char separator = '/');

	/** @overload */
	Path &appendInPlace(const char *str, char separator = '/');

	/**
	 * Returns this path with the given path appended (out-of-place).
	 * Does not automatically add a directory separator.
	 */
	Path append(const Path &x) const;

	/** @overload */
	Path append(const String &str, char separator = '/') const;

	/** @overload */
	Path append(const char *str, char separator = '/') const;

	/**
	 * Appends exactly one component, without any separators
	 * and prepends a separator if necessarry
	 */
	Path appendComponent(const String &x) const;

	/**
	 * Joins the given path to this path (in-place).
	 * Automatically adds a directory separator.
	 */
	Path &joinInPlace(const Path &x);

	/** @overload */
	Path &joinInPlace(const String &str, char separator = '/');

	/** @overload */
	Path &joinInPlace(const char *str, char separator = '/');

	/**
	 * Returns this path joined with the given path (out-of-place).
	 * Automatically adds a directory separator.
	 */
	Path join(const Path &x) const;

	/** @overload */
	Path join(const String &str, char separator = '/') const;

	/** @overload */
	Path join(const char *str, char separator = '/') const;

	/**
	 * Convert path from Punycode
	 */
	Path punycodeDecode() const;

        /**
	 * Convert path to Punycode
	 */
	Path punycodeEncode() const;

        /**
	 * Check pattern match similar matchString
	 */
	bool matchPattern(const Path& pattern) const;

	/**
	 * Normalize path to a canonical form. In particular:
	 * - trailing separators are removed:  /foo/bar/ -> /foo/bar
	 * - double separators (= empty components) are removed:   /foo//bar -> /foo/bar
	 * - dot components are removed:  /foo/./bar -> /foo/bar
	 * - double dot components are removed:  /foo/baz/../bar -> /foo/bar
	 *
	 * @return      the normalized path
	 */
	Path normalize() const;

	/**
	 * Splits into path components. After every component except
	 * last there is an implied separator. First component is empty
	 * if path starts with a separator. Last component is empty if
	 * the path ends with a separator. Other components may be empty if
	 * 2 separots follow each other
	 */
	StringArray splitComponents() const;


	/**
	 * Opposite of splitComponents
	 */
	static Path joinComponents(const StringArray& c);
};

/** @} */

} // End of namespace Common

#endif
