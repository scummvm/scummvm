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

#ifndef M4_RIDDLE_ROOMS_SECTION4_ROOM401_H
#define M4_RIDDLE_ROOMS_SECTION4_ROOM401_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room401 : public Room {
private:
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int32 _val4 = 0;
	int _agentMode = 0;
	int _agentShould = 0;
	int _val7 = 0;
	const char *_digiName1 = nullptr;
	const char *_digiName2 = nullptr;
	const char *_digiName3 = nullptr;
	const char *_digiName4 = nullptr;
	int _401a01 = 0;
	int _401a02 = 0;
	int _401a03 = 0;
	int _401a04 = 0;
	int _401a05 = 0;
	int _401a06 = 0;
	int _401rp01 = 0;
	int _rip1 = 0;
	int _rip2 = 0;
	int _rip3 = 0;
	int _rip4 = 0;
	machine *_agent = nullptr;
	machine *_ripMach = nullptr;
	const char *_digiName = nullptr;
	int _ctr1 = 0;
		
	void conv401a();
	int getItemCount() const;

public:
	Room401() : Room() {}
	~Room401() override {}

	void init() override;
	void daemon() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
