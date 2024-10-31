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

#ifndef M4_RIDDLE_ROOMS_SECTION2_ROOM203_H
#define M4_RIDDLE_ROOMS_SECTION2_ROOM203_H

#include "m4/riddle/rooms/section2/section2_room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room203 : public Section2Room {
private:
	int _val1 = 0;
	int _val2 = 0;
	int _val3 = 0;
	int _val4 = 0;
	int _val5 = 0;
	int _val6 = 0;
	int _val7 = 0;
	int _val8 = 0;
	int _val9 = 0;
	int _val10 = 0;
	int _val11 = 0;
	int _trigger1 = -1;
	int _trigger2 = -1;
	int _trigger3 = -1;
	int _trigger4 = -1;
	int _trigger5 = -1;
	int _shadow3 = 0;
	int _ripHeadTurn = 0;
	int _ripLooksAtHeads = 0;
	int _203sg01 = 0;
	machine *_mei = nullptr;
	machine *_gk = nullptr;
	int _gkFrame = 0;
	int _gkMode = 0, _gkShould = 0;
	machine *_oldLady = nullptr;
	int _oldLadyFrame = 0;
	int _oldLadyNoHelmet = 0;
	int _oldLadyFeedingBirds = 0;
	int _oldLadyMode = 0, _oldLadyShould = 0;
	machine *_pigeons1 = nullptr;
	machine *_pigeons3 = nullptr;
	int _pigeonsSeries1 = 0;
	int _pigeonsSeries3 = 0;
	machine *_official = nullptr;
	machine *_officialShadow = nullptr;
	int _officialStander = 0;
	int _officialMode = 0, _officialShould = 0;
	int _officialTurn9_11 = 0;
	int _officialTurn11_3 = 0;
	int _officialTurn3_7 = 0;
	machine *_peasant = nullptr;
	machine *_peasantShadow = nullptr;
	int _peasantSeries = 0;
	int _peasantSeriesShadow = 0;
	int _peasantRocks = 0;
	int _peasantRocksShadow = 0;
	int _peskyYellsThief = 0;
	int _peasantMode = 0, _peasantShould = 0;
	int _peasantX = 0;
	int _peasantY = 0;
	int _peasantScale = 0;
	int _peasantLayer = 0;
	bool _flag1 = false;

	void setupHelmetHotspot();
	void setupPeasantHotspot(int mode);
	void setupGk();
	void setupOldLady();
	void setupPigeons();
	void setupOfficial();
	void setupPeasant();

public:
	Room203() : Section2Room() {}
	~Room203() override {}

	void init() override;
	void daemon() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
