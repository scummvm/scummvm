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
protected:
	TwinEEngine *_engine;
public:
	Holomap(TwinEEngine *engine) : _engine(engine) {}
	virtual ~Holomap() {}

	/**
	 * Set Holomap location position
	 * @param locationIdx Scene where position must be set
	 */
	virtual bool setHoloPos(int32 locationIdx) = 0;

	virtual bool loadLocations() = 0;

	virtual const char *getLocationName(int index) const = 0;

	/**
	 * Clear Holomap location position
	 * @param locationIdx Scene where position must be cleared
	 */
	virtual void clrHoloPos(int32 locationIdx) = 0;

	virtual void drawHolomapTrajectory(int32 trajectoryIndex) = 0;

	/** Load Holomap content */
	virtual void initHoloDatas() = 0;

	/** Main holomap process loop */
	virtual void holoMap() = 0;
};

} // namespace TwinE

#endif
