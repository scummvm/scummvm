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

#include "twine/resources/resources.h"
#include "common/util.h"
#include "twine/audio/sound.h"
#include "twine/renderer/screens.h"
#include "twine/scene/animations.h"
#include "twine/scene/scene.h"
#include "twine/text.h"

namespace TwinE {

Resources::~Resources() {
	for (size_t i = 0; i < ARRAYSIZE(inventoryTable); ++i) {
		free(inventoryTable[i]);
	}
	for (size_t i = 0; i < ARRAYSIZE(spriteTable); ++i) {
		free(spriteTable[i]);
	}
	for (size_t i = 0; i < ARRAYSIZE(animTable); ++i) {
		free(animTable[i]);
	}
	for (size_t i = 0; i < ARRAYSIZE(samplesTable); ++i) {
		free(samplesTable[i]);
	}
	free(fontPtr);
	free(spriteShadowPtr);
	free(holomapSurfacePtr);
	free(holomapImagePtr);
	free(holomapTwinsenModelPtr);
	free(holomapTwinsenArrowPtr);
	free(holomapArrowPtr);
	free(_engine->_screens->mainPalette);
}

void Resources::initPalettes() {
	const int32 size = HQR::getAllocEntry(&_engine->_screens->mainPalette, Resources::HQR_RESS_FILE, RESSHQR_MAINPAL);
	if (size == 0) {
		error("Failed to load main palette");
	}
	_engine->_screens->convertPalToRGBA(_engine->_screens->mainPalette, _engine->_screens->mainPaletteRGBA);

	memcpy(_engine->_screens->palette, _engine->_screens->mainPalette, NUMOFCOLORS * 3);

	_engine->_screens->convertPalToRGBA(_engine->_screens->palette, _engine->_screens->paletteRGBA);
	_engine->setPalette(_engine->_screens->paletteRGBA);
}

void Resources::preloadSprites() {
	const int32 numEntries = HQR::numEntries(Resources::HQR_SPRITES_FILE);
	if (numEntries > NUM_SPRITES) {
		error("Max allowed sprites exceeded: %i/%i", numEntries, NUM_SPRITES);
	}
	debug("preload %i sprites", numEntries);
	for (int32 i = 0; i < numEntries; i++) {
		spriteSizeTable[i] = HQR::getAllocEntry(&spriteTable[i], Resources::HQR_SPRITES_FILE, i);
		if (!spriteData[i].loadFromBuffer(spriteTable[i], spriteSizeTable[i])) {
			error("Failed to load sprite %i", i);
		}
	}
}

void Resources::preloadAnimations() {
	const int32 numEntries = HQR::numEntries(Resources::HQR_ANIM_FILE);
	if (numEntries > NUM_ANIMS) {
		error("Max allowed animations exceeded: %i/%i", numEntries, NUM_ANIMS);
	}
	debug("preload %i animations", numEntries);
	for (int32 i = 0; i < numEntries; i++) {
		animSizeTable[i] = HQR::getAllocEntry(&animTable[i], Resources::HQR_ANIM_FILE, i);
	}
}

void Resources::preloadSamples() {
	const int32 numEntries = HQR::numEntries(Resources::HQR_SAMPLES_FILE);
	if (numEntries > NUM_SAMPLES) {
		error("Max allowed samples exceeded: %i/%i", numEntries, NUM_SAMPLES);
	}
	debug("preload %i samples", numEntries);
	for (int32 i = 0; i < numEntries; i++) {
		samplesSizeTable[i] = HQR::getAllocEntry(&samplesTable[i], Resources::HQR_SAMPLES_FILE, i);
		if (samplesSizeTable[i] == 0) {
			warning("Failed to load sample %i", i);
			continue;
		}
		// Fix incorrect sample files first byte
		if (*(samplesTable[i]) != 'C') {
			debug(0, "Sample %i has incorrect magic id", i);
			*(samplesTable[i]) = 'C';
		}
	}
}

void Resources::preloadInventoryItems() {
	const int32 numEntries = HQR::numEntries(Resources::HQR_INVOBJ_FILE);
	if (numEntries > NUM_INVENTORY_ITEMS) {
		error("Max allowed inventory items exceeded: %i/%i", numEntries, NUM_INVENTORY_ITEMS);
	}
	debug("preload %i inventory items", numEntries);
	for (int32 i = 0; i < numEntries; i++) {
		inventorySizeTable[i] = HQR::getAllocEntry(&inventoryTable[i], Resources::HQR_INVOBJ_FILE, i);
	}
}

void Resources::initResources() {
	initPalettes();

	fontBufSize = HQR::getAllocEntry(&fontPtr, Resources::HQR_RESS_FILE, RESSHQR_LBAFONT);
	if (fontBufSize == 0) {
		error("Failed to load font");
	}

	_engine->_text->setFontParameters(2, 8);
	_engine->_text->setFontColor(14);
	_engine->_text->setTextCrossColor(136, 143, 2);

	spriteShadowSize = HQR::getAllocEntry(&spriteShadowPtr, Resources::HQR_RESS_FILE, RESSHQR_SPRITESHADOW);
	if (spriteShadowSize == 0) {
		error("Failed to load sprite shadow");
	}

	if (!spriteBoundingBox.loadFromHQR(Resources::HQR_RESS_FILE, RESSHQR_SPRITEBOXDATA)) {
		error("Failed to load sprite bounding box data");
	}

	holomapSurfaceSize = HQR::getAllocEntry(&holomapSurfacePtr, Resources::HQR_RESS_FILE, RESSHQR_HOLOSURFACE);
	if (holomapSurfaceSize == 0) {
		error("Failed to load holomap surface");
	}

	holomapImageSize = HQR::getAllocEntry(&holomapImagePtr, Resources::HQR_RESS_FILE, RESSHQR_HOLOIMG);
	if (holomapImageSize == 0) {
		error("Failed to load holomap image");
	}

	holomapTwinsenModelSize = HQR::getAllocEntry(&holomapTwinsenModelPtr, Resources::HQR_RESS_FILE, RESSHQR_HOLOTWINMDL);
	if (holomapTwinsenModelSize == 0) {
		error("Failed to load holomap twinsen model");
	}

	holomapArrowSize = HQR::getAllocEntry(&holomapArrowPtr, Resources::HQR_RESS_FILE, RESSHQR_HOLOARROWMDL);
	if (holomapArrowSize == 0) {
		error("Failed to load holomap arrow model");
	}

	holomapTwinsenArrowSize = HQR::getAllocEntry(&holomapTwinsenArrowPtr, Resources::HQR_RESS_FILE, RESSHQR_HOLOTWINARROWMDL);
	if (holomapTwinsenArrowSize == 0) {
		error("Failed to load holomap twinsen arrow model");
	}

	preloadSprites();
	preloadAnimations();
	preloadSamples();
	preloadInventoryItems();
}

} // namespace TwinE
