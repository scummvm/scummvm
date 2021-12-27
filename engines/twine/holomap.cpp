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

#include "twine/holomap.h"
#include "common/algorithm.h"
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

#define HOLOMAP_ARROW		(1 << 0)
#define HOLOMAP_VISITED		(1 << 1)
#define HOLOMAP_UNK3		(1 << 2)
#define HOLOMAP_UNK4		(1 << 3)
#define HOLOMAP_UNK5		(1 << 4)
#define HOLOMAP_UNK6		(1 << 5)
#define HOLOMAP_UNK7		(1 << 6)
#define HOLOMAP_CAN_FOCUS	(1 << 7)
#define HOLOMAP_RESET		(HOLOMAP_VISITED | HOLOMAP_UNK3 | HOLOMAP_UNK4 | HOLOMAP_UNK5 | HOLOMAP_UNK6 | HOLOMAP_UNK7)
#define HOLOMAP_ACTIVE		(HOLOMAP_CAN_FOCUS | HOLOMAP_ARROW)

static const float zDistanceHolomap = 9500.0f;
static const float zDistanceTrajectory = 5300.0f;

Holomap::Holomap(TwinEEngine *engine) : _engine(engine) {}

int32 Holomap::distance(float distance) const {
	const float w = (float)_engine->width() / (float)_engine->originalWidth();
	const float h = (float)_engine->height() / (float)_engine->originalHeight();
	const float f = MIN<float>(w, h);
	return (int32)(distance / f);
}

int32 Holomap::scale(float val) const {
	const float w = (float)_engine->width() / (float)_engine->originalWidth();
	const float h = (float)_engine->height() / (float)_engine->originalHeight();
	const float f = MIN<float>(w, h);
	return (int32)(val * f);
}

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
		_locations[i].angleX = ClampAngle(stream.readSint16LE());
		_locations[i].angleY = ClampAngle(stream.readSint16LE());
		_locations[i].size = stream.readSint16LE();
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
	assert(locationIdx >= 0 && locationIdx <= ARRAYSIZE(_engine->_gameState->_holomapFlags));
	_engine->_gameState->_holomapFlags[locationIdx] = HOLOMAP_ACTIVE;
	if (_engine->_gameState->hasItem(InventoryItems::kiHolomap)) {
		_engine->_redraw->addOverlay(OverlayType::koInventoryItem, InventoryItems::kiHolomap, 0, 0, 0, OverlayPosType::koNormal, 3);
	}
}

void Holomap::clearHolomapPosition(int32 locationIdx) {
	assert(locationIdx >= 0 && locationIdx <= ARRAYSIZE(_engine->_gameState->_holomapFlags));
	_engine->_gameState->_holomapFlags[locationIdx] &= HOLOMAP_RESET;
	_engine->_gameState->_holomapFlags[locationIdx] |= HOLOMAP_UNK7;
}

void Holomap::loadHolomapGFX() {
	constexpr TwineResource resource(Resources::HQR_RESS_FILE, RESSHQR_HOLOPAL);
	_engine->_screens->loadCustomPalette(resource);

	int32 j = HOLOMAP_PALETTE_INDEX * 3;
	const int32 n = NUM_HOLOMAPCOLORS * 3;
	for (int32 i = 0; i < n; i += 3, j += 3) {
		_paletteHolomap[i + 0] = _engine->_screens->_palette[j + 0];
		_paletteHolomap[i + 1] = _engine->_screens->_palette[j + 1];
		_paletteHolomap[i + 2] = _engine->_screens->_palette[j + 2];
	}

	j = HOLOMAP_PALETTE_INDEX * 3;
	for (int32 i = n; i < 2 * n - 3; i += 3, j += 3) {
		_paletteHolomap[i + 0] = _engine->_screens->_palette[j + 0];
		_paletteHolomap[i + 1] = _engine->_screens->_palette[j + 1];
		_paletteHolomap[i + 2] = _engine->_screens->_palette[j + 2];
	}

	prepareHolomapProjectedPositions();

	Common::SeekableReadStream *surfaceStream = HQR::makeReadStream(TwineResource(Resources::HQR_RESS_FILE, RESSHQR_HOLOSURFACE));
	if (surfaceStream == nullptr) {
		error("Failed to load holomap surface");
	}
	prepareHolomapSurface(surfaceStream);
	delete surfaceStream;
	_holomapPaletteIndex = 0;
}

void Holomap::prepareHolomapSurface(Common::SeekableReadStream *holomapSurfaceStream) {
	int holomapSurfaceArrayIdx = 0;
	_engine->_renderer->setBaseRotation(0, 0, 0);
	for (int alpha = -ANGLE_90; alpha <= ANGLE_90; alpha += ANGLE_11_25) {
		const int32 rot = holomapSurfaceStream->readByte();
		holomapSurfaceStream->seek(-1, SEEK_CUR);
		for (int beta = 0; beta < ANGLE_360; beta += ANGLE_11_25) {
			const int32 rotX = holomapSurfaceStream->readByte();
			const IVec3 &rotVec = _engine->_renderer->getHolomapRotation(rotX, alpha, beta);
			_holomapSurface[holomapSurfaceArrayIdx].x = rotVec.x;
			_holomapSurface[holomapSurfaceArrayIdx].y = rotVec.y;
			_holomapSurface[holomapSurfaceArrayIdx].z = rotVec.z;
			++holomapSurfaceArrayIdx;
		}
		const IVec3 &rotVec = _engine->_renderer->getHolomapRotation(rot, alpha, 0);
		_holomapSurface[holomapSurfaceArrayIdx].x = rotVec.x;
		_holomapSurface[holomapSurfaceArrayIdx].y = rotVec.y;
		_holomapSurface[holomapSurfaceArrayIdx].z = rotVec.z;
		++holomapSurfaceArrayIdx;
	}
	assert(holomapSurfaceStream->eos());
}

void Holomap::prepareHolomapProjectedPositions() {
	int projectedIndex = 0;
	for (int32 alpha = -ANGLE_90; alpha <= ANGLE_90; alpha += ANGLE_11_25) {
		for (int32 beta = 0; beta < ANGLE_360; beta += ANGLE_11_25) {
			_projectedSurfacePositions[projectedIndex].x2 = _engine->_screens->lerp(0, 0xffff, ANGLE_360 - 1, beta);
			if (alpha == ANGLE_90) {
				_projectedSurfacePositions[projectedIndex].y2 = -1;
			} else {
				_projectedSurfacePositions[projectedIndex].y2 = ((alpha + ANGLE_90) * ANGLE_90) / 2;
			}
			++projectedIndex;
		}
		_projectedSurfacePositions[projectedIndex].x2 = -1;
		if (alpha == ANGLE_90) {
			_projectedSurfacePositions[projectedIndex].y2 = -1;
		} else {
			_projectedSurfacePositions[projectedIndex].y2 = ((alpha + ANGLE_90) * ANGLE_90) / 2;
		}
		++projectedIndex;
	}
}

void Holomap::prepareHolomapPolygons() {
	int holomapSortArrayIdx = 0;
	int holomapSurfaceArrayIdx = 0;
	_projectedSurfaceIndex = 0;
	for (int32 alpha = -ANGLE_90; alpha <= ANGLE_90; alpha += ANGLE_11_25) {
		for (int32 beta = 0; beta < ANGLE_11_25; ++beta) {
			IVec3 *vec = &_holomapSurface[holomapSurfaceArrayIdx++];
			const IVec3 &destPos = _engine->_renderer->getBaseRotationPosition(vec->x, vec->y, vec->z);
			if (alpha != ANGLE_90) {
				_holomapSort[holomapSortArrayIdx].z = destPos.z;
				_holomapSort[holomapSortArrayIdx].projectedPosIdx = _projectedSurfaceIndex;
				++holomapSortArrayIdx;
			}
			const IVec3 &projPos = _engine->_renderer->projectPositionOnScreen(destPos);
			_projectedSurfacePositions[_projectedSurfaceIndex].x1 = projPos.x;
			_projectedSurfacePositions[_projectedSurfaceIndex].y1 = projPos.y;
			++_projectedSurfaceIndex;
		}
		IVec3 *vec = &_holomapSurface[holomapSurfaceArrayIdx++];
		const IVec3 &destPos = _engine->_renderer->getBaseRotationPosition(vec->x, vec->y, vec->z);
		const IVec3 &projPos = _engine->_renderer->projectPositionOnScreen(destPos);
		_projectedSurfacePositions[_projectedSurfaceIndex].x1 = projPos.x;
		_projectedSurfacePositions[_projectedSurfaceIndex].y1 = projPos.y;
		++_projectedSurfaceIndex;
	}
	assert(holomapSortArrayIdx == ARRAYSIZE(_holomapSort));
	assert(holomapSurfaceArrayIdx == ARRAYSIZE(_holomapSurface));
	assert(_projectedSurfaceIndex == ARRAYSIZE(_projectedSurfacePositions));
	Common::sort(_holomapSort, _holomapSort + ARRAYSIZE(_holomapSort), [](const HolomapSort &a, const HolomapSort &b) { return a.z < b.z; });
}

bool Holomap::isTriangleVisible(const Vertex *vertices) const {
	const int32 iVar2 = ((int32)vertices[1].x - (int32)vertices[0].x) *
						((int32)vertices[0].y - (int32)vertices[2].y);
	const int32 iVar1 = ((int32)vertices[1].y - (int32)vertices[0].y) *
						((int32)vertices[0].x - (int32)vertices[2].x);
	return iVar2 - iVar1 != 0 && iVar1 <= iVar2;
}

#define SURFACE_POS_OFFSET ((ANGLE_360 / ANGLE_11_25) + 1)
void Holomap::renderHolomapSurfacePolygons(uint8 *holomapImage, uint32 holomapImageSize) {
	prepareHolomapPolygons();
	for (int32 i = 0; i < ARRAYSIZE(_holomapSort); ++i) {
		assert(_holomapSort[i].projectedPosIdx + 34 < _projectedSurfaceIndex);
		const HolomapProjectedPos &pos1 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 0];
		const HolomapProjectedPos &pos2 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 0 + SURFACE_POS_OFFSET];
		const HolomapProjectedPos &pos3 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 1];
		Vertex vertexCoordinates[3];
		vertexCoordinates[0].x = pos1.x1;
		vertexCoordinates[0].y = pos1.y1;
		vertexCoordinates[1].x = pos2.x1;
		vertexCoordinates[1].y = pos2.y1;
		vertexCoordinates[2].x = pos3.x1;
		vertexCoordinates[2].y = pos3.y1;
		if (isTriangleVisible(vertexCoordinates)) {
			Vertex textureCoordinates[3];
			textureCoordinates[0].x = pos1.x2;
			textureCoordinates[0].y = pos1.y2;
			textureCoordinates[1].x = pos2.x2;
			textureCoordinates[1].y = pos2.y2;
			textureCoordinates[2].x = pos3.x2;
			textureCoordinates[2].y = pos3.y2;
			_engine->_renderer->renderHolomapVertices(vertexCoordinates, textureCoordinates, holomapImage, holomapImageSize);
		}
		const HolomapProjectedPos &pos4 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 0 + SURFACE_POS_OFFSET];
		const HolomapProjectedPos &pos5 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 1 + SURFACE_POS_OFFSET];
		const HolomapProjectedPos &pos6 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 1];
		vertexCoordinates[0].x = pos4.x1;
		vertexCoordinates[0].y = pos4.y1;
		vertexCoordinates[1].x = pos5.x1;
		vertexCoordinates[1].y = pos5.y1;
		vertexCoordinates[2].x = pos6.x1;
		vertexCoordinates[2].y = pos6.y1;
		if (isTriangleVisible(vertexCoordinates)) {
			Vertex textureCoordinates[3];
			textureCoordinates[0].x = pos4.x2;
			textureCoordinates[0].y = pos4.y2;
			textureCoordinates[1].x = pos5.x2;
			textureCoordinates[1].y = pos5.y2;
			textureCoordinates[2].x = pos6.x2;
			textureCoordinates[2].y = pos6.y2;
			_engine->_renderer->renderHolomapVertices(vertexCoordinates, textureCoordinates, holomapImage, holomapImageSize);
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

void Holomap::renderHolomapPointModel(const IVec3 &angle, int32 x, int32 y) {
	_engine->_renderer->setBaseRotation(x, y, 0);
	const IVec3 &destPos = _engine->_renderer->getBaseRotationPosition(0, 0, 1000);
	_engine->_renderer->setBaseTranslation(0, 0, 0);
	_engine->_renderer->setBaseRotation(angle);
	_engine->_renderer->setBaseRotationPos(0, 0, distance(zDistanceTrajectory));
	_engine->_interface->resetClip();
	Common::Rect dirtyRect;
	_engine->_renderer->renderIsoModel(destPos, x, y, ANGLE_0, _engine->_resources->_holomapPointModelPtr, dirtyRect);
	_engine->copyBlockPhys(dirtyRect);
}

void Holomap::renderHolomapVehicle(uint &frameNumber, ActorMoveStruct &move, AnimTimerDataStruct &animTimerData, BodyData &bodyData, AnimData &animData) {
	const int16 newAngle = move.getRealAngle(_engine->_lbaTime);
	if (move.numOfStep == 0) {
		_engine->_movements->setActorAngleSafe(ANGLE_0, -ANGLE_90, 500, &move);
	}

	if (_engine->_animations->setModelAnimation(frameNumber, animData, bodyData, &animTimerData)) {
		frameNumber++;
		if (frameNumber >= animData.getNumKeyframes()) {
			frameNumber = animData.getLoopFrame();
		}
	}
	const Common::Rect rect(0, _engine->height() - 280, 200, _engine->height() - 1);
	_engine->_renderer->setCameraPosition(rect.width() / 2, _engine->height() - 80, 128, 900, 900);
	_engine->_renderer->setCameraAngle(0, 0, 0, 60, 128, 0, distance(30000));
	_engine->_renderer->setLightVector(-60, 128, 0);
	// background of the vehicle
	_engine->_interface->drawFilledRect(rect, COLOR_BLACK);
	Common::Rect dummy;
	_engine->_renderer->renderIsoModel(0, 0, 0, ANGLE_0, newAngle, ANGLE_0, bodyData, dummy);
	_engine->copyBlockPhys(rect);
}

void Holomap::drawHolomapTrajectory(int32 trajectoryIndex) {
	if (_engine->isDemo()) {
		return;
	}
	debug("Draw trajectory index %i", trajectoryIndex);

	const Trajectory *data = _engine->_resources->getTrajectory(trajectoryIndex);
	if (data == nullptr) {
		warning("Failed to load trajectory data for index %i", trajectoryIndex);
		return;
	}

	_engine->exitSceneryView();
	_engine->_interface->resetClip();
	_engine->_screens->clearScreen();

	loadHolomapGFX();
	_engine->setPalette(_engine->_screens->_paletteRGBACustom);

	ScopedEngineFreeze timeFreeze(_engine);
	const int32 cameraPosX = _engine->width() / 2 + 80;
	const int32 cameraPosY = _engine->height() / 2;
	_engine->_renderer->setCameraPosition(cameraPosX, cameraPosY, 128, 1024, 1024);
	_engine->_renderer->setCameraAngle(0, 0, 0, data->pos.x, data->pos.y, data->pos.z, distance(zDistanceTrajectory));

	constexpr TwineResource holomapImageRes(Resources::HQR_RESS_FILE, RESSHQR_HOLOIMG);
	uint8 *holomapImagePtr = nullptr;
	const int32 holomapImageSize = HQR::getAllocEntry(&holomapImagePtr, holomapImageRes);
	if (holomapImageSize == 0) {
		error("Failed to load holomap image");
	}
	renderHolomapSurfacePolygons(holomapImagePtr, holomapImageSize);

	const Location &loc = _locations[data->locationIdx];
	renderHolomapPointModel(data->pos, loc.angleX, loc.angleY);

	ActorMoveStruct move;
	AnimTimerDataStruct animTimerData;
	AnimData animData;
	animData.loadFromHQR(Resources::HQR_RESS_FILE, data->getAnimation(), _engine->isLBA1());
	BodyData bodyData;
	bodyData.loadFromHQR(Resources::HQR_RESS_FILE, data->getModel(), _engine->isLBA1());
	uint frameNumber = 0;
	int32 frameTime = _engine->_lbaTime;
	int16 trajAnimFrameIdx = 0;

	int32 waterPaletteChangeTimer = 0;
	bool fadeInPalette = true;
	_engine->_input->enableKeyMap(holomapKeyMapId);
	for (;;) {
		FrameMarker frame(_engine);
		_engine->readKeys();
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			break;
		}

		if (!fadeInPalette && waterPaletteChangeTimer < _engine->_lbaTime) {
			// animate the water surface
			_engine->setPalette(HOLOMAP_PALETTE_INDEX, NUM_HOLOMAPCOLORS, &_paletteHolomap[3 * _holomapPaletteIndex++]);
			if (_holomapPaletteIndex == NUM_HOLOMAPCOLORS) {
				_holomapPaletteIndex = 0;
			}
			waterPaletteChangeTimer = _engine->_lbaTime + 3;
		}

		renderHolomapVehicle(frameNumber, move, animTimerData, bodyData, animData);

		// now render the holomap path
		_engine->_renderer->setCameraPosition(cameraPosX, cameraPosY, 128, 1024, 1024);
		_engine->_renderer->setCameraAngle(0, 0, 0, data->pos.x, data->pos.y, data->pos.z, distance(zDistanceTrajectory));
		_engine->_renderer->setLightVector(data->pos.x, data->pos.y, 0);

		// animate the path from point 1 to point 2 by rendering a point model on each position
		// on the globe every 40 timeunits
		if (frameTime + 40 <= _engine->_lbaTime) {
			frameTime = _engine->_lbaTime;
			int32 modelX;
			int32 modelY;
			if (trajAnimFrameIdx < data->numAnimFrames) {
				modelX = data->positions[trajAnimFrameIdx].x;
				modelY = data->positions[trajAnimFrameIdx].y;
			} else {
				if (data->numAnimFrames < trajAnimFrameIdx) {
					break;
				}
				modelX = loc.angleX;
				modelY = loc.angleY;
			}
			renderHolomapPointModel(data->pos, modelX, modelY);
			++trajAnimFrameIdx;
		}

		if (fadeInPalette) {
			fadeInPalette = false;
			//_engine->_screens->fadeToPal(_engine->_screens->_paletteRGBACustom);
		}
		++_engine->_lbaTime;
	}

	_engine->_screens->clearScreen();
	_engine->setPalette(_engine->_screens->_paletteRGBA);
	_engine->_gameState->initEngineProjections();
	_engine->_interface->loadClip();

	_engine->_text->initSceneTextBank();
	_engine->_input->enableKeyMap(mainKeyMapId);

	free(holomapImagePtr);
}

int32 Holomap::getNextHolomapLocation(int32 currentLocation, int32 dir) const {
	const int32 idx = currentLocation;
	for (int32 i = currentLocation + dir; i != idx; i += dir) {
		if (i < 0) {
			i = NUM_LOCATIONS - 1;
		} else {
			i %= NUM_LOCATIONS;
		}
		if (i == _engine->_scene->_currentSceneIdx || (_engine->_gameState->_holomapFlags[i] & HOLOMAP_ACTIVE) != 0u) {
			return i;
		}
	}
	return -1;
}

void Holomap::renderLocations(int xRot, int yRot, int zRot, bool lower) {
	int n = 0;
	DrawListStruct drawListArray[NUM_LOCATIONS];
	for (int locationIdx = 0; locationIdx < NUM_LOCATIONS; ++locationIdx) {
		if ((_engine->_gameState->_holomapFlags[locationIdx] & HOLOMAP_CAN_FOCUS) || locationIdx == _engine->_scene->_currentSceneIdx) {
			const Location &loc = _locations[locationIdx];
			_engine->_renderer->setBaseRotation(loc.angleX, loc.angleY, 0);
			const IVec3 &destPos = _engine->_renderer->getBaseRotationPosition(0, 0, loc.size + 1000);
			const IVec3 &destPos2 = _engine->_renderer->getBaseRotationPosition(0, 0, 1500);
			_engine->_renderer->setBaseRotation(xRot, yRot, zRot, true);
			_engine->_renderer->setBaseRotationPos(0, 0, distance(zDistanceHolomap));
			const IVec3 &destPos3 = _engine->_renderer->getBaseRotationPosition(destPos);
			const IVec3 &destPos4 = _engine->_renderer->getBaseRotationPosition(destPos2);
			bool visible;
			if (lower) {
				visible = destPos3.z <= destPos4.z;
			} else {
				visible = destPos4.z <= destPos3.z;
			}
			if (!visible) {
				continue;
			}
			uint8 flags = _engine->_gameState->_holomapFlags[locationIdx] & HOLOMAP_ARROW;
			if (locationIdx == _engine->_scene->_currentSceneIdx) {
				flags |= 2u; // model type
			}
			DrawListStruct &drawList = drawListArray[n];
			drawList.posValue = destPos3.z;
			drawList.actorIdx = locationIdx;
			drawList.type = flags;
			drawList.x = destPos.x;
			drawList.y = destPos.y;
			drawList.z = destPos.z;
			++n;
		}
	}
	_engine->_redraw->sortDrawingList(drawListArray, n);
	for (int i = 0; i < n; ++i) {
		const DrawListStruct &drawList = drawListArray[i];
		const uint16 flags = drawList.type;
		const BodyData *bodyData = nullptr;
		if (flags == 1u) {
			bodyData = &_engine->_resources->_holomapArrowPtr;
		} else if (flags == 2u) {
			bodyData = &_engine->_resources->_holomapTwinsenModelPtr;
		} else if (flags == 3u) {
			bodyData = &_engine->_resources->_holomapTwinsenArrowPtr;
		}
		if (bodyData != nullptr) {
			const int32 angleX = _locations[drawList.actorIdx].angleX;
			const int32 angleY = _locations[drawList.actorIdx].angleY;
			Common::Rect dummy;
			_engine->_renderer->renderIsoModel(drawList.x, drawList.y, drawList.z, angleX, angleY, ANGLE_0, *bodyData, dummy);
		}
	}
}

void Holomap::processHolomap() {
	ScopedEngineFreeze freeze(_engine);

	const int32 alphaLightTmp = _engine->_scene->_alphaLight;
	const int32 betaLightTmp = _engine->_scene->_betaLight;

	_engine->exitSceneryView();
	_engine->_gameState->initEngineProjections();

	_engine->_screens->fadeToBlack(_engine->_screens->_paletteRGBA);
	_engine->_sound->stopSamples();
	_engine->_interface->saveClip();
	_engine->_interface->resetClip();
	_engine->_screens->clearScreen();
	_engine->_screens->fadeToBlack(_engine->_screens->_paletteRGBA);

	loadHolomapGFX();

	_engine->_text->initTextBank(TextBankId::Inventory_Intro_and_Holomap);
	_engine->_text->setFontCrossColor(COLOR_9);
	const int32 cameraPosX = _engine->width() / 2;
	const int32 cameraPosY = scale(190);
	_engine->_renderer->setCameraPosition(cameraPosX, cameraPosY, 128, 1024, 1024);

	constexpr TwineResource holomapImageRes(Resources::HQR_RESS_FILE, RESSHQR_HOLOIMG);
	uint8 *holomapImagePtr = nullptr;
	const int32 holomapImageSize = HQR::getAllocEntry(&holomapImagePtr, holomapImageRes);
	if (holomapImageSize == 0) {
		error("Failed to load holomap image");
	}

	int32 currentLocation = _engine->_scene->_currentSceneIdx;
	_engine->_text->drawHolomapLocation(_locations[currentLocation].textIndex);

	int32 time = _engine->_lbaTime;
	int32 xRot = _locations[currentLocation].angleX;
	int32 yRot = _locations[currentLocation].angleY;
	bool rotate = false;
	bool redraw = true;
	int waterPaletteChangeTimer = 0;
	bool fadeInPalette = true;
	_engine->_input->enableKeyMap(holomapKeyMapId);
	for (;;) {
		FrameMarker frame(_engine);
		_engine->_input->readKeys();
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			break;
		}

		if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapPrev)) {
			const int32 nextLocation = getNextHolomapLocation(currentLocation, -1);
			if (nextLocation != -1 && currentLocation != nextLocation) {
				currentLocation = nextLocation;
				_engine->_text->drawHolomapLocation(_locations[currentLocation].textIndex);
				time = _engine->_lbaTime;
				rotate = true;
			}
		} else if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapNext)) {
			const int32 nextLocation = getNextHolomapLocation(currentLocation, 1);
			if (nextLocation != -1 && currentLocation != nextLocation) {
				currentLocation = nextLocation;
				_engine->_text->drawHolomapLocation(_locations[currentLocation].textIndex);
				time = _engine->_lbaTime;
				rotate = true;
			}
		}

		if (_engine->_input->isActionActive(TwinEActionType::HolomapDown)) {
			xRot += ANGLE_2;
			rotate = true;
			time = _engine->_lbaTime;
		} else if (_engine->_input->isActionActive(TwinEActionType::HolomapUp)) {
			xRot -= ANGLE_2;
			rotate = true;
			time = _engine->_lbaTime;
		}

		if (_engine->_input->isActionActive(TwinEActionType::HolomapRight)) {
			yRot += ANGLE_2;
			rotate = true;
			time = _engine->_lbaTime;
		} else if (_engine->_input->isActionActive(TwinEActionType::HolomapLeft)) {
			yRot -= ANGLE_2;
			rotate = true;
			time = _engine->_lbaTime;
		}

		if (rotate) {
			const int32 dt = _engine->_lbaTime - time;
			xRot = _engine->_collision->getAverageValue(ClampAngle(xRot), _locations[currentLocation].angleX, 75, dt);
			yRot = _engine->_collision->getAverageValue(ClampAngle(yRot), _locations[currentLocation].angleY, 75, dt);
			redraw = true;
		}

		if (!fadeInPalette && waterPaletteChangeTimer < _engine->_lbaTime) {
			// animate the water surface
			_engine->setPalette(HOLOMAP_PALETTE_INDEX, NUM_HOLOMAPCOLORS, &_paletteHolomap[3 * _holomapPaletteIndex++]);
			if (_holomapPaletteIndex == NUM_HOLOMAPCOLORS) {
				_holomapPaletteIndex = 0;
			}
			waterPaletteChangeTimer = _engine->_lbaTime + 3;
			redraw = true;
		}

		if (redraw) {
			redraw = false;
			const Common::Rect &rect = _engine->centerOnScreenX(scale(300), 0, scale(330));
			_engine->_interface->drawFilledRect(rect, COLOR_BLACK);
			_engine->_renderer->setBaseRotation(xRot, yRot, 0, true);
			_engine->_renderer->setLightVector(xRot, yRot, 0);
			renderLocations(xRot, yRot, 0, false);
			_engine->_renderer->setBaseRotation(xRot, yRot, 0, true);
			_engine->_renderer->setBaseRotationPos(0, 0, distance(zDistanceHolomap));
			renderHolomapSurfacePolygons(holomapImagePtr, holomapImageSize);
			renderLocations(xRot, yRot, 0, true);
			drawHolomapText(_engine->width() / 2, 25, "HoloMap");
			if (rotate) {
				const Common::Rect &targetRect = _engine->centerOnScreen(40, 40);
				_engine->_menu->drawRectBorders(targetRect.left, cameraPosY - 20, targetRect.right, cameraPosY + 20);
			}
		}

		if (rotate && xRot == _locations[currentLocation].angleX && yRot == _locations[currentLocation].angleY) {
			rotate = false;
		}

		++_engine->_lbaTime;

		if (fadeInPalette) {
			fadeInPalette = false;
			_engine->_screens->fadeToPal(_engine->_screens->_paletteRGBACustom);
		}
	}

	_engine->_screens->clearScreen();
	_engine->_text->_drawTextBoxBackground = true;
	_engine->setPalette(_engine->_screens->_paletteRGBA);
	_engine->_scene->_alphaLight = alphaLightTmp;
	_engine->_scene->_betaLight = betaLightTmp;

	_engine->_gameState->initEngineProjections();
	_engine->_interface->loadClip();

	_engine->_input->enableKeyMap(mainKeyMapId);
	_engine->_text->initSceneTextBank();

	free(holomapImagePtr);
}

const char *Holomap::getLocationName(int index) const {
	assert(index >= 0 && index <= ARRAYSIZE(_locations));
	return _locations[index].name;
}

} // namespace TwinE
