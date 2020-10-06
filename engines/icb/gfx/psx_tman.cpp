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

#include "engines/icb/common/px_common.h"
#include "engines/icb/debug.h"
#include "engines/icb/gfx/psx_tman.h"
#include "engines/icb/gfx/psx_clut.h"

namespace ICB {

TextureManager::TextureManager() { Init(0, 0, 0, 0); }

TextureManager::TextureManager(short nx0, short ny0, short nx1, short ny1) { Init(nx0, ny0, nx1, ny1); }

void TextureManager::Init(short nx0, short ny0, short nx1, short ny1) {
	x0 = nx0;
	y0 = ny0;
	x1 = nx1;
	y1 = ny1;
	tileW = (short)((x1 - x0) / N_TILES_X);
	tileH = (short)((y1 - y0) / N_TILES_Y);
	nSlotsUsed = 0;
	nPalettesUsed = 0;
	int t;
	for (t = 0; t < MAX_NUMBER_TILES; t++) {
		inuse[t] = 0;
	}
	int s;
	for (s = 0; s < MAX_NUMBER_SLOTS; s++) {
		tSlots[s].id = 0;
		tSlots[s].age = 0;
	}
	for (s = 0; s < MAX_NUMBER_PALETTES; s++) {
		pSlots[s].id = 0;
		pSlots[s].age = 0;
	}
}

TextureManager::~TextureManager() {}

TextureInfo *TextureManager::FindTexture(u_int id, u_int age) {
	u_int s;
	TextureInfo *slot = tSlots;
	for (s = 0; s < MAX_NUMBER_SLOTS; s++) {
		if (slot->id == id) {
			slot->age = age;
			return slot;
		}
		slot++;
	}
	// printf( "Texture %X not found", id );
	return NULL;
}

TextureInfo *TextureManager::AddTexture(uint32 * /*tim_ptr*/, u_int id, u_int age, u_short imgW, u_short imgH) {
	// printf( "Adding Texture %X", id );
	if (id == 0) {
		Message_box("AddTexture 0 ID");
		return NULL;
	}

	// Try to find a slot for it
	u_int t = 0;
	u_int s = 0;
	u_int purges = 0;

	u_int oldest;
	u_int fitted = 0;
	TextureInfo *slot;
	u_int xt = 0;
	u_int yt = 0;
	u_int ntx0 = (imgW + tileW - 1) / tileW;
	u_int nty0 = (imgH + tileH - 1) / tileH;

	u_int x, y;
	u_int xend, yend;
	u_int dy = N_TILES_X;
	u_int place0, place;
	while (fitted == 0) {
		xt = 0;
		yt = 0;
		for (t = 0; t < MAX_NUMBER_TILES; t++) {
			if (inuse[t] == 0) {
				// Found a slot which isn't in use
				// Does it fit into a single tile
				if ((imgW <= tileW) && (imgH <= tileH)) {
					fitted = 1;
					inuse[t] = 1;
					break;
				} else {
					// Covers more than one tile
					// Can it physically fit in ?
					xend = xt + ntx0;
					yend = yt + nty0;
					if ((xend <= N_TILES_X) && (yend <= N_TILES_Y)) {

						// so need to check all the tiles that would be occupied
						place0 = xt + yt * dy;
						for (y = yt; y < yend; y++) {
							place = place0;
							for (x = xt; x < xend; x++) {
								if (inuse[place] == 1) {
									// printf("x %d y %d place %d NOTOK", x, y, place );
									break;
								}
								place++;
							}
							if (x != xend)
								break;
							place0 += dy;
						}
						if (y == yend) {
							// Hey its ok
							fitted = 1;
							// Mark the tiles as being used
							place0 = xt + yt * dy;
							for (y = yt; y < yend; y++) {
								place = place0;
								for (x = xt; x < xend; x++) {
									inuse[place] = 1;
									place++;
								}
								place0 += dy;
							}
							break;
						}
					}
				}
			}
			xt++;
			if (xt == N_TILES_X) {
				xt = 0;
				yt++;
				// ERROR
				if (yt > N_TILES_Y) {
					Message_box("BAD yt");
					yt = N_TILES_Y - 1;
				}
			}
		}
		// Oh dear couldn't find a slot for it
		// Have to purge out the oldest resource
		if (t == MAX_NUMBER_TILES) {
			slot = tSlots;
			purges = MAX_NUMBER_SLOTS;
			oldest = 0xFFFFFFFF;
			for (s = 0; s < MAX_NUMBER_SLOTS; s++) {
				// Only look at slots with id != 0
				if (slot->id != 0) {
					if (slot->age < oldest) {
						oldest = slot->age;
						purges = s;
					}
				}
				slot++;
			}
			// ERROR
			if (purges == MAX_NUMBER_SLOTS) {
				Message_box("bad purges");
				purges = 0;
			}
			// Purge
			tSlots[purges].id = 0;
			tSlots[purges].age = 0;
			// Now more tricky reset the inuse flags
			RECT16 *pr = &(tSlots[purges].r);
			u_int tx = (pr->x - x0) / tileW;
			u_int ty = (pr->y - y0) / tileH;
			u_int ntx = pr->w / tileW;
			u_int nty = pr->h / tileH;
			if (ntx == 0)
				ntx = 1;
			if (nty == 0)
				nty = 1;

			// printf( "tx %d ty %d ntx %d nty %d", tx, ty, ntx, nty );
			place0 = tx + ty * dy;
			for (y = ty; y < (ty + nty); y++) {
				place = place0;
				for (x = tx; x < (tx + ntx); x++) {
					inuse[place] = 0;
					place++;
				}
				place0 += dy;
			}
		}
	}

	// A slot has been found for it : hooray
	slot = tSlots;
	for (s = 0; s < MAX_NUMBER_SLOTS; s++) {
		// Find the first slot with id 0
		if (slot->id == 0)
			break;
		slot++;
	}
	if (s == MAX_NUMBER_SLOTS) {
		Message_box("BAD slot");
		slot = tSlots;
		s = 0;
	}
	slot->id = id;
	slot->age = age;
	if (xt > N_TILES_X) {
		Message_box("bad xt %d", xt);
	}
	if (yt > N_TILES_Y) {
		Message_box("bad yt %d", yt);
	}
	slot->r.x = (short)(xt * tileW + x0);
	slot->r.y = (short)(yt * tileH + y0);
	slot->r.w = imgW;
	slot->r.h = imgH;
	slot->tsb = (short)getTPage(1, 0, slot->r.x, slot->r.y); // 8-bit, b+f
	slot->cba = (short)getClut(CHARACTER_CLUT_X, (CHARACTER_CLUT_Y + s));
	slot->uoffset = (u_char)(slot->r.x - ((slot->tsb & 0xF) << 6));
	slot->voffset = (u_char)(slot->r.y - (((slot->tsb >> 4) & 0x1) << 8));
	return slot;
}

void TextureManager::PurgeAll(void) { Init(x0, y0, x1, y1); }

PaletteInfo *TextureManager::FindPalette(u_int id, u_int age) {
	u_int s;
	PaletteInfo *slot = pSlots;
	for (s = 0; s < MAX_NUMBER_SLOTS; s++) {
		if (slot->id == id) {
			slot->age = age;
			return slot;
		}
		slot++;
	}
	return NULL;
}

PaletteInfo *TextureManager::AddPalette(uint32 * /*clut_ptr*/, u_int id, u_int age) {
	if (nPalettesUsed < MAX_NUMBER_PALETTES) {
		PaletteInfo *slot = &(pSlots[nPalettesUsed]);
		slot->x = CHARACTER_ALTERNATE_CLUT_X;
		slot->y = (short)(CHARACTER_ALTERNATE_CLUT_Y + nPalettesUsed);
		slot->cba = (short)getClut(slot->x, slot->y);
		slot->id = id;
		slot->age = age;

		nPalettesUsed++;
		return slot;
	}
	return NULL;
}

} // End of namespace ICB
