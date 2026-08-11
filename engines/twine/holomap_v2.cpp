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

#include "twine/holomap_v2.h"
#include "common/algorithm.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "twine/audio/sound.h"
#include "twine/menu/interface.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/scene.h"
#include "twine/shared.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

// HQR indices in holomap.hqr for LBA2
#define HQR_COORMAPP_HMM 0
#define HQR_TWINSUN_HMT 1
#define HQR_TWINSUN_HMG 2

#define HOLO_FLAG_ACTIVE 1
#define HOLO_FLAG_ASKED 2

int32 HolomapV2::distance(float dist) const {
	const float w = (float)_engine->width() / (float)_engine->originalWidth();
	const float h = (float)_engine->height() / (float)_engine->originalHeight();
	const float f = MIN<float>(w, h);
	return (int32)(dist / f);
}

int32 HolomapV2::scale(float val) const {
	const float w = (float)_engine->width() / (float)_engine->originalWidth();
	const float h = (float)_engine->height() / (float)_engine->originalHeight();
	const float f = MIN<float>(w, h);
	return (int32)(val * f);
}

bool HolomapV2::setHoloPos(int32 locationIdx) {
	assert(locationIdx >= 0 && locationIdx < HOLO_MAX_CUBE);
	Location &loc = _locations[HOLO_MAX_OBJECTIF + locationIdx];
	if (loc.FlagHolo & HOLO_FLAG_ASKED) {
		return false; // already asked
	}
	loc.FlagHolo |= HOLO_FLAG_ACTIVE;
	return true;
}

void HolomapV2::clrHoloPos(int32 locationIdx) {
	assert(locationIdx >= 0 && locationIdx < HOLO_MAX_CUBE);
	Location &loc = _locations[HOLO_MAX_OBJECTIF + locationIdx];
	loc.FlagHolo &= ~HOLO_FLAG_ACTIVE;
	loc.FlagHolo |= HOLO_FLAG_ASKED;
}

bool HolomapV2::loadLocations() {
	return HQR::getEntry((uint8 *)_locations, Resources::HQR_HOLOMAP_FILE, RESSHQR_ARROWBIN) != 0;
}

const char *HolomapV2::getLocationName(int index) const {
	return "";
}

void HolomapV2::computeCoorMapping() {
	int idx = 0;
	for (int32 alpha = -LBAAngles::ANGLE_90; alpha <= LBAAngles::ANGLE_90; alpha += HOLO_STEP_ANGLE) {
		for (int32 beta = 0; beta < LBAAngles::ANGLE_360; beta += HOLO_STEP_ANGLE) {
			_projectedSurfacePositions[idx].x2 = (uint16)ruleThree32(0, 255 * 256 + 255, LBAAngles::ANGLE_360 - 1, beta);
			if (alpha == LBAAngles::ANGLE_90) {
				_projectedSurfacePositions[idx].y2 = 255 * 256 + 255;
			} else {
				_projectedSurfacePositions[idx].y2 = (uint16)(((alpha + LBAAngles::ANGLE_90) * 256) / 2);
			}
			++idx;
		}
		// wrap column
		_projectedSurfacePositions[idx].x2 = 255 * 256 + 255;
		if (alpha == LBAAngles::ANGLE_90) {
			_projectedSurfacePositions[idx].y2 = 255 * 256 + 255;
		} else {
			_projectedSurfacePositions[idx].y2 = (uint16)(((alpha + LBAAngles::ANGLE_90) * 256) / 2);
		}
		++idx;
	}
}

void HolomapV2::computeCoorGlobe(Common::SeekableReadStream *surfaceStream) {
	int idx = 0;
	_engine->_renderer->setAngleCamera(0, 0, 0);
	for (int32 alpha = -LBAAngles::ANGLE_90; alpha <= LBAAngles::ANGLE_90; alpha += HOLO_STEP_ANGLE) {
		const int64 startPos = surfaceStream->pos();
		const int32 firstAlt = surfaceStream->readByte();
		surfaceStream->seek(startPos, SEEK_SET);

		for (int32 beta = 0; beta < LBAAngles::ANGLE_360; beta += HOLO_STEP_ANGLE) {
			const int32 alt = surfaceStream->readByte();
			const int32 normal = HOLO_RAYON_PLANET + alt * 2;
			const IVec2 &rotVec = _engine->_renderer->rotate(normal, 0, alpha);
			const IVec2 &rotVec2 = _engine->_renderer->rotate(rotVec.x, 0, beta);
			const IVec3 &rotVec3 = _engine->_renderer->worldRotatePoint(IVec3(rotVec2.x, rotVec.y, rotVec2.y));
			_holomapSurface[idx].x = rotVec3.x;
			_holomapSurface[idx].y = rotVec3.y;
			_holomapSurface[idx].z = rotVec3.z;
			++idx;
		}
		// wrap: use first altitude of this row
		const int32 normal = HOLO_RAYON_PLANET + firstAlt * 2;
		const IVec2 &rotVec = _engine->_renderer->rotate(normal, 0, alpha);
		const IVec2 &rotVec2 = _engine->_renderer->rotate(rotVec.x, 0, 0);
		const IVec3 &rotVec3 = _engine->_renderer->worldRotatePoint(IVec3(rotVec2.x, rotVec.y, rotVec2.y));
		_holomapSurface[idx].x = rotVec3.x;
		_holomapSurface[idx].y = rotVec3.y;
		_holomapSurface[idx].z = rotVec3.z;
		++idx;
	}
}

void HolomapV2::computeGlobeProj() {
	int sortIdx = 0;
	int surfIdx = 0;
	int projIdx = 0;
	for (int32 alpha = -LBAAngles::ANGLE_90; alpha <= LBAAngles::ANGLE_90; alpha += HOLO_STEP_ANGLE) {
		for (int32 beta = 0; beta < HOLO_GLOBE_COLS; ++beta) {
			const IVec3 &destPos = _engine->_renderer->worldRotatePoint(_holomapSurface[surfIdx]);
			if (alpha != LBAAngles::ANGLE_90) {
				_holomapSort[sortIdx].z = (int16)destPos.z;
				_holomapSort[sortIdx].projectedPosIdx = projIdx;
				++sortIdx;
			}
			const IVec3 &projPos = _engine->_renderer->projectPoint(destPos);
			_projectedSurfacePositions[projIdx].x1 = (int16)projPos.x;
			_projectedSurfacePositions[projIdx].y1 = (int16)projPos.y;
			++projIdx;
			++surfIdx;
		}
	}
	Common::sort(_holomapSort, _holomapSort + HOLO_GLOBE_QUADS,
		[](const HolomapSort &a, const HolomapSort &b) { return a.z < b.z; });
}

void HolomapV2::drawHoloMap() {
	computeGlobeProj();
	for (int32 i = 0; i < HOLO_GLOBE_QUADS; ++i) {
		const int base = _holomapSort[i].projectedPosIdx;
		const HolomapProjectedPos &pos1 = _projectedSurfacePositions[base];
		const HolomapProjectedPos &pos2 = _projectedSurfacePositions[base + HOLO_GLOBE_COLS];
		const HolomapProjectedPos &pos3 = _projectedSurfacePositions[base + 1];

		ComputedVertex vertexCoordinates[3];
		vertexCoordinates[0].x = pos1.x1;
		vertexCoordinates[0].y = pos1.y1;
		vertexCoordinates[1].x = pos2.x1;
		vertexCoordinates[1].y = pos2.y1;
		vertexCoordinates[2].x = pos3.x1;
		vertexCoordinates[2].y = pos3.y1;
		if (isPolygonVisible(vertexCoordinates)) {
			ComputedVertex textureCoordinates[3];
			textureCoordinates[0].x = pos1.x2;
			textureCoordinates[0].y = pos1.y2;
			textureCoordinates[1].x = pos2.x2;
			textureCoordinates[1].y = pos2.y2;
			textureCoordinates[2].x = pos3.x2;
			textureCoordinates[2].y = pos3.y2;
			_engine->_renderer->asmTexturedTriangleNoClip(vertexCoordinates, textureCoordinates, _holomapImagePtr, _holomapImageSize);
		}

		const HolomapProjectedPos &pos4 = _projectedSurfacePositions[base + HOLO_GLOBE_COLS];
		const HolomapProjectedPos &pos5 = _projectedSurfacePositions[base + HOLO_GLOBE_COLS + 1];
		const HolomapProjectedPos &pos6 = _projectedSurfacePositions[base + 1];
		vertexCoordinates[0].x = pos4.x1;
		vertexCoordinates[0].y = pos4.y1;
		vertexCoordinates[1].x = pos5.x1;
		vertexCoordinates[1].y = pos5.y1;
		vertexCoordinates[2].x = pos6.x1;
		vertexCoordinates[2].y = pos6.y1;
		if (isPolygonVisible(vertexCoordinates)) {
			ComputedVertex textureCoordinates[3];
			textureCoordinates[0].x = pos4.x2;
			textureCoordinates[0].y = pos4.y2;
			textureCoordinates[1].x = pos5.x2;
			textureCoordinates[1].y = pos5.y2;
			textureCoordinates[2].x = pos6.x2;
			textureCoordinates[2].y = pos6.y2;
			_engine->_renderer->asmTexturedTriangleNoClip(vertexCoordinates, textureCoordinates, _holomapImagePtr, _holomapImageSize);
		}
	}
}

void HolomapV2::drawListHoloGlobe(bool frontFace) {
	struct SortEntry {
		int32 z;
		int32 numObj;
		int16 xw, yw, zw;
	};
	SortEntry sortList[HOLO_MAX_ARROW];
	int nbobjets = 0;

	const int numCube = _engine->_scene->_numCube;

	for (int n = HOLO_MAX_OBJECTIF; n < HOLO_MAX_ARROW; ++n) {
		if (!((_locations[n].FlagHolo & HOLO_FLAG_ACTIVE) || n == (HOLO_MAX_OBJECTIF + numCube))) {
			continue;
		}
		const Location &loc = _locations[n];
		if (loc.Planet != 0) { // only Twinsun for now
			continue;
		}

		_engine->_renderer->setAngleCamera(loc.Alpha, loc.Beta, 0);
		const IVec3 &m = _engine->_renderer->worldRotatePoint(IVec3(0, 0, HOLO_RAYON_PLANET + loc.Alt));
		const IVec3 &m1 = _engine->_renderer->worldRotatePoint(IVec3(0, 0, HOLO_RAYON_PLANET + 500));

		_engine->_renderer->setInverseAngleCamera(_holoAlpha, _holoBeta, _holoGamma);
		_engine->_renderer->setCameraRotation(0, 0, distance((float)_zoomPlanet));

		const IVec3 &destPos = _engine->_renderer->worldRotatePoint(m);
		const IVec3 &destPos2 = _engine->_renderer->worldRotatePoint(m1);

		if (!frontFace) {
			if (destPos2.z > destPos.z) continue;
		} else {
			if (destPos2.z < destPos.z) continue;
		}

		sortList[nbobjets].z = destPos.z;
		sortList[nbobjets].numObj = n;
		sortList[nbobjets].xw = (int16)m.x;
		sortList[nbobjets].yw = (int16)m.y;
		sortList[nbobjets].zw = (int16)m.z;
		++nbobjets;
	}

	if (nbobjets == 0) return;

	// sort back to front
	Common::sort(sortList, sortList + nbobjets,
		[](const SortEntry &a, const SortEntry &b) { return a.z < b.z; });

	for (int i = 0; i < nbobjets; ++i) {
		const SortEntry &entry = sortList[i];
		const Location &loc = _locations[entry.numObj];
		Common::Rect dummy;
		_engine->_renderer->affObjetIso(entry.xw, entry.yw, entry.zw,
			loc.Alpha, loc.Beta, LBAAngles::ANGLE_0,
			_engine->_resources->_holomapArrowPtr, dummy);
	}
}

void HolomapV2::drawReticule() {
	const int32 cx = _engine->width() / 2;
	const int32 cy = scale(220);
	const int32 sz = 16;
	_engine->_menu->drawRectBorders(cx - sz, cy - sz, cx + sz, cy + sz, 15, 15);
}

bool HolomapV2::goToArrow() {
	const int32 dt = _engine->timerRef - _moveTimer;
	_holoAlpha = boundRuleThree(_holoAlpha, _destAlpha, 75, dt);
	_holoBeta = boundRuleThree(_holoBeta, _destBeta, 75, dt);
	return (_holoAlpha == _destAlpha && _holoBeta == _destBeta);
}

void HolomapV2::initHoloDatas() {
	Common::SeekableReadStream *surfaceStream = HQR::makeReadStream(
		TwineResource(Resources::HQR_HOLOMAP_FILE, HQR_TWINSUN_HMT));
	if (surfaceStream == nullptr) {
		error("Failed to load holomap surface for LBA2");
	}
	computeCoorMapping();
	computeCoorGlobe(surfaceStream);
	delete surfaceStream;
}

void HolomapV2::holoTraj(int32 trajectoryIndex) {
	// TODO: implement trajectory animation for LBA2
	warning("HolomapV2::holoTraj(%d) not yet implemented", trajectoryIndex);
}

void HolomapV2::holoMap() {
	const int32 alphaLightTmp = _engine->_scene->_alphaLight;
	const int32 betaLightTmp = _engine->_scene->_betaLight;

	_engine->saveTimer(false);
	_engine->_screens->fadeToBlack(_engine->_screens->_ptrPal);
	_engine->_sound->stopSamples();
	_engine->_interface->unsetClip();
	_engine->_screens->clearScreen();

	initHoloDatas();

	const int32 cameraPosX = _engine->width() / 2;
	const int32 cameraPosY = scale(220);
	_engine->_renderer->setProjection(cameraPosX, cameraPosY, 128, 1024, 1024);

	// Load globe texture
	_holomapImagePtr = nullptr;
	_holomapImageSize = HQR::getAllocEntry(&_holomapImagePtr,
		TwineResource(Resources::HQR_HOLOMAP_FILE, HQR_TWINSUN_HMG));
	if (_holomapImageSize == 0) {
		error("Failed to load holomap image for LBA2");
	}

	// Initial camera pointing at current location
	const int numCube = _engine->_scene->_numCube;
	if (numCube >= 0 && numCube < HOLO_MAX_CUBE) {
		const Location &loc = _locations[HOLO_MAX_OBJECTIF + numCube];
		_holoAlpha = loc.Alpha & (LBAAngles::ANGLE_360 - 1);
		_holoBeta = loc.Beta & (LBAAngles::ANGLE_360 - 1);
	} else {
		_holoAlpha = 0;
		_holoBeta = 0;
	}
	_holoGamma = 0;
	_zoomPlanet = HOLO_ZOOM_INIT_PLANET;
	_zoomPlanetDest = HOLO_ZOOM_PLANET;
	_automove = false;
	_flagRedraw = true;
	_flagPal = true;
	_flagHoloEnd = false;
	_numObjectif = -1;
	_oldObjectif = -1;

	_engine->_input->enableKeyMap(holomapKeyMapId);

	for (;;) {
		FrameMarker frame(_engine);
		_engine->_input->readKeys();
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			break;
		}

		// Navigation
		if (!_automove) {
			if (_engine->_input->isActionActive(TwinEActionType::HolomapUp)) {
				_holoAlpha -= LBAAngles::ANGLE_2;
				_holoAlpha = ClampAngle(_holoAlpha);
				_flagRedraw = true;
			} else if (_engine->_input->isActionActive(TwinEActionType::HolomapDown)) {
				_holoAlpha += LBAAngles::ANGLE_2;
				_holoAlpha = ClampAngle(_holoAlpha);
				_flagRedraw = true;
			}
			if (_engine->_input->isActionActive(TwinEActionType::HolomapLeft)) {
				_holoBeta -= LBAAngles::ANGLE_2;
				_holoBeta = ClampAngle(_holoBeta);
				_flagRedraw = true;
			} else if (_engine->_input->isActionActive(TwinEActionType::HolomapRight)) {
				_holoBeta += LBAAngles::ANGLE_2;
				_holoBeta = ClampAngle(_holoBeta);
				_flagRedraw = true;
			}
		}

		if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapPrev)) {
			// search prev active arrow
			for (int n = HOLO_MAX_OBJECTIF; n < HOLO_MAX_ARROW; ++n) {
				if (_locations[n].FlagHolo & HOLO_FLAG_ACTIVE) {
					_destAlpha = _locations[n].Alpha & (LBAAngles::ANGLE_360 - 1);
					_destBeta = _locations[n].Beta & (LBAAngles::ANGLE_360 - 1);
					_moveTimer = _engine->timerRef;
					_automove = true;
					_flagRedraw = true;
					break;
				}
			}
		} else if (_engine->_input->toggleActionIfActive(TwinEActionType::HolomapNext)) {
			// search next active arrow
			for (int n = HOLO_MAX_ARROW - 1; n >= HOLO_MAX_OBJECTIF; --n) {
				if (_locations[n].FlagHolo & HOLO_FLAG_ACTIVE) {
					_destAlpha = _locations[n].Alpha & (LBAAngles::ANGLE_360 - 1);
					_destBeta = _locations[n].Beta & (LBAAngles::ANGLE_360 - 1);
					_moveTimer = _engine->timerRef;
					_automove = true;
					_flagRedraw = true;
					break;
				}
			}
		}

		if (_automove) {
			if (goToArrow()) {
				_automove = false;
			}
			_flagRedraw = true;
		}

		// Zoom animation
		if (_zoomPlanet < _zoomPlanetDest) {
			_zoomPlanet += 100;
			if (_zoomPlanet > _zoomPlanetDest) _zoomPlanet = _zoomPlanetDest;
			_flagRedraw = true;
		}

		// Render
		if (_flagRedraw) {
			_flagRedraw = false;
			const Common::Rect rect(0, 0, _engine->width() - 1, _engine->height() - 1);
			_engine->_interface->box(rect, COLOR_BLACK);

			_engine->_renderer->setInverseAngleCamera(_holoAlpha, _holoBeta, _holoGamma);
			_engine->_renderer->setLightVector(_holoAlpha, _holoBeta, 0);

			// Draw objects behind globe
			drawListHoloGlobe(false);

			// Draw globe
			_engine->_renderer->setInverseAngleCamera(_holoAlpha, _holoBeta, _holoGamma);
			_engine->_renderer->setCameraRotation(0, 0, distance((float)_zoomPlanet));
			drawHoloMap();

			// Draw objects in front of globe
			drawListHoloGlobe(true);

			if (_automove) {
				drawReticule();
			}

			_engine->copyBlockPhys(rect);
		}

		// Fade in first time
		if (_flagPal) {
			_flagPal = false;
			_engine->_screens->fadeToPal(_engine->_screens->_palettePcx);
		}

		++_engine->timerRef;
	}

	_engine->_screens->fadeToBlack(_engine->_screens->_palettePcx);
	_engine->_scene->_alphaLight = alphaLightTmp;
	_engine->_scene->_betaLight = betaLightTmp;
	_engine->_gameState->init3DGame();
	_engine->_input->enableKeyMap(mainKeyMapId);
	_engine->restoreTimer();

	free(_holomapImagePtr);
	_holomapImagePtr = nullptr;
}

} // namespace TwinE
