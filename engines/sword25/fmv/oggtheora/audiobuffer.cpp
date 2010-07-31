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
#include <windows.h>
#include <queue>

using namespace std;

// -----------------------------------------------------------------------------

struct BS_AudioBuffer::Impl
{
	CRITICAL_SECTION CS;
	queue<signed short> Buffer;
};

// -----------------------------------------------------------------------------

BS_AudioBuffer::BS_AudioBuffer() : t(new Impl())
{
	InitializeCriticalSection(&t->CS);
}

// -----------------------------------------------------------------------------

BS_AudioBuffer::~BS_AudioBuffer()
{
	DeleteCriticalSection(&t->CS);

	delete t;
}

// -----------------------------------------------------------------------------

void BS_AudioBuffer::Push(signed short * SamplePtr, unsigned int SampleCount)
{
	EnterCriticalSection(&t->CS);

	signed short * SampleEndPtr = SamplePtr + SampleCount;
	while (SamplePtr != SampleEndPtr) t->Buffer.push(*SamplePtr++);

	LeaveCriticalSection(&t->CS);
}

// -----------------------------------------------------------------------------

unsigned int BS_AudioBuffer::Pop(signed short * SamplePtr, unsigned int SampleCount)
{
	EnterCriticalSection(&t->CS);

	unsigned int i = 0;
	for (; i < SampleCount && !t->Buffer.empty(); ++i)
	{
		SamplePtr[i] = t->Buffer.front();
		t->Buffer.pop();
	}

	LeaveCriticalSection(&t->CS);

	return i;
}

// -----------------------------------------------------------------------------

unsigned int BS_AudioBuffer::Size() const
{
	EnterCriticalSection(&t->CS);
	unsigned int result = t->Buffer.size();
	LeaveCriticalSection(&t->CS);

	return result;
}
