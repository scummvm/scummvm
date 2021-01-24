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
#include "common/stream.h"
#include "common/types.h"
#include "twine/audio/sound.h"
#include "twine/menu/interface.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/scene/animations.h"
#include "twine/scene/collision.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/movements.h"
#include "twine/scene/scene.h"
#include "twine/shared.h"
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

void Holomap::loadHolomapGFX() {
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

	needToLoadHolomapGFX = 0;
}

static int sortHolomapSurfaceCoordsByDepth(const void *a1, const void *a2) {
	return *(const int16 *)a1 - *(const int16 *)a2;
}

void Holomap::prepareHolomapSurface() {
	Common::MemoryReadStream stream(_engine->_resources->holomapSurfacePtr, _engine->_resources->holomapSurfaceSize);
	int holomapSurfaceArrayIdx = 0;
	int projectedIndex = 0;
	for (int32 angle = -ANGLE_90; angle <= ANGLE_90; angle += ANGLE_11_25) {
		int rotation = 0;
		for (int32 stepWidth = ANGLE_11_25; stepWidth != 0; --stepWidth) {
			const int32 destX = stream.readSint16LE();
			const int32 destY = stream.readSint16LE();
			const int32 destZ = stream.readSint16LE();
			_engine->_renderer->getBaseRotationPosition(destX, destY, destZ);
			if (angle != ANGLE_90) {
				_holomapSurface[holomapSurfaceArrayIdx].z = destZ;
				_holomapSurface[holomapSurfaceArrayIdx].projectedPosIdx = projectedIndex;
				++holomapSurfaceArrayIdx;
			}
			_engine->_renderer->projectPositionOnScreen(destX, destY, destZ);
			_projectedSurfacePositions[projectedIndex].x = _engine->_renderer->projPosX;
			_projectedSurfacePositions[projectedIndex].y = _engine->_renderer->projPosY;
			_projectedSurfacePositions[projectedIndex].unk1 = _engine->_screens->crossDot(0, 0xffff, ANGLE_360 - 1, rotation);
			if (angle == ANGLE_90) {
				_projectedSurfacePositions[projectedIndex].unk2 = 0xffff;
			} else {
				_projectedSurfacePositions[projectedIndex].unk2 = (int16)(((angle + ANGLE_90) * ANGLE_90) / 2);
			}
			rotation = rotation + ANGLE_11_25;
			++projectedIndex;
		}
		const int32 destX = stream.readSint16LE();
		const int32 destY = stream.readSint16LE();
		const int32 destZ = stream.readSint16LE();
		_engine->_renderer->getBaseRotationPosition(destX, destY, destZ);
		_engine->_renderer->projectPositionOnScreen(destX, destY, destZ);
		_projectedSurfacePositions[projectedIndex].x = _engine->_renderer->projPosX;
		_projectedSurfacePositions[projectedIndex].y = _engine->_renderer->projPosY;
		_projectedSurfacePositions[projectedIndex].unk1 = 0xffff;
		if (angle == ANGLE_90) {
			_projectedSurfacePositions[projectedIndex].unk2 = 0xffff;
		} else {
			_projectedSurfacePositions[projectedIndex].unk2 = (int16)(((angle + ANGLE_90) * ANGLE_90) / 2);
		}
		++projectedIndex;
	}
	qsort(_holomapSurface, ARRAYSIZE(_holomapSurface), sizeof(HolomapSurface), sortHolomapSurfaceCoordsByDepth);
}

void Holomap::renderHolomapSurfacePolygons() {
	prepareHolomapSurface();
	for (int32 i = 0; i < ARRAYSIZE(_holomapSurface); ++i) {
		// const HolomapProjectedPos &pos1 = _projectedSurfacePositions[_holomapSurface[i].projectedPosIdx + 0];
		// const HolomapProjectedPos &pos2 = _projectedSurfacePositions[_holomapSurface[i].projectedPosIdx + 1];
		// const HolomapProjectedPos &pos3 = _projectedSurfacePositions[_holomapSurface[i].projectedPosIdx + 2];
		// const HolomapProjectedPos &pos4 = _projectedSurfacePositions[_holomapSurface[i].projectedPosIdx + 3];
		// TODO: build triangles from projected pos with texcoords from holomapimg
# if 0
		v2 = _projectedSurfacePositions + 2 * *(_WORD *)(i + _holomapSurface + 2);
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
			sub_2E894();   // handles polyTab, polyTab2, circleBuffer, maybe model rendering?
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
#endif
	}
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

void Holomap::renderHolomapModel(byte *mdl_1, int32 x_2, int32 y_3, int32 zPos) {
	_engine->_renderer->setBaseRotation(x_2, y_3, 0);
	_engine->_renderer->getBaseRotationPosition(0, 0, zPos + 1000);
	const int32 iVar1 = _engine->_renderer->destZ;
	const int32 y_2 = _engine->_renderer->destY;
	const int32 x_1 = _engine->_renderer->destX;
	int32 trajRotPosX = 0; // TODO DAT_0043f706 4th short value of the trajectory data TrajectoryData::x
	int32 trajRotPosY = 0; // TODO DAT_0043f710 5th short value of the trajectory data TrajectoryData::y
	int32 trajRotPosZ = 0; // TODO _DAT_0043f7f8 6th short value of the trajectory data TrajectoryData::z
	_engine->_renderer->setCameraAngle(0, 0, 0, trajRotPosX, trajRotPosY, trajRotPosZ, 5300);
	_engine->_renderer->getBaseRotationPosition(x_1, y_2, iVar1);
	_engine->_interface->resetClip();
	_engine->_renderer->renderIsoModel(x_1, y_2, iVar1, x_2, y_3, 0, mdl_1);
}

Holomap::TrajectoryData Holomap::loadTrajectoryData(int32 trajectoryIdx) {
	TrajectoryData data;
	Common::MemoryReadStream stream(_engine->_resources->holomapPointAnimPtr, _engine->_resources->holomapPointAnimSize);
	for (int32 trajIdx = 0; trajIdx < trajectoryIdx; ++trajIdx) {
		stream.skip(12);
		const int16 animVal = stream.readSint16LE();
		stream.skip(4 * animVal);
	}
	data.locationIdx = stream.readSint16LE();
	data.trajLocationIdx = stream.readSint16LE();
	data.vehicleIdx = stream.readSint16LE();
	data.x = stream.readSint16LE();
	data.y = stream.readSint16LE();
	data.z = stream.readSint16LE();
	data.numAnimFrames = stream.readSint16LE();
	assert(data.numAnimFrames < ARRAYSIZE(data.positions));
	for (int32 i = 0; i < data.numAnimFrames; ++i) {
		data.positions[i].x = stream.readSint16LE();
		data.positions[i].y = stream.readSint16LE();
	}
	return data;
}

void Holomap::drawHolomapTrajectory(int32 trajectoryIndex) {
	debug("Draw trajectory index %i", trajectoryIndex);
	const Holomap::TrajectoryData &data = loadTrajectoryData(trajectoryIndex);
	ScopedEngineFreeze timeFreeze(_engine);
	ScopedKeyMap holomapKeymap(_engine, holomapKeyMapId);
	_engine->_renderer->setCameraPosition(400, 240, 128, 1024, 1024);
	loadHolomapGFX();

	renderHolomapSurfacePolygons();

	const Location &loc = _locations[data.locationIdx];
	renderHolomapModel(_engine->_resources->holomapPointModelPtr, loc.x, loc.y, 0);

	_engine->flip();
	ActorMoveStruct move;
	AnimTimerDataStruct animData;
	uint8 *animPtr = nullptr;
	HQR::getAllocEntry(&animPtr, Resources::HQR_RESS_FILE, data.getAnimation());
	uint8 *modelPtr = nullptr;
	HQR::getAllocEntry(&modelPtr, Resources::HQR_RESS_FILE, data.getModel());
	int frameNumber = 0;
	int frameTime = _engine->lbaTime;
	int trajAnimFrameIdx = 0;
	for (;;) {
		ScopedFPS scopedFps;
		_engine->readKeys();
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			break;
		}

#if 0
		// TODO
		if (!v28) {
			setPalette2(576 / 3, 96 / 3, (int)&paletteHolomap[3 * needToLoadHolomapGFX++]);
			if (needToLoadHolomapGFX == 96 / 3) {
				needToLoadHolomapGFX = 0;
			}
		}
#endif
		const int16 newAngle = move.getRealAngle(_engine->lbaTime);
		if (move.numOfStep == 0) {
			const int startAngle = ANGLE_0;
			_engine->_movements->setActorAngleSafe(startAngle, startAngle - ANGLE_90, 500, &move);
		}

		if (_engine->_animations->setModelAnimation(frameNumber, animPtr, modelPtr, &animData)) {
			frameNumber++;
			if (frameNumber >= _engine->_animations->getNumKeyframes(animPtr)) {
				frameNumber = _engine->_animations->getStartKeyframe(animPtr);
			}
		}
		_engine->_renderer->setCameraPosition(100, 400, 128, 900, 900);
		_engine->_renderer->setCameraAngle(0, 0, 0, 0x3c, 0x80, 0, 30000);
		_engine->_renderer->setLightVector(0xffffffc4, 0x80, 0);
		_engine->_interface->drawSplittedBox(Common::Rect(0, 200, 199, 0x1df), 0);
		_engine->_renderer->renderIsoModel(0, 0, 0, 0, newAngle, 0, modelPtr);
		_engine->copyBlockPhys(0, 200, 199, 0x1df);
		_engine->_renderer->setCameraPosition(400, 240, 128, 1024, 1024);
		const int32 trajRotPosX = data.x; // DAT_0043f706
		const int32 trajRotPosY = data.y; // DAT_0043f710
		const int32 trajRotPosZ = data.z; // _DAT_0043f7f8
		_engine->_renderer->setCameraAngle(0, 0, 0, trajRotPosX, trajRotPosY, trajRotPosZ, 0x14b4);
		_engine->_renderer->setLightVector(trajRotPosX, trajRotPosY, 0);
		if (frameTime + 40 <= _engine->lbaTime) {
			frameTime = _engine->lbaTime;
			int model_x;
			int model_y;
			if (trajAnimFrameIdx < data.numAnimFrames) {
				model_x = data.positions[trajAnimFrameIdx].x;
				model_y = data.positions[trajAnimFrameIdx].y;
			} else {
				if (data.numAnimFrames < trajAnimFrameIdx) {
					break;
				}
				model_x = _locations[data.trajLocationIdx].x;
				model_y = _locations[data.trajLocationIdx].y;
			}
			renderHolomapModel(_engine->_resources->holomapPointModelPtr, model_x, model_y, 0);
			trajAnimFrameIdx = trajAnimFrameIdx + 1;
		}
#if 0
		if (v28) {
			v28 = 0;
			_engine->_screens->fadeToPal((int)palette);
		}
#endif
	}
	_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBA);
	// TODO: flip()?
	free(animPtr);
	free(modelPtr);
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

void Holomap::renderLocations(int xRot, int yRot, int zRot, bool lower) {
	int n = 0;
	for (int locationIdx = 0; locationIdx < NUM_LOCATIONS; ++locationIdx) {
		if ((_engine->_gameState->holomapFlags[locationIdx] & 0x80) || locationIdx == _engine->_scene->currentSceneIdx) {
			uint8 flags = _engine->_gameState->holomapFlags[locationIdx] & 1;
			if (locationIdx == _engine->_scene->currentSceneIdx) {
				flags |= 2u; // model type
			}
			const Location &loc = _locations[locationIdx];
			_engine->_renderer->setBaseRotation(loc.x, loc.y, 0);
			_engine->_renderer->getBaseRotationPosition(0, 0, loc.z + 1000);
			int32 v20 = _engine->_renderer->destX;
			int32 v18 = _engine->_renderer->destY;
			int32 v19 = _engine->_renderer->destZ;
			_engine->_renderer->getBaseRotationPosition(0, 0, 1500);
			int32 v22 = _engine->_renderer->destX;
			int32 v23 = _engine->_renderer->destY;
			int32 v24 = _engine->_renderer->destZ;
			_engine->_renderer->setBaseRotation(xRot, yRot, zRot);
			_engine->_renderer->baseRotPosX = 0;
			_engine->_renderer->baseRotPosY = 0;
			_engine->_renderer->baseRotPosZ = 9500;
			_engine->_renderer->getBaseRotationPosition(v20, v18, v19);
			int v21 = _engine->_renderer->destZ;
			_engine->_renderer->getBaseRotationPosition(v22, v23, v24);
			if (lower) {
				if (v21 > _engine->_renderer->destY) {
					continue;
				}
			} else {
				if (v21 < _engine->_renderer->destY) {
					continue;
				}
			}

			DrawListStruct &drawList = _engine->_redraw->drawList[n];
			drawList.posValue = v21;
			drawList.actorIdx = locationIdx;
			drawList.type = 0;
			drawList.x = v20;
			drawList.y = v18;
			drawList.z = v21;
			drawList.offset = n;
			drawList.field_C = flags;
			drawList.field_E = 0;
			drawList.field_10 = 0;
			++n;
		}
	}
	_engine->_redraw->sortDrawingList(_engine->_redraw->drawList, n);
	for (int v13 = 0; v13 < n; ++v13) {
		const DrawListStruct &drawList = _engine->_redraw->drawList[v13];
		const uint16 flags = drawList.field_C;
		const uint8 *bodyPtr = nullptr;
		if (flags == 1u) {
			bodyPtr = _engine->_resources->holomapArrowPtr;
		} else if (flags == 2u) {
			bodyPtr = _engine->_resources->holomapTwinsenModelPtr;
		} else if (flags == 3u) {
			bodyPtr = _engine->_resources->holomapTwinsenArrowPtr;
		}
		if (bodyPtr != nullptr) {
			int32 angleX = _locations[drawList.actorIdx].x;
			int32 angleY = _locations[drawList.actorIdx].y;
			_engine->_renderer->renderIsoModel(drawList.x, drawList.y, drawList.z, angleX, angleY, 0, bodyPtr);
		}
	}
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
	renderHolomapSurfacePolygons();
	_engine->_renderer->setCameraPosition(_engine->width() / 2, 190, 128, 1024, 1024);

	_engine->_text->initTextBank(TextBankId::Inventory_Intro_and_Holomap);
	_engine->_text->setFontCrossColor(9);

	drawHolomapText(_engine->width() / 2, 25, "HoloMap"); // TODO: fix the index
	int currentLocation = _engine->_scene->currentSceneIdx;
	drawHolomapLocation(currentLocation);
	_engine->flip();

	// TODO: load RESSHQR_HOLOSURFACE and project the texture to the surface
	//_engine->_screens->loadImage(RESSHQR_HOLOIMG, RESSHQR_HOLOPAL);

	int32 time = _engine->lbaTime;
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
				time = _engine->lbaTime;
				rotate = true;
			}
		} else if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapNext)) {
			const int32 nextLocation = getNextHolomapLocation(currentLocation, 1);
			if (nextLocation != -1) {
				currentLocation = nextLocation;
				drawHolomapLocation(currentLocation);
				time = _engine->lbaTime;
				rotate = true;
			}
		}

		if (_engine->_input->isActionActive(TwinEActionType::HolomapLeft)) {
			xRot += 8;
			rotate = true;
			time = _engine->lbaTime;
		} else if (_engine->_input->isActionActive(TwinEActionType::HolomapRight)) {
			xRot -= 8;
			rotate = true;
			time = _engine->lbaTime;
		}

		if (_engine->_input->isActionActive(TwinEActionType::HolomapUp)) {
			yRot += 8;
			rotate = true;
			time = _engine->lbaTime;
		} else if (_engine->_input->isActionActive(TwinEActionType::HolomapDown)) {
			yRot -= 8;
			rotate = true;
			time = _engine->lbaTime;
		}

		if (rotate) {
			const int32 dt = _engine->lbaTime - time;
			xRot = _engine->_collision->getAverageValue(xRot, _locations[currentLocation].x, 75, dt);
			yRot = _engine->_collision->getAverageValue(yRot, _locations[currentLocation].y, 75, dt);
			redraw = true;
		}

		if (redraw) {
			redraw = false;
			const Common::Rect rect(170, 0, 470, 330);
			_engine->_interface->drawSplittedBox(rect, 0);
			_engine->_renderer->setBaseRotation(xRot, yRot, 0);
			_engine->_renderer->setLightVector(xRot, yRot, 0);
			// renderLocations(xRot, yRot, 0, false); // TODO: activate me
			_engine->_renderer->setBaseRotation(xRot, yRot, 0);
			renderHolomapSurfacePolygons();
			// renderLocations(xRot, yRot, 0, true); // TODO: activate me
			if (rotate) {
				_engine->_menu->drawBox(300, 170, 340, 210);
			}
			//_engine->copyBlockPhys(rect);
			_engine->flip();
		}

		if (rotate && xRot == _locations[currentLocation].x && yRot == _locations[currentLocation].y) {
			rotate = false;
		}

		++_engine->lbaTime;

		// TODO: text afterwards on top (not before as it is currently implemented)?
		// pos 0x140,0x19?

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
