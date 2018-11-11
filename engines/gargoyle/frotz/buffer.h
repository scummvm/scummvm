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

#ifndef GARGOYLE_FROTZ_BUFFER
#define GARGOYLE_FROTZ_BUFFER

#include "gargoyle/frotz/frotz_types.h"

namespace Gargoyle {
namespace Frotz {

#define TEXT_BUFFER_SIZE 200

/**
 * Text buffer class
 */
class Buffer {
public:
	zchar _buffer[TEXT_BUFFER_SIZE];
	size_t _bufPos;
	bool _locked;
	zchar _prevC;
public:
	/**
	 * Constructor
	 */
	Buffer();

	/**
	 * Copy the contents of the text buffer to the output streams.
	 */
	void flush();

	 /**
	  * High level output function.
	  */
	void printChar(zchar c);

	 /**
	  * High level newline function.
	  */
	void newLine();

	/**
	 * Returns true if the buffer is empty
	 */
	bool empty() const { return !_bufPos; }
};

} // End of namespace Frotz
} // End of namespace Gargoyle

#endif
