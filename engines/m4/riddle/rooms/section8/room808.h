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

#ifndef M4_RIDDLE_ROOMS_SECTION8_ROOM808_H
#define M4_RIDDLE_ROOMS_SECTION8_ROOM808_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room808 : public Room {
public:
	Room808() : Room() {}
	~Room808() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;


private:
	bool getWalkPath(machine *machine, int32 walk_x, int32 walk_y);

	int32 _807Rp04Series = 0;
	int32 _808McupSeries = 0;
	int32 _808Rp01Series = 0;
	int32 _808Rp02Series = 0;
	int32 _808RpupSeries = 0;
	int32 _mctd61Series = 0;
	int32 _mctd82aSeries = 0;
	int32 _ripLooksAroundInAweSeries = 0;
	int32 _ripMedReach1HandPos2Series = 0;
	int32 _ripPos3LookAroundSeries = 0;
	int32 _ripTalkerPos5Series = 0;
	int32 _rptmr15Series = 0;

	machine *_808ChainMach = nullptr;
	machine *_808HandleSpriteMach = nullptr;
	machine *_808PosMach = nullptr;
	machine *_mcTrekMach = nullptr;
};


} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
