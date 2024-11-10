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

private:
	void initWalkerSeries();
	void addMovingMeiHotspot();
	void deleteMalletHotspot();
	void addLookMalletHotspot();

	int32 _field4 = 0;
	int32 _field10 = 0;
	int32 _field14 = 0;
	int32 _field18_triggerNum = 0;
	int32 _field24_triggerNum = 0;
	int32 _field28_triggerNum = 0;
	int32 _field2C = 0;
	int32 _field40 = 0;
	int32 _field44_triggerNum = 0;
	int32 _field48_triggerNum = 0;
	int32 _field68 = 0;
	int32 _field78_series = 0;
	int32 _fieldBC_trigger = 0;
	int32 _fieldC0_trigger = 0;
	int32 _fieldC4 = 0;
	int32 _fieldCC_trigger = 0;
	int32 _fieldD4 = 0;
	int32 _fieldD8 = 0;
	int32 _fieldDC = 0;
	int32 _fieldE0 = 0;
	int32 _fieldE4_walkerDestX = 0;
	int32 _fieldEC = 0;
	int32 _fieldF0 = 0;
	int32 _fieldFC_infoX = 0;
	int32 _field100_infoY = 0;
	int32 _field104 = 0;
	int32 _field108 = 0;
	int32 _field180 = 0;
	int32 _field184 = 0;
	int32 _field188 = 0;

	int32 _field90_series = 0;
	int32 _ripTrekHandTalkPos3Series = 0;
	int32 _courtyardGongSeries = 0;
	int32 _malletSpriteSeries = 0;
	int32 _meiShowsRipHerPassesSeries = 0;
	int32 _ripDropsSeries = 0;
	int32 _ripTrekHeadTurnPos5Series = 0;
	int32 _ripTrekLHandTalkPos4Series = 0;
	int32 _ripTrekLowReachPos2Series = 0;
	int32 _ripTrekTalkerPos3Series = 0;

	machine *_courtyardGongMach = nullptr;
	machine *_mcMach = nullptr;
	machine *_malletSpriteMach = nullptr;
	machine *_ripDeltaMachineStateMach = nullptr;
	machine *_safariShadow3Mach = nullptr;
	machine *_silverButterflyCoinMach = nullptr;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
