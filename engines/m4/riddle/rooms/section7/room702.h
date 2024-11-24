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

#ifndef M4_RIDDLE_ROOMS_SECTION7_ROOM702_H
#define M4_RIDDLE_ROOMS_SECTION7_ROOM702_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room702 : public Room {
public:
	Room702() : Room() {}
	~Room702() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;
	void syncGame(Common::Serializer &s) override;

private:
	static void callback(frac16 myMessage, machine *sender);
	void conv702a();

	bool _alreadyBeen702Fl = false; // Unused : Set but never used

	int32 _field40 = 0;
	int32 _field44_mode = 0;
	int32 _field48_should = 0;
	int32 _field4C_triggerNum = 0;
	int32 _field50_mode = 0;
	int32 _field54_should = 0;
	int32 _field58 = 0; // Useless (but synchronized?) - Always -1?

	int32 _702GuardShadow1Series = 0;
	int32 _702GuardShadow2Series = 0;
	int32 _guardStepsAsideTalksSeries = 0;
	int32 _guardTalksAndBowsSeries = 0;
	int32 _ringCloseupSeries = 0;
	int32 _ripSafariWalkerPos1Series = 0;
	int32 _ripShowsRingSeries = 0;
	int32 _safariShadow1Series = 0;

	machine *_ringCloseupMach = nullptr;
	machine *_guardMach = nullptr;
	machine *_guardShadowMach = nullptr;
	machine *_ripTalksGuardMach = nullptr;
	machine *_ripTalksGuardShadowMach = nullptr;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
