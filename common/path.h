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

#ifndef COMMON_PATH_H
#define COMMON_PATH_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Common {

/**
 * @defgroup common_path Path
 * @ingroup common
 *
 * @brief API for working with paths.
 *
 * @{
 */

const char DIR_SEPARATOR = '\x1f'; // unit separator

/**
 * Simple path class. Allows simple conversion to/from path strings with
 * arbitrary directory separators, providing a common representation.
 * 
 * Internally, this is just a simple wrapper around a String, using
 * '\x1f' (unit separator) as a directory separator. As this is not
 * a printable character, it should not appear in file names, unlike
 * '/', '\', or ':', which are allowed on certain platforms.
 */
class Path {
private:
	String _str;

public:
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

	/** Construct a path consisting of the given character. */
	Path(char c, char separator = '/');

private:
	void setString(const char *str, char separator = '/');

public:
	/**
	 * Returns the unmodified, internal representation of the path,
	 * using '\x1f' as a directory separator.
	 */
	const String &rawString() const { return _str; }

	/**
	 * Converts a path to a string using the given directory separator.
	 * 
	 * @param separator The character used to separate directory names.
	 *                  Defaults to '/'.
	 */
	String toString(char separator = '/') const;

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

	/** @overload */
	Path &operator=(char c);

	/**
	 * Append the given path to this path. Does not automatically
	 * add a directory separator.
	 */
	Path &operator+=(const Path &str);

	/** @overload */
	Path &operator+=(const char *str);

	/** @overload */
	Path &operator+=(const String &str);

	/** @overload */
	Path &operator+=(char c);
};

/**
 * Concatenate paths @p x and @p y. Does not automatically
 * add a directory separator between them.
 */
Path operator+(const Path &x, const Path &y);

/** @overload */
Path operator+(const Path &x, const String &y);

/** @overload */
Path operator+(const Path &x, const char *y);

/** @overload */
Path operator+(const Path &x, char y);

/** @} */

} // End of namespace Common

#endif
