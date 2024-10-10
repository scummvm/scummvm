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

#define MAX_OBJECTIF 50
#define MAX_CUBE 255

namespace TwinE {

/**
 * The Holomap shows the hero position. The arrows (@c RESSHQR_HOLOARROWMDL) represent important places in your quest - they automatically disappear once that part of
 * the quest is done (@c clearHolomapPosition()). You can rotate the holoamp by pressing ctrl+cursor keys - but only using the cursor keys, you can scroll through the
 * text for the visible arrows.
 */
class HolomapV2 : public Holomap {
private:
	using Super = Holomap;

public:
	HolomapV2(TwinEEngine *engine) : Super(engine) {}
	virtual ~HolomapV2() = default;

	struct Location {
		int32 X; // Position Island X Y Z
		int32 Y;
		int32 Z;
		int32 Alpha; // Position Planet Alpha, Beta and Altitude
		int32 Beta;
		int32 Alt;
		int32 Mess;
		int8 ObjFix;    // Eventual Obj Inventory 3D (FREE NOT USED!)
		uint8 FlagHolo; // Flag for Planet display, active, etc.
		uint8 Planet;
		uint8 Island;
	};
	static_assert(sizeof(Location) == 32, "Invalid Location size");
	Location _locations[MAX_OBJECTIF + MAX_CUBE];

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

	void holoTraj(int32 trajectoryIndex) override;

	/** Load Holomap content */
	void initHoloDatas() override;

	/** Main holomap process loop */
	void holoMap() override;
};

} // namespace TwinE

#endif
