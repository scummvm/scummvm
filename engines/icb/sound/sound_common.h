/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_SOUNDCOMMON_H__INCLUDED_
#define ICB_SOUNDCOMMON_H__INCLUDED_

#include "engines/icb/sound/direct_sound.h"

#include "common/stream.h"

namespace ICB {

typedef struct _wavHeader {
	char riff[4];
	uint32 fileLength;
	char wavID[4];
	char format[4];
	uint32 formatLen;
	uint16 formatTag;
	uint16 channels;
	uint32 samplesPerSec;
	uint32 avgBytesPerSec;
	uint16 blockAlign;
	uint16 bitsPerSample;

} _wavHeader;

// Common code between the three sound managers
bool8 openWav(Common::SeekableReadStream *filePtr, _wavHeader &header, uint32 &length, uint32 &byteOffsetInCluster, int &lengthInCycles);

} // End of namespace ICB

#endif
