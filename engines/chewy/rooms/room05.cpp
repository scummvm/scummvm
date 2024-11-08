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
#include "chewy/ani_dat.h"
#include "chewy/room.h"
#include "chewy/rooms/room05.h"

namespace Chewy {
namespace Rooms {

void Room5::entry() {
	if (_G(gameState).R5Terminal)
		_G(det)->startDetail(6, 255, ANI_FRONT);
}

void Room5::pushButton() {
	if (_G(gameState).R5Terminal) {
		int16 strNr;
		if (_G(gameState).R5Door == false) {
			startSetAILWait(9, 1, ANI_FRONT);
			_G(gameState).room_e_obj[6].Attribut = EXIT_TOP;
			strNr = 1;
		} else {
			startSetAILWait(9, 1, ANI_BACK);
			_G(gameState).room_e_obj[6].Attribut = 255;
			strNr = 0;
		}
		_G(atds)->set_all_ats_str(29, strNr, ATS_DATA);
		_G(gameState).R5Door ^= 1;
		_G(obj)->calc_rsi_flip_flop(SIB_DOOR_R5);
	} else {
		startAadWait(1);
	}
}

} // namespace Rooms
} // namespace Chewy
