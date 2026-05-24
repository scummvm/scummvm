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

#ifndef TWINE_HOLOMAPV2_H
#define TWINE_HOLOMAPV2_H

#include "twine/holomap.h"
#include "twine/shared.h"

namespace TwinE {

#define HOLO_MAX_OBJECTIF 50
#define HOLO_MAX_CUBE 255
#define HOLO_MAX_ARROW (HOLO_MAX_OBJECTIF + HOLO_MAX_CUBE)

// Globe mesh parameters (LBA2 uses 4x angle factor)
#define HOLO_STEP_ANGLE 128
#define HOLO_GLOBE_ALPHA_STEPS ((LBAAngles::ANGLE_360 / HOLO_STEP_ANGLE) + 1) // 33
#define HOLO_GLOBE_BETA_STEPS ((LBAAngles::ANGLE_180 / HOLO_STEP_ANGLE) + 1)  // 17 (alpha from -90 to +90 = 180 degrees)
// Actually: alpha from -1024 to 1024 step 128 = 17 rows, beta from 0 to 4096 step 128 = 32 cols + 1 wrap = 33
#define HOLO_GLOBE_COLS 33
#define HOLO_GLOBE_ROWS 17
#define HOLO_GLOBE_VERTICES (HOLO_GLOBE_COLS * HOLO_GLOBE_ROWS)
#define HOLO_GLOBE_QUADS ((HOLO_GLOBE_COLS - 1) * (HOLO_GLOBE_ROWS - 1)) // 512

#define HOLO_RAYON_PLANET 1000
#define HOLO_ZOOM_PLANET 8000
#define HOLO_ZOOM_INIT_PLANET 3000

class HolomapV2 : public Holomap {
private:
	using Super = Holomap;

public:
	struct Location {
		int32 X = 0;
		int32 Y = 0;
		int32 Z = 0;
		int32 Alpha = 0;
		int32 Beta = 0;
		int32 Alt = 0;
		int32 Mess = 0;
		int8 ObjFix = 0;
		uint8 FlagHolo = 0u;
		uint8 Planet = 0u;
		uint8 Island = 0u;
	};
	static_assert(sizeof(Location) == 32, "Invalid Location size");

private:
	Location _locations[HOLO_MAX_ARROW];

	// Globe mesh data
	IVec3 _holomapSurface[HOLO_GLOBE_VERTICES];

	struct HolomapProjectedPos {
		int16 x1 = 0; // screen X
		int16 y1 = 0; // screen Y
		uint16 x2 = 0; // texture U
		uint16 y2 = 0; // texture V
	};
	HolomapProjectedPos _projectedSurfacePositions[HOLO_GLOBE_VERTICES];

	struct HolomapSort {
		int16 z = 0;
		uint16 projectedPosIdx = 0;
	};
	HolomapSort _holomapSort[HOLO_GLOBE_QUADS];

	// Globe rendering state
	int32 _holoAlpha = 0;
	int32 _holoBeta = 0;
	int32 _holoGamma = 0;
	int32 _zoomPlanet = HOLO_ZOOM_INIT_PLANET;
	int32 _zoomPlanetDest = HOLO_ZOOM_PLANET;

	// Camera interpolation
	int32 _destAlpha = 0;
	int32 _destBeta = 0;
	int32 _moveTimer = 0;
	bool _automove = false;

	// Objective/selection state
	int32 _numObjectif = -1;
	int32 _oldObjectif = -1;

	// UI state
	bool _flagRedraw = true;
	bool _flagPal = true;
	bool _flagHoloEnd = false;

	// Holomap image
	uint8 *_holomapImagePtr = nullptr;
	int32 _holomapImageSize = 0;

	void computeCoorMapping();
	void computeCoorGlobe(Common::SeekableReadStream *surfaceStream);
	void computeGlobeProj();
	void drawHoloMap();
	void drawListHoloGlobe(bool frontFace);
	void drawReticule();
	bool goToArrow();

	int32 distance(float dist) const;
	int32 scale(float val) const;

public:
	HolomapV2(TwinEEngine *engine) : Super(engine) {}
	virtual ~HolomapV2() = default;

	bool setHoloPos(int32 locationIdx) override;
	bool loadLocations() override;
	const char *getLocationName(int index) const override;
	void clrHoloPos(int32 locationIdx) override;
	void holoTraj(int32 trajectoryIndex) override;
	void initHoloDatas() override;
	void holoMap() override;
};

} // namespace TwinE

#endif
