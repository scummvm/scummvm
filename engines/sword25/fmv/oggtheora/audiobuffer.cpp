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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* 
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/fmv/oggtheora/audiobuffer.h"

// -----------------------------------------------------------------------------

namespace Sword25 {

// -----------------------------------------------------------------------------

AudioBuffer::AudioBuffer() {
}

// -----------------------------------------------------------------------------

AudioBuffer::~AudioBuffer() {
}

// -----------------------------------------------------------------------------

void AudioBuffer::Push(signed short *SamplePtr, unsigned int SampleCount) {
	signed short *SampleEndPtr = SamplePtr + SampleCount;
	while (SamplePtr != SampleEndPtr) _buffer.push(*SamplePtr++);
}

// -----------------------------------------------------------------------------

unsigned int AudioBuffer::Pop(signed short *SamplePtr, unsigned int SampleCount) {
	unsigned int i = 0;
	for (; i < SampleCount && !_buffer.empty(); ++i) {
		SamplePtr[i] = _buffer.front();
		_buffer.pop();
	}

	return i;
}

// -----------------------------------------------------------------------------

unsigned int AudioBuffer::Size() const {
	unsigned int result = _buffer.size();

	return result;
}

} // End of namespace Sword25
