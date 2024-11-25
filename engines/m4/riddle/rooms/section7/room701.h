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
	const char *_field60_digiName = nullptr; // Useless: Never set
	const char *_field64_digiName = nullptr; // Useless: Never set

	const char *_itemDigiName = nullptr;
	Common::String _convDigiName_1 = ""; // Useless: Assigned but not used?
	Common::String _convDigiName_2 = "";

	bool _field88 = false;
	bool _field8C_unusedFl = false; // Set but never used
	bool _field90 = false;
	bool _fieldBC_unusedFl = false;
	bool _field130 = false;
	bool _alreadyBeenHereFl = false;

	int32 _field50_counter = 0;
	int32 _field68_mode = 0;
	int32 _field6C_should = 0;
	int32 _field72_triggerNum = -1; // Useless : always -1
	int32 _field94_mode = 0;
	int32 _field98_should = 0;
	int32 _field9E_triggerNum = -1; // Useless : always -1
	int32 _conv701aNode = 0;
	int32 _travelDest = 0;
	int32 _lastInventoryCheck = 0;
	int32 _fieldC8 = 0; // Useless: Set but never used
	int32 _inventoryCheckArray[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Useless: Most likely a residual of debug trace
	int32 _inventoryCheckCounter = 0;
	int32 _hasCrystalSkull = 0;
	int32 _hasStickAnsShellMap = 0;
	int32 _hasWheeledToy = 0;
	int32 _hasRebusAmulet = 0;
	int32 _hasShrunkenHead = 0;
	int32 _hasSilverButterfly = 0;
	int32 _hasPostageStamp = 0;
	int32 _hasGermanBankNote = 0;
	int32 _hasWhaleBoneHorn = 0;
	int32 _hasChisel = 0;
	int32 _hasIncenseBurner = 0;
	int32 _hasRomanovEmerald = 0;

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
