// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

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
