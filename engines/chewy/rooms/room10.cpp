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
#include "chewy/rooms/room10.h"

namespace Chewy {
namespace Rooms {

void Room10::entry() {
	if (!_G(gameState).R10Surimy) {
		_G(out)->setPointer(_G(workptr));
		_G(out)->map_spr2screen(_G(ablage)[_G(room_blk).AkAblage], _G(gameState).scrollx, _G(gameState).scrolly);
		_G(out)->setPointer(nullptr);
		_G(fx)->blende1(_G(workptr), _G(screen0), _G(pal), 150, 0, 0);
		_G(gameState).R10Surimy = true;
		_G(fx_blend) = BLEND_NONE;
		flic_cut(FCUT_003);
		setPersonPos(0, 130, P_CHEWY, P_RIGHT);
		autoMove(2, P_CHEWY);
		hideCur();
		start_spz(CH_TALK6, 255, false, P_CHEWY);
		startAadWait(101);
		showCur();

	} else if (_G(gameState).R10SurimyOk) {
		_G(room)->set_timer_status(3, TIMER_STOP);
	}
}

void Room10::get_surimy() {
	hideCur();
	autoMove(4, P_CHEWY);
	start_aad(104, 0);
	flic_cut(FCUT_005);
	_G(gameState).R10SurimyOk = true;
	_G(room)->set_timer_status(3, TIMER_STOP);
	_G(atds)->set_ats_str(77, TXT_MARK_LOOK, 1, ATS_DATA);
	invent_2_slot(18);

	delInventory(_G(gameState).AkInvent);
	showCur();
}

} // namespace Rooms
} // namespace Chewy
