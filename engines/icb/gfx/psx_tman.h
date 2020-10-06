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

#ifndef ICB_PSX_TMAN_H
#define ICB_PSX_TMAN_H

#include "engines/icb/gfx/psx_pcgpu.h"

namespace ICB {

#define MAX_NUMBER_SLOTS 8
#define MAX_NUMBER_PALETTES 10

#define N_TILES_X 4
#define N_TILES_Y 4

#define MAX_NUMBER_TILES (N_TILES_X * N_TILES_Y)

typedef struct TextureInfo {
	// DO NOT CHANGE THIS SECTION OF THE STRUCTURE
	// AS PSX ASSMEBLER ROUTINES RELY ON IT BEING LIKE THIS
	short tsb; // tpf | abr | texture_page  = getTpate( tpf, abr, x, y );
	short cba; // cy | cx = getClut(cx,cy)
	u_char uoffset;
	u_char voffset;
	short padding;
	// DO WHAT YOU LIKE FROM HERE ONWARDS
	u_int id;
	u_int age;
	RECT16 r;
} TextureInfo;

typedef struct PaletteInfo {
	u_int id;
	u_int age;
	short x;
	short y;
	short cba; // cy | cx = getClut(cx,cy)
	short padding;
} PaletteInfo;

class TextureManager {
public:
	TextureManager();
	TextureManager(short nx0, short ny0, short nx1, short ny1);
	~TextureManager();
	void Init(short nx0, short ny0, short nx1, short ny1);
	TextureInfo *FindTexture(u_int id, u_int age);
	TextureInfo *AddTexture(uint32 *tim_ptr, u_int id, u_int age, u_short imgW, u_short imgH);

	PaletteInfo *FindPalette(u_int id, u_int age);
	PaletteInfo *AddPalette(uint32 *clut_ptr, u_int id, u_int age);

	void PurgeAll(void);

	TextureInfo tSlots[MAX_NUMBER_SLOTS];
	PaletteInfo pSlots[MAX_NUMBER_PALETTES];
	u_char inuse[MAX_NUMBER_TILES];
	short x0, y0;
	short x1, y1;
	u_short tileW, tileH;
	u_int nSlotsUsed;
	u_int nPalettesUsed;
};

} // End of namespace ICB

#endif // #ifndef PSX_TMAN_H
