/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifndef AGOS_VGA_H
#define AGOS_VGA_H

namespace AGOS {

#include "common/pack-start.h"	// START STRUCT PACKING

// Feeble Files
struct VgaFileHeader2_Feeble {
	uint16 imageCount;
	uint16 x_2;
	uint16 animationCount;
	uint16 x_3;
	uint16 imageTable;
	uint16 x_4;
	uint16 animationTable;
	uint16 x_5;
};

struct ImageHeader_Feeble {
	uint16 id;
	uint16 x_1;
	uint16 scriptOffs;
	uint16 x_2;
};

struct AnimationHeader_Feeble {
	uint16 scriptOffs;
	uint16 x_2;
	uint16 id;
};

// Simon 1/2
struct ImageHeader_Simon {
	uint16 id;
	uint16 color;
	uint16 x_2;
	uint16 scriptOffs;
};

struct AnimationHeader_Simon {
	uint16 id;
	uint16 x_2;
	uint16 scriptOffs;
};


// Elvira 1/2 and Waxworks
struct ImageHeader_WW {
	uint16 id;
	uint16 color;
	uint16 x_2;
	uint16 scriptOffs;
};

struct AnimationHeader_WW {
	uint16 id;
	uint16 x_1;
	uint16 x_2;
	uint16 scriptOffs;
};

// Common
struct VgaFileHeader2_Common {
	uint16 x_1;
	uint16 imageCount;
	uint16 x_2;
	uint16 animationCount;
	uint16 x_3;
	uint16 imageTable;
	uint16 x_4;
	uint16 animationTable;
	uint16 x_5;
};

#include "common/pack-end.h"	// END STRUCT PACKING

enum DrawFlags {
	kDFFlip           = 0x1,
	kDFNonTrans       = 0x2,
	kDFSkipStoreBG    = 0x4,
	kDFCompressed     = 0x8,
	kDFCompressedFlip = 0x10,
	kDFMasked         = 0x20,

	// Feeble specific
	kDFOverlayed      = 0x10,
	kDFScaled         = 0x40,
	kDFShaded         = 0x80
};

struct VC10_state {
	int16 image;
	uint16 flags;
	byte palette;
	byte paletteMod;

	int16 x, y;
	uint16 width, height;
	uint16 draw_width, draw_height;
	uint16 x_skip, y_skip;

	byte *surf2_addr;
	uint surf2_pitch;

	byte *surf_addr;
	uint surf_pitch;

	uint16 dl, dh;

	const byte *srcPtr;
	int8 depack_cont;

	byte depack_dest[480];

	VC10_state() { memset(this, 0, sizeof(*this)); }
};

byte *vc10_depackColumn(VC10_state *vs);

} // End of namespace AGOS

#endif
