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

#ifndef M4_BURGER_ROOMS_SECTION4_ROOM402_H
#define M4_BURGER_ROOMS_SECTION4_ROOM402_H

#include "m4/burger/rooms/section4/section4_room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room402 : public Section4Room {
private:
	static const char *SAID[][4];
	static const seriesStreamBreak SERIES1[];
	static const seriesStreamBreak SERIES2[];
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
	const char *_digiName = nullptr;
	KernelTriggerType _newMode = KT_DAEMON;
	bool _stolieSet = false;
	Series _stolie;
	bool _series2Set = false;
	Series _series2;
	Series _series3;
	int _dr01 = -1, _dr01s = -1;
	int _dr02 = -1, _dr02s = -1;
	int _dr03 = -1, _dr03s = -1;
	int _dr08 = -1, _dr08s = -1;
	int _pe01 = -1, _pe01s = -1;
	int _pe02 = -1, _pe02s = -1;
	int _pe03 = -1, _pe03s = -1;
	int _pe04 = -1, _pe04s = -1;
	int _wi01 = -1, _wi01s = -1;
	int _wi02 = -1, _wi02s = -1;
	int _digiTrigger = 0;
	int _stolieShould = 0;
	int _elmoShould = 0;
	int _val4 = 0;
	int _elmoMode = 0;
	int _stolieMode = 0;

	void conv84();
	void playDigiName();
	void playRandom1();
	void playRandom2();
	void freeStolie();
	void freeSeries2();
	void loadDr1();
	void freeDr1();
	void loadDr2();
	void freeDr2();
	void loadDr8();
	void freeDr8();
	void loadPe1();
	void freePe1();
	void loadPe2();
	void freePe2();
	void loadPe3();
	void freePe3();
	void loadPe4();
	void freePe4();

public:
	Room402() : Section4Room() {}
	~Room402() override {}

	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
