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

#ifndef SWORD25_AUDIOBUFFER_H
#define SWORD25_AUDIOBUFFER_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"

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
