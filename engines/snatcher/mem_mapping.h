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

#ifndef SNATCHER_MEM_MAPPING_H
#define SNATCHER_MEM_MAPPING_H

#include "common/scummsys.h"

namespace Snatcher {

namespace MemMapping {

extern uint32 MEM_GFXDATA_00;
extern uint32 MEM_GFXDATA_01;
extern uint32 MEM_GFXDATA_02;
extern uint32 MEM_RAWDATA_00;
extern uint32 MEM_UIDATA_00;
extern uint32 MEM_VERBDATA_00;
extern uint32 MEM_VERBDATA_01;
extern uint32 MEM_VERBDATA_02;
extern uint32 MEM_VERBDATA_03;
extern uint32 MEM_UIDATA_01;
extern uint32 MEM_UIDATA_02;
extern uint32 MEM_UIDATA_03;
extern uint32 MEM_UIDATA_04;
extern uint32 MEM_VERBDATA_04;
extern uint32 MEM_VERBDATA_05;
extern uint32 MEM_UIDATA_05;
extern uint32 MEM_PALDATA_00;
extern uint32 MEM_PALDATA_01;
extern uint32 MEM_PALDATA_02;
extern uint32 MEM_PALDATA_03;
extern uint32 MEM_PALDATA_04;
extern uint32 MEM_GFXDATA_04;
extern uint32 MEM_GFXDATA_05;
extern uint32 MEM_GFXDATA_06;
extern uint32 MEM_GFXDATA_07;
extern uint32 MEM_ACTIONSEQDATA_00;
extern uint32 MEM_ACTIONSEQDATA_01;
extern uint32 MEM_ACTIONSEQDATA_02;
extern uint32 MEM_ACTIONSEQDATA_03;
extern uint32 MEM_ACTIONSEQDATA_04;
extern uint32 MEM_ACTIONSEQDATA_05;
extern uint32 MEM_ACTIONSEQDATA_06;
extern uint32 MEM_ACTIONSEQDATA_07;
extern uint32 MEM_ACTIONSEQDATA_08;
extern uint32 MEM_ACTIONSEQDATA_09;
extern uint32 MEM_ACTIONSEQDATA_10;
extern uint32 MEM_ACTIONSEQDATA_11;
extern uint32 MEM_ACTIONSEQDATA_12;
extern uint32 MEM_ACTIONSEQDATA_13;
extern uint32 MEM_ACTIONSEQDATA_14;
extern uint32 MEM_ACTIONSEQDATA_15;
extern uint32 MEM_ACTIONSEQDATA_16;
extern uint32 MEM_ACTIONSEQDATA_17;
extern uint32 MEM_ACTIONSEQDATA_18;
extern uint32 MEM_ACTIONSEQDATA_19;
extern uint32 MEM_ACTIONSEQDATA_20;
extern uint32 MEM_ACTIONSEQDATA_21;
extern uint32 MEM_UIDATA_06;
extern uint32 MEM_PALDATA_05;
extern uint32 MEM_UIDATA_07;
extern uint32 MEM_UIDATA_08;
extern uint32 MEM_UIDATA_09;
extern uint32 MEM_UIDATA_10;
extern uint32 MEM_PALDATA_06;
extern uint32 MEM_RAWDATA_01;
extern uint32 MEM_RAWDATA_02;
extern uint32 MEM_RAWDATA_03;
extern uint32 MEM_PALDATA_07;
extern uint32 MEM_GFXDATA_10;
extern uint32 MEM_RAWDATA_04;
extern uint32 MEM_RAWDATA_05;
extern uint32 MEM_RAWDATA_06;
extern uint32 MEM_PALDATA_10;
extern uint32 MEM_PALDATA_11;
extern uint32 MEM_PALDATA_12;
extern uint32 MEM_TIMERDATA_00;
extern uint32 MEM_TIMERDATA_01;
extern uint32 MEM_PALDATA_20;
extern uint32 MEM_PALDATA_21;
extern uint32 MEM_PALDATA_22;
extern uint32 MEM_PALDATA_23;
extern uint32 MEM_PALDATA_30;
extern uint32 MEM_PALDATA_31;
extern uint32 MEM_TIMERDATA_02;
extern uint32 MEM_PALDATA_40;
extern uint32 MEM_PALDATA_41;
extern uint32 MEM_PALDATA_42;

extern uint32 MEM_MODULE_LOCATION;
extern uint32 MEM_SCRIPT_LOCATION;
extern uint32 MEM_SUBCODE_LOCATION;

extern void initMapping(int mappingNum);

} // End of namespace MemMapping

} // End of namespace Snatcher

#endif // SNATCHER_MEM_MAPPING_H

