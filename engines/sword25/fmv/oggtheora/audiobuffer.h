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

#ifndef BS_AUDIOBUFFER_H
#define BS_AUDIOBUFFER_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/common.h"

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_AudioBuffer
{
public:
	BS_AudioBuffer();
	virtual ~BS_AudioBuffer();

	void Push(signed short * SamplePtr, unsigned int SampleCount);
	unsigned int Pop(signed short * SamplePtr, unsigned int SampleCount);
	unsigned int Size() const;

private:
	// PIMPL Pattern
	struct Impl;
	Impl * t;

	// Kopie verbieten
	BS_AudioBuffer(const BS_AudioBuffer &);
	const BS_AudioBuffer & operator=(const BS_AudioBuffer &);
};

#endif
