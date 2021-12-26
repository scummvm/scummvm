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
// Specialized interface for reading plain text from the underlying source
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_TEXT_READER_H
#define AGS_SHARED_UTIL_TEXT_READER_H

#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class TextReader {
public:
	virtual ~TextReader() {}

	virtual bool IsValid() const = 0;

	// Read single character
	virtual char    ReadChar() = 0;
	// Read defined number of characters
	virtual String  ReadString(size_t length) = 0;
	// Read till line break
	virtual String  ReadLine() = 0;
	// Read till end of available data
	virtual String  ReadAll() = 0;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
