/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "common/file.h"

#include "simon/simon.h"
#include "simon/intern.h"

namespace Simon {

void SimonEngine::loadIconFile() {
	Common::File in;
	if (getPlatform() == Common::kPlatformAcorn)
		in.open("ICONDATA");
	else if (getPlatform() == Common::kPlatformAmiga)
		in.open("icon.pkd");
	else
		in.open("ICON.DAT");
	uint size;

	if (in.isOpen() == false)
		error("Can't open icons file 'ICON.DAT'");

	size = in.size();

	_iconFilePtr = (byte *)malloc(size);
	if (_iconFilePtr == NULL)
		error("Out of icon memory");

	in.read(_iconFilePtr, size);
	in.close();
}

void SimonEngine::loadIconData() {
	loadZone(8);
	VgaPointersEntry *vpe = &_vgaBufferPointers[8];

	// FIXME: For reasons unknown, the first 16 bytes are not part of the
	// actual image data. This could be an indication of some deeper
	// problem elsewhere. For now, adding 16 fixes the drawing of the
	// Oracle icon.

	byte *src = vpe->vgaFile2 + 16;

	_iconFilePtr = (byte *)malloc(43 * 336);
	if (_iconFilePtr == NULL)
		error("Out of icon memory");

	memcpy(_iconFilePtr, src, 43 * 336);
	o_unfreezeBottom();
}

// Thanks to Stuart Caie for providing the original
// C conversion upon which this function is based.
void decompressIconAmiga (byte *dst, byte *src, byte base, uint pitch) {
	byte icon_pln[288];
	byte *i, *o, x, y;

	// Decode RLE planar icon data
	i = src;
	o = icon_pln;
	while (o < &icon_pln[288]) {
		x = *i++;
		if (x < 128) {
			do {
				*o++ = *i++;
				*o++ = *i++;
				*o++ = *i++;
			} while (x-- > 0);
		} else {
			x = 256 - x;
			do {
				*o++ = i[0];
				*o++ = i[1];
				*o++ = i[2];
			} while (x-- > 0);
			i += 3;
		}
	}

	// Translate planar data to chunky (very slow method)
	for (y = 0; y < 24; y++) {
		for (x = 0; x < 24; x++) {
			byte pixel =
				  (icon_pln[((     y) * 3) + (x >> 3)] & (1 << (7 - (x & 7))) ? 1 : 0)
				| (icon_pln[((24 + y) * 3) + (x >> 3)] & (1 << (7 - (x & 7))) ? 2 : 0)
				| (icon_pln[((48 + y) * 3) + (x >> 3)] & (1 << (7 - (x & 7))) ? 4 : 0)
				| (icon_pln[((72 + y) * 3) + (x >> 3)] & (1 << (7 - (x & 7))) ? 8 : 0);
			if (pixel)
				dst[x] = pixel | base;
		}
		dst += pitch;
	}
}

static void decompressIcon(byte *dst, byte *src, uint w, uint h_org, byte base, uint pitch) {
	int8 reps;
	byte color_1, color_2;
	byte *dst_org = dst;
	uint h = h_org;

	for (;;) {
		reps = *src++;
		if (reps < 0) {
			reps--;
			color_1 = *src >> 4;
			if (color_1 != 0)
				color_1 |= base;
			color_2 = *src++ & 0xF;
			if (color_2 != 0)
				color_2 |= base;

			do {
				if (color_1 != 0)
					*dst = color_1;
				dst += pitch;
				if (color_2 != 0)
					*dst = color_2;
				dst += pitch;

				// reached bottom?
				if (--h == 0) {
					// reached right edge?
					if (--w == 0)
						return;
					dst = ++dst_org;
					h = h_org;
				}
			} while (++reps != 0);
		} else {
			do {
				color_1 = *src >> 4;
				if (color_1 != 0)
					*dst = color_1 | base;
				dst += pitch;

				color_2 = *src++ & 0xF;
				if (color_2 != 0)
					*dst = color_2 | base;
				dst += pitch;

				// reached bottom?
				if (--h == 0) {
					// reached right edge?
					if (--w == 0)
						return;
					dst = ++dst_org;
					h = h_org;
				}
			} while (--reps >= 0);
		}
	}
}

void SimonEngine::draw_icon_c(WindowBlock *window, uint icon, uint x, uint y) {
	byte *dst;
	byte *src;

	_lockWord |= 0x8000;
	dst = getFrontBuf();

	if (getGameType() == GType_SIMON1) {
		// Simon 1
		dst += (x + window->x) * 8;
		dst += (y * 25 + window->y) * _dxSurfacePitch;

		if (getPlatform() == Common::kPlatformAmiga) {
			src = _iconFilePtr;
			src += READ_BE_UINT32(&((uint32 *)src)[icon]);
			decompressIconAmiga (dst, src, 224, _dxSurfacePitch);
		} else {
			src = _iconFilePtr;
			src += READ_LE_UINT16(&((uint16 *)src)[icon]);
			decompressIcon(dst, src, 24, 12, 224, _dxSurfacePitch);
		}
	} else {
		// Simon 2
		dst += 110;
		dst += x;
		dst += (y + window->y) * _dxSurfacePitch;

		src = _iconFilePtr;
		src += READ_LE_UINT16(&((uint16 *)src)[icon * 2 + 0]);
		decompressIcon(dst, src, 20, 10, 224, _dxSurfacePitch);

		src = _iconFilePtr;
		src += READ_LE_UINT16(&((uint16 *)src)[icon * 2 + 1]);
		decompressIcon(dst, src, 20, 10, 208, _dxSurfacePitch);
	}

	_lockWord &= ~0x8000;
}

void SimonEngine::drawIconArray(uint num, Item *itemRef, int line, int classMask) {
	if (getGameType() == GType_FF) {
		drawIconArray_FF(num, itemRef, line, classMask);
	} else {
		drawIconArray_Simon(num, itemRef, line, classMask);
	}
}

void SimonEngine::drawIconArray_Simon(uint num, Item *itemRef, int line, int classMask) {
	Item *item_ptr_org = itemRef;
	WindowBlock *window;
	uint width_div_3, height_div_3;
	uint j, k, i, num_sibs_with_flag;
	bool item_again;
	uint x_pos, y_pos;

	window = _windowArray[num & 7];

	if (getGameType() == GType_SIMON1) {
		width_div_3 = window->width / 3;
		height_div_3 = window->height / 3;
	} else {
		width_div_3 = 100;
		height_div_3 = 40;
	}

	i = 0;

	if (window == NULL)
		return;

	if (window->iconPtr)
		removeIconArray(num);

	window->iconPtr = (IconBlock *) malloc(sizeof(IconBlock));
	window->iconPtr->itemRef = itemRef;
	window->iconPtr->upArrow = -1;
	window->iconPtr->downArrow = -1;
	window->iconPtr->line = line;
	window->iconPtr->classMask = classMask;

	itemRef = derefItem(itemRef->child);

	while (itemRef && line-- != 0) {
		num_sibs_with_flag = 0;
		while (itemRef && width_div_3 > num_sibs_with_flag) {
			if ((classMask == 0 || itemRef->classFlags & classMask) && has_item_childflag_0x10(itemRef))
				if (getGameType() == GType_SIMON1) {
					num_sibs_with_flag++;
				} else {
					num_sibs_with_flag += 20;
				}
			itemRef = derefItem(itemRef->sibling);
		}
	}

	if (itemRef == NULL) {
		window->iconPtr->line = 0;
		itemRef = derefItem(item_ptr_org->child);
	}

	x_pos = 0;
	y_pos = 0;
	item_again = false;
	k = 0;
	j = 0;

	while (itemRef) {
		if ((classMask == 0 || itemRef->classFlags & classMask) && has_item_childflag_0x10(itemRef)) {
			if (item_again == false) {
				window->iconPtr->iconArray[k].item = itemRef;
				if (getGameType() == GType_SIMON1) {
					draw_icon_c(window, itemGetIconNumber(itemRef), x_pos * 3, y_pos);
					window->iconPtr->iconArray[k].boxCode =
						setupIconHitArea(window, 0, x_pos * 3, y_pos, itemRef);
				} else {
					draw_icon_c(window, itemGetIconNumber(itemRef), x_pos, y_pos);
					window->iconPtr->iconArray[k].boxCode =
						setupIconHitArea(window, 0, x_pos, y_pos, itemRef);
				}
				k++;
			} else {
				window->iconPtr->iconArray[k].item = NULL;
				j = 1;
			}
			x_pos += (getGameType() == GType_SIMON1) ? 1 : 20;

			if (x_pos >= width_div_3) {
				x_pos = 0;

				y_pos += (getGameType() == GType_SIMON1) ? 1 : 20;
				if (y_pos >= height_div_3)
					item_again = true;
			}
		}
		itemRef = derefItem(itemRef->sibling);
	}

	window->iconPtr->iconArray[k].item = NULL;

	if (j != 0 || window->iconPtr->line != 0) {
		addArrows(window, num);
	}
}

void SimonEngine::drawIconArray_FF(uint num, Item *itemRef, int line, int classMask) {
	Item *item_ptr_org = itemRef;
	WindowBlock *window;
	uint16 flagnumber = 201;
	uint16 iconperline = 458;
	uint16 iconsdown = 384;
	uint16 idone = 0;
	uint16 icount = 0;
	uint16 xp = 188, yp = 306;
	int k;
	_iOverflow = 0;

	line = _variableArray[30];
	if (line == 0)
		_variableArray[31] = 0;

	window = _windowArray[num & 7];
	if (window == NULL)
		return;

	for (k = flagnumber; k <= flagnumber + 18; k++)
		_variableArray[k] = 0;

	if (window->iconPtr)
		removeIconArray(num);

	window->iconPtr=(IconBlock *)malloc(sizeof(IconBlock));
        window->iconPtr->itemRef = itemRef;
	window->iconPtr->upArrow = -1;
	window->iconPtr->downArrow = -1;
	window->iconPtr->line = line;
	window->iconPtr->classMask = classMask;

	itemRef = derefItem(itemRef->child);
	k = flagnumber;

	while (itemRef && (line > 65)) {
		uint16 ct = xp;
		while (itemRef && ct < iconperline) {
			if ((classMask == 0) || ((itemRef->classFlags & classMask) != 0)) {
				if (has_item_childflag_0x10(itemRef)) {
					ct += 45;
					k++;
				}
			}
			itemRef = derefItem(itemRef->sibling);
		}
		line -= 52;
		if (k == (flagnumber + 18))
			k = flagnumber;
	}	
	yp -= line;	// Adjust starting y

	if (itemRef == NULL) {
		window->iconPtr->line = 0;
		itemRef = derefItem(item_ptr_org->child);
	}

	while (itemRef) {
		if ((classMask != 0) && ((itemRef->classFlags & classMask) == 0))
			goto l1;
		if (has_item_childflag_0x10(itemRef) == 0)
			goto l1;
		if (!idone) {
/*
 *	Create thee icon and graphics rendering
 */
			window->iconPtr->iconArray[icount].item = itemRef;
			_variableArray[k] = itemGetIconNumber(itemRef);
			window->iconPtr->iconArray[icount++].boxCode =
				setupIconHitArea(window, k++, xp, yp, itemRef);
		} else {
/*
 *	Just remember the overflow has occured
 */
			window->iconPtr->iconArray[icount].item = NULL;	/* END MARKINGS */
			_iOverflow = 1;
		}
		xp += 45;
		if (xp >= iconperline) {	/* End of line ? */
			if (k == (flagnumber + 18))
				k = flagnumber;
			xp = 188;
			yp += 52;		/* Move down */
			if (yp >= iconsdown) {	/* Full ? */
				idone = 1;	/* Note completed screen */
			}
		}
l1:;		itemRef = derefItem(itemRef->sibling);
	}
	window->iconPtr->iconArray[icount].item = NULL;	/* END MARKINGS */
	if (_variableArray[30] == 0) {
		if (yp != 306)
			_variableArray[31] = 52;
		if ((xp == 188) && (yp == 358))
			_variableArray[31] = 0;
	}
	addArrows(window, num);		/* Plot arrows and add their boxes */
}

void SimonEngine::addArrows(WindowBlock *window, uint num) {
	setArrowHitAreas(window, num);

	window->iconPtr->upArrow = _scrollUpHitArea;
	window->iconPtr->downArrow = _scrollDownHitArea;
}

void SimonEngine::setArrowHitAreas(WindowBlock *window, uint num) {
	HitArea *ha;

	ha = findEmptyHitArea();
	_scrollUpHitArea = ha - _hitAreas;
	if (getGameType() == GType_FF) {
		ha->x = 496;
		ha->y = 279;
		ha->width = 30;
		ha->height = 45;
		ha->flags = 0x24;
		ha->id = 0x7FFB;
		ha->priority = 100;
		ha->window = window;
		ha->verb = 1;
	} else if (getGameType() == GType_SIMON2) {
		ha->x = 81;
		ha->y = 158;
		ha->width = 12;
		ha->height = 26;
		ha->flags = 36;
		ha->id = 0x7FFB;
		ha->priority = 100;
		ha->window = window;
		ha->verb = 1;
	} else {
		ha->x = 308;
		ha->y = 149;
		ha->width = 12;
		ha->height = 17;
		ha->flags = 0x24;
		ha->id = 0x7FFB;
		ha->priority = 100;
		ha->window = window;
		ha->verb = 1;
	}

	ha = findEmptyHitArea();
	_scrollDownHitArea = ha - _hitAreas;

	if (getGameType() == GType_FF) {
		ha->x = 496;
		ha->y = 324;
		ha->width = 30;
		ha->height = 44;
		ha->flags = 0x24;
		ha->id = 0x7FFC;
		ha->priority = 100;
		ha->window = window;
		ha->verb = 1;
	} else if (getGameType() == GType_SIMON2) {
		ha->x = 227;
		ha->y = 162;
		ha->width = 12;
		ha->height = 26;
		ha->flags = 36;
		ha->id = 0x7FFC;
		ha->priority = 100;
		ha->window = window;
		ha->verb = 1;
	} else {
		ha->x = 308;
		ha->y = 176;
		ha->width = 12;
		ha->height = 17;
		ha->flags = 0x24;
		ha->id = 0x7FFC;
		ha->priority = 100;
		ha->window = window;
		ha->verb = 1;

		o_kill_sprite_simon1(128);
		loadSprite(0, 1, 128, 0, 0, 14);
	}
}

uint SimonEngine::setupIconHitArea(WindowBlock *window, uint num, uint x, uint y, Item *item_ptr) {
	HitArea *ha;

	ha = findEmptyHitArea();

	if (getGameType() == GType_FF) {
		ha->x = x;
		ha->y = y;
		ha->item_ptr = item_ptr;
		ha->width = 45;
		ha->height = 44;
		ha->flags = 0xA0;
		ha->id = num;
		ha->priority = 100;
		ha->verb = 208;
	} else if (getGameType() == GType_SIMON2) {
		ha->x = x + 110;
		ha->y = window->y + y;
		ha->item_ptr = item_ptr;
		ha->width = 20;
		ha->height = 20;
		ha->flags = 0xB0;
		ha->id = 0x7FFD;
		ha->priority = 100;
		ha->verb = 208;
	} else {
		ha->x = (x + window->x) * 8;
		ha->y = y * 25 + window->y;
		ha->item_ptr = item_ptr;
		ha->width = 24;
		ha->height = 24;
		ha->flags = 0xB0;
		ha->id = 0x7FFD;
		ha->priority = 100;
		ha->verb = 208;
	}

	return ha - _hitAreas;
}

void SimonEngine::removeIconArray(uint num) {
	WindowBlock *window;
	uint16 curWindow;
	uint16 i;

	window = _windowArray[num & 7];
	curWindow = _curWindow;

	if (window == NULL || window->iconPtr == NULL)
		return;

	if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		changeWindow(num);
		windowPutChar(12);
		changeWindow(curWindow);
	}

	for (i = 0; window->iconPtr->iconArray[i].item != NULL; i++) {
		delete_hitarea_by_index(window->iconPtr->iconArray[i].boxCode);
	}

	if (window->iconPtr->upArrow != -1) {
		delete_hitarea_by_index(window->iconPtr->upArrow);
	}

	if (window->iconPtr->downArrow != -1) {
		delete_hitarea_by_index(window->iconPtr->downArrow);
		if (getGameType() == GType_SIMON1)
			removeArrows(window, num);
	}

	free(window->iconPtr);
	window->iconPtr = NULL;

	_fcsData1[num] = 0;
	_fcsData2[num] = 0;
}

void SimonEngine::removeArrows(WindowBlock *window, uint num) {
	o_kill_sprite_simon1(128);
}

} // End of namespace Simon
