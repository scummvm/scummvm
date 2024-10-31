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

#ifndef M4_RIDDLE_ROOMS_SECTION2_ROOM205_H
#define M4_RIDDLE_ROOMS_SECTION2_ROOM205_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room205 : public Room {
public:
	Room205() : Room() {}
	~Room205() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	int32 _fieldD8 = 0;
	int32 _fieldDC = 0;
	int32 _fieldE0 = 0;
	int32 _fieldE4 = 0;
	int32 _fieldE8 = 0;
	int32 _field198 = 0;
	int32 _field19C = 0;
	int32 _field1A0 = 0;

	int32 _205FireInBrazierSeries = 0;
	int32 _205GunFireSeries = 0;
	int32 _205mc01Series = 0;
	int32 _205mc02Series = 0;
	int32 _205rp1Series = 0;
	int32 _205rp02Series = 0;
	int32 _205TabletsSeries = 0;
	int32 _ripGetsShotSeries = 0;
	int32 _ripTrekLowReacherPos5Series = 0;
	int32 _ripTrekLowReachPos2Series = 0;
	int32 _ripTrekMedReachHandPos1Series = 0;
	
	machine *_205CharcoalSpriteMach = nullptr;
	machine *_205FireInBrazierMach = nullptr;
	machine *_205GunInBrazierMach = nullptr;
	machine *_205LeftEntranceTabletMach = nullptr;
	machine *_205MeiStanderMach = nullptr;
	machine *_205TabletsMach = nullptr;
	machine *_mcEntranceTrekMach = nullptr;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
