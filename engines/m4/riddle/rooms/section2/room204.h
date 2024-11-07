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

#ifndef M4_RIDDLE_ROOMS_SECTION2_ROOM204_H
#define M4_RIDDLE_ROOMS_SECTION2_ROOM204_H

#include "m4/riddle/rooms/section2/section2_room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room204 : public Section2Room {
public:
	Room204() : Section2Room() {}
	~Room204() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	void initWalkerSeries();
	void addMovingMeiHotspot();
	void deleteMalletHotspot();
	void addLookMalletHotspot();

	int32 _field4 = 0;
	int32 _field40 = 0;
	int32 _field44 = 0;
	int32 _field48 = 0;
	int32 _field68 = 0;
	int32 _fieldBC = 0;
	int32 _fieldC4 = 0;
	int32 _fieldDC = 0;
	int32 _fieldE0 = 0;
	int32 _fieldE4 = 0;
	int32 _fieldEC = 0;
	int32 _fieldF0 = 0;
	int32 _field104 = 0;
	int32 _field108 = 0;
	int32 _field180 = 0;
	int32 _field184 = 0;
	int32 _field188 = 0;

	int32 _courtyardGongSeries = 0;
	int32 _malletSpriteSeries = 0;
	int32 _ripDropsSeries = 0;

	machine *_courtyardGongMach = nullptr;
	machine *_mcMach = nullptr;
	machine *_malletSpriteMach = nullptr;
	machine *_silverButterflyCoinMach = nullptr;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
