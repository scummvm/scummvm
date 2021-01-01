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

	struct Location {
		uint16 x = 0;
		uint16 y = 0;
		uint16 z = 0;
		uint16 textIndex = 0;
	};

	int32 _numLocations = 0;
	Location _locations[NUM_LOCATIONS];

	int32 needToLoadHolomapGFX = 0;
	uint8 paletteHolomap[NUMOFCOLORS * 3]{0};

	void drawHolomapText(int32 centerx, int32 top, const char *title);
	void drawHolomapLocation();

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

	void loadGfxSub1();
	void loadGfxSub2();

	/** Load Holomap content */
	void loadHolomapGFX();

	/** Main holomap process loop */
	void processHolomap();
};

} // namespace TwinE

#endif
