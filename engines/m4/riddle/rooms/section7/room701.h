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

#ifndef M4_RIDDLE_ROOMS_SECTION7_ROOM701_H
#define M4_RIDDLE_ROOMS_SECTION7_ROOM701_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room701 : public Room {
public:
	Room701() : Room() {}
	~Room701() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	const char *_itemDigiName = nullptr;

	int32 _field50 = 0;
	int32 _field88 = 0;
	int32 _field8C = 0;
	int32 _field9E = 0;
	int32 _field130 = 0;
	int32 _field134 = 0;

	int32 _701rp01Series = 0;
	int32 _701rp99Series = 0;
	int32 _agentGetTelegramSeries = 0;
	int32 _agentGiveParcelSeries = 0;
	int32 _agentShowMapSeries = 0;
	int32 _agentTalkLoopTjSeries = 0;
	int32 _ripTrekHandTalkPos3Series = 0;
	int32 _ripTrekMedReachPos3Series = 0;
	int32 _ripTrekTalkerPos3Series = 0;
	
	machine *_agentPoshExpressMach = nullptr;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
