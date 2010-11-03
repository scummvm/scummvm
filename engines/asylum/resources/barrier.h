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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_BARRIER_H
#define ASYLUM_BARRIER_H

#include "asylum/system/sound.h"

#include "asylum/shared.h"

namespace Asylum {

class Barrier {
public:
	Barrier();
	virtual ~Barrier();

	bool   visible();
	int32 getRandomId(); // TODO Give this a better name?
	bool   onscreen();

	// TODO document this function
	int  checkFlags();
	// TODO document this function
	void setNextFrame(int flags);

	/** .text:0040D0E0
	 * Check if any items in the barrier sound array are playing,
	 * and based on their flag values, stop them accordingly
	 */
	void updateSoundItems(Sound *snd);

	int32		   id;
	uint32		   resId;
	int32		   x;
	int32		   y;
	Common::Rect   boundingRect;
	int32		   field_20;
	uint32		   frameIdx;
	uint32		   frameCount;
	int32		   field_2C;
	int32		   field_30;
	int32		   field_34;
	int32		   flags;
	int32		   field_3C;
	uint8		   name[52];
	int32		   field_74; // XXX looks like fields
	int32		   field_78; // 74 => 80 have something
	int32		   field_7C; // to do with calculating
	int32		   field_80; // actor intersection
	int32		   polyIdx;
	int32		   flags2;
	GameFlag	   gameFlags[10];
	int32		   field_B4;
	int32		   tickCount;
	int32		   tickCount2;
	int32		   field_C0;
	int32		   priority;
	int32		   actionListIdx;
	SoundItem	   soundItems[16];
	FrameSoundItem frameSoundItems[50];
	int32		   field_67C;
	int32		   soundX;
	int32		   soundY;
	int32		   field_688;
	int32		   field_68C[5];
	int32		   soundResId;
	int32		   field_6A4;

}; // end of class Barrier

} // end of namespace Asylum

#endif
