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

#include "twine/holomap_v1.h"
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

// these are lba1 specific
#define HOLOMAP_ARROW (1 << 0)
#define HOLOMAP_VISITED (1 << 1)
#define HOLOMAP_UNK3 (1 << 2)
#define HOLOMAP_UNK4 (1 << 3)
#define HOLOMAP_UNK5 (1 << 4)
#define HOLOMAP_UNK6 (1 << 5)
#define HOLOMAP_CUBE_DONE (1 << 6)
#define HOLOMAP_CAN_FOCUS (1 << 7)
#define HOLOMAP_ACTIVE (HOLOMAP_CAN_FOCUS | HOLOMAP_ARROW)

static const float ZOOM_BIG_HOLO = 9500.0f;
static const float zDistanceTrajectory = 5300.0f;
static const int SIZE_CURSOR = 20;

int32 HolomapV1::distance(float distance) const {
	const float w = (float)_engine->width() / (float)_engine->originalWidth();
	const float h = (float)_engine->height() / (float)_engine->originalHeight();
	const float f = MIN<float>(w, h);
	return (int32)(distance / f);
}

int32 HolomapV1::scale(float val) const {
	const float w = (float)_engine->width() / (float)_engine->originalWidth();
	const float h = (float)_engine->height() / (float)_engine->originalHeight();
	const float f = MIN<float>(w, h);
	return (int32)(val * f);
}

bool HolomapV1::loadLocations() {
	uint8 *locationsPtr = nullptr;
	const int32 locationsSize = HQR::getAllocEntry(&locationsPtr, Resources::HQR_RESS_FILE, RESSHQR_HOLOARROWINFO);
	if (locationsSize == 0) {
		warning("Could not find holomap locations at index %i in %s", RESSHQR_HOLOARROWINFO, Resources::HQR_RESS_FILE);
		return false;
	}

	Common::MemoryReadStream stream(locationsPtr, locationsSize, DisposeAfterUse::YES);
	_numHoloPos = locationsSize / 8;
	if (_numHoloPos > _engine->numHoloPos()) {
		warning("Amount of locations (%i) exceeds the maximum of %i", _numHoloPos, _engine->numHoloPos());
		return false;
	}

	_engine->_text->initDial(TextBankId::Inventory_Intro_and_Holomap);
	for (int32 i = 0; i < _numHoloPos; i++) {
		_listHoloPos[i].alpha = stream.readSint16LE();
		_listHoloPos[i].beta = stream.readSint16LE();
		_listHoloPos[i].size = stream.readSint16LE();
		_listHoloPos[i].mess = (TextId)stream.readSint16LE();

		if (_engine->_text->getMenuText(_listHoloPos[i].mess, _listHoloPos[i].name, sizeof(_listHoloPos[i].name))) {
			debug(2, "Scene %i: %s", i, _listHoloPos[i].name);
			continue;
		}
		debug(2, "Could not get location text for index %i", i);
	}
	return true;
}

bool HolomapV1::setHoloPos(int32 locationIdx) {
	assert(locationIdx >= 0 && locationIdx < _engine->numHoloPos());
	if (_engine->isLBA1()) {
		_engine->_gameState->_holomapFlags[locationIdx] = HOLOMAP_ACTIVE;
		return true;
	}
	// TODO: lba2
	_engine->_gameState->_holomapFlags[locationIdx] = HOLOMAP_ACTIVE | HOLOMAP_VISITED;
	return true;
}

void HolomapV1::clrHoloPos(int32 locationIdx) {
	assert(locationIdx >= 0 && locationIdx <= ARRAYSIZE(_engine->_gameState->_holomapFlags));
	_engine->_gameState->_holomapFlags[locationIdx] &= ~HOLOMAP_ACTIVE;
	_engine->_gameState->_holomapFlags[locationIdx] |= HOLOMAP_CUBE_DONE;
}

void HolomapV1::initHoloDatas() {
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

	computeCoorMapping();

	Common::SeekableReadStream *surfaceStream = HQR::makeReadStream(TwineResource(Resources::HQR_RESS_FILE, RESSHQR_HOLOSURFACE));
	if (surfaceStream == nullptr) {
		error("Failed to load holomap surface");
	}
	computeCoorGlobe(surfaceStream);
	delete surfaceStream;
	_holomapPaletteIndex = 0;
}

void HolomapV1::computeCoorGlobe(Common::SeekableReadStream *holomapSurfaceStream) {
	int holomapSurfaceArrayIdx = 0;
	_engine->_renderer->setAngleCamera(0, 0, 0);
	for (int alpha = -LBAAngles::ANGLE_90; alpha <= LBAAngles::ANGLE_90; alpha += LBAAngles::ANGLE_11_25) {
		const int32 rot = holomapSurfaceStream->readByte();
		holomapSurfaceStream->seek(-1, SEEK_CUR);
		for (int beta = 0; beta < LBAAngles::ANGLE_360; beta += LBAAngles::ANGLE_11_25) {
			const int32 normal = 1000 + holomapSurfaceStream->readByte() * 2;
			const IVec2 &rotVec = _engine->_renderer->rotate(normal, 0, alpha);
			const IVec2 &rotVec2 = _engine->_renderer->rotate(rotVec.x, 0, beta);
			const IVec3 &rotVec3 = _engine->_renderer->worldRotatePoint(IVec3(rotVec2.x, rotVec.y, rotVec2.y));

			_holomapSurface[holomapSurfaceArrayIdx].x = rotVec3.x;
			_holomapSurface[holomapSurfaceArrayIdx].y = rotVec3.y;
			_holomapSurface[holomapSurfaceArrayIdx].z = rotVec3.z;
			++holomapSurfaceArrayIdx;
		}
		const int32 normal = 1000 + rot * 2;
		const IVec2 &rotVec = _engine->_renderer->rotate(normal, 0, alpha);
		const IVec2 &rotVec2 = _engine->_renderer->rotate(rotVec.x, 0, 0);
		const IVec3 &rotVec3 = _engine->_renderer->worldRotatePoint(IVec3(rotVec2.x, rotVec.y, rotVec2.y));
		_holomapSurface[holomapSurfaceArrayIdx].x = rotVec3.x;
		_holomapSurface[holomapSurfaceArrayIdx].y = rotVec3.y;
		_holomapSurface[holomapSurfaceArrayIdx].z = rotVec3.z;
		++holomapSurfaceArrayIdx;
	}
	assert(holomapSurfaceStream->eos());
}

void HolomapV1::computeCoorMapping() {
	int projectedIndex = 0;
	for (int32 alpha = -LBAAngles::ANGLE_90; alpha <= LBAAngles::ANGLE_90; alpha += LBAAngles::ANGLE_11_25) {
		for (int32 beta = 0; beta < LBAAngles::ANGLE_360; beta += LBAAngles::ANGLE_11_25) {
			_projectedSurfacePositions[projectedIndex].x2 = _engine->_screens->lerp(0, 255 * LBAAngles::ANGLE_90 + 255, LBAAngles::ANGLE_360 - 1, beta);
			if (alpha == LBAAngles::ANGLE_90) {
				_projectedSurfacePositions[projectedIndex].y2 = 255 * LBAAngles::ANGLE_90 + 255;
			} else {
				_projectedSurfacePositions[projectedIndex].y2 = ((alpha + LBAAngles::ANGLE_90) * LBAAngles::ANGLE_90) / 2;
			}
			++projectedIndex;
		}
		_projectedSurfacePositions[projectedIndex].x2 = 255 * LBAAngles::ANGLE_90 + 255;
		if (alpha == LBAAngles::ANGLE_90) {
			_projectedSurfacePositions[projectedIndex].y2 = 255 * LBAAngles::ANGLE_90 + 255;
		} else {
			_projectedSurfacePositions[projectedIndex].y2 = ((alpha + LBAAngles::ANGLE_90) * LBAAngles::ANGLE_90) / 2;
		}
		++projectedIndex;
	}
}

void HolomapV1::computeGlobeProj() {
	int holomapSortArrayIdx = 0;
	int holomapSurfaceArrayIdx = 0;
	_projectedSurfaceIndex = 0;
	for (int32 alpha = -LBAAngles::ANGLE_90; alpha <= LBAAngles::ANGLE_90; alpha += LBAAngles::ANGLE_11_25) {
		for (int32 beta = 0; beta < LBAAngles::ANGLE_11_25; ++beta) {
			IVec3 *vec = &_holomapSurface[holomapSurfaceArrayIdx++];
			const IVec3 &destPos = _engine->_renderer->worldRotatePoint(*vec);
			if (alpha != LBAAngles::ANGLE_90) {
				_holomapSort[holomapSortArrayIdx].z = (int16)destPos.z;
				_holomapSort[holomapSortArrayIdx].projectedPosIdx = _projectedSurfaceIndex;
				++holomapSortArrayIdx;
			}
			const IVec3 &projPos = _engine->_renderer->projectPoint(destPos);
			_projectedSurfacePositions[_projectedSurfaceIndex].x1 = projPos.x;
			_projectedSurfacePositions[_projectedSurfaceIndex].y1 = projPos.y;
			++_projectedSurfaceIndex;
		}
		IVec3 *vec = &_holomapSurface[holomapSurfaceArrayIdx++];
		const IVec3 &destPos = _engine->_renderer->worldRotatePoint(*vec);
		const IVec3 &projPos = _engine->_renderer->projectPoint(destPos);
		_projectedSurfacePositions[_projectedSurfaceIndex].x1 = projPos.x;
		_projectedSurfacePositions[_projectedSurfaceIndex].y1 = projPos.y;
		++_projectedSurfaceIndex;
	}
	assert(holomapSortArrayIdx == ARRAYSIZE(_holomapSort));
	assert(holomapSurfaceArrayIdx == ARRAYSIZE(_holomapSurface));
	assert(_projectedSurfaceIndex == ARRAYSIZE(_projectedSurfacePositions));
	Common::sort(_holomapSort, _holomapSort + ARRAYSIZE(_holomapSort), [](const HolomapSort &a, const HolomapSort &b) { return a.z < b.z; });
}

#define SURFACE_POS_OFFSET ((LBAAngles::ANGLE_360 / LBAAngles::ANGLE_11_25) + 1)
void HolomapV1::drawHoloMap(uint8 *holomapImage, uint32 holomapImageSize) {
	computeGlobeProj();
	for (int32 i = 0; i < ARRAYSIZE(_holomapSort); ++i) {
		assert(_holomapSort[i].projectedPosIdx + 34 < _projectedSurfaceIndex);
		const HolomapProjectedPos &pos1 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 0];
		const HolomapProjectedPos &pos2 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 0 + SURFACE_POS_OFFSET];
		const HolomapProjectedPos &pos3 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 1];
		ComputedVertex vertexCoordinates[3];
		vertexCoordinates[0].x = (int16)pos1.x1;
		vertexCoordinates[0].y = (int16)pos1.y1;
		vertexCoordinates[1].x = (int16)pos2.x1;
		vertexCoordinates[1].y = (int16)pos2.y1;
		vertexCoordinates[2].x = (int16)pos3.x1;
		vertexCoordinates[2].y = (int16)pos3.y1;
		if (isPolygonVisible(vertexCoordinates)) {
			ComputedVertex textureCoordinates[3];
			textureCoordinates[0].x = (int16)pos1.x2;
			textureCoordinates[0].y = (int16)pos1.y2;
			textureCoordinates[1].x = (int16)pos2.x2;
			textureCoordinates[1].y = (int16)pos2.y2;
			textureCoordinates[2].x = (int16)pos3.x2;
			textureCoordinates[2].y = (int16)pos3.y2;
			_engine->_renderer->asmTexturedTriangleNoClip(vertexCoordinates, textureCoordinates, holomapImage, holomapImageSize);
		}
		const HolomapProjectedPos &pos4 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 0 + SURFACE_POS_OFFSET];
		const HolomapProjectedPos &pos5 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 1 + SURFACE_POS_OFFSET];
		const HolomapProjectedPos &pos6 = _projectedSurfacePositions[_holomapSort[i].projectedPosIdx + 1];
		vertexCoordinates[0].x = (int16)pos4.x1;
		vertexCoordinates[0].y = (int16)pos4.y1;
		vertexCoordinates[1].x = (int16)pos5.x1;
		vertexCoordinates[1].y = (int16)pos5.y1;
		vertexCoordinates[2].x = (int16)pos6.x1;
		vertexCoordinates[2].y = (int16)pos6.y1;
		if (isPolygonVisible(vertexCoordinates)) {
			ComputedVertex textureCoordinates[3];
			textureCoordinates[0].x = (int16)pos4.x2;
			textureCoordinates[0].y = (int16)pos4.y2;
			textureCoordinates[1].x = (int16)pos5.x2;
			textureCoordinates[1].y = (int16)pos5.y2;
			textureCoordinates[2].x = (int16)pos6.x2;
			textureCoordinates[2].y = (int16)pos6.y2;
			_engine->_renderer->asmTexturedTriangleNoClip(vertexCoordinates, textureCoordinates, holomapImage, holomapImageSize);
		}
	}
}

void HolomapV1::drawHolomapText(int32 centerx, int32 top, const char *title) {
	const int32 size = _engine->_text->sizeFont(title);
	const int32 x = centerx - size / 2;
	const int32 y = top;
	_engine->_text->setFontColor(COLOR_WHITE);
	_engine->_text->drawText(x, y, title);
}

void HolomapV1::drawHoloObj(const IVec3 &angle, int32 alpha, int32 beta) {
	_engine->_renderer->setAngleCamera(alpha, beta, 0);
	const IVec3 &m = _engine->_renderer->worldRotatePoint(IVec3(0, 0, 1000));
	_engine->_renderer->setFollowCamera(0, 0, 0, angle.x, angle.y, angle.z, distance(zDistanceTrajectory));
	_engine->_interface->unsetClip();
	Common::Rect dirtyRect;
	_engine->_renderer->renderIsoModel(m, alpha, beta, LBAAngles::ANGLE_0, _engine->_resources->_holomapPointModelPtr, dirtyRect);
	_engine->copyBlockPhys(dirtyRect);
}

void HolomapV1::renderHolomapVehicle(uint &frameNumber, ActorMoveStruct &move, AnimTimerDataStruct &animTimerData, BodyData &bodyData, AnimData &animData) {
	const int16 newAngle = move.getRealAngle(_engine->timerRef);
	if (move.timeValue == 0) {
		_engine->_movements->initRealAngle(LBAAngles::ANGLE_0, -LBAAngles::ANGLE_90, 500, &move);
	}

	if (_engine->_animations->doSetInterAnimObjet(frameNumber, animData, bodyData, &animTimerData)) {
		frameNumber++;
		if (frameNumber >= animData.getNumKeyframes()) {
			frameNumber = animData.getLoopFrame();
		}
	}
	const Common::Rect rect(0, _engine->height() - 280, 200, _engine->height() - 1);
	_engine->_renderer->setProjection(rect.width() / 2, _engine->height() - 80, 128, 900, 900);
	_engine->_renderer->setFollowCamera(0, 0, 0, 60, 128, 0, distance(30000));
	_engine->_renderer->setLightVector(-60, 128, 0);
	// background of the vehicle
	_engine->_interface->box(rect, COLOR_BLACK);
	Common::Rect dummy;
	_engine->_renderer->affObjetIso(0, 0, 0, LBAAngles::ANGLE_0, newAngle, LBAAngles::ANGLE_0, bodyData, dummy);
	_engine->copyBlockPhys(rect);
}

void HolomapV1::drawHolomapTrajectory(int32 trajectoryIndex) {
	if (_engine->isDemo()) {
		return;
	}
	debug("Draw trajectory index %i", trajectoryIndex);

	const Trajectory *data = _engine->_resources->getTrajectory(trajectoryIndex);
	if (data == nullptr) {
		warning("Failed to load trajectory data for index %i", trajectoryIndex);
		return;
	}

	_engine->_interface->unsetClip();
	_engine->_screens->clearScreen();

	initHoloDatas();
	_engine->setPalette(_engine->_screens->_paletteRGBACustom);

	ScopedEngineFreeze timeFreeze(_engine);
	const int32 cameraPosX = _engine->width() / 2 + 80;
	const int32 cameraPosY = _engine->height() / 2;
	_engine->_renderer->setProjection(cameraPosX, cameraPosY, 128, 1024, 1024);
	_engine->_renderer->setFollowCamera(0, 0, 0, data->pos.x, data->pos.y, data->pos.z, distance(zDistanceTrajectory));

	constexpr TwineResource holomapImageRes(Resources::HQR_RESS_FILE, RESSHQR_HOLOIMG);
	uint8 *holomapImagePtr = nullptr;
	const int32 holomapImageSize = HQR::getAllocEntry(&holomapImagePtr, holomapImageRes);
	if (holomapImageSize == 0) {
		error("Failed to load holomap image");
	}
	drawHoloMap(holomapImagePtr, holomapImageSize);

	const Location &loc = _listHoloPos[data->locationIdx];
	drawHoloObj(data->pos, loc.alpha, loc.beta);

	ActorMoveStruct move;
	AnimTimerDataStruct animTimerData;
	AnimData animData;
	animData.loadFromHQR(Resources::HQR_RESS_FILE, data->getAnimation(), _engine->isLBA1());
	BodyData bodyData;
	bodyData.loadFromHQR(Resources::HQR_RESS_FILE, data->getModel(), _engine->isLBA1());
	uint frameNumber = 0;
	int32 frameTime = _engine->timerRef;
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

		if (!fadeInPalette && waterPaletteChangeTimer < _engine->timerRef) {
			// animate the water surface
			_engine->setPalette(HOLOMAP_PALETTE_INDEX, NUM_HOLOMAPCOLORS, &_paletteHolomap[3 * _holomapPaletteIndex++]);
			if (_holomapPaletteIndex == NUM_HOLOMAPCOLORS) {
				_holomapPaletteIndex = 0;
			}
			waterPaletteChangeTimer = _engine->timerRef + 3;
		}

		renderHolomapVehicle(frameNumber, move, animTimerData, bodyData, animData);

		// now render the holomap path
		_engine->_renderer->setProjection(cameraPosX, cameraPosY, 128, 1024, 1024);
		_engine->_renderer->setFollowCamera(0, 0, 0, data->pos.x, data->pos.y, data->pos.z, distance(zDistanceTrajectory));
		_engine->_renderer->setLightVector(data->pos.x, data->pos.y, 0);

		// animate the path from point 1 to point 2 by rendering a point model on each position
		// on the globe every 40 timeunits
		if (frameTime + 40 <= _engine->timerRef) {
			frameTime = _engine->timerRef;
			int32 modelX;
			int32 modelY;
			if (trajAnimFrameIdx < data->numAnimFrames) {
				modelX = data->positions[trajAnimFrameIdx].x;
				modelY = data->positions[trajAnimFrameIdx].y;
			} else {
				if (data->numAnimFrames < trajAnimFrameIdx) {
					break;
				}
				modelX = loc.alpha;
				modelY = loc.beta;
			}
			drawHoloObj(data->pos, modelX, modelY);
			++trajAnimFrameIdx;
		}

		if (fadeInPalette) {
			fadeInPalette = false;
			//_engine->_screens->fadeToPal(_engine->_screens->_paletteRGBACustom);
		}
		++_engine->timerRef;
		debugC(3, kDebugLevels::kDebugTime, "Holomap time: %i", _engine->timerRef);
	}

	_engine->_screens->clearScreen();
	_engine->setPalette(_engine->_screens->_paletteRGBA);
	_engine->_gameState->init3DGame();
	_engine->_interface->restoreClip();

	_engine->_text->initSceneTextBank();
	_engine->_input->enableKeyMap(mainKeyMapId);

	free(holomapImagePtr);
}

int32 HolomapV1::searchNextArrow(int32 num) const {
	const int maxLocations = _engine->numHoloPos();
	for (int32 n = num + 1; n < maxLocations; ++n) {
		if ((_engine->_gameState->_holomapFlags[n] & HOLOMAP_ACTIVE) != 0u) {
			return n;
		}
	}
	return -1;
}

int32 HolomapV1::searchPrevArrow(int32 num) const {
	int32 n;
	const int maxLocations = _engine->numHoloPos();

	if (num == -1) {
		num = maxLocations;
	}

	for (n = num - 1; n >= 0; n--) {
		if ((_engine->_gameState->_holomapFlags[n] & HOLOMAP_ACTIVE) != 0u) {
			return n;
		}
	}
	return -1;
}

void HolomapV1::drawListPos(int calpha, int cbeta, int cgamma, bool pos) {
	int nbobjets = 0;
	DrawListStruct listTri[MAX_HOLO_POS_2];
	const int numCube = _engine->_scene->_currentSceneIdx;
	const int maxHoloPos = _engine->numHoloPos();
	for (int n = 0; n < maxHoloPos; ++n) {
		if (!(_engine->_gameState->_holomapFlags[n] & HOLOMAP_CAN_FOCUS) && n != numCube) {
			continue;
		}
		const Location &ptrpos = _listHoloPos[n];
		_engine->_renderer->setAngleCamera(ptrpos.alpha, ptrpos.beta, 0);
		const IVec3 &m = _engine->_renderer->worldRotatePoint(IVec3(0, 0, 1000 + ptrpos.size));
		const IVec3 &m1 = _engine->_renderer->worldRotatePoint(IVec3(0, 0, 1500));
		_engine->_renderer->setInverseAngleCamera(calpha, cbeta, cgamma);
		_engine->_renderer->setCameraRotation(0, 0, distance(ZOOM_BIG_HOLO));

		const IVec3 &destPos3 = _engine->_renderer->worldRotatePoint(m);
		const IVec3 &destPos4 = _engine->_renderer->worldRotatePoint(m1);

		if (!pos) {
			if (destPos4.z > destPos3.z) {
				continue;
			}
		} else {
			if (destPos4.z < destPos3.z) {
				continue;
			}
		}
		uint32 t = (uint32)_engine->_gameState->_holomapFlags[n] & HOLOMAP_ARROW;
		if (n == numCube) {
			t |= HOLOMAP_VISITED; // model type
		}
		DrawListStruct &drawList = listTri[nbobjets];
		drawList.z = destPos3.z;
		drawList.numObj = n;
		drawList.num = t;
		drawList.xw = m.x;
		drawList.yw = m.y;
		drawList.zw = m.z;
		++nbobjets;
	}
	_engine->_redraw->sortDrawingList(listTri, nbobjets);
	for (int i = 0; i < nbobjets; ++i) {
		const DrawListStruct &drawList = listTri[i];
		const uint32 flags = drawList.num;
		const BodyData *ptr3do = nullptr;
		if (flags == HOLOMAP_ARROW) {
			ptr3do = &_engine->_resources->_holomapArrowPtr;
		} else if (flags == HOLOMAP_VISITED) {
			ptr3do = &_engine->_resources->_holomapTwinsenModelPtr;
		} else if (flags == (HOLOMAP_ARROW | HOLOMAP_VISITED)) {
			ptr3do = &_engine->_resources->_holomapTwinsenArrowPtr;
		}
		if (ptr3do != nullptr) {
			const int32 alpha = _listHoloPos[drawList.numObj].alpha;
			const int32 beta = _listHoloPos[drawList.numObj].beta;
			Common::Rect dummy;
			// first scene with twinsen model: x = 0, y = -497, z -764, a 432, b: 172
			_engine->_renderer->affObjetIso(drawList.xw, drawList.yw, drawList.zw, alpha, beta, LBAAngles::ANGLE_0, *ptr3do, dummy);
		}
	}
}

void HolomapV1::holoMap() {
	const int32 alphaLightTmp = _engine->_scene->_alphaLight;
	const int32 betaLightTmp = _engine->_scene->_betaLight;

	_engine->_gameState->init3DGame();

	_engine->_screens->fadeToBlack(_engine->_screens->_paletteRGBA);
	_engine->_sound->stopSamples();
	_engine->_interface->unsetClip();
	_engine->_screens->clearScreen();
	_engine->_screens->fadeToBlack(_engine->_screens->_paletteRGBA);

	initHoloDatas();

	const int32 cameraPosX = _engine->width() / 2;
	const int32 cameraPosY = scale(190);
	_engine->_renderer->setProjection(cameraPosX, cameraPosY, 128, 1024, 1024);

	_engine->_text->initDial(TextBankId::Inventory_Intro_and_Holomap);
	_engine->_text->setFontCrossColor(COLOR_9);

	constexpr TwineResource holomapImageRes(Resources::HQR_RESS_FILE, RESSHQR_HOLOIMG);
	uint8 *holomapImagePtr = nullptr;
	const int32 holomapImageSize = HQR::getAllocEntry(&holomapImagePtr, holomapImageRes);
	if (holomapImageSize == 0) {
		error("Failed to load holomap image");
	}

	int32 current = _engine->_scene->_currentSceneIdx;
	_engine->_text->drawHolomapLocation(_listHoloPos[current].mess);

	int32 otimer = _engine->timerRef;
	int32 dalpha = ClampAngle(_listHoloPos[current].alpha);
	int32 dbeta = ClampAngle(_listHoloPos[current].beta);
	int32 calpha = dalpha;
	int32 cbeta = dbeta;
	int32 cgamma = 0;
	int32 oalpha = dalpha;
	int32 obeta = dbeta;
	bool automove = false;
	bool flagredraw = true;
	int waterPaletteChangeTimer = 0;
	bool flagpal = true;
	_engine->_input->enableKeyMap(holomapKeyMapId);
	for (;;) {
		FrameMarker frame(_engine);
		_engine->_input->readKeys();
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			break;
		}

		if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapPrev)) {
			current = searchPrevArrow(current);
			if (current == -1) {
				current = _engine->_scene->_currentSceneIdx;
			}
			_engine->_text->drawHolomapLocation(_listHoloPos[current].mess);
			oalpha = calpha;
			obeta = cbeta;
			otimer = _engine->timerRef;
			dalpha = _listHoloPos[current].alpha;
			dbeta = _listHoloPos[current].beta;
			automove = true;
			flagredraw = true;
		} else if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapNext)) {
			current = searchNextArrow(current);
			if (current == -1) {
				current = _engine->_scene->_currentSceneIdx;
			}
			_engine->_text->drawHolomapLocation(_listHoloPos[current].mess);
			oalpha = calpha;
			obeta = cbeta;
			otimer = _engine->timerRef;
			dalpha = _listHoloPos[current].alpha;
			dbeta = _listHoloPos[current].beta;
			automove = true;
			flagredraw = true;
		}

		if (!automove) {
			if (_engine->_input->isActionActive(TwinEActionType::HolomapDown)) {
				calpha += LBAAngles::ANGLE_2;
				calpha = ClampAngle(calpha);
				flagredraw = true;
			} else if (_engine->_input->isActionActive(TwinEActionType::HolomapUp)) {
				calpha -= LBAAngles::ANGLE_2;
				calpha = ClampAngle(calpha);
				flagredraw = true;
			}
			if (_engine->_input->isActionActive(TwinEActionType::HolomapRight)) {
				cbeta += LBAAngles::ANGLE_2;
				cbeta = ClampAngle(cbeta);
				flagredraw = true;
			} else if (_engine->_input->isActionActive(TwinEActionType::HolomapLeft)) {
				cbeta -= LBAAngles::ANGLE_2;
				cbeta = ClampAngle(cbeta);
				flagredraw = true;
			}
		}

		if (automove) {
			const int32 dt = _engine->timerRef - otimer;
			calpha = _engine->_collision->boundRuleThree(oalpha, dalpha, 75, dt);
			cbeta = _engine->_collision->boundRuleThree(obeta, dbeta, 75, dt);
			flagredraw = true;
		}

		if (!flagpal && waterPaletteChangeTimer < _engine->timerRef) {
			// animate the water surface
			_engine->setPalette(HOLOMAP_PALETTE_INDEX, NUM_HOLOMAPCOLORS, &_paletteHolomap[3 * _holomapPaletteIndex++]);
			if (_holomapPaletteIndex == NUM_HOLOMAPCOLORS) {
				_holomapPaletteIndex = 0;
			}
			waterPaletteChangeTimer = _engine->timerRef + 3;
			flagredraw = true;
		}

		if (flagredraw) {
			flagredraw = false;
			const Common::Rect &rect = _engine->centerOnScreenX(scale(300), 0, scale(330));
			// clip reduces the bad effect of https://bugs.scummvm.org/ticket/12074
			// but it's not part of the original code
			_engine->_interface->memoClip();
			_engine->_interface->setClip(rect);
			_engine->_interface->box(rect, COLOR_BLACK);
			_engine->_renderer->setInverseAngleCamera(calpha, cbeta, cgamma);
			_engine->_renderer->setLightVector(calpha, cbeta, 0);
			drawListPos(calpha, cbeta, cgamma, false);
			_engine->_renderer->setInverseAngleCamera(calpha, cbeta, cgamma);
			_engine->_renderer->setCameraRotation(0, 0, distance(ZOOM_BIG_HOLO));
			drawHoloMap(holomapImagePtr, holomapImageSize);
			drawListPos(calpha, cbeta, cgamma, true);
			_engine->_interface->restoreClip();
			drawHolomapText(_engine->width() / 2, 25, "HoloMap");
			if (automove) {
				// draw cursor
				const Common::Rect &targetRect = _engine->centerOnScreen(SIZE_CURSOR * 2, SIZE_CURSOR * 2);
				_engine->_menu->drawRectBorders(targetRect.left, cameraPosY - 20, targetRect.right, cameraPosY + 20, 15, 15);
			}
		}

		if (automove && dalpha == calpha && dbeta == cbeta) {
			automove = false;
		}

		++_engine->timerRef;
		debugC(3, kDebugLevels::kDebugTime, "Holomap time: %i", _engine->timerRef);

		if (flagpal) {
			flagpal = false;
			_engine->_screens->fadeToPal(_engine->_screens->_paletteRGBACustom);
		}
	}

	_engine->_screens->clearScreen();
	_engine->_text->_flagMessageShade = true;
	_engine->setPalette(_engine->_screens->_paletteRGBA);
	_engine->_scene->_alphaLight = alphaLightTmp;
	_engine->_scene->_betaLight = betaLightTmp;

	_engine->_gameState->init3DGame();
	_engine->_interface->restoreClip();

	_engine->_input->enableKeyMap(mainKeyMapId);
	_engine->_text->initSceneTextBank();

	free(holomapImagePtr);
}

const char *HolomapV1::getLocationName(int index) const {
	assert(index >= 0 && index <= ARRAYSIZE(_listHoloPos));
	return _listHoloPos[index].name;
}

} // namespace TwinE
