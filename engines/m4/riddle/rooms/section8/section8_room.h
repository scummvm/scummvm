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

#ifndef M4_RIDDLE_ROOMS_SECTION8_SECTION8ROOM_H
#define M4_RIDDLE_ROOMS_SECTION8_SECTION8ROOM_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Section8Room : public Room {
public:
	Section8Room() : Room() {}
	~Section8Room() override {}

	void preload() override;
	void init() override {}
	void pre_parser() override;
	void parser() override;
	void daemon() override;

protected:
	int32 _byte1A19BC = 0;
	int32 _field68 = 0;

	// The array seems to be set and never read. Idem for the index
	int32 _unkArrayIndex = 0;
	int32 _unkArray[8];

	int32 _824fire2Serie = 0;
	int32 _case7RandVal = 0;
	int32 _coordArrayId = 0;
	int32 _counter1 = 0;
	int32 _currentRoom = 0;
	int32 _dynSerie1 = 0;
	int32 _guessFacing = 0;
	int32 _guessIndex = 0;
	int32 _guessX = 0;
	int32 _mctdSerie = 0;
	int32 _meiHandsBehindBack = 0;
	int32 _ripAttemptsPush = 0;
	int32 _ripLooksAround = 0;
	int32 _ripTakerPos5 = 0;
	int32 _ripTrekLowReach = 0;
	int32 _rptldSerie = 0;
	int32 _savedNextRoom = 0;
	int32 _savedPlayerInfoFacing = 0;
	int32 _savedPlayerInfoX = 0;
	int32 _savedPlayerInfoY = 0;
	int32 _savedRandom = 0;
	
	int32 _var1 = 0;
	int32 _var2 = 0;
	int32 _var3 = 0;
	int32 _var4 = 0;
	int32 _var5 = 0;

	Common::String _currentSeriesName;
	Common::String _guessHotspotName;

	machine *_824fire1Mach = nullptr;
	machine *_824fire2Mach = nullptr;
	machine *_844postMach = nullptr;
	machine *_dynSerie1Mach = nullptr;
	machine *_machArr[4] = {nullptr, nullptr, nullptr, nullptr};
	machine *_mcTrekMach = nullptr;
	machine *_ripPushMach = nullptr;

	int32 subCE498(int32 val1);
	void sendWSMessage_3840000(machine *machine, int32 trigger);
	int32 subCE52E(int32 val1);
	void moveScreen(int32 dx, int32 dy);
	void getSeriesName(int32 val1, bool true_or_False);
	int32 daemonSub1(int32 dx, bool ascendingFl);
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
