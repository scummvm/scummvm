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

#ifndef M4_RIDDLE_ROOMS_SECTION5_ROOM501_H
#define M4_RIDDLE_ROOMS_SECTION5_ROOM501_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room501 : public Room {
private:
	machine *_ripley = nullptr;
	machine *_shadow = nullptr;
	machine *_agent = nullptr;
	machine *_clock = nullptr;
	machine *_paper = nullptr;
	machine *_deltaPuffinMachine = nullptr;
	int _agentTalkLoop = 0;
	int _agentStridesForward = 0;
	int _puffinExchange = 0;
	const char *_digiName = nullptr;
	int _convEntry = 0;
	bool _flag = false;
	int _ripTalkLoop = 0;
	int _ripSeries1 = 0;
	int _ripParcelExchange = 0;
	int _ripMoneyExchange = 0;
	int _ripSignsPaper = 0;
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val7 = 0;
	int _val8 = 0;
	int _val9 = 0;
	int _xyzzy1 = 0;
	int _xyzzy2 = 0;
	int _xyzzy3 = 0;
	int _xyzzy4 = 0;
	int _xyzzy5 = 0;
	int _xyzzy6 = 0;
	int _xyzzy7 = 0;
	int _xyzzy8 = 0;
	int _xyzzy9 = 0;
	int _xyzzy10 = 0;
	int _itemsCount = 0;
	int _items[12];
	int32 _hasItems = 0;
	int32 _hasLetter = 0;
	int32 _hasCrystalSkull = 0;
	int32 _hasStickAndShellMap = 0;
	int32 _hasWheeledToy = 0;
	int32 _hasRebusAmulet = 0;
	int32 _hasShrunkenHead = 0;
	int32 _hasSilverButterfly = 0;
	int32 _hasPostageStamp = 0;
	int32 _hasGermanBanknote = 0;
	int32 _hasWhaleBoneHorn = 0;
	int32 _hasChisel = 0;
	int32 _hasIncenseBurner = 0;
	int32 _hasRomanovEmerald = 0;
	const char *_queuedDigi[4];
	int _ripSketching = 0;

	void conv501a();

	/**
	 * Sets up the items array with any key items the player
	 * has in their inventory
	 * @return	True if the player has at least one item
	 */
	bool updateItems();

public:
	Room501();
	~Room501() override {}

	void init() override;
	void daemon() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
