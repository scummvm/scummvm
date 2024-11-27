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

#ifndef M4_RIDDLE_ROOMS_SECTION6_ROOM604_H
#define M4_RIDDLE_ROOMS_SECTION6_ROOM604_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room604 : public Room {
private:
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	int _val5 = 0;
	int _ripLowReach1 = 0;
	int _ripLowReach2 = 0;
	int _ripMedReach1 = 0;
	int _ripMedReach2 = 0;
	int _ripAction = 0;
	machine *_ripley = nullptr;
	machine *_sparkPlugTool = nullptr;
	machine *_sparkPlugWire = nullptr;
	machine *_flame = nullptr;
	machine *_shedDoor = nullptr;
	machine *_pullCord1 = nullptr;
	machine *_pullCord2 = nullptr;
	machine *_doorWireGone = nullptr;
	machine *_cord = nullptr;
	machine *_tiedWires = nullptr;
	machine *_generatorCord = nullptr;
	machine *_shadow = nullptr;

	void daemon1();
	void useWhaleBoneHorn();
	void takePullCord();
	void takeSparkPlugTool();
	void takeLighter();
	void pullCordPlug();

	/**
	 * This Ripley is no more! It has ceased to be! It's expired
	 * and gone to meet its maker! This is a late Ripley!
	 * It's a stiff! Bereft of life, it rests in peace!
	 * If you hadn't nailed it to the perch, it would be
	 * pushing up the daisies! It's rung down the curtain
	 * and joined the choir invisible. This is an ex-Ripley!
	 */
	void killRipley();

	void takeWire();

public:
	Room604() : Room() {}
	~Room604() override {}

	void preload() override;
	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
