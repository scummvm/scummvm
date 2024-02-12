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

#ifndef M4_BURGER_ROOMS_SECTION4_ROOM407_H
#define M4_BURGER_ROOMS_SECTION4_ROOM407_H

#include "m4/burger/rooms/section4/section4_room.h"
#include "m4/burger/walker.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room407 : public Section4Room {
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
	static int32 _speechNum;
	Series _vp02;
	Series _dz;
	int _dzS1 = 0, _dzS2 = 0;
	machine *_rx = nullptr;
	noWalkRect *_walk1 = nullptr;
	Common::String _digiName;
	bool _flag1 = false;
	int _val1 = 0;
	int _drumzShould = 0;
	int _roxyShould = 0;
	int _roxyState = 0;
	int _vipeShould = 0;
	//int _digiTrigger = 0;

	void loadSeries();
	void conv87();
	void conv88();
	int getDrumzShould() const;
	void freeDz();
	void playConvSound();

public:
	Room407();
	~Room407() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
