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
 */
/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included VGMTrans_LICENSE.txt file
 */
#ifndef AUDIO_SOUNDFONT_COMMON_H
#define AUDIO_SOUNDFONT_COMMON_H

#include "common/scummsys.h"
#include "common/array.h"

enum LoopMeasure {
	LM_SAMPLES, LM_BYTES
};

struct Loop {
	Loop()
			: loopStatus(-1),
			  loopType(0),
			  loopStartMeasure(LM_BYTES),
			  loopLengthMeasure(LM_BYTES),
			  loopStart(0),
			  loopLength(0) {}

	int loopStatus;
	uint32 loopType;
	uint8 loopStartMeasure;
	uint8 loopLengthMeasure;
	uint32 loopStart;
	uint32 loopLength;
};

struct SizeOffsetPair {
	uint32 size;
	uint32 offset;

	SizeOffsetPair() : size(0), offset(0) {}

	SizeOffsetPair(uint32 offset_, uint32 size_) : size(size_), offset(offset_) {}
};

template<class T>
void DeleteVect(Common::Array<T *> &array) {
	for (typename Common::Array<T *>::iterator iter = array.begin(); iter != array.end(); iter++) {
		delete (*iter);
	}
	array.clear();
}

#endif // AUDIO_SOUNDFONT_COMMON_H
