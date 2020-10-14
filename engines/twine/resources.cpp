/** @file resources.cpp
	@brief
	This file contains the definitions of most used game resources.

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "resources.h"
#include "text.h"
#include "scene.h"
#include "animations.h"
#include "screens.h"
#include "sdlengine.h"
#include "sound.h"

int8 * HQR_RESS_FILE			= "ress.hqr";
int8 * HQR_TEXT_FILE			= "text.hqr";
int8 * HQR_FLASAMP_FILE			= "flasamp.hqr";
int8 * HQR_MIDI_MI_DOS_FILE		= "midi_mi.hqr";
int8 * HQR_MIDI_MI_WIN_FILE		= "midi_mi_win.hqr";
int8 * HQR_MIDI_MI_WIN_MP3_FILE = "midi_mi_win_mp3.hqr";
int8 * HQR_MIDI_MI_WIN_OGG_FILE = "midi_mi_win_ogg.hqr";
int8 * HQR_SAMPLES_FILE			= "samples.hqr";
int8 * HQR_LBA_GRI_FILE			= "lba_gri.hqr";
int8 * HQR_LBA_BLL_FILE			= "lba_bll.hqr";
int8 * HQR_LBA_BRK_FILE			= "lba_brk.hqr";
int8 * HQR_SCENE_FILE			= "scene.hqr";
int8 * HQR_SPRITES_FILE			= "sprites.hqr";
int8 * HQR_FILE3D_FILE			= "file3d.hqr";
int8 * HQR_BODY_FILE			= "body.hqr";
int8 * HQR_ANIM_FILE			= "anim.hqr";
int8 * HQR_INVOBJ_FILE			= "invobj.hqr";

/** Init palettes */
void initPalettes() {
	// Init standard palette
	hqrGetallocEntry(&mainPalette, HQR_RESS_FILE, RESSHQR_MAINPAL);
	convertPalToRGBA(mainPalette, mainPaletteRGBA);

	memcpy(palette, mainPalette, NUMOFCOLORS * 3);

	convertPalToRGBA(palette, paletteRGBA);
	setPalette(paletteRGBA);

	// We use it now
	palCustom = 0;
}

/** Preload all sprites */
void preloadSprites() {
	int32 i;
	int32 numEntries = hqrNumEntries(HQR_SPRITES_FILE) - 1;

	for (i = 0; i < numEntries; i++) {
		spriteSizeTable[i] = hqrGetallocEntry(&spriteTable[i], HQR_SPRITES_FILE, i);
	}
}

/** Preload all animations */
void preloadAnimations() {
	int32 i;
	int32 numEntries = hqrNumEntries(HQR_ANIM_FILE) - 1;

	for (i = 0; i < numEntries; i++) {
		animSizeTable[i] = hqrGetallocEntry(&animTable[i], HQR_ANIM_FILE, i);
	}
}

/** Preload all animations */
void preloadSamples() {
	int32 i;
	int32 numEntries = hqrNumEntries(HQR_SAMPLES_FILE) - 1;

	for (i = 0; i < numEntries; i++) {
		samplesSizeTable[i] = hqrGetallocEntry(&samplesTable[i], HQR_SAMPLES_FILE, i);
	}
}

/** Preload all animations */
void preloadInventoryItems() {
	int32 i;
	int32 numEntries = hqrNumEntries(HQR_INVOBJ_FILE) - 1;

	for (i = 0; i < numEntries; i++) {
		inventorySizeTable[i] = hqrGetallocEntry(&inventoryTable[i], HQR_INVOBJ_FILE, i);
	}
}

/** Initialize resource pointers */
void initResources() {
	// Menu and in-game palette
	initPalettes();

	// load LBA font
	hqrGetallocEntry(&fontPtr, HQR_RESS_FILE, RESSHQR_LBAFONT);

	setFontParameters(2, 8);
	setFontColor(14);
	setTextCrossColor(136, 143, 2);

	hqrGetallocEntry(&spriteShadowPtr, HQR_RESS_FILE, RESSHQR_SPRITESHADOW);

	// load sprite actors bounding box data
	hqrGetallocEntry(&spriteBoundingBoxPtr, HQR_RESS_FILE, RESSHQR_SPRITEBOXDATA);

	preloadSprites();
	preloadAnimations();
	//preloadSamples();
	preloadInventoryItems();
}
