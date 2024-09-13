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

#include "twine/resources/resources.h"
#include "common/file.h"
#include "common/tokenizer.h"
#include "common/util.h"
#include "twine/audio/sound.h"
#include "twine/parser/anim3ds.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/hqr.h"
#include "twine/scene/animations.h"
#include "twine/scene/scene.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

Resources::~Resources() {
	for (size_t i = 0; i < ARRAYSIZE(_spriteTable); ++i) {
		free(_spriteTable[i]);
	}
	for (size_t i = 0; i < ARRAYSIZE(_samplesTable); ++i) {
		free(_samplesTable[i]);
	}
	free(_fontPtr);
	free(_sjisFontPtr);
}

void Resources::initPalettes() {
	uint8 *mainPalette = nullptr;
	const int32 size = HQR::getAllocEntry(&mainPalette, Resources::HQR_RESS_FILE, RESSHQR_MAINPAL);
	if (size == 0) {
		error("Failed to load main palette");
	}
	_engine->_screens->convertPalToRGBA(mainPalette, _engine->_screens->_mainPaletteRGBA);

	memcpy(_engine->_screens->_palette, mainPalette, NUMOFCOLORS * 3);

	_engine->_screens->convertPalToRGBA(_engine->_screens->_palette, _engine->_screens->_paletteRGBA);
	_engine->setPalette(_engine->_screens->_paletteRGBA);
	free(mainPalette);
}

void Resources::preloadAnim3DS() {
	const int index = HQR::numEntries(Resources::HQR_ANIM3DS_FILE) - 1;
	_anim3DSData.loadFromHQR(Resources::HQR_ANIM3DS_FILE, index, _engine->isLBA1());
}

void Resources::loadEntityData(EntityData &entityData, int32 &index) {
	if (_engine->isLBA1()) {
		TwineResource modelRes(Resources::HQR_FILE3D_FILE, index);
		if (!entityData.loadFromHQR(modelRes, _engine->isLBA1())) {
			error("Failed to load actor 3d data for index: %i", index);
		}
	} else {
		// TODO: don't allocate each time
		TwineResource modelRes(Resources::HQR_RESS_FILE, 44);
		uint8 *file3dBuf = nullptr;
		const int32 holomapImageSize = HQR::getAllocEntry(&file3dBuf, modelRes);
		if (!entityData.loadFromBuffer((uint8 *)(file3dBuf + *(((uint32 *)file3dBuf) + (index))), holomapImageSize, _engine->isLBA1())) {
			delete file3dBuf;
			error("Failed to load actor 3d data for index: %i", index);
		}
		delete file3dBuf;
	}
}

const T_ANIM_3DS *Resources::getAnim(int index) const {
	if (index < 0 || index >= (int)_anim3DSData.getAnims().size()) {
		return nullptr;
	}
	return &_anim3DSData.getAnims()[index];
}

void Resources::preloadSprites() {
	const int32 numEntries = HQR::numEntries(Resources::HQR_SPRITES_FILE);
	const int32 maxSprites = _engine->isLBA1() ? 200 : NUM_SPRITES;
	if (numEntries > maxSprites) {
		error("Max allowed sprites exceeded: %i/%i", numEntries, maxSprites);
	}
	debug("preload %i sprites", numEntries);
	for (int32 i = 0; i < numEntries; i++) {
		_spriteSizeTable[i] = HQR::getAllocEntry(&_spriteTable[i], Resources::HQR_SPRITES_FILE, i);
		if (!_spriteData[i].loadFromBuffer(_spriteTable[i], _spriteSizeTable[i], _engine->isLBA1())) {
			warning("Failed to load sprite %i", i);
		}
	}
}

void Resources::preloadAnimations() {
	const int32 numEntries = HQR::numEntries(Resources::HQR_ANIM_FILE);
	const int32 maxAnims = _engine->isLBA1() ? 600 : NUM_ANIMS;
	if (numEntries > maxAnims) {
		error("Max allowed animations exceeded: %i/%i", numEntries, maxAnims);
	}
	debug("preload %i animations", numEntries);
	for (int32 i = 0; i < numEntries; i++) {
		_animData[i].loadFromHQR(Resources::HQR_ANIM_FILE, i, _engine->isLBA1());
	}
}

static bool isLba1BlankSampleEntry(int32 index) {
	// these indices contain blank hqr entries
	const int32 blankIndices[] = {80, 81, 82, 83, 115, 118, 120, 124, 125, 139, 140, 154, 155};
	for (int j = 0; j < ARRAYSIZE(blankIndices); ++j) {
		if (index == blankIndices[j]) {
			return true;
		}
	}
	return false;
}

void Resources::preloadSamples() {
	const int32 numEntries = HQR::numEntries(Resources::HQR_SAMPLES_FILE);
	const int32 maxSamples = _engine->isLBA1() ? 243 : NUM_SAMPLES;
	if (numEntries > maxSamples) {
		error("Max allowed samples exceeded: %i/%i", numEntries, maxSamples);
	}
	debug("preload %i samples", numEntries);
	for (int32 i = 0; i < numEntries; i++) {
		if (_engine->isLBA1() && isLba1BlankSampleEntry(i)) {
			_samplesSizeTable[i] = 0;
			_samplesTable[i] = nullptr;
			continue;
		}
		_samplesSizeTable[i] = HQR::getAllocEntry(&_samplesTable[i], Resources::HQR_SAMPLES_FILE, i);
		if (_samplesSizeTable[i] == 0) {
			warning("Failed to load sample %i", i);
			continue;
		}
		// Fix incorrect sample files first byte
		if (*_samplesTable[i] != 'C') {
			debug("Sample %i has incorrect magic id (size: %u)", i, _samplesSizeTable[i]);
			*_samplesTable[i] = 'C';
		}
	}
}

void Resources::preloadInventoryItems() {
	if (!_engine->isLBA1()) {
		// lba2 has this data in code
		return;
	}
	const int32 numEntries = HQR::numEntries(Resources::HQR_INVOBJ_FILE);
	if (numEntries > NUM_INVENTORY_ITEMS) {
		error("Max allowed inventory items exceeded: %i/%i", numEntries, NUM_INVENTORY_ITEMS);
	}
	debug("preload %i inventory items", numEntries);
	for (int32 i = 0; i < numEntries; i++) {
		_inventoryTable[i].loadFromHQR(Resources::HQR_INVOBJ_FILE, i, _engine->isLBA1());
	}
}

void Resources::initResources() {
	initPalettes();

	_fontBufSize = HQR::getAllocEntry(&_fontPtr, Resources::HQR_RESS_FILE, RESSHQR_LBAFONT);
	if (_fontBufSize == 0) {
		error("Failed to load font");
	}

	const int kMinSjisSize = 11072 * 24 * 3;
	Common::File f24;
	if (f24.open("FNT24.DAT") && f24.size() >= kMinSjisSize) {
		// Rest is garbage
		_sjisFontPtr = (byte *)malloc(kMinSjisSize);
		assert(_sjisFontPtr);
		f24.read(_sjisFontPtr, kMinSjisSize);
	}

	_engine->_text->setFontParameters(2, 8);
	_engine->_text->setFontColor(COLOR_14);
	_engine->_text->setTextCrossColor(136, 143, 2);

	if (_engine->isLBA1()) {
		if (!_spriteShadowPtr.loadFromHQR(TwineResource(Resources::HQR_RESS_FILE, RESSHQR_SPRITESHADOW), _engine->isLBA1())) {
			error("Failed to load shadow sprites");
		}

		if (!_spriteBoundingBox.loadFromHQR(TwineResource(Resources::HQR_RESS_FILE, RESSHQR_SPRITEBOXDATA), _engine->isLBA1())) {
			error("Failed to load sprite bounding box data");
		}

		if (!_holomapTwinsenModelPtr.loadFromHQR(TwineResource(Resources::HQR_RESS_FILE, RESSHQR_HOLOTWINMDL), _engine->isLBA1())) {
			error("Failed to load holomap twinsen model");
		}

		if (!_holomapPointModelPtr.loadFromHQR(TwineResource(Resources::HQR_RESS_FILE, RESSHQR_HOLOPOINTMDL), _engine->isLBA1())) {
			error("Failed to load holomap point model");
		}

		if (!_holomapArrowPtr.loadFromHQR(TwineResource(Resources::HQR_RESS_FILE, RESSHQR_HOLOARROWMDL), _engine->isLBA1())) {
			error("Failed to load holomap arrow model");
		}

		if (!_holomapTwinsenArrowPtr.loadFromHQR(TwineResource(Resources::HQR_RESS_FILE, RESSHQR_HOLOTWINARROWMDL), _engine->isLBA1())) {
			error("Failed to load holomap twinsen arrow model");
		}

		if (!_trajectories.loadFromHQR(TwineResource(Resources::HQR_RESS_FILE, RESSHQR_HOLOPOINTANIM), _engine->isLBA1())) {
			error("Failed to parse trajectory data");
		}
		debug("preload %i trajectories", (int)_trajectories.getTrajectories().size());
	} else if (_engine->isLBA2()) {
		preloadAnim3DS();
	}

	preloadSprites();
	preloadAnimations();
	preloadSamples();
	preloadInventoryItems();

	const int32 bodyCount = HQR::numEntries(Resources::HQR_BODY_FILE);
	const int32 maxBodies = _engine->isLBA1() ? 200 : NUM_BODIES;
	if (bodyCount > maxBodies) {
		error("Max body count exceeded: %i", bodyCount);
	}
	for (int32 i = 0; i < bodyCount; ++i) {
		if (!_bodyData[i].loadFromHQR(TwineResource(Resources::HQR_BODY_FILE, i), _engine->isLBA1())) {
			error("HQR ERROR: Parsing body entity for model %i failed", i);
		}
	}

	loadMovieInfo();

	const int32 textEntryCount = _engine->isLBA1() ? 28 : 30;
	for (int32 i = 0; i < textEntryCount / 2; ++i) {
		if (!_textData.loadFromHQR(Resources::HQR_TEXT_FILE, (TextBankId)i, _engine->_cfgfile._languageId, _engine->isLBA1(), textEntryCount)) {
			error("HQR ERROR: Parsing textbank %i failed", i);
		}
	}
	debug("Loaded %i text banks", textEntryCount / 2);
}

const TextEntry *Resources::getText(TextBankId textBankId, TextId index) const {
	return _textData.getText(textBankId, index);
}

const Trajectory *Resources::getTrajectory(int index) const {
	return _trajectories.getTrajectory(index);
}

int Resources::findSmkMovieIndex(const char *name) const {
	Common::String smkName = name;
	smkName.toLowercase();
	const Common::Array<int32> &info = getMovieInfo(smkName);
	return info[0];
}

void Resources::loadMovieInfo() {
	uint8 *content = nullptr;
	int32 size;
	if (_engine->isLBA1()) {
		size = HQR::getAllocEntry(&content, Resources::HQR_RESS_FILE, RESSHQR_FLAINFO);
	} else {
		size = HQR::getAllocEntry(&content, Resources::HQR_RESS_FILE, 48);
	}
	if (size == 0) {
		return;
	}
	const Common::String str((const char *)content, size);
	free(content);
	debug(3, "movie info:\n%s", str.c_str());
	Common::StringTokenizer tok(str, "\r\n");
	int videoIndex = 0;
	while (!tok.empty()) {
		Common::String line = tok.nextToken();
		if (_engine->isLBA1()) {
			Common::StringTokenizer lineTok(line);
			if (lineTok.empty()) {
				continue;
			}
			const Common::String &name = lineTok.nextToken();
			Common::Array<int32> frames;
			while (!lineTok.empty()) {
				const Common::String &frame = lineTok.nextToken();
				const int32 frameIdx = atoi(frame.c_str());
				frames.push_back(frameIdx);
			}
			_movieInfo.setVal(name, frames);
		} else {
			Common::Array<int32> info(1);
			info[0] = videoIndex;
			line.toLowercase();
			if (line.hasSuffix(".smk")) {
				line = line.substr(0, line.size() - 4);
			}
			_movieInfo.setVal(line, info);
			debug(4, "movie name %s mapped to hqr index %i", line.c_str(), videoIndex);
			++videoIndex;
		}
	}
}

const Common::Array<int32> &Resources::getMovieInfo(const Common::String &name) const {
	return _movieInfo.getVal(name);
}

} // namespace TwinE
