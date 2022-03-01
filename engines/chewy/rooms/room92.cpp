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

#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/room.h"
#include "chewy/rooms/room92.h"

namespace Chewy {
namespace Rooms {

void Room92::entry() {
	const int oldScrollX = _G(spieler.scrollx);
	_G(spieler).scrollx = 0;
	cur_2_inventory();
	hideCur();
	hide_person();
	startAadWait(_G(spieler).SVal2);
	switch (_G(spieler).SVal2) {
	case 487:
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_103);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(spieler).scrollx = oldScrollX;
		show_person();
		switch_room(_G(spieler).SVal1);
		break;
	case 488:
		_G(flags).StaticUseTxt = true;
		flic_cut(FCUT_106);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_105);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_104);
		register_cutscene(27);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_103);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(spieler).PersonRoomNr[P_HOWARD] = 46;
		_G(spieler).flags33_8 = true;
		show_person();
		switch_room(46);
		break;
	case 538:
		_G(flags).StaticUseTxt = true;
		flic_cut(FCUT_106);
		show_person();
		_G(spieler).PersonRoomNr[P_HOWARD] = 94;
		_G(spieler).flags35_8 = true;
		switch_room(94);
		break;
	default:
		show_person();
		_G(spieler).scrollx = oldScrollX;
		switch_room(_G(spieler).SVal1);
		break;
	}
	_G(spieler).SVal2 = 5000;
}

} // namespace Rooms
} // namespace Chewy
