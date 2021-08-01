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

const char DIR_SEPARATOR = '\x1f'; // unit separator

class Path {
private:
	String _str;

public:
	Path() {}
	Path(const Path &path);
	Path(const char *str, char separator = '/');
	Path(const String &str, char separator = '/');
	Path(char c, char separator = '/');

private:
	void setString(const char *str, char separator = '/');

public:
	const String &rawString() const { return _str; }
	String toString(char separator = '/') const;

	bool operator==(const Path &x) const;
	bool operator!=(const Path &x) const;

	bool empty() const;

	Path &operator=(const Path &str);
	Path &operator=(const char *str);
	Path &operator=(const String &str);
	Path &operator=(char c);
	Path &operator+=(const Path &str);
	Path &operator+=(const char *str);
	Path &operator+=(const String &str);
	Path &operator+=(char c);
};

Path operator+(const Path &x, const Path &y);
Path operator+(const Path &x, const String &y);
Path operator+(const Path &x, const char *y);
Path operator+(const Path &x, char y);

} // End of namespace Common

#endif
