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

//=============================================================================
//
// Specialized interface for writing plain text to the underlying source
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_TEXTWRITER_H
#define AGS_SHARED_UTIL_TEXTWRITER_H

#include "util/string.h"

namespace AGS {
namespace Common {

class TextWriter {
public:
	virtual ~TextWriter() = default;

	virtual bool    IsValid() const                         = 0;

	// Write single character
	virtual void    WriteChar(char c)                       = 0;
	// Write string as a plain text (without null-terminator)
	virtual void    WriteString(const String &str)          = 0;
	// Write string and add line break at the end
	virtual void    WriteLine(const String &str)            = 0;
	// Write formatted string (see *printf)
	virtual void    WriteFormat(const char *fmt, ...)       = 0;
	virtual void    WriteLineBreak()                        = 0;
};

} // namespace Common
} // namespace AGS

#endif
