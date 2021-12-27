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

#ifndef TWINE_HOLOMAP_H
#define TWINE_HOLOMAP_H

#include "twine/shared.h"
#include "common/scummsys.h"

#define NUM_HOLOMAPCOLORS 32
#define HOLOMAP_PALETTE_INDEX 192

namespace Common {
class SeekableReadStream;
}

namespace TwinE {

class TwinEEngine;
class BodyData;
class AnimData;
struct ActorMoveStruct;
struct Vertex;
struct AnimTimerDataStruct;

/**
 * The Holomap shows the hero position. The arrows (@c RESSHQR_HOLOARROWMDL) represent important places in your quest - they automatically disappear once that part of
 * the quest is done (@c clearHolomapPosition()). You can rotate the holoamp by pressing ctrl+cursor keys - but only using the cursor keys, you can scroll through the
 * text for the visible arrows.
 */
class Holomap {
private:
	TwinEEngine *_engine;

	bool isTriangleVisible(const Vertex *vertices) const;

	struct Location {
		int16 angleX;
		int16 angleY;
		int16 size;
		TextId textIndex = TextId::kNone;
		char name[30] = "";
	};

	IVec3 _holomapSurface[561];

	// original game size: 2244 (lba1)
	struct HolomapSort {
		int16 z = 0;
		uint16 projectedPosIdx = 0;
	};
	HolomapSort _holomapSort[512];

	struct HolomapProjectedPos {
		int16 x1 = 0;
		int16 y1 = 0;
		int16 x2 = 0;
		int16 y2 = 0;
	};
	HolomapProjectedPos _projectedSurfacePositions[561];
	int _projectedSurfaceIndex = 0;
	//float _distanceModifier = 1.0f;

	int32 _numLocations = 0;
	Location _locations[NUM_LOCATIONS];

	int32 _holomapPaletteIndex = 0;
	uint8 _paletteHolomap[NUMOFCOLORS * 3]{0};

	void drawHolomapText(int32 centerx, int32 top, const char *title);
	int32 getNextHolomapLocation(int32 currentLocation, int32 dir) const;

	void renderLocations(int xRot, int yRot, int zRot, bool lower);

	/**
	 * Renders a holomap path with single path points appearing slowly one after another
	 */
	void renderHolomapPointModel(const IVec3 &angle, int32 x, int32 y);
	void prepareHolomapSurface(Common::SeekableReadStream *holomapSurfaceStream);
	void prepareHolomapProjectedPositions();
	void prepareHolomapPolygons();
	void renderHolomapSurfacePolygons(uint8 *holomapImage, uint32 holomapImageSize);
	void renderHolomapVehicle(uint &frameNumber, ActorMoveStruct &move, AnimTimerDataStruct &animTimerData, BodyData &bodyData, AnimData &animData);

	/**
	 * Controls the size/zoom of the holomap planet
	 */
	int32 distance(float distance) const;
	int32 scale(float val) const;

public:
	Holomap(TwinEEngine *engine);

	/**
	 * Set Holomap location position
	 * @param locationIdx Scene where position must be set
	 */
	void setHolomapPosition(int32 locationIdx);

	bool loadLocations();

	const char *getLocationName(int index) const;

	/**
	 * Clear Holomap location position
	 * @param locationIdx Scene where position must be cleared
	 */
	void clearHolomapPosition(int32 locationIdx);

	void drawHolomapTrajectory(int32 trajectoryIndex);

	/** Load Holomap content */
	void loadHolomapGFX();

	/** Main holomap process loop */
	void processHolomap();
};

} // namespace TwinE

#endif
