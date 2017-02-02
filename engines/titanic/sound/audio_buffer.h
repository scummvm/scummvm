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
public:
	Common::Array<byte> _buffer;
	int _fieldC;
	int _field10;
	bool _flag;
	int _field18;
public:
	CAudioBuffer(int bufferSize);
	~CAudioBuffer();

	void reset();
	const byte *getDataPtr1() const;
	const byte *getDataPtr2() const;
	const byte *getPtr1() const;
	const byte *getPtr2() const;
	int getC() const { return _fieldC; }
	int get10() const { return _field10; }
	void setC(int val);
	void set10(int val);
	void update();

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
