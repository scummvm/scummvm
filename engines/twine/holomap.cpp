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
#include "twine/menu/interface.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/scene/animations.h"
#include "twine/scene/movements.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/scene/collision.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/scene.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

Holomap::Holomap(TwinEEngine *engine) : _engine(engine) {}

bool Holomap::loadLocations() {
	uint8 *locationsPtr = nullptr;
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
	_engine->_text->setFontColor(COLOR_WHITE);
	// TODO: handle @ newline
	// TODO: faded in? - looks like it - printText10 was used
	_engine->_text->drawText(x, y, title);
}

void Holomap::drawHolomapTrajectory(int32 trajectoryIndex) {
	debug("Draw trajectory index %i", trajectoryIndex);
#if 0
	ScopedEngineFreeze timeFreeze(_engine);
	_engine->_renderer->setCameraPosition(400, 240, 128, 1024, 1024);
	_engine->_renderer->setCameraAngle(0, 0, 0, (int)(short)DAT_0043f706, (int)(short)DAT_0043f710, (int)DAT_0043f7f8, 0x14b4);
	ActorMoveStruct move;
	const uint8 *animPtr = nullptr;
	int frameNumber = 0;
	int frameTime = _engine->lbaTime;
	for (;;) {
		ScopedFPS scopedFps;
		_engine->readKeys();
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			break;
		}

		// TODO
		if (!v28) {
			setPalette2(192, 32, (int)&palette2[3 * needToLoadHolomapGFX++]);
			if (needToLoadHolomapGFX == 32) {
				needToLoadHolomapGFX = 0;
			}
		}
		const int16 newAngle = move.getRealAngle(_engine->lbaTime);
		if (move.numOfStep == 0) {
			const int startAngle = ANGLE_0;
			_engine->_movements->setActorAngleSafe(startAngle, startAngle - ANGLE_90, 500, &move);
		}

		if (_engine->_animations->setModelAnimation(frameNumber, animPtr, modelPtr, &move)) {
			frameNumber++;
			if (frameNumber >= _engine->_animations->getNumKeyframes(animPtr)) {
				frameNumber = _engine->_animations->getStartKeyframe(animPtr);
			}
		}
		_engine->_renderer->setCameraPosition(100, 400, 128, 900, 900);
		_engine->_renderer->setCameraAngle(0, 0, 0, angle1, angle2, angle3, 5300);
		_engine->_renderer->setLightVector(angle1, angle2, 0);
		_engine->_interface->blitBox(v5);
		_engine->_renderer->renderIsoModel(v19, v5);
		_engine->copyBlockPhys(v5);
		_engine->_renderer->setCameraPosition(400, 240, 128, 1024, 1024);
		_engine->_renderer->setCameraAngle(v5);
		_engine->_renderer->setLightVector(v5);
		if (frameTime + 40 <= _engine->lbaTime) {
			frameTime = _engine->lbaTime;
			if (v17 >= v29 && v17 > v29) {
				break;
			}
			++v17;
			_engine->_renderer->setBaseRotation(v0);
			_engine->_renderer->getBaseRotationPosition(v1, v0);
			_engine->_renderer->setCameraAngle(v0);
			_engine->_renderer->getBaseRotationPosition(v2, v0);
			_engine->_interface->resetClip();
			_engine->_renderer->renderIsoModel(v3, v0);
			_engine->copyBlockPhys(v0);
		}
		if (v28) {
			v28 = 0;
			_engine->_screens->fadeToPal((int)palette);
		}
	}
	_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBA);
	// TODO: flip()?
#endif
}

void Holomap::drawHolomapLocation(int32 location) {
	char title[256] = "";
	_engine->_text->getMenuText(_locations[location].textIndex, title, sizeof(title));
	const int padding = 17;
	Common::Rect rect;
	rect.left = padding - 1;
	rect.top = _engine->height() - 146;
	rect.right = _engine->width() - padding;
	rect.bottom = _engine->height() - padding;
	_engine->_interface->blitBox(rect, _engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->_menu->drawBox(rect);
	rect.grow(-1);
	_engine->_interface->drawTransparentBox(rect, 3);
	_engine->_interface->blitBox(rect, _engine->frontVideoBuffer, _engine->workVideoBuffer);
	const int32 height = _engine->_text->getCharHeight(title[0]);
	drawHolomapText(rect.left + (rect.right - rect.left) / 2, rect.top + (rect.bottom - rect.top) / 2 - height / 2, title);
	rect.grow(1);
	_engine->copyBlockPhys(rect);
}

int32 Holomap::getNextHolomapLocation(int32 currentLocation, int32 dir) const {
	const uint32 idx = currentLocation;
	for (uint32 i = currentLocation + dir; i != idx; i = (i + dir) % NUM_LOCATIONS) {
		if (_engine->_gameState->holomapFlags[i % NUM_LOCATIONS] & 0x81) {
			return i;
		}
	}
	return -1;
}

#if 0
static int sortHolomapSurfaceCoordsByX(int a1, int a2) {
	return *(_WORD *)a1 - *(_WORD *)a2;
}

int fullRedrawS2S1S3(int a1, int a2)
{
	int result = 24 * (a1 - a2) >> 9;
	return orthoProjX + result;
}

int sub_2654F(int a1, int a2)
{
  return getBaseRotationPosition(*(_DWORD *)(a2 + 8), a1, *(_DWORD *)(a2 + 12), *(_DWORD *)(a2 + 16));
}

#endif

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
	_engine->_renderer->setCameraPosition(_engine->width() / 2, 190, 128, 1024, 1024);

	_engine->_text->initTextBank(TextBankId::Inventory_Intro_and_Holomap);
	_engine->_text->setFontCrossColor(9);

	drawHolomapText(_engine->width() / 2, 25, "HoloMap"); // TODO: fix the index
	int currentLocation = _engine->_scene->currentSceneIdx;
	drawHolomapLocation(currentLocation);
	_engine->flip();

	// TODO: load RESSHQR_HOLOSURFACE and project the texture to the surface
	//_engine->_screens->loadImage(RESSHQR_HOLOIMG, RESSHQR_HOLOPAL);

	const int32 time = _engine->lbaTime;
	int32 xRot = 0;
	int32 yRot = 0;
	bool rotate = false;
	bool redraw = true;
	ScopedKeyMap holomapKeymap(_engine, holomapKeyMapId);
	for (;;) {
		FrameMarker frame;
		ScopedFPS scopedFps;
		_engine->_input->readKeys();
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			break;
		}

		if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapPrev)) {
			const int32 nextLocation = getNextHolomapLocation(currentLocation, -1);
			if (nextLocation != -1) {
				currentLocation = nextLocation;
				drawHolomapLocation(currentLocation);
			}
		} else if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapNext)) {
			const int32 nextLocation = getNextHolomapLocation(currentLocation, 1);
			if (nextLocation != -1) {
				currentLocation = nextLocation;
				drawHolomapLocation(currentLocation);
			}
		}

		if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapLeft)) {
			xRot += 8;
			rotate = true;
		} else if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapRight)) {
			xRot -= 8;
			rotate = true;
		}

		if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapUp)) {
			yRot += 8;
			rotate = true;
		} else if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapDown)) {
			yRot -= 8;
			rotate = true;
		}
		if (rotate) {
			xRot = _engine->_collision->getAverageValue(xRot, _locations[currentLocation].x, 75, _engine->lbaTime - time);
			yRot = _engine->_collision->getAverageValue(yRot, _locations[currentLocation].y, 75, _engine->lbaTime - time);
			redraw = true;
		}

		if (rotate && xRot == _locations[currentLocation].x && yRot == _locations[currentLocation].y) {
			rotate = false;
		}

		if (redraw) {
			redraw = false;
#if 0
			_engine->_interface->blitBox(rect, _engine->workVideoBuffer, _engine->frontVideoBuffer);
			_engine->_renderer->setCameraAngle();
			_engine->_renderer->setLightVector(v0);
#endif
			int n = 0;
			for (int locationIdx = 0; locationIdx < NUM_LOCATIONS; ++locationIdx) {
				if ((_engine->_gameState->holomapFlags[locationIdx] & 0x80) || locationIdx == _engine->_scene->currentSceneIdx) {
					uint8 flags = _engine->_gameState->holomapFlags[locationIdx] & 1;
					if (locationIdx == _engine->_scene->currentSceneIdx) {
						flags |= 2u; // model type
					}
#if 0
					const Location &loc = _locations[locationIdx];
					_engine->_renderer->setBaseRotation(angles);
					_engine->_renderer->getBaseRotationPosition(loc.x, loc.y, loc.z);
					int32 v20 = _engine->_renderer->destX;
					int32 v19 = _engine->_renderer->destZ;
					int32 v18 = _engine->_renderer->destY;
					_engine->_renderer->getBaseRotationPosition(v9, v4);
					_engine->_renderer->setBaseRotation(v4);
					_engine->_renderer->baseRotPosX = 0;
					_engine->_renderer->baseRotPosY = 0;
					_engine->_renderer->baseRotPosZ = 9500;
					_engine->_renderer->getBaseRotationPosition(v10, v4);
					int32 v21 = _engine->_renderer->destY;
					_engine->_renderer->getBaseRotationPosition(v11, v4);
					if (a4) {
						if (v21 > _engine->_renderer->destY) {
							continue;
						}
					} else {
						if (v21 < _engine->_renderer->destY) {
							continue;
						}
					}

					DrawListStruct &drawList = _engine->_redraw->drawList[n];
					drawList.posValue = locationIdx;
					drawList.y = v18; // ? TODO
					drawList.z = v19;
					drawList.x = v20;
					drawList.y = v21;
					drawList.field_A = flags;
#endif
					++n;
				}
			}
			// TODO: sort drawlist (for what? y?)
#if 0
			for (int v13 = 0; v13 < n; ++v13) {
				const DrawListStruct &drawList = _engine->_redraw->drawList[n];
				const uint8 flags = drawList.field_A;
				const uint8* bodyPtr = nullptr;
				if (flags < 2u) {
					if (flags == 1) {
						bodyPtr = _engine->_resources->holomapArrowPtr;
					}
				} else {
					if (flags <= 2u) {
						bodyPtr = _engine->_resources->holomapTwinsenModelPtr;
					} else {
						if (flags == 3) {
							bodyPtr = _engine->_resources->holomapTwinsenArrowPtr;
						}
					}
				}
				if (bodyPtr != nullptr) {
					_engine->_renderer->renderIsoModel(x, y, z, angleX, angleY, angleZ, bodyPtr);
				}
			}

			_engine->_renderer->setCameraAngle();

			_engine->_renderer->baseRotPosX = 0;
			_engine->_renderer->baseRotPosY = 0;
			_engine->_renderer->baseRotPosZ = 9500;
			v12 = -256;
			v1 = videoPtr1;
			v2 = videoPtr2;  // holomap surface - 748 * 4 == 4488 (size of that buffer)
			v3 = videoPtr11; // rotated x and y positions sorted
			do {
				v13 = 0;
				while (1) {
					v7 = v2 + 2;     // rot y
					v8 = v2 + 4;     // rot z
					if (v13 >= 1024) {// 1024 == 360 degree
						break;
					}
					destZ = *(_WORD *)v7;
					v4 = *(_WORD *)v8;
					destX = *(_WORD *)v2;
					destY = v4;
					v2 += 6; // advance to next entry in holomap surface surface coordinates
					sub_2654F(v8, v0);
					if (v12 != 256) {
						*(_WORD *)v3 = destY;
						v5 = v3 + 2;
						*(_WORD *)v5 = (v1 - videoPtr1) >> 1;
						v3 = v5 + 2;
					}
					_engine->_renderer->projectPositionOnScreen(v0);
					*(_WORD *)v1 = _engine->_renderer->projPosX;
					v6 = v1 + 2;
					v13 += 32;
					*(_WORD *)v6 = _engine->_renderer->projPosY;
					v1 = v6 + 6;
				}
				destZ = *(_WORD *)v7;
				destY = *(_WORD *)v8;
				destX = *(_WORD *)v2;
				v2 += 6;
				sub_2654F(v8, v0);
				_engine->_renderer->projectPositionOnScreen(v0);
				*(_WORD *)v1 = _engine->_renderer->projPosX;
				v10 = v1 + 2;
				v12 += 32;
				*(_WORD *)v10 = _engine->_renderer->projPosY;
				v1 = v10 + 6;
			} while (v12 <= 256);
			qsort(videoPtr11, 512, 4, sortHolomapSurfaceCoordsByX);

			v1 = 0;
			do { // y
				v2 = videoPtr1 + 2 * *(_WORD *)(v1 + videoPtr11 + 2);
				backDialogueBoxRight = *(_WORD *)v2;
				backDialogueBoxBottom = *(_WORD *)(v2 + 2);
				back2DialogueBoxRight = *(_WORD *)(v2 + 264);
				back2DialogueBoxBottom = *(_WORD *)(v2 + 266);
				back3DialogueBoxRight = *(_WORD *)(v2 + 8);
				back3DialogueBoxBottom = *(_WORD *)(v2 + 10);
				if (sub_26DAB(v0)) {
					word_4B776 = *(_WORD *)(v2 + 4);
					word_4B778 = *(_WORD *)(v2 + 6);
					word_4B77C = *(_WORD *)(v2 + 268);
					word_4B77E = *(_WORD *)(v2 + 270);
					word_4B782 = *(_WORD *)(v2 + 12);
					word_4B784 = *(_WORD *)(v2 + 14);
					sub_2E894(); // handles polyTab, polyTab2, circleBuffer, maybe model rendering?
					sub_2F1D1(v0); // some kind of blitting?
				}
				backDialogueBoxRight = *(_WORD *)(v2 + 264);
				backDialogueBoxBottom = *(_WORD *)(v2 + 266);
				back2DialogueBoxRight = *(_WORD *)(v2 + 272);
				back2DialogueBoxBottom = *(_WORD *)(v2 + 274);
				back3DialogueBoxRight = *(_WORD *)(v2 + 8);
				back3DialogueBoxBottom = *(_WORD *)(v2 + 10);
				v3 = sub_26DAB(v0);
				if (v3) {
					word_4B776 = *(_WORD *)(v2 + 268);
					word_4B778 = *(_WORD *)(v2 + 270);
					word_4B77C = *(_WORD *)(v2 + 276);
					word_4B77E = *(_WORD *)(v2 + 278);
					word_4B782 = *(_WORD *)(v2 + 12);
					word_4B784 = *(_WORD *)(v2 + 14);
					sub_2E894();
					LOBYTE(v3) = sub_2F1D1(v0); // some kind of blitting?
				}
				v1 += 4;
			} while (v1 != 4 * 512);

			if (v10) {
				blitRect();
			}
			_engine->_renderer->copyBlockPhys(v0);
#endif
			// TODO
		}

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
