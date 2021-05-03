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
#include "twine/parser/anim.h"
#include "twine/parser/holomap.h"
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
	_numLocations = locationsSize / 8;
	if (_numLocations > NUM_LOCATIONS) {
		warning("Amount of locations (%i) exceeds the maximum of %i", _numLocations, NUM_LOCATIONS);
		return false;
	}

	_engine->_text->initTextBank(TextBankId::Inventory_Intro_and_Holomap);
	for (int32 i = 0; i < _numLocations; i++) {
		_locations[i].angle.x = ClampAngle(stream.readSint16LE());
		_locations[i].angle.y = ClampAngle(stream.readSint16LE());
		_locations[i].angle.z = ClampAngle(stream.readSint16LE());
		_locations[i].textIndex = (TextId)stream.readUint16LE();

		if (_engine->_text->getMenuText(_locations[i].textIndex, _locations[i].name, sizeof(_locations[i].name))) {
			debug(2, "Scene %i: %s", i, _locations[i].name);
			continue;
		}
		debug(2, "Could not get location text for index %i", i);
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
		_paletteHolomap[i + 0] = _engine->_screens->palette[j + 0];
		_paletteHolomap[i + 1] = _engine->_screens->palette[j + 1];
		_paletteHolomap[i + 2] = _engine->_screens->palette[j + 2];
	}

	j = 576;
	for (int32 i = 96; i < 189; i += 3, j += 3) {
		_paletteHolomap[i + 0] = _engine->_screens->palette[j + 0];
		_paletteHolomap[i + 1] = _engine->_screens->palette[j + 1];
		_paletteHolomap[i + 2] = _engine->_screens->palette[j + 2];
	}

	prepareHolomapProjectedPositions();
	prepareHolomapSurface();
	_holomapPaletteIndex = 0;
}

static int sortHolomapSurfaceCoordsByDepth(const void *a1, const void *a2) {
	return (int)*(const int16 *)a1 - (int)*(const int16 *)a2;
}

// verified with disassembly
void Holomap::prepareHolomapSurface() {
	Common::MemoryReadStream stream(_engine->_resources->holomapSurfacePtr, _engine->_resources->holomapSurfaceSize);
	int holomapSurfaceArrayIdx = 0;
	_engine->_renderer->setBaseRotation(0, 0, 0);
	for (int angle = -ANGLE_90; angle <= ANGLE_90; angle += ANGLE_11_25) {
		int rotation = 0;
		for (int i = 0; i <= ANGLE_11_25; ++i, rotation += ANGLE_11_25) {
			const int32 rotX = stream.readByte();
			const IVec3& rotVec = _engine->_renderer->getHolomapRotation(rotX, angle, rotation);
			_holomapSurface[holomapSurfaceArrayIdx].x = rotVec.x;
			_holomapSurface[holomapSurfaceArrayIdx].y = rotVec.y;
			_holomapSurface[holomapSurfaceArrayIdx].z = rotVec.z;
			++holomapSurfaceArrayIdx;
		}
	}
	assert(stream.eos());
}

// verified with disassembly
void Holomap::prepareHolomapProjectedPositions() {
	int projectedIndex = 0;
	for (int32 angle = -ANGLE_90; angle <= ANGLE_90; angle += ANGLE_11_25) {
		int rotation = 0;
		for (int32 i = 0; i < ANGLE_11_25; ++i) {
			_projectedSurfacePositions[projectedIndex].unk1 = _engine->_screens->crossDot(0, 0xffff, ANGLE_360 - 1, rotation);
			if (angle == ANGLE_90) {
				_projectedSurfacePositions[projectedIndex].unk2 = -1;
			} else {
				_projectedSurfacePositions[projectedIndex].unk2 = ((angle + ANGLE_90) * ANGLE_90) / 2;
			}
			rotation += ANGLE_11_25;
			++projectedIndex;
		}
		_projectedSurfacePositions[projectedIndex].unk1 = -1;
		if (angle == ANGLE_90) {
			_projectedSurfacePositions[projectedIndex].unk2 = -1;
		} else {
			_projectedSurfacePositions[projectedIndex].unk2 = ((angle + ANGLE_90) * ANGLE_90) / 2;
		}
		++projectedIndex;
	}
}

// verified with disassembly
void Holomap::prepareHolomapPolygons() {
	int holomapSortArrayIdx = 0;
	int holomapSurfaceArrayIdx = 0;
	_projectedSurfaceIndex = 0;
	for (int32 angle = -ANGLE_90; angle <= ANGLE_90; angle += ANGLE_11_25) {
		int rotation = 0;
		for (int32 stepWidth = 0; stepWidth < ANGLE_11_25; ++stepWidth) {
			IVec3* vec = &_holomapSurface[holomapSurfaceArrayIdx++];
			_engine->_renderer->getBaseRotationPosition(vec->x, vec->y, vec->z);
			if (angle != ANGLE_90) {
				_holomapSort[holomapSortArrayIdx].z = _engine->_renderer->destPos.z;
				_holomapSort[holomapSortArrayIdx].projectedPosIdx = _projectedSurfaceIndex;
				++holomapSortArrayIdx;
			}
			_engine->_renderer->projectXYPositionOnScreen(_engine->_renderer->destPos);
			_projectedSurfacePositions[_projectedSurfaceIndex].x = _engine->_renderer->projPos.x;
			_projectedSurfacePositions[_projectedSurfaceIndex].y = _engine->_renderer->projPos.y;
			rotation += ANGLE_11_25;
			++_projectedSurfaceIndex;
		}
		IVec3* vec = &_holomapSurface[holomapSurfaceArrayIdx++];
		_engine->_renderer->getBaseRotationPosition(vec->x, vec->y, vec->z);
		_engine->_renderer->projectXYPositionOnScreen(_engine->_renderer->destPos);
		_projectedSurfacePositions[_projectedSurfaceIndex].x = _engine->_renderer->projPos.x;
		_projectedSurfacePositions[_projectedSurfaceIndex].y = _engine->_renderer->projPos.y;
		rotation += ANGLE_11_25;
		++_projectedSurfaceIndex;
	}
	assert(holomapSortArrayIdx == ARRAYSIZE(_holomapSort));
	assert(holomapSurfaceArrayIdx == ARRAYSIZE(_holomapSurface));
	assert(_projectedSurfaceIndex == ARRAYSIZE(_projectedSurfacePositions));
	qsort(_holomapSort, ARRAYSIZE(_holomapSort), sizeof(HolomapSort), sortHolomapSurfaceCoordsByDepth);
}

bool Holomap::isTriangleVisible(const Vertex *vertices) const {
	const int32 iVar2 = ((int32)vertices[1].x - (int32)vertices[0].x) *
						((int32)vertices[0].y - (int32)vertices[2].y);
	const int32 iVar1 = ((int32)vertices[1].y - (int32)vertices[0].y) *
						((int32)vertices[0].x - (int32)vertices[2].x);
	return iVar2 - iVar1 != 0 && iVar1 <= iVar2;
}

void Holomap::renderHolomapSurfacePolygons() {
	prepareHolomapPolygons();
	for (int32 i = 0; i < ARRAYSIZE(_holomapSort); ++i) {
		assert(_holomapSort[i].projectedPosIdx + 34 < _projectedSurfaceIndex);
		const HolomapProjectedPos &pos1 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 0];
		const HolomapProjectedPos &pos2 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 33];
		const HolomapProjectedPos &pos3 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 1];
		Vertex vertexCoordinates[3];
		vertexCoordinates[0].x = pos1.x;
		vertexCoordinates[0].y = pos1.y;
		vertexCoordinates[1].x = pos2.x;
		vertexCoordinates[1].y = pos2.y;
		vertexCoordinates[2].x = pos3.x;
		vertexCoordinates[2].y = pos3.y;
		if (isTriangleVisible(vertexCoordinates)) {
			Vertex vertexAngles[3];
			vertexAngles[0].x = pos1.unk1;
			vertexAngles[0].y = pos1.unk2;
			vertexAngles[1].x = pos2.unk1;
			vertexAngles[1].y = pos2.unk2;
			vertexAngles[2].x = pos3.unk1;
			vertexAngles[2].y = pos3.unk2;
			_engine->_renderer->renderHolomapVertices(vertexCoordinates, vertexAngles);
		}
		const HolomapProjectedPos &pos4 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 33];
		const HolomapProjectedPos &pos5 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 34];
		const HolomapProjectedPos &pos6 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 1];
		vertexCoordinates[0].x = pos4.x;
		vertexCoordinates[0].y = pos4.y;
		vertexCoordinates[1].x = pos5.x;
		vertexCoordinates[1].y = pos5.y;
		vertexCoordinates[2].x = pos6.x;
		vertexCoordinates[2].y = pos6.y;
		if (isTriangleVisible(vertexCoordinates)) {
			Vertex vertexAngles[3];
			vertexAngles[0].x = pos4.unk1;
			vertexAngles[0].y = pos4.unk2;
			vertexAngles[1].x = pos5.unk1;
			vertexAngles[1].y = pos5.unk2;
			vertexAngles[2].x = pos6.unk1;
			vertexAngles[2].y = pos6.unk2;
			_engine->_renderer->renderHolomapVertices(vertexCoordinates, vertexAngles);
		}
	}
}

void Holomap::drawHolomapText(int32 centerx, int32 top, const char *title) {
	const int32 size = _engine->_text->getTextSize(title);
	const int32 x = centerx - size / 2;
	const int32 y = top;
	_engine->_text->setFontColor(COLOR_WHITE);
	_engine->_text->drawText(x, y, title);
}

void Holomap::renderHolomapModel(const BodyData &bodyData, int32 x, int32 y, int32 zPos) {
	_engine->_renderer->setBaseRotation(x, y, 0);
	_engine->_renderer->getBaseRotationPosition(0, 0, zPos + 1000);
	_engine->_renderer->getBaseRotationPosition(_engine->_renderer->destPos.x, _engine->_renderer->destPos.y, _engine->_renderer->destPos.z);
	_engine->_interface->resetClip();
	_engine->_renderer->renderIsoModel(_engine->_renderer->destPos.x, _engine->_renderer->destPos.y, _engine->_renderer->destPos.z, x, y, 0, bodyData);
}

void Holomap::drawHolomapTrajectory(int32 trajectoryIndex) {
	debug("Draw trajectory index %i", trajectoryIndex);

	const Trajectory *data = _engine->_resources->getTrajectory(trajectoryIndex);
	if (data == nullptr) {
		warning("Failed to load trajectory data for index %i", trajectoryIndex);
		return;
	}

	_engine->exitSceneryView();
	_engine->_interface->resetClip();
	_engine->_screens->clearScreen();
	_engine->setPalette(_engine->_screens->paletteRGBA);

	loadHolomapGFX();
	ScopedEngineFreeze timeFreeze(_engine);
	_engine->_renderer->setCameraPosition(400, 240, 128, 1024, 1024);
	_engine->_renderer->setCameraAngle(0, 0, 0, data->pos.x, data->pos.y, data->pos.z, 5300);

	renderHolomapSurfacePolygons();

	const Location &loc = _locations[data->locationIdx];
	renderHolomapModel(_engine->_resources->holomapPointModelPtr, loc.angle.x, loc.angle.y, 0);

	_engine->flip();
	ActorMoveStruct move;
	AnimTimerDataStruct animTimerData;
	AnimData animData;
	animData.loadFromHQR(Resources::HQR_RESS_FILE, data->getAnimation());
	BodyData bodyData;
	bodyData.loadFromHQR(Resources::HQR_RESS_FILE, data->getModel());
	uint frameNumber = 0;
	int32 frameTime = _engine->lbaTime;
	int16 trajAnimFrameIdx = 0;

	int32 local18 = 0;
	bool fadeInPalette = true;
	_engine->_input->enableKeyMap(holomapKeyMapId);
	for (;;) {
		ScopedFPS scopedFps;
		_engine->readKeys();
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			break;
		}

		if (!fadeInPalette && local18 < _engine->lbaTime) {
			//const Common::Rect rect(170, 50, 470, 330);
			//_engine->_interface->setClip(rect);
			_engine->setPalette(192, 32, &_paletteHolomap[3 * _holomapPaletteIndex++]);
			//_engine->copyBlockPhys(rect);
			//_engine->_interface->resetClip();
			if (_holomapPaletteIndex == 32) {
				_holomapPaletteIndex = 0;
			}
			local18 = _engine->lbaTime + 3;
		}

		const int16 newAngle = move.getRealAngle(_engine->lbaTime);
		if (move.numOfStep == 0) {
			_engine->_movements->setActorAngleSafe(ANGLE_0, -ANGLE_90, 500, &move);
		}

		if (_engine->_animations->setModelAnimation(frameNumber, animData, bodyData, &animTimerData)) {
			frameNumber++;
			if (frameNumber >= animData.getNumKeyframes()) {
				frameNumber = animData.getLoopFrame();
			}
		}
		_engine->_renderer->setCameraPosition(100, 400, 128, 900, 900);
		_engine->_renderer->setCameraAngle(0, 0, 0, 60, 128, 0, 30000);
		_engine->_renderer->setLightVector(-60, 128, 0);
		const Common::Rect rect(0, 200, 199, 479);
		_engine->_interface->drawFilledRect(rect, COLOR_BLACK);
		_engine->_renderer->renderIsoModel(0, 0, 0, 0, newAngle, 0, bodyData);
		_engine->copyBlockPhys(rect);
		_engine->_renderer->setCameraPosition(400, 240, 128, 1024, 1024);
		_engine->_renderer->setCameraAngle(0, 0, 0, data->pos.x, data->pos.y, data->pos.z, 5300);
		_engine->_renderer->setLightVector(data->pos.x, data->pos.y, 0);
		if (frameTime + 40 <= _engine->lbaTime) {
			frameTime = _engine->lbaTime;
			int32 modelX;
			int32 modelY;
			if (trajAnimFrameIdx < data->numAnimFrames) {
				modelX = data->positions[trajAnimFrameIdx].x;
				modelY = data->positions[trajAnimFrameIdx].y;
			} else {
				if (data->numAnimFrames < trajAnimFrameIdx) {
					break;
				}
				modelX = _locations[data->trajLocationIdx].angle.x;
				modelY = _locations[data->trajLocationIdx].angle.y;
			}
			renderHolomapModel(_engine->_resources->holomapPointModelPtr, modelX, modelY, 0);
			++trajAnimFrameIdx;
		}

		if (fadeInPalette) {
			fadeInPalette = false;
			// TODO: this does a flip - which puts stuff onto the screen that shouldn't be there
			//_engine->_screens->fadeToPal(_engine->_screens->paletteRGBA);
		}
	}

	_engine->_screens->clearScreen();
	_engine->setPalette(_engine->_screens->paletteRGBA);
	_engine->_gameState->initEngineProjections();
	_engine->_interface->loadClip();

	_engine->_text->initSceneTextBank();
	_engine->_input->enableKeyMap(mainKeyMapId);
}

int32 Holomap::getNextHolomapLocation(int32 currentLocation, int32 dir) const {
	const int32 idx = currentLocation;
	int32 i = currentLocation + dir;
	if (i < 0) {
		i = NUM_LOCATIONS - 1;
	} else {
		i %= NUM_LOCATIONS;
	}
	for (; i != idx; i = (i + dir) % NUM_LOCATIONS) {
		if (_engine->_gameState->holomapFlags[i] & 0x81) {
			return i;
		}
	}
	return -1;
}

void Holomap::renderLocations(int xRot, int yRot, int zRot, bool lower) {
	int n = 0;
	for (int locationIdx = 0; locationIdx < NUM_LOCATIONS; ++locationIdx) {
		if ((_engine->_gameState->holomapFlags[locationIdx] & 0x80) || locationIdx == _engine->_scene->currentSceneIdx) {
			const Location &loc = _locations[locationIdx];
			_engine->_renderer->setBaseRotation(loc.angle.x, loc.angle.y, 0);
			_engine->_renderer->getBaseRotationPosition(0, 0, loc.angle.z + 1000);
			int32 xpos1 = _engine->_renderer->destPos.x;
			int32 ypos1 = _engine->_renderer->destPos.y;
			int32 zpos1 = _engine->_renderer->destPos.z;
			_engine->_renderer->getBaseRotationPosition(0, 0, 1500);
			int32 xpos2 = _engine->_renderer->destPos.x;
			int32 ypos2 = _engine->_renderer->destPos.y;
			int32 zpos2 = _engine->_renderer->destPos.z;
			_engine->_renderer->setBaseRotation(xRot, yRot, zRot, true);
			int32 zpos1_copy = zpos1;
			_engine->_renderer->baseRotPos.x = 0;
			_engine->_renderer->baseRotPos.y = 0;
			_engine->_renderer->baseRotPos.z = 9500;
			_engine->_renderer->getBaseRotationPosition(xpos1, ypos1, zpos1);
			int32 zpos1_copy2 = _engine->_renderer->destPos.z;
			_engine->_renderer->getBaseRotationPosition(xpos2, ypos2, zpos2);
			if (lower) {
				if (zpos1_copy2 > _engine->_renderer->destPos.z) {
					continue;
				}
			} else {
				if (_engine->_renderer->destPos.z > zpos1_copy2) {
					continue;
				}
			}
			uint8 flags = _engine->_gameState->holomapFlags[locationIdx] & 1;
			if (locationIdx == _engine->_scene->currentSceneIdx) {
				flags |= 2u; // model type
			}
			DrawListStruct &drawList = _engine->_redraw->drawList[n];
			drawList.posValue = zpos1_copy2;
			drawList.actorIdx = locationIdx;
			drawList.type = flags;
			drawList.x = xpos1;
			drawList.y = ypos1;
			drawList.z = zpos1_copy;
			++n;
		}
	}
	_engine->_redraw->sortDrawingList(_engine->_redraw->drawList, n);
	for (int i = 0; i < n; ++i) {
		const DrawListStruct &drawList = _engine->_redraw->drawList[i];
		const uint16 flags = drawList.type;
		const BodyData *bodyData = nullptr;
		if (flags == 1u) {
			bodyData = &_engine->_resources->holomapArrowPtr;
		} else if (flags == 2u) {
			bodyData = &_engine->_resources->holomapTwinsenModelPtr;
		} else if (flags == 3u) {
			bodyData = &_engine->_resources->holomapTwinsenArrowPtr;
		}
		if (bodyData != nullptr) {
			int32 angleX = _locations[drawList.actorIdx].angle.x;
			int32 angleY = _locations[drawList.actorIdx].angle.y;
			_engine->_renderer->renderIsoModel(drawList.x, drawList.y, drawList.z, angleX, angleY, 0, *bodyData);
		}
	}
}

void Holomap::processHolomap() {
	ScopedEngineFreeze freeze(_engine);

	const int32 alphaLightTmp = _engine->_scene->alphaLight;
	const int32 betaLightTmp = _engine->_scene->betaLight;

	_engine->exitSceneryView();

	_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBA);
	_engine->_sound->stopSamples();
	_engine->_interface->saveClip();
	_engine->_interface->resetClip();
	_engine->_screens->clearScreen();
	_engine->setPalette(_engine->_screens->paletteRGBA);

	loadHolomapGFX();

	_engine->_text->initTextBank(TextBankId::Inventory_Intro_and_Holomap);
	_engine->_text->setFontCrossColor(COLOR_9);
	_engine->_renderer->setCameraPosition(_engine->width() / 2, 190, 128, 1024, 1024);

	int32 currentLocation = _engine->_scene->currentSceneIdx;
	_engine->_text->drawHolomapLocation(_locations[currentLocation].textIndex);
	_engine->flip();

	int32 time = _engine->lbaTime;
	int32 xRot = ClampAngle(_locations[currentLocation].angle.x);
	int32 yRot = ClampAngle(_locations[currentLocation].angle.y);
	bool rotate = false;
	bool redraw = true;
	int local18 = 0;
	bool fadeInPalette = true;
	_engine->_input->enableKeyMap(holomapKeyMapId);
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
				_engine->_text->drawHolomapLocation(_locations[currentLocation].textIndex);
				time = _engine->lbaTime;
				rotate = true;
			}
		} else if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapNext)) {
			const int32 nextLocation = getNextHolomapLocation(currentLocation, 1);
			if (nextLocation != -1) {
				currentLocation = nextLocation;
				_engine->_text->drawHolomapLocation(_locations[currentLocation].textIndex);
				time = _engine->lbaTime;
				rotate = true;
			}
		}

		if (_engine->_input->isActionActive(TwinEActionType::HolomapLeft)) {
			xRot += ANGLE_1;
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
			xRot = _engine->_collision->getAverageValue(ClampAngle(xRot), _locations[currentLocation].angle.x, 75, dt);
			yRot = _engine->_collision->getAverageValue(ClampAngle(yRot), _locations[currentLocation].angle.y, 75, dt);
			redraw = true;
		}

		if (!fadeInPalette && local18 < _engine->lbaTime) {
			//const Common::Rect rect(170, 50, 470, 330);
			//_engine->_interface->setClip(rect);
			_engine->setPalette(192, 32, &_paletteHolomap[3 * _holomapPaletteIndex++]);
			//_engine->copyBlockPhys(rect);
			//_engine->_interface->resetClip();
			if (_holomapPaletteIndex == 32) {
				_holomapPaletteIndex = 0;
			}
			local18 = _engine->lbaTime + 3;
			redraw = true;
		}

		if (redraw) {
			redraw = false;
			const Common::Rect rect(170, 0, 470, 330);
			_engine->_interface->drawFilledRect(rect, COLOR_BLACK);
			_engine->_renderer->setBaseRotation(xRot, yRot, 0, true);
			_engine->_renderer->setLightVector(xRot, yRot, 0);
			renderLocations(xRot, yRot, 0, false);
			_engine->_renderer->setBaseRotation(xRot, yRot, 0, true);
			_engine->_renderer->baseRotPos.x = 0;
			_engine->_renderer->baseRotPos.y = 0;
			_engine->_renderer->baseRotPos.z = 9500;
			renderHolomapSurfacePolygons();
			renderLocations(xRot, yRot, 0, true);
			drawHolomapText(_engine->width() / 2, 25, "HoloMap");
			if (rotate) {
				_engine->_menu->drawBox(300, 170, 340, 210);
			}
			_engine->copyBlockPhys(rect);
		}

		if (rotate && xRot == _locations[currentLocation].angle.x && yRot == _locations[currentLocation].angle.y) {
			rotate = false;
		}

		++_engine->lbaTime;

		// TODO: text afterwards on top (not before as it is currently implemented)?
		// pos 0x140,0x19?

		//_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
		//_engine->flip();
		if (fadeInPalette) {
			fadeInPalette = false;
			// TODO: this does a flip - which puts stuff onto the screen that shouldn't be there
			//_engine->_screens->fadeToPal(_engine->_screens->paletteRGBA);
		}
	}

	_engine->_screens->clearScreen();
	_engine->_text->drawTextBoxBackground = true;
	_engine->setPalette(_engine->_screens->paletteRGBA);
	_engine->_scene->alphaLight = alphaLightTmp;
	_engine->_scene->betaLight = betaLightTmp;

	_engine->_gameState->initEngineProjections();
	_engine->_interface->loadClip();

	_engine->_input->enableKeyMap(mainKeyMapId);
	_engine->_text->initSceneTextBank();
}

} // namespace TwinE
