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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DUNE_MAP_REGION_H
#define DUNE_MAP_REGION_H

#include "dune/map_renderer.h"

namespace Dune {

// DOS 55DD: release matching MAP2 deposits, retaining ground/place bits.
void liberateMapDeposits(byte *world, uint worldSize, const byte *deposits,
		uint depositSize, const bool *released);

typedef void (*VegetationPlaceCallback)(uint offset, void *context);

// DOS shares these between nested painters. Capturing a fort replaces the
// span callback and centre even after its ownership circle returns.
struct MapRegionPaintState {
	uint16 longitude = 0;
	int latitude = 0;
	byte owner = 0;
	byte pattern = 0x44;
};

// DOS 0x64B2's paired spans, painted by 0x646F (ownership) or 0x653A
// (vegetation). Owner zero selects vegetation, 0x20/0x30 select a house.
// The callback runs when vegetation encounters a marked, non-green point.
void paintMapRegion(byte *world, uint worldSize, const MapLatitude *latitudes,
		uint16 longitude, int latitude, uint radius, byte owner = 0,
		VegetationPlaceCallback onPlace = nullptr, void *context = nullptr,
		MapRegionPaintState *sharedState = nullptr);

// DOS 0x65B6: seventy LFSR steps per day, thinning green to Atreides ground.
// changed reflects BP != 0, used to request one strategic-map rebuild.
uint16 decayMapVegetation(byte *world, uint worldSize, uint16 seed, bool &changed);

} // End of namespace Dune

#endif
