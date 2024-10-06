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

#ifndef TWINE_HOLOMAPV1_H
#define TWINE_HOLOMAPV1_H

#include "twine/holomap.h"

#define NUM_HOLOMAPCOLORS 32
#define HOLOMAP_PALETTE_INDEX 192

namespace TwinE {

/**
 * The Holomap shows the hero position. The arrows (@c RESSHQR_HOLOARROWMDL) represent important places in your quest - they automatically disappear once that part of
 * the quest is done (@c clearHolomapPosition()). You can rotate the holoamp by pressing ctrl+cursor keys - but only using the cursor keys, you can scroll through the
 * text for the visible arrows.
 */
class HolomapV1 : public Holomap {
private:
	using Super = Holomap;

	struct Location {
		int16 alpha;
		int16 beta;
		int16 size;
		TextId mess = TextId::kNone;
		char name[30] = "";
	};

	IVec3 _holomapSurface[561];

	// original game size: 2244 (lba1)
	struct HolomapSort {
		int16 z = 0;
		uint16 projectedPosIdx = 0;
	};
	HolomapSort _holomapSort[16 * 32];

	struct HolomapProjectedPos {
		uint16 x1 = 0;
		uint16 y1 = 0;
		uint16 x2 = 0;
		uint16 y2 = 0;
	};
	HolomapProjectedPos _projectedSurfacePositions[561];
	int _projectedSurfaceIndex = 0;
	// float _distanceModifier = 1.0f;

	int32 _numHoloPos = 0;
	Location _listHoloPos[MAX_HOLO_POS];

	int32 _holomapPaletteIndex = 0;
	uint8 _paletteHolomap[NUMOFCOLORS * 3]{0};

	void drawHolomapText(int32 centerx, int32 top, const char *title);
	int32 searchNextArrow(int32 num) const;
	int32 searchPrevArrow(int32 num) const;

	void drawListPos(int xRot, int yRot, int zRot, bool lower);

	/**
	 * Renders a holomap path with single path points appearing slowly one after another
	 */
	void drawHoloObj(const IVec3 &angle, int32 x, int32 y);
	void computeCoorGlobe(Common::SeekableReadStream *holomapSurfaceStream);
	void computeCoorMapping();
	void computeGlobeProj();
	void drawHoloMap(uint8 *holomapImage, uint32 holomapImageSize);
	void renderHolomapVehicle(uint &frameNumber, ActorMoveStruct &move, AnimTimerDataStruct &animTimerData, BodyData &bodyData, AnimData &animData);

	/**
	 * Controls the size/zoom of the holomap planet
	 */
	int32 distance(float distance) const;
	int32 scale(float val) const;

public:
	HolomapV1(TwinEEngine *engine) : Super(engine) {}
	virtual ~HolomapV1() = default;

	/**
	 * Set Holomap location position
	 * @param locationIdx Scene where position must be set
	 */
	bool setHoloPos(int32 locationIdx) override;

	bool loadLocations() override;

	const char *getLocationName(int index) const override;

	/**
	 * Clear Holomap location position
	 * @param locationIdx Scene where position must be cleared
	 */
	void clrHoloPos(int32 locationIdx) override;

	void drawHolomapTrajectory(int32 trajectoryIndex) override;

	/** Load Holomap content */
	void initHoloDatas() override;

	/** Main holomap process loop */
	void holoMap() override;
};

} // namespace TwinE

#endif
