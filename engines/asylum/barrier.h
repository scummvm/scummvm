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

#ifndef ASYLUM_BARRIER_H_
#define ASYLUM_BARRIER_H_

#include "asylum/sound.h"

namespace Asylum {

class Barrier {
public:
	Barrier();
	virtual ~Barrier();

	bool   visible();
	uint32 getRandomId(); // TODO Give this a better name?
	bool   onscreen();

	// TODO document this function
	int  checkFlags();
	// TODO document this function
	void setNextFrame(int flags);

	uint32		   id;
	uint32		   resId;
	uint32		   x;
	uint32		   y;
	Common::Rect   boundingRect;
	uint32		   field_20;
	uint32		   frameIdx;
	uint32		   frameCount;
	uint32		   field_2C;
	uint32		   field_30;
	uint32		   field_34;
	uint32		   flags;
	uint32		   field_3C;
	uint8		   name[52]; // field_40 till field_70;
	uint32		   field_74;
	uint32		   field_78;
	uint32		   field_7C;
	uint32		   field_80;
	uint32		   polyIdx;
	uint32		   flags2;
	uint32		   gameFlags[10];
	uint32		   field_B4;
	uint32		   tickCount;
	uint32		   tickCount2;
	uint32		   field_C0;
	uint32		   priority;
	uint32		   actionListIdx;
	SoundItem	   soundItems[16];
	FrameSoundItem frameSoundItems[50];
	uint32		   field_67C;
	uint32		   soundX;
	uint32		   soundY;
	uint32		   field_688;
	uint32		   field_68C[5];
	uint32		   soundResId;
	uint32		   field_6A4;

}; // end of class Barrier

} // end of namespace Asylum

#endif
