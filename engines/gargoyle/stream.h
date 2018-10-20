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

#ifndef GARGOYLE_STREAM_H
#define GARGOYLE_STREAM_H

#include "common/stream.h"

namespace Gargoyle {

class Window;

/**
 * Implements the stream for writing text to a window
 */
class WindowStream : public Common::WriteStream {
private:
	uint32 _rock;
	Window *_window;
public:
	/**
	 * Constructor
	 */
	WindowStream(Window *window, uint32 rock = 0) : Common::WriteStream(),
		_window(window), _rock(rock) {}

	/**
	 * Write to the stream
	 */
	virtual uint32 write(const void *dataPtr, uint32 dataSize);
	
	/**
	 * Flush the stream
	 */
	virtual bool flush();

	/**
	 * Finalize and close this stream
	 */
	virtual void finalize() { flush(); }

	/**
	 * Returns the stream position
	 */
	virtual int32 pos() const { return 0; }
};

/*
 * Get the length of a unicode string
 */
size_t strlen_uni(const uint32 *s);

} // End of namespace Gargoyle

#endif
