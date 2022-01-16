/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TINSEL_MAREELS_H	// prevent multiple includes
#define TINSEL_MAREELS_H

#include "tinsel/dw.h"	// for SCNHANDLE
#include "tinsel/movers.h"

namespace Tinsel {

void SetWalkReels(MOVER *pMover, int scale,
		SCNHANDLE al, SCNHANDLE ar, SCNHANDLE af, SCNHANDLE aa);

void SetStandReels(MOVER *pMover, int scale,
		SCNHANDLE al, SCNHANDLE ar, SCNHANDLE af, SCNHANDLE aa);

void SetTalkReels(MOVER *pMover, int scale,
		SCNHANDLE al, SCNHANDLE ar, SCNHANDLE af, SCNHANDLE aa);

SCNHANDLE GetMoverTalkReel(MOVER *pActor, TFTYPE dirn);

void SetScalingReels(int actor, int scale, int direction,
		SCNHANDLE left, SCNHANDLE right, SCNHANDLE forward, SCNHANDLE away);

SCNHANDLE ScalingReel(int ano, int scale1, int scale2, DIRECTION reel);

void RebootScalingReels();

void TouchMoverReels();

} // End of namespace Tinsel

#endif
