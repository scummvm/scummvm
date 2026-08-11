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

#ifndef M4_RIDDLE_ROOMS_SECTION8_ROOM809_H
#define M4_RIDDLE_ROOMS_SECTION8_ROOM809_H

#include "m4/riddle/rooms/section8/section8_room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room809 : public Section8Room {
public:
	Room809() : Section8Room() {}
	~Room809() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;
	void syncGame(Common::Serializer &s) override;

private:
	bool checkSaid();
	int32 getMcDestX(int32 xPos, bool facing);

	bool _field20Fl = false;

	// CHECKME: The array and its index don't seem to be used
	byte _byte1A1990[4] = {0, 0, 0, 0};
	int32 _field24_index = 0;

	const char *_enableHotspotName = nullptr;

	int32 _mcFacing = 0;
	int32 _mcPosX = 0;
	int32 _mcTrekDestX = 0;
	int32 _playerDestX = 0;
	int32 _playerDestY = 0;
	int32 _playerFacing = 0;

	int32 _809hallSeries = 0;
	int32 _809MusicFadingVol = 0;
	int32 _mcHandsBehindBackSeries = 0;
	int32 _ripTalkerPos5Series = 0;
	int32 _ripTrekHeadTurnPos5Series = 0;

	machine *_809crossMach = nullptr;
	machine *_809hallMach = nullptr;
	machine *_809rp01Mach = nullptr;
	machine *_mcTrekMach = nullptr;

};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
