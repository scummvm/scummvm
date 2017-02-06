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

#ifndef TITANIC_AUDIO_BUFFER_H
#define TITANIC_AUDIO_BUFFER_H

#include "common/array.h"
#include "common/mutex.h"

namespace Titanic {

class CAudioBuffer {
private:
	Common::Mutex _mutex;
private:
	/**
	 * Gets the beginning of stored audio data
	 */
	byte *getBegin();

	/**
	 * Gets the end of the stored audio data
	 */
	byte *getEnd();

	/**
	 * Reverses the audio buffer
	 */
	void reverse();
public:
	Common::Array<byte> _buffer;
	int _readBytesLeft;
	int _writeBytesLeft;
	bool _flag;
	bool _disabled;
public:
	CAudioBuffer(int bufferSize);
	~CAudioBuffer();

	/**
	 * Resets the audio buffer
	 */
	void reset();

	/**
	 * Gets a pointer to the start of previously written data
	 */
	uint16 *getReadPtr();

	/**
	 * Returns the number of bytes that can be read
	 */
	int getBytesToRead() const { return _readBytesLeft; }

	/**
	 * Advances the read index
	 */
	void advanceRead(int size);

	/**
	 * Gets a pointer to the remainder of the audio buffer that
	 * can be written to
	 */
	uint16 *getWritePtr();

	/**
	 * Returns how many bytes can be written before hitting the
	 * end of the audio buffer
	 */
	int getWriteBytesLeft() const { return _writeBytesLeft; }
	
	/**
	 * Advances the write pointer by the specified number of bytes
	 */
	void advanceWrite(int size);

	/**
	 * Enters a critical section
	 */
	void enterCriticalSection();

	/**
	 * Leave a critical section
	 */
	void leaveCriticalSection();
};

} // End of namespace Titanic

#endif /* TITANIC_AUDIO_BUFFER_H */
