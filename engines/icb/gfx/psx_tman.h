/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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
	int16 tsb; // tpf | abr | texture_page  = getTpate( tpf, abr, x, y );
	int16 cba; // cy | cx = getClut(cx,cy)
	uint8 uoffset;
	uint8 voffset;
	int16 padding;
	// DO WHAT YOU LIKE FROM HERE ONWARDS
	uint32 id;
	uint32 age;
	RECT16 r;
} TextureInfo;

typedef struct PaletteInfo {
	uint32 id;
	uint32 age;
	int16 x;
	int16 y;
	int16 cba; // cy | cx = getClut(cx,cy)
	int16 padding;
} PaletteInfo;

class TextureManager {
public:
	TextureManager();
	TextureManager(int16 nx0, int16 ny0, int16 nx1, int16 ny1);
	~TextureManager();
	void Init(int16 nx0, int16 ny0, int16 nx1, int16 ny1);
	TextureInfo *FindTexture(uint32 id, uint32 age);
	TextureInfo *AddTexture(uint32 *tim_ptr, uint32 id, uint32 age, uint16 imgW, uint16 imgH);

	PaletteInfo *FindPalette(uint32 id, uint32 age);
	PaletteInfo *AddPalette(uint32 *clut_ptr, uint32 id, uint32 age);

	void PurgeAll(void);

	TextureInfo tSlots[MAX_NUMBER_SLOTS];
	PaletteInfo pSlots[MAX_NUMBER_PALETTES];
	uint8 inuse[MAX_NUMBER_TILES];
	int16 x0, y0;
	int16 x1, y1;
	uint16 tileW, tileH;
	uint32 nSlotsUsed;
	uint32 nPalettesUsed;
};

} // End of namespace ICB

#endif // #ifndef PSX_TMAN_H
