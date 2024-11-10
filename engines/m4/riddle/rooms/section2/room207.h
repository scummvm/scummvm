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

#ifndef M4_RIDDLE_ROOMS_SECTION2_ROOM207_H
#define M4_RIDDLE_ROOMS_SECTION2_ROOM207_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room207 : public Room {
public:
	Room207() : Room() {}
	~Room207() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;
	void syncGame(Common::Serializer &s) override;

private:
	int32 _field88 = 0; // unused?
	int32 _field8A = 0;
	int32 _peasantMode = 0;
	int32 _peasantShould = 0;
	bool _fieldA4 = false; // always false??
	int32 _fieldA8 = 0; // unused ?
	bool _pipeFlag = false;
	int32 _fieldAE_rnd = 0;
	bool _fieldB2 = false;
	int32 _fieldB6_counter = 0;
	int32 _fieldC2 = 0;
	bool _fieldC6 = false; // always false??

	bool _commandNotAllowedFl = false;
	bool _digi207r04PlayedFl = false;
	const char *_digiName = nullptr;
	int32 _digiTriggerNum = 0;
	int32 _dispatchTriggerNum = 0;
	bool _dollarFl = false;
	bool _hiddenWalkerFl = false; // Always false??
	KernelTriggerType _nextTriggerMode = KT_PARSE;
	bool _node1Entry2Fl = false;
	bool _ripForegroundFl = false;
	
	int32 _leadPipeSeries = 0;
	int32 _metalRimSeries = 0;
	int32 _oneFrameMoneySpriteSeries = 0;
	int32 _peasant3ToSquatSeries = 0;
	int32 _peasantSquatTo9Series = 0;
	int32 _peskyBegLoopSeries = 0;
	int32 _peskyPointsRipBackgroundSeries = 0;
	int32 _peskyPointsRipForegroundSeries = 0;
	int32 _peskyRockLoopSeries = 0;
	int32 _ppShowsAmuletTakesMoneySeries = 0;
	int32 _rebusAmuletSeries = 0;
	int32 _ripHeadDownTalkOffTd33Series = 0;
	int32 _ripHeadDownTalkOffTd33Series2 = 0;
	int32 _ripTrekArmsXPos3Series = 0;
	int32 _ripTrekLowReacherPos1Series = 0;
	int32 _sevenSpokesSeries = 0;
	int32 _shadowPeskyFrom9ToSquatSeries = 0;
	int32 _wheelSeries = 0;
	int32 _withoutAmuletSeries = 0;
	int32 _woodenRimSeries = 0;

	machine *_dropsWheelMach = nullptr;
	machine *_leadPipeMach = nullptr;
	machine *_leadPipeMach2 = nullptr;
	machine *_metalRimMach = nullptr;
	machine *_oneFrameMoneySpriteMach = nullptr;
	machine *_ppSquatMach = nullptr;
	machine *_ppWalkerMach = nullptr;
	machine *_rebusAmuletMach = nullptr;
	machine *_ripInConvMach = nullptr;
	machine *_ripTrekLowReachMach = nullptr;
	machine *_ripTrekLowReacherPos5Mach = nullptr;
	machine *_safariShadow3Mach = nullptr;
	machine *_sevenSpokesMach = nullptr;
	machine *_wheelMach = nullptr;

	void convHandler();
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
