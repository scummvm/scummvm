/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include <string.h>
#include "common/scummsys.h"
#include "sky/skydefs.h"
#include "sky/sky.h"

#define FADE_JUMP		2
#define SCROLL_JUMP		16

#define VGA_COLOURS		256
#define GAME_COLOURS		240

uint8 top16Colours[] =
{
	0, 0, 0,
	38, 38, 38,
	63, 63, 63,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	54, 54, 54,
	45, 47, 49,
	32, 31, 41,
	29, 23, 37,
	23, 18, 30,
	49, 11, 11,
	39, 5, 5,
	29, 1, 1,
	63, 63, 63
};

void SkyState::initialiseScreen(void) {
	
	int i;
	uint8 tmpPal[1024];

	_system->init_size(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
	_backScreen = (uint8 *)malloc(FULL_SCREEN_WIDTH * FULL_SCREEN_HEIGHT);
	_gameGrid = (uint8 *)malloc(GRID_X * GRID_Y * 2);
	_workPalette = (uint8 *)malloc(VGA_COLOURS * 3);

	//blank the first 240 colors of the palette 
	memset(tmpPal, 0, GAME_COLOURS * 4);

	//set the remaining colors
	for (i = 0; i < (VGA_COLOURS-GAME_COLOURS); i++) {
		tmpPal[GAME_COLOURS + i * 4] = (top16Colours[i * 3] << 2) + (top16Colours[i * 3] & 3);
		tmpPal[GAME_COLOURS + i * 4 + 1] = (top16Colours[i * 3 + 1] << 2) + (top16Colours[i * 3 + 1] & 3);
		tmpPal[GAME_COLOURS + i * 4 + 2] = (top16Colours[i * 3 + 2] << 2) + (top16Colours[i * 3 + 2] & 3);
		tmpPal[GAME_COLOURS + i * 4 + 3] = 0x00; 
	}

	//set the palette
	_system->set_palette(tmpPal, 0, 256);
}

//set a new palette, pal is a pointer to dos vga rgb components 0..63
void SkyState::setPalette(uint8 *pal) {
	
	convertPalette(pal, _palette);
	_system->set_palette(_palette, 0, 256);
}

void SkyState::convertPalette(uint8 *inPal, uint8* outPal) { //convert 3 byte 0..63 rgb to 4byte 0..255 rgbx
	
	int i;

	for (i = 0; i < VGA_COLOURS; i++) {
		outPal[4 * i] = (inPal[3 * i] << 2) + (inPal[3 * i] & 3);
		outPal[4 * i + 1] = (inPal[3 * i + 1] << 2) + (inPal[3 * i + 1] & 3);
		outPal[4 * i + 2] = (inPal[3 * i + 2] << 2) + (inPal[3 * i + 2] & 3);
		outPal[4 * i + 3] = 0x00;
	}
}
