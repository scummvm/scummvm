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

#ifndef SNATCHER_PALEVENT_SCD_H
#define SNATCHER_PALEVENT_SCD_H

#include "common/scummsys.h"

namespace Snatcher {

struct PalEventSCD {
	uint8 cmd;
	uint8 delay;
	uint8 countDown;
	uint8 destOffset;
	uint8 len;
	const uint32 *srcOffsetCur;
	const uint32 *srcOffsets;
	const uint8 *srcOrig;
	uint8 progress;
	uint8 fD;
	uint8 fE;
	uint8 fF;
};

} // End of namespace Snatcher

#endif // SNATCHER_PALEVENT_SCD_H
