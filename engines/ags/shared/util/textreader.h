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
// Specialized interface for reading plain text from the underlying source
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_TEXTREADER_H
#define AGS_SHARED_UTIL_TEXTREADER_H

#include "util/string.h"

namespace AGS {
namespace Common {

class TextReader {
public:
	virtual ~TextReader() = default;

	virtual bool IsValid() const            = 0;

	// Read single character
	virtual char    ReadChar()              = 0;
	// Read defined number of characters
	virtual String  ReadString(size_t length) = 0;
	// Read till line break
	virtual String  ReadLine()              = 0;
	// Read till end of available data
	virtual String  ReadAll()               = 0;
};

} // namespace Common
} // namespace AGS

#endif
