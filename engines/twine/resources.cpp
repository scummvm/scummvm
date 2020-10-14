/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "twine/resources.h"
#include "twine/animations.h"
#include "twine/scene.h"
#include "twine/screens.h"
#include "twine/sound.h"
#include "twine/text.h"

namespace TwinE {

void Resources::initPalettes() {
	// Init standard palette
	_engine->_hqrdepack->hqrGetallocEntry(&_engine->_screens->mainPalette, Resources::HQR_RESS_FILE, RESSHQR_MAINPAL);
	_engine->_screens->copyPal(_engine->_screens->mainPalette, _engine->_screens->mainPaletteRGB);

	memcpy(_engine->_screens->palette, _engine->_screens->mainPalette, NUMOFCOLORS * 3);

	_engine->_screens->copyPal(_engine->_screens->palette, _engine->_screens->paletteRGB);
	_engine->setPalette(_engine->_screens->paletteRGB);

	// We use it now
	_engine->_screens->palCustom = 0;
}

void Resources::preloadSprites() {
	int32 i;
	int32 numEntries = _engine->_hqrdepack->hqrNumEntries(Resources::HQR_SPRITES_FILE) - 1;

	for (i = 0; i < numEntries; i++) {
		_engine->_actor->spriteSizeTable[i] = _engine->_hqrdepack->hqrGetallocEntry(&_engine->_actor->spriteTable[i], Resources::HQR_SPRITES_FILE, i);
	}
}

void Resources::preloadAnimations() {
	int32 i;
	int32 numEntries = _engine->_hqrdepack->hqrNumEntries(Resources::HQR_ANIM_FILE) - 1;

	for (i = 0; i < numEntries; i++) {
		_engine->_animations->animSizeTable[i] = _engine->_hqrdepack->hqrGetallocEntry(&_engine->_animations->animTable[i], Resources::HQR_ANIM_FILE, i);
	}
}

void Resources::preloadSamples() {
	int32 i;
	int32 numEntries = _engine->_hqrdepack->hqrNumEntries(Resources::HQR_SAMPLES_FILE) - 1;

	for (i = 0; i < numEntries; i++) {
		_engine->_sound->samplesSizeTable[i] = _engine->_hqrdepack->hqrGetallocEntry(&_engine->_sound->samplesTable[i], Resources::HQR_SAMPLES_FILE, i);
	}
}

void Resources::preloadInventoryItems() {
	int32 i;
	int32 numEntries = _engine->_hqrdepack->hqrNumEntries(Resources::HQR_INVOBJ_FILE) - 1;

	for (i = 0; i < numEntries; i++) {
		inventorySizeTable[i] = _engine->_hqrdepack->hqrGetallocEntry(&inventoryTable[i], Resources::HQR_INVOBJ_FILE, i);
	}
}

void Resources::initResources() {
	// Menu and in-game palette
	initPalettes();

	// load LBA font
	_engine->_hqrdepack->hqrGetallocEntry(&_engine->_text->fontPtr, Resources::HQR_RESS_FILE, RESSHQR_LBAFONT);

	_engine->_text->setFontParameters(2, 8);
	_engine->_text->setFontColor(14);
	_engine->_text->setTextCrossColor(136, 143, 2);

	_engine->_hqrdepack->hqrGetallocEntry(&_engine->_scene->spriteShadowPtr, Resources::HQR_RESS_FILE, RESSHQR_SPRITESHADOW);

	// load sprite actors bounding box data
	_engine->_hqrdepack->hqrGetallocEntry(&_engine->_scene->spriteBoundingBoxPtr, Resources::HQR_RESS_FILE, RESSHQR_SPRITEBOXDATA);

	preloadSprites();
	preloadAnimations();
	//preloadSamples();
	preloadInventoryItems();
}

} // namespace TwinE
