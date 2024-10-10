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

#ifndef M4_RIDDLE_ROOMS_ROOM_H
#define M4_RIDDLE_ROOMS_ROOM_H

#include "m4/core/rooms.h"
#include "m4/adv_r/conv_io.h"
#include "m4/core/imath.h"
#include "m4/graphics/gr_series.h"
#include "m4/adv_r/adv_hotspot.h"
#include "m4/riddle/triggers.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

#define HAS(ITEM) (player_said(ITEM) && inv_player_has(ITEM))
#define HERE(ITEM) (player_said(ITEM) && inv_object_is_here(ITEM))

class Room : public M4::Room {
private:
	static int _ripSketching;

protected:
	void restoreAutosave();

	/**
	 * Checks various game flags for updates
	 * @param flag		If set, does extra checks
	 * @return	A count of the flag changes done
	 */
	int checkFlags(bool flag);

	void setFlag45();

	/**
	 * Get the number of key items placed in room 305 (display room)
	 */
	int getNumKeyItemsPlaced() const;

	bool setItemsPlacedFlags();
	const char *getItemsPlacedDigi() const;

	/**
	 * Sets all the hotspots to be inactive
	 */
	void disableHotspots();

	/**
	 * Sets all the hotspots to be active
	 */
	void enableHotspots();

	bool checkStrings() const;

public:
	Room() : M4::Room() {}
	~Room() override {}

	void preload() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
