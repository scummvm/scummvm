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

#ifndef TWINE_HOLOMAP_H
#define TWINE_HOLOMAP_H

#include "twine/renderer/renderer.h"
#include "common/scummsys.h"
#include "twine/twine.h"

namespace TwinE {

#define NUM_LOCATIONS 150

class TwinEEngine;

/**
 * The Holomap shows the hero position. The arrows (@c RESSHQR_HOLOARROWMDL) represent important places in your quest - they automatically disappear once that part of
 * the quest is done (@c clearHolomapPosition()). You can rotate the holoamp by pressing ctrl+cursor keys - but only using the cursor keys, you can scroll through the
 * text for the visible arrows.
 */
class Holomap {
private:
	TwinEEngine *_engine;

	int16 clip_or_depth_DAT_00433444 = 0;
	uint32 y_DAT_00433448 = 0;
	uint16 tex_coords_maybe_DAT_0043336a = 0;
	uint16 tex_coords_maybe_DAT_0043336c = 0;
	uint16 tex_coords_maybe_DAT_00433370 = 0;
	uint16 tex_coords_maybe_DAT_00433372 = 0;
	uint16 tex_coords_maybe_DAT_00433376 = 0;
	uint16 tex_coords_maybe_DAT_00433378 = 0;

	void holomap_surface_load_FUN_0042194d(Vertex *vertexCoordinates, int32 y_1, int16 param_2, uint8* holomapSurfaceImgOutPtr);
	bool vertices_FUN_00423ebb(const Vertex *vertices) const;
	void vertices_FUN_00421010(Vertex *vertexCoordinates);
	void vertices_FUN_00420fad(int32 y1, int32 x1, int32 y2, int32 x2, int16 *vertexCoordinatePtr);

	struct Location {
		uint16 x = 0;
		uint16 y = 0;
		uint16 z = 0;
		uint16 textIndex = 0;
	};

	enum HolomapVehicle {
		FerryBoat = 31,
		Motorbike = 33,
		Car = 35,
		FishingBoat = 37,
		Catamaran = 39,
		Hovercraft = 41,
		Dino = 43,
		ArmyBoat = 45,
		HamalayiTransporter = 47
	};
	struct HolomapSurface {
		int16 x = 0;
		int16 y = 0;
		int16 z = 0;
	};
	HolomapSurface _holomapSurface[561];

	struct HolomapSort {
		int16 z = 0;
		uint16 projectedPosIdx = 0;
	};
	HolomapSort _holomapSort[512];

	struct HolomapProjectedPos {
		int16 x = 0;
		int16 y = 0;
		uint16 unk1 = 0;
		uint16 unk2 = 0;
	};
	HolomapProjectedPos _projectedSurfacePositions[2048];

	struct TrajectoryData {
		int16 locationIdx;
		int16 trajLocationIdx;
		int16 vehicleIdx;
		int16 x;
		int16 y;
		int16 z;
		int16 numAnimFrames;
		struct TrajectoryPos {
			int16 x;
			int16 y;
		};
		TrajectoryPos positions[512];

		/**
		 * The HQR index of the vehicle model for the holomap
		 * @note Multiplied by 2 because the model index is always followed by the corresponding animation index for that model
		 */
		int32 getModel() const {
			return 2 * vehicleIdx + HolomapVehicle::FerryBoat;
		}

		int32 getAnimation() const {
			return getModel() + 1;
		}
	};

	int32 _numLocations = 0;
	Location _locations[NUM_LOCATIONS];

	int32 holomapPaletteIndex = 0;
	uint8 paletteHolomap[NUMOFCOLORS * 3]{0};

	void drawHolomapText(int32 centerx, int32 top, const char *title);
	int32 getNextHolomapLocation(int32 currentLocation, int32 dir) const;

	void renderLocations(int xRot, int yRot, int zRot, bool lower);
	TrajectoryData loadTrajectoryData(int32 trajectoryIdx);

	void renderHolomapModel(const uint8 *bodyPtr, int32 x, int32 y, int32 zPos);

	void prepareHolomapSurface();
	void prepareHolomapProjectedPositions();
	void prepareHolomapPolygons();
	void renderHolomapSurfacePolygons();

public:
	Holomap(TwinEEngine *engine);

	/**
	 * Set Holomap location position
	 * @param locationIdx Scene where position must be set
	 */
	void setHolomapPosition(int32 locationIdx);

	bool loadLocations();

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
