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

#ifndef M4_RIDDLE_ROOMS_SECTION7_ROOM701_H
#define M4_RIDDLE_ROOMS_SECTION7_ROOM701_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room701 : public Room {
public:
	Room701() : Room() {}
	~Room701() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	void conv701a();
	void updateCounter();
	int32 inventoryCheck();

	const char *_field58_digiName = nullptr;
	const char *_field5C_digiName = nullptr;
	const char *_field60_digiName = nullptr;
	const char *_field64_digiName = nullptr;

	const char *_itemDigiName = nullptr;
	Common::String _convDigiName_1 = "";
	Common::String _convDigiName_2 = "";

	int32 _field50_counter = 0;
	int32 _field68 = 0;
	int32 _field6C = 0;
	int32 _field72_triggerNum = -1;
	int32 _field88 = 0;
	int32 _field8C = 0;
	int32 _field90 = 0;
	int32 _field94 = 0;
	int32 _field98 = 0;
	int32 _field9E_triggerNum = -1;
	int32 _fieldB4 = 0;
	int32 _fieldB8 = 0;
	int32 _fieldBC = 0;
	int32 _fieldC4 = 0;
	int32 _fieldC8 = 0;
	int32 _fieldD0[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int32 _fieldFC_index = 0;
	int32 _field100 = 0;
	int32 _field104 = 0;
	int32 _field108 = 0;
	int32 _field10C = 0;
	int32 _field110 = 0;
	int32 _field114 = 0;
	int32 _field118 = 0;
	int32 _field11C = 0;
	int32 _field120 = 0;
	int32 _field124 = 0;
	int32 _field128 = 0;
	int32 _field12C = 0;
	int32 _field130 = 0;
	int32 _field134 = 0;

	int32 _701rp01Series = 0;
	int32 _701rp99Series = 0;
	int32 _agentExchangeMoneySeries = 0;
	int32 _agentGetTelegramSeries = 0;
	int32 _agentGiveParcelSeries = 0;
	int32 _agentShowMapSeries = 0;
	int32 _agentSignsForMoneySeries = 0;
	int32 _agentTalkLoopTjSeries = 0;
	int32 _ripSketchingInNotebookPos3Series = 0;
	int32 _ripTrekHandTalkPos3Series = 0;
	int32 _ripTrekMedReachPos3Series = 0;
	int32 _ripTrekTalkerPos3Series = 0;
	
	machine *_agentPoshExpressMach = nullptr;
	machine *_agentPoshExpressMach02 = nullptr;
	machine *_agentPoshExpressMach03 = nullptr;
	machine *_ripTalksAgentMach = nullptr;
	machine *_safariShadow3Mach = nullptr;
};


} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
