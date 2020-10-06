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

#ifndef ICB_BINK_STUB_H
#define ICB_BINK_STUB_H

#include "engines/icb/common/px_rcutypes.h"

namespace ICB {

#define BINKSURFACE24 0
#define BINKSURFACE32 0
#define BINKFROMMEMORY 0
#define BINKNOSKIP 0

struct BINK {
	uint32 Width;
	uint32 Height;
	uint32 Frames;
	uint32 FrameNum;
	uint32 LastFrameNum;

	BINK() {
		Width = 640;
		Height = 480;
		Frames = 1;
		FrameNum = 1;
		LastFrameNum = 1;
	}
};

typedef BINK *HBINK;

HBINK BinkOpen(const char *name, uint32 flags);
void BinkClose(HBINK handle);

const char *BinkGetError();

int32 BinkWait(HBINK handle);
void BinkService(HBINK handle);

int32 BinkDoFrame(HBINK handle);
void BinkNextFrame(HBINK handle);

int32 BinkCopyToBuffer(HBINK handle, void *dst, int32 dstPitch, uint32 dstHeight, uint32 dstX, uint32 dstY, uint32 flags);
void BinkGoto(HBINK handle, uint32 frame, int32 flags);

void BinkSoundUseDirectSound(uint32 flags);
void BinkSetVolume(HBINK handle, int volume);

} // End of namespace ICB

#endif // BINK_STUB_H
