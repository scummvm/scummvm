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

#ifndef M4_RIDDLE_ROOMS_SECTION2_ROOM205_H
#define M4_RIDDLE_ROOMS_SECTION2_ROOM205_H

#include "m4/riddle/rooms/room.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room205 : public Room {
public:
	Room205() : Room() {}
	~Room205() override {}

	void preload() override;
	void init() override;
	void pre_parser() override;
	void parser() override;
	void daemon() override;

private:
	int32 _fieldD8 = 0;
	int32 _fieldDC = 0;
	int32 _fieldE0 = 0;
	int32 _fieldE4 = 0;
	int32 _field198 = 0;
	int32 _field19C = 0; // unused??
	int32 _field1A0 = 0;

	bool _askUnhideMyWalkerFl = false;
	bool _showMeiTalkFl = false;

	int32 _205all0Series = 0;
	int32 _205all1Series = 0;
	int32 _205all4Series = 0;
	int32 _205all5Series = 0;
	int32 _205all6Series = 0;
	int32 _205all7Series = 0;
	int32 _205FireInBrazierSeries = 0;
	int32 _205Fite1Series = 0;
	int32 _205Fite2Series = 0;
	int32 _205Fite3Series = 0;
	int32 _205Fite4Series = 0;
	int32 _205GunFireSeries = 0;
	int32 _205GunPointedSeries = 0;
	int32 _205JournalRippedPopupSeries = 0;
	int32 _205ktlk1Series = 0;
	int32 _205ktlk2Series = 0;
	int32 _205mc01Series = 0;
	int32 _205mc02Series = 0;
	int32 _205mc03Series = 0;
	int32 _205MeiSighAndTalkSeries = 0;
	int32 _205mtlk1Series = 0;
	int32 _205RipGetsBitchSlappedSeries = 0;
	int32 _205rp01Series = 0;
	int32 _205rp02Series = 0;
	int32 _205rp03Series = 0;
	int32 _205rtlk1Series = 0;
	int32 _205rtlk2Series = 0;
	int32 _205rtlk3Series = 0;
	int32 _205rtlk4Series = 0;
	int32 _205rtlk5Series = 0;
	int32 _205ShenGouStaresSeries = 0;
	int32 _205strlk1Series;
	int32 _205TabletsSeries = 0;
	int32 _ripGetsShotSeries = 0;
	int32 _ripTalkerPos5Series = 0;
	int32 _ripTrekHeadTurnPos5Series = 0;
	int32 _ripTrekLowReacherPos5Series = 0;
	int32 _ripTrekLowReachPos2Series = 0;
	int32 _ripTrekMedReachHandPos1Series = 0;
	int32 _unkInventoryId = 0;

	machine *_205all0Mach = nullptr;
	machine *_205all9Mach = nullptr;
	machine *_205CharcoalSpriteMach = nullptr;
	machine *_205FireInBrazierMach = nullptr;
	machine *_205GunInBrazierMach = nullptr;
	machine *_205GunPointedMach = nullptr;
	machine *_205JournalCharcoalPopupMach = nullptr;
	machine *_205LeftEntranceTabletMach = nullptr;
	machine *_205MeiStanderMach = nullptr;
	machine *_205rp1Mach = nullptr;
	machine *_205TabletsMach = nullptr;
	machine *_candlemanShadow3Mach = nullptr;
	machine *_kuangsShadow2Mach = nullptr;
	machine *_kuangTalkerMach = nullptr;
	machine *_kuangWalker205Mach = nullptr;
	machine *_mcEntranceTrekMach = nullptr;
	machine *_safariShadow1Mach = nullptr;
	machine *_shenGuoShadow12Mach = nullptr;
	machine *_shenSuitGunWalkerMach = nullptr;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
