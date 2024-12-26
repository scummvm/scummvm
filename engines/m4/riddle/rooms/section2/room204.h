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

#ifndef M4_RIDDLE_ROOMS_SECTION2_ROOM204_H
#define M4_RIDDLE_ROOMS_SECTION2_ROOM204_H

#include "m4/riddle/rooms/section2/section2_room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room204 : public Section2Room {
public:
	Room204() : Section2Room() {}
	~Room204() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;
	void syncGame(Common::Serializer &s) override;

private:
	void addLookMalletHotspot();
	void addMovingMeiHotspot();
	void conv204a();
	void deleteMalletHotspot();
	void deleteMeiCheiHotspot();
	void gameSetScale(int32 frontY, int32 backY, int32 frontS, int32 backS);
	void handleRipBangsBong();
	void initWalkerSeries();
	void initPriestWalker();
	void killMcMach();
	void killPriestWalkerMach();
	void setWalkerDestX();

	int32 _dword1A189C = 0;

	bool _checkNode10Fl = false;
	bool _checkNode10NegWhoEntry1Fl = false;
	bool _checkNode11NegWhoEntry0Fl = false;
	bool _checkNode20Fl = false;
	bool _ripMachineFlag = false;
	bool _meiMachineFlag = false;

	int32 _field4 = 0;
	int32 _field10 = 0;
	int32 _field14 = 0;
	int32 _field18_triggerNum = 0;
	int32 _field24_triggerNum = 0;
	int32 _field28_triggerNum = 0;
	int32 _field2C = 0;
	int32 _field34_x = 0;
	int32 _field38_y = 0;
	int32 _field3C_facing = 0;
	int32 _field40 = 0;
	int32 _field44_triggerNum = 0;
	int32 _field48_triggerNum = 0;
	int32 _ripBangsBongSeries = 0;
	int32 _fieldBC_trigger = 0;
	int32 _fieldC0_trigger = 0;
	int32 _fieldC4 = 0;
	int32 _fieldC8_trigger = 0;
	int32 _fieldCC_trigger = 0;
	int32 _fieldD0 = 0;
	int32 _fieldD4 = 0;
	int32 _fieldD8_facing = 0;
	int32 _fieldE0_x = 0;
	int32 _fieldE4_walkerDestX = 0;
	int32 _fieldEC = 0;
	int32 _fieldF0 = 0;
	int32 _fieldF4 = 0;
	int32 _fieldF8 = 0;
	int32 _fieldFC_infoX = 0;
	int32 _field100_infoY = 0;
	int32 _field104 = 0;
	int32 _field108 = 0;
	int32 _field10C_x = 0;
	int32 _field110_y = 0;
	int32 _field114_facing = 0;
	int32 _field118_scale = 0;
	int32 _field11C_depth = 0;
	int32 _field124 = 0;
	int32 _field128 = 0;
	int32 _field12C_triggerNum = 0;
	int32 _field130 = 0;
	int32 _field134 = 0;
	int32 _field138 = 0;
	int32 _field13C_triggerNum = 0;
	int32 _field140 = 0;
	int32 _field144_triggerNum = 0;
	int32 _field16C = 0;
	int32 _field180 = 0;
	int32 _field184 = 0;
	int32 _field188 = 0;
	
	int32 _204pu99Series = 0;
	int32 _acolyteGrabsMalletSeries = 0;
	int32 _acolyteSaysHaltSeries = 0;
	int32 _courtyardGongSeries = 0;
	int32 _malletSpriteSeries = 0;
	int32 _meiReadsTabletsSeries = 0;
	int32 _meiShowsRipHerPassesSeries = 0;
	int32 _meiTalksPos3Series = 0;
	int32 _meiTrekRtHandOutPos2Series = 0;
	int32 _meiTrekTalkerPos4Series = 0;
	int32 _priestTurnsFrom3To9Series = 0;
	int32 _priestWalkerSeries = 0;
	int32 _ripDropsSeries = 0;
	int32 _ripSketchingInNotebookPos2Series = 0;
	int32 _ripTrekHandTalkPos3Series = 0;
	int32 _ripTrekHeadTurnPos5Series = 0;
	int32 _ripTrekLHandTalkPos4Series = 0;
	int32 _ripTrekLowReachPos2Series = 0;
	int32 _ripTrekMedReachHandPos1Series = 0;
	int32 _ripTrekTalkerPos3Series = 0;
	int32 _ripTrekTwoHandTalkPos2Series = 0;

	machine *_204pu05Mach = nullptr;
	machine *_204pu99Mach = nullptr;
	machine *_acolyteGuardingEntranceMach = nullptr;
	machine *_courtyardGongMach = nullptr;
	machine *_mcMach = nullptr;
	machine *_malletSpriteMach = nullptr;
	machine *_meiChenOtherStatesMach = nullptr;
	machine *_priestTurningStateMach = nullptr;
	machine *_priestWalkerMach = nullptr;
	machine *_ripDeltaMachineStateMach = nullptr;
	machine *_safariShadow2Mach = nullptr;
	machine *_safariShadow3Mach = nullptr;
	machine *_silverButterflyCoinMach = nullptr;
};


} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
