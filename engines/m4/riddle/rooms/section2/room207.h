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

private:
	int32 _field4C = 0;
	int32 _field50 = 0;
	int32 _field88 = 0;
	KernelTriggerType _field89_triggerMode = KT_PARSE;
	int32 _field8A = 0;
	int32 _field8E = 0;
	int32 _field92 = 0;
	const char *_field96_digiName = nullptr;
	int32 _field9A_triggerNum = 0;
	int32 _field9E_triggerNum = 0;
	int32 _fieldA2 = 0;
	int32 _fieldA4 = 0;
	int32 _fieldA6 = 0;
	int32 _fieldA8 = 0;
	int32 _fieldAA = 0;
	int32 _fieldAC_ripForegroundFl = 0;
	int32 _fieldAE = 0;
	int32 _fieldB2 = 0;
	int32 _fieldB6 = 0;
	int32 _fieldBA = 0;
	int32 _fieldBE = 0;
	int32 _fieldC2 = 0;
	int32 _fieldC6 = 0;
	int32 _fieldCA = 0;

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
	int32 _sevenSpokesSeries = 0;
	int32 _shadowPeskyFrom9ToSquatSeries = 0;
	int32 _wheelSeries = 0;
	int32 _withoutAmuletSeries = 0;
	int32 _woodenRimSeries = 0;

	machine *_leadPipeMach = nullptr;
	machine *_metalRimMach = nullptr;
	machine *_ppWalkerMach = nullptr;
	machine *_rebusAmuletMach = nullptr;
	machine *_sevenSpokesMach = nullptr;
	machine *_wheelMach = nullptr;
	machine *_ppSquatMach = nullptr;

	machine *_field84 = nullptr;
	machine *_field68 = nullptr;

};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
