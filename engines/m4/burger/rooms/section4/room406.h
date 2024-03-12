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

#ifndef M4_BURGER_ROOMS_SECTION4_ROOM406_H
#define M4_BURGER_ROOMS_SECTION4_ROOM406_H

#include "m4/burger/rooms/section4/section4_room.h"
#include "m4/burger/walker.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room406 : public Section4Room {
private:
	static const char *SAID[][4];
	static const WilburMatch MATCH[];
	static const seriesStreamBreak SERIES1[];
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
	static const seriesPlayBreak PLAY24[];
	static const seriesPlayBreak PLAY25[];
	static const seriesPlayBreak PLAY26[];
	static const seriesPlayBreak PLAY27[];
	static const seriesPlayBreak PLAY28[];
	static const seriesPlayBreak PLAY29[];
	static const seriesPlayBreak PLAY30[];
	static const seriesPlayBreak PLAY31[];
	static const seriesPlayBreak PLAY32[];
	static const seriesPlayBreak PLAY33[];
	static const seriesPlayBreak PLAY34[];
	static const seriesPlayBreak PLAY35[];
	static const seriesPlayBreak PLAY36[];
	static int32 _state1;
	static int32 _state2;
	static int32 _state3;
	static int32 _state4;
	Series _coll;
	Series _gate;
	Series _mg03;
	int32 _gateS1 = 0, _gateS2 = 0;
	int32 _tire = 0;
	const char *_seriesName = nullptr;
	machine *_tt = nullptr;
	machine *_tts = nullptr;
	machine *_fish = nullptr;
	machine *_wi03 = nullptr;
	machine *_coll406 = nullptr;
	int32 _fishS = 0;
	Common::String _randomDigi;
	noWalkRect *_walk1 = nullptr;
	noWalkRect *_walk2 = nullptr;
	noWalkRect *_walk3 = nullptr;
	noWalkRect *_walk4 = nullptr;
	HotSpotRec *_hotspot = nullptr;
	bool _flag1 = false;
	bool _flag2 = false;
	bool _flag3 = false;
	int _ticks = 0;
	int _dogShould = 0;
	int _val2 = 0;
	//int _val3 = 0;
	int _val4 = 0;
	int _mayorShould = 0;
	int _val6 = 0;
	int _val7 = 0;

	void loadSeries();
	void setHotspots1();
	void setHotspots2();
	void setHotspots3();
	void setHotspots4();
	void setupFish();
	void setupTt();
	void setNoWalk();
	void parseJail();
	void playRandom(int trigger);
	int tabooAreaY(int x) const;
	void triggerPreparser();

public:
	Room406();
	~Room406() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
