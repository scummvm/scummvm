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

#include "snatcher/mem_mapping.h"

namespace Snatcher {

namespace MemMapping {

uint32 MEM_GFXDATA_00                	=	0;
uint32 MEM_GFXDATA_01                	=	0;
uint32 MEM_GFXDATA_02                	=	0;
uint32 MEM_RAWDATA_00                	=	0;
uint32 MEM_UIDATA_00                 	=	0;
uint32 MEM_VERBDATA_00					=	0;
uint32 MEM_VERBDATA_01					=	0;
uint32 MEM_VERBDATA_02					=	0;
uint32 MEM_VERBDATA_03					=	0;
uint32 MEM_UIDATA_01                 	=	0;
uint32 MEM_UIDATA_02                 	=	0;
uint32 MEM_UIDATA_03                 	=	0;
uint32 MEM_UIDATA_04                 	=	0;
uint32 MEM_VERBDATA_04					=	0;
uint32 MEM_VERBDATA_05					=	0;
uint32 MEM_UIDATA_05                 	=	0;
uint32 MEM_PALDATA_00                	=	0;
uint32 MEM_PALDATA_01                	=	0;
uint32 MEM_PALDATA_02                	=	0;
uint32 MEM_PALDATA_03                	=	0;
uint32 MEM_PALDATA_04                	=	0;
uint32 MEM_GFXDATA_04                	=	0;
uint32 MEM_GFXDATA_05                	=	0;
uint32 MEM_GFXDATA_06                	=	0;
uint32 MEM_GFXDATA_07                	=	0;
uint32 MEM_ACTIONSEQDATA_00          	=	0;
uint32 MEM_ACTIONSEQDATA_01          	=	0;
uint32 MEM_ACTIONSEQDATA_02          	=	0;
uint32 MEM_ACTIONSEQDATA_03          	=	0;
uint32 MEM_ACTIONSEQDATA_04          	=	0;
uint32 MEM_ACTIONSEQDATA_05          	=	0;
uint32 MEM_ACTIONSEQDATA_06          	=	0;
uint32 MEM_ACTIONSEQDATA_07          	=	0;
uint32 MEM_ACTIONSEQDATA_08          	=	0;
uint32 MEM_ACTIONSEQDATA_09          	=	0;
uint32 MEM_ACTIONSEQDATA_10          	=	0;
uint32 MEM_ACTIONSEQDATA_11          	=	0;
uint32 MEM_ACTIONSEQDATA_12          	=	0;
uint32 MEM_ACTIONSEQDATA_13          	=	0;
uint32 MEM_ACTIONSEQDATA_14          	=	0;
uint32 MEM_ACTIONSEQDATA_15          	=	0;
uint32 MEM_ACTIONSEQDATA_16          	=	0;
uint32 MEM_ACTIONSEQDATA_17          	=	0;
uint32 MEM_ACTIONSEQDATA_18          	=	0;
uint32 MEM_ACTIONSEQDATA_19          	=	0;
uint32 MEM_ACTIONSEQDATA_20          	=	0;
uint32 MEM_ACTIONSEQDATA_21          	=	0;
uint32 MEM_UIDATA_06                 	=	0;
uint32 MEM_PALDATA_05                	=	0;
uint32 MEM_UIDATA_07                 	=	0;
uint32 MEM_UIDATA_08                 	=	0;
uint32 MEM_UIDATA_09                 	=	0;
uint32 MEM_UIDATA_10                 	=	0;
uint32 MEM_PALDATA_06                	=	0;
uint32 MEM_RAWDATA_01                	=	0;
uint32 MEM_RAWDATA_02                	=	0;
uint32 MEM_RAWDATA_03                	=	0;
uint32 MEM_PALDATA_07                	=	0;
uint32 MEM_GFXDATA_10                	=	0;
uint32 MEM_RAWDATA_04                	=	0;
uint32 MEM_RAWDATA_05                	=	0;
uint32 MEM_RAWDATA_06                	=	0;
uint32 MEM_PALDATA_10                	=	0;
uint32 MEM_PALDATA_11                	=	0;
uint32 MEM_PALDATA_12                	=	0;
uint32 MEM_TIMERDATA_00					=	0;
uint32 MEM_TIMERDATA_01					=	0;
uint32 MEM_PALDATA_20                	=	0;
uint32 MEM_PALDATA_21                	=	0;
uint32 MEM_PALDATA_22                	=	0;
uint32 MEM_PALDATA_23                	=	0;
uint32 MEM_PALDATA_30                	=	0;
uint32 MEM_PALDATA_31                	=	0;
uint32 MEM_TIMERDATA_02					=	0;
uint32 MEM_PALDATA_40                	=	0;
uint32 MEM_PALDATA_41                	=	0;
uint32 MEM_PALDATA_42                	=	0;

uint32 MEM_MODULE_LOCATION				=	0;
uint32 MEM_SCRIPT_LOCATION				=	0;
uint32 MEM_SUBCODE_LOCATION				=	0;

void initMapping(int mappingNum) {
	MEM_GFXDATA_00                	=	0x0ECB6;
	MEM_GFXDATA_01                	=	0x0ECC4;
	MEM_GFXDATA_02                	=	0x0ED0E;
	MEM_RAWDATA_00                	=	0x100D8;
	MEM_UIDATA_00                 	=	0x109E8;
	MEM_VERBDATA_00					=	0x10EFA;
	MEM_VERBDATA_01					=	0x10F10;
	MEM_VERBDATA_02					=	0x10F26;
	MEM_VERBDATA_03					=	0x11454;
	MEM_UIDATA_01                 	=	0x114DC;
	MEM_UIDATA_02                 	=	0x11512;
	MEM_UIDATA_03                 	=	0x115CA;
	MEM_UIDATA_04                 	=	0x11938;
	MEM_VERBDATA_04					=	0x11942;
	MEM_VERBDATA_05					=	0x11946;
	MEM_UIDATA_05                 	=	0x11D12;
	MEM_PALDATA_00                	=	0x11E6A;
	MEM_PALDATA_01                	=	0x11E74;
	MEM_PALDATA_02                	=	0x11E7E;
	MEM_PALDATA_03                	=	0x11E8C;
	MEM_PALDATA_04                	=	0x11E92;
	MEM_GFXDATA_04                	=	0x11ED8;
	MEM_GFXDATA_05                	=	0x11EE6;
	MEM_GFXDATA_06                	=	0x11EF4;
	MEM_GFXDATA_07                	=	0x11F02;
	MEM_ACTIONSEQDATA_00          	=	0x11F10;
	MEM_ACTIONSEQDATA_01          	=	0x11F20;
	MEM_ACTIONSEQDATA_02          	=	0x11F30;
	MEM_ACTIONSEQDATA_03          	=	0x11F40;
	MEM_ACTIONSEQDATA_04          	=	0x11F50;
	MEM_ACTIONSEQDATA_05          	=	0x11F60;
	MEM_ACTIONSEQDATA_06          	=	0x11F70;
	MEM_ACTIONSEQDATA_07          	=	0x11F80;
	MEM_ACTIONSEQDATA_08          	=	0x11F90;
	MEM_ACTIONSEQDATA_09          	=	0x11FA0;
	MEM_ACTIONSEQDATA_10          	=	0x11FB0;
	MEM_ACTIONSEQDATA_11          	=	0x11FC0;
	MEM_ACTIONSEQDATA_12          	=	0x11FD0;
	MEM_ACTIONSEQDATA_13          	=	0x11FE0;
	MEM_ACTIONSEQDATA_14          	=	0x11FE6;
	MEM_ACTIONSEQDATA_15          	=	0x11FEC;
	MEM_ACTIONSEQDATA_16          	=	0x11FFE;
	MEM_ACTIONSEQDATA_17          	=	0x12010;
	MEM_ACTIONSEQDATA_18          	=	0x12C9A;
	MEM_ACTIONSEQDATA_19          	=	0x12CE2;
	MEM_ACTIONSEQDATA_20          	=	0x12EA8;
	MEM_ACTIONSEQDATA_21          	=	0x12EBC;
	MEM_UIDATA_06                 	=	0x13552;
	MEM_PALDATA_05                	=	0x13622;
	MEM_UIDATA_07                 	=	0x1387C;
	MEM_UIDATA_08                 	=	0x13894;
	MEM_UIDATA_09                 	=	0x138AC;
	MEM_UIDATA_10                 	=	0x1391C;
	MEM_PALDATA_06                	=	0x139D0;
	MEM_RAWDATA_01                	=	0x13FEC;
	MEM_RAWDATA_02                	=	0x144C6;
	MEM_RAWDATA_03                	=	0x146CE;
	MEM_PALDATA_07                	=	0x14B1C;
	MEM_GFXDATA_10                	=	0x14B4E;
	MEM_RAWDATA_04                	=	0x14B7C;
	MEM_RAWDATA_05                	=	0x14EAC;
	MEM_RAWDATA_06                	=	0x16CDC;
	MEM_PALDATA_10                	=	0x288E6;
	MEM_PALDATA_11                	=	0x28944;
	MEM_PALDATA_12                	=	0x290F2;
	MEM_TIMERDATA_00                =	0x290FE;
	MEM_TIMERDATA_01                =	0x2929A;
	MEM_PALDATA_20                	=	0x292E2;
	MEM_PALDATA_21                	=	0x292F0;
	MEM_PALDATA_22                	=	0x292FE;
	MEM_PALDATA_23                	=	0x2931A;
	MEM_PALDATA_30                	=	0x2943A;
	MEM_PALDATA_31                	=	0x2942C;
	MEM_TIMERDATA_02                =	0x2A85C;
	MEM_PALDATA_40                	=	0x2AA4A;
	MEM_PALDATA_41                	=	0x2AA58;
	MEM_PALDATA_42                	=	0x2AA66;

	MEM_MODULE_LOCATION				=	0x28000;
	MEM_SCRIPT_LOCATION				=	0x1A800;
	MEM_SUBCODE_LOCATION			=	0xD400;

	if (mappingNum == 1)
		MEM_RAWDATA_06              =	0x16CEC;
}

} // End of namespace MemMapping

} // End of namespace Snatcher


