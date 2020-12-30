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

#include "twine/holomap.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "common/types.h"
#include "twine/audio/sound.h"
#include "twine/scene/gamestate.h"
#include "twine/resources/hqr.h"
#include "twine/menu/interface.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/resources.h"
#include "twine/scene/scene.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

Holomap::Holomap(TwinEEngine *engine) : _engine(engine) {}

bool Holomap::loadLocations() {
	uint8 *locationsPtr;
	const int32 locationsSize = HQR::getAllocEntry(&locationsPtr, Resources::HQR_RESS_FILE, RESSHQR_HOLOARROWINFO);
	if (locationsSize == 0) {
		warning("Could not find holomap locations at index %i in %s", RESSHQR_HOLOARROWINFO, Resources::HQR_RESS_FILE);
		return false;
	}

	Common::MemoryReadStream stream(locationsPtr, locationsSize, DisposeAfterUse::YES);
	_numLocations = locationsSize / sizeof(Location);
	if (_numLocations > NUM_LOCATIONS) {
		warning("Amount of locations (%i) exceeds the maximum of %i", _numLocations, NUM_LOCATIONS);
		return false;
	}

	for (int i = 0; i < _numLocations; i++) {
		_locations[i].x = stream.readUint16LE();
		_locations[i].y = stream.readUint16LE();
		_locations[i].z = stream.readUint16LE();
		_locations[i].textIndex = stream.readUint16LE();
	}
	return true;
}

void Holomap::setHolomapPosition(int32 locationIdx) {
	assert(locationIdx >= 0 && locationIdx <= ARRAYSIZE(_engine->_gameState->holomapFlags));
	_engine->_gameState->holomapFlags[locationIdx] = 0x81;
	if (_engine->_gameState->hasItem(InventoryItems::kiHolomap)) {
		_engine->_redraw->addOverlay(OverlayType::koInventoryItem, InventoryItems::kiHolomap, 0, 0, 0, OverlayPosType::koNormal, 3);
	}
}

void Holomap::clearHolomapPosition(int32 locationIdx) {
	assert(locationIdx >= 0 && locationIdx <= ARRAYSIZE(_engine->_gameState->holomapFlags));
	_engine->_gameState->holomapFlags[locationIdx] &= 0x7E;
	_engine->_gameState->holomapFlags[locationIdx] |= 0x40;
}

void Holomap::loadGfxSub1() {
	// TODO
}

void Holomap::loadGfxSub2() {
	// TODO
}

void Holomap::loadHolomapGFX() {
	uint8 *videoPtr3 = (uint8 *)_engine->workVideoBuffer.getBasePtr(174, 12);
	uint8 *videoPtr4 = (uint8 *)_engine->workVideoBuffer.getBasePtr(78, 13);
	uint8 *videoPtr5 = (uint8 *)_engine->workVideoBuffer.getBasePtr(334, 115);

	HQR::getEntry(videoPtr3, Resources::HQR_RESS_FILE, RESSHQR_HOLOSURFACE);
	HQR::getEntry(videoPtr4, Resources::HQR_RESS_FILE, RESSHQR_HOLOIMG);

	uint8 *videoPtr6 = videoPtr5 + HQR::getEntry(videoPtr5, Resources::HQR_RESS_FILE, RESSHQR_HOLOTWINMDL);
	uint8 *videoPtr7 = videoPtr6 + HQR::getEntry(videoPtr6, Resources::HQR_RESS_FILE, RESSHQR_HOLOARROWMDL);
	uint8 *videoPtr8 = videoPtr7 + HQR::getEntry(videoPtr7, Resources::HQR_RESS_FILE, RESSHQR_HOLOTWINARROWMDL);

	Renderer::prepareIsoModel(videoPtr5);
	Renderer::prepareIsoModel(videoPtr6);
	Renderer::prepareIsoModel(videoPtr7);
	Renderer::prepareIsoModel(videoPtr8);

	// TODO:
	// uint8 *videoPtr1 = (uint8 *)_engine->workVideoBuffer.getPixels();
	// uint8 *videoPtr2 = videoPtr1 + 4488;
	// uint8 *videoPtr11 = videoPtr8 + HQR::getEntry(videoPtr8, Resources::HQR_RESS_FILE, RESSHQR_HOLOPOINTMDL);
	// uint8 *videoPtr10 = videoPtr11 + 4488;
	// uint8 *videoPtr12 = videoPtr10 + HQR::getEntry(videoPtr10, Resources::HQR_RESS_FILE, RESSHQR_HOLOARROWINFO);
	// uint8 *videoPtr13 = videoPtr12 + HQR::getEntry(videoPtr12, Resources::HQR_RESS_FILE, RESSHQR_HOLOPOINTANIM);

	_engine->_screens->loadCustomPalette(RESSHQR_HOLOPAL);

	int32 j = 576;
	for (int32 i = 0; i < 96; i += 3, j += 3) {
		paletteHolomap[i + 0] = _engine->_screens->palette[j + 0];
		paletteHolomap[i + 1] = _engine->_screens->palette[j + 1];
		paletteHolomap[i + 2] = _engine->_screens->palette[j + 2];
	}

	j = 576;
	for (int32 i = 96; i < 189; i += 3, j += 3) {
		paletteHolomap[i + 0] = _engine->_screens->palette[j + 0];
		paletteHolomap[i + 1] = _engine->_screens->palette[j + 1];
		paletteHolomap[i + 2] = _engine->_screens->palette[j + 2];
	}

	loadGfxSub1();
	loadGfxSub2();

	needToLoadHolomapGFX = 0;
}

void Holomap::drawHolomapText(int32 centerx, int32 top, const char *title) {
	const int32 size = _engine->_text->getTextSize(title);
	const int32 x = centerx - size / 2;
	const int32 y = top;
	_engine->_text->setFontColor(15);
	_engine->_text->drawText(x, y, title);
	_engine->copyBlockPhys(x, y, x + 400, y + 100);
}

void Holomap::drawHolomapTrajectory(int32 trajectoryIndex) {
	debug("Draw trajectory index %i", trajectoryIndex);
	// TODO
}

void Holomap::drawHolomapLocation() {
	char title[256] = "";
	for (int i = 0; i < NUM_LOCATIONS; ++i) {
		if (!(_engine->_gameState->holomapFlags[i] & 0x81)) {
			continue;
		}
		_engine->_text->getMenuText(200 + _locations[i].textIndex, title, sizeof(title));
		break;
	}
	const int padding = 17;
	Common::Rect rect;
	rect.left = padding - 1;
	rect.top = SCREEN_HEIGHT - 146;
	rect.right = SCREEN_WIDTH - padding;
	rect.bottom = SCREEN_HEIGHT - padding;
	_engine->_menu->drawBox(rect);
	rect.grow(-1);
	_engine->_interface->drawTransparentBox(rect, 3);
	const int32 height = _engine->_text->getCharHeight(title[0]);
	drawHolomapText(rect.left + (rect.right - rect.left) / 2, rect.top + (rect.bottom - rect.top) / 2 - height / 2, title);
}

void Holomap::processHolomap() {
	ScopedEngineFreeze freeze(_engine);

	const int32 alphaLightTmp = _engine->_scene->alphaLight;
	const int32 betaLightTmp = _engine->_scene->betaLight;

	_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBA);
	_engine->_sound->stopSamples();
	_engine->_interface->saveClip();
	_engine->_interface->resetClip();
	_engine->_screens->clearScreen();
	_engine->setPalette(_engine->_screens->paletteRGBA);

	loadHolomapGFX();
	_engine->_renderer->setCameraPosition(SCREEN_WIDTH / 2, 190, 128, 1024, 1024);

	_engine->_text->initTextBank(TextBankId::Inventory_Intro_and_Holomap);
	_engine->_text->setFontCrossColor(9);

	drawHolomapText(SCREEN_WIDTH / 2, 25, "Holomap"); // TODO: fix the index
	drawHolomapLocation();
	_engine->flip();

	ScopedKeyMap holomapKeymap(_engine, holomapKeyMapId);
	for (;;) {
		ScopedFPS scopedFps;
		_engine->_input->readKeys();
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			break;
		}

		if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapLeft)) {
		} else if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapRight)) {
		}

		if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapUp)) {
		} else if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapDown)) {
		}

		// TODO

		//_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
		//_engine->flip();
	}

	_engine->_text->drawTextBoxBackground = true;
	_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBA);
	_engine->_scene->alphaLight = alphaLightTmp;
	_engine->_scene->betaLight = betaLightTmp;

	_engine->_gameState->initEngineProjections();
	_engine->_interface->loadClip();

	_engine->_text->initSceneTextBank();
}

} // namespace TwinE
