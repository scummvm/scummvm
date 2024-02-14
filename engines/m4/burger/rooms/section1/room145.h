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

#ifndef M4_BURGER_ROOMS_SECTION1_ROOM145_H
#define M4_BURGER_ROOMS_SECTION1_ROOM145_H

#include "m4/burger/rooms/room.h"
#include "m4/burger/walker.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room145 : public Room {
private:
	static const char *SAID[][4];
	static const WilburMatch MATCH[];
	static const seriesPlayBreak PLAY1[];
	static const seriesPlayBreak PLAY2[];
	static const seriesPlayBreak PLAY3[];
	static const seriesPlayBreak PLAY4[];
	static const seriesPlayBreak PLAY5[];
	static const seriesPlayBreak PLAY6[];
	static const seriesPlayBreak PLAY7[];
	static const seriesPlayBreak PLAY8[];
	static const seriesPlayBreak PLAY9[];
	static const seriesPlayBreak PLAY10[];
	static const seriesPlayBreak PLAY11[];
	static const seriesPlayBreak PLAY12[];
	static const seriesPlayBreak PLAY13[];
	static const seriesPlayBreak PLAY14[];
	static const seriesPlayBreak PLAY15[];
	static const seriesPlayBreak PLAY16[];
	static const seriesPlayBreak PLAY17[];
	static const seriesPlayBreak PLAY18[];
	static const seriesPlayBreak PLAY19[];
	static const seriesPlayBreak PLAY20[];
	static const seriesPlayBreak PLAY21[];
	static const seriesPlayBreak PLAY22[];
	static const seriesPlayBreak PLAY23[];
	static int32 _state1;
	static int32 _state2;
	static int32 _state3;
	noWalkRect *_walk1 = nullptr;
	noWalkRect *_walk2 = nullptr;
	noWalkRect *_walk3 = nullptr;
	machine *_amplifier = nullptr;
	Series _vipe;
	Series _roxy;
	const char *_digiName1 = nullptr;
	const char *_digiName2 = nullptr;
	bool _flag1 = false;
	int _duration = 0;
	int _drumzState = 0;
	int _roxyTalkTo = 0;
	int _roxyState = 0;
	int _vipeState = 0;
	int _val5 = 0;

	void loadDrum();
	void loadRx();
	void conv21();
	void conv22();
	void conv23();
	void vipeSpeaking();
	void roxySpeaking();
	void loadSeries1();
	int getRandomDrumzState() const;
	void resetRoxy();
	void playRandomDigi1();
	void playRandomDigi2();

public:
	Room145();
	~Room145() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
