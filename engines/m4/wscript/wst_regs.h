
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

#ifndef M4_WSCRIPT_WST_REGS_H
#define M4_WSCRIPT_WST_REGS_H

#include "m4/m4_types.h"

namespace M4 {

enum {
	IDX_TIMER				= 0,
	IDX_TAG					= 1,
	IDX_LAYER				= 2,
	
	IDX_W					= 3,
	IDX_H					= 4,
	IDX_X					= 5,
	IDX_Y					= 6,
	IDX_S					= 7,
	IDX_R					= 8,
	
	IDX_CELS_HASH			= 9,
	IDX_CELS_INDEX			= 10,
	IDX_CELS_COUNT			= 11,
	IDX_CELS_FRAME_RATE		= 12,
	IDX_CELS_PIX_SPEED		= 13,
	
	IDX_TARG_S				= 14,
	IDX_TARG_R				= 15,
	IDX_TARG_X				= 16,
	IDX_TARG_Y				= 17,

	IDX_DELTA_S				= 18,
	IDX_DELTA_R				= 19,
	IDX_DELTA_X				= 20,
	IDX_DELTA_Y				= 21,
	
	IDX_VELOCITY			= 22,
	IDX_THETA				= 23,
	
	IDX_ZTEMP1				= 24,
	IDX_ZTEMP2				= 25,
	IDX_ZTEMP3				= 26,
	IDX_ZTEMP4				= 27,
	IDX_ZTEMP5				= 28,
	IDX_ZTEMP6				= 29,
	IDX_ZTEMP7				= 30,
	IDX_ZTEMP8				= 31,

	IDX_MACH_ID				= 32
};

#define IDX_COUNT			33

#define IDX_BEZ_CTRL		IDX_TARG_X
#define IDX_BEZ_COEFF		IDX_ZTEMP1

#define TAG_COUNT			8
#define TAG_NONE			0x00000000
#define TAG_TARGS			0x00000001
#define TAG_BEZ				0x00000002
#define TAG_DELTAS			0x00000004
#define TAG_VECTORS			0x00000008
#define TAG_TEXT			0x00000010
#define TAG_MOVE_CEL		0x00004000
#define TAG_MAP_CEL			0x00002000

#define GLOB_COUNT 39

extern const char *myRegLabels[];
extern const char *myGlobLabels[];
extern const char *tagLabels[];

} // End of namespace M4

#endif
