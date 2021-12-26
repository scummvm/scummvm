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

//=============================================================================
//
// Specialized interface for writing plain text to the underlying source
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_TEXT_WRITER_H
#define AGS_SHARED_UTIL_TEXT_WRITER_H

#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class TextWriter {
public:
	virtual ~TextWriter() {}

	virtual bool    IsValid() const = 0;

	// Write single character
	virtual void    WriteChar(char c) = 0;
	// Write string as a plain text (without null-terminator)
	virtual void    WriteString(const String &str) = 0;
	// Write string and add line break at the end
	virtual void    WriteLine(const String &str) = 0;
	// Write formatted string (see *printf)
	virtual void    WriteFormat(const char *fmt, ...) = 0;
	virtual void    WriteLineBreak() = 0;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
