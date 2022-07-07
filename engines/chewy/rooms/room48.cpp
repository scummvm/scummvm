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
#include "chewy/rooms/room48.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room48::entry() {
	_G(mouseLeftClick) = false;
	_G(gameState).scrollx = 0;
	_G(gameState).scrolly = 0;
	showCur();
	calc_pic();
	_G(SetUpScreenFunc) = setup_func;
	_G(timer_nr)[0] = _G(room)->set_timer(255, 20);
}

bool Room48::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0])
		frage();
	else
		return true;

	return false;
}

void Room48::calc_pic() {
	for (int16 i = 0; i < 2; i++) {
		_G(atds)->setControlBit(312 + i, ATS_ACTIVE_BIT);
		_G(gameState).R48Auswahl[3 + i] = 0;
	}

	_G(gameState).R48Auswahl[0] = 1;
	_G(gameState).R48Auswahl[1] = 1;
	_G(gameState).R48Auswahl[2] = 1;

	if (_G(obj)->checkInventory(VCARD_INV)) {
		_G(gameState).R48Auswahl[3] = 1;
		_G(atds)->delControlBit(312, ATS_ACTIVE_BIT);
		_G(det)->showStaticSpr(4);
	}

	if (_G(obj)->checkInventory(KAPPE_INV)) {
		_G(gameState).R48Auswahl[4] = 1;
		_G(atds)->delControlBit(313, ATS_ACTIVE_BIT);
		_G(det)->showStaticSpr(4);
	}
}

void Room48::frage() {
	if (!_G(flags).AutoAniPlay) {
		_G(flags).AutoAniPlay = true;
		hideCur();
		startSetAILWait(1, 1, ANI_FRONT);
		_G(det)->showStaticSpr(6);
		startSetAILWait(1, 1, ANI_BACK);
		_G(det)->hideStaticSpr(6);
		_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
		showCur();
		_G(flags).AutoAniPlay = false;
	}
}

void Room48::setup_func() {
	for (int16 i = 0; i < 5; i++)
		_G(det)->hideStaticSpr(1 + i);

	if (_G(flags).ShowAtsInvTxt) {
		if (_G(menu_display) == 0) {
			_G(menu_item) = CUR_USE;
			cur_2_inventory();
			cursorChoice(CUR_POINT);
			const int16 idx = _G(det)->maus_vector(g_events->_mousePos.x, g_events->_mousePos.y);

			if (idx != -1) {
				if (_G(gameState).R48Auswahl[idx]) {
					_G(det)->showStaticSpr(1 + idx);

					if (_G(mouseLeftClick)) {
						int16 r_nr;
						switch (idx) {
						case 0:
							r_nr = 45;
							break;

						case 1:
							r_nr = 49;
							break;

						case 2:
							r_nr = 54;
							break;

						case 3:
							r_nr = 57;
							break;

						case 4:
							r_nr = 56;
							break;

						default:
							r_nr = -1;
							break;
						}

						if (r_nr != -1) {
							_G(SetUpScreenFunc) = nullptr;
							_G(det)->hideStaticSpr(1 + idx);
							hideCur();
							_G(room)->set_timer_status(255, TIMER_STOP);
							_G(room)->set_timer_status(0, TIMER_STOP);
							_G(det)->stopDetail(0);
							_G(det)->del_static_ani(0);
							startSetAILWait(2, 1, ANI_FRONT);
							_G(det)->stopSound(0);
							_G(menu_item) = CUR_WALK;
							cursorChoice(_G(menu_item));
							showCur();
							_G(gameState).R48TaxiEntry = true;
							_G(mouseLeftClick) = false;
							setupScreen(DO_SETUP);

							for (int16 i = 0; i < MAX_PERSON; i++) {
								if (_G(gameState).R48TaxiPerson[i]) {
									_G(gameState)._personHide[i] = false;
									_G(gameState).R48TaxiPerson[i] = false;
								}
							}

							if (_G(gameState)._personRoomNr[P_HOWARD] == 48) {
								_G(gameState)._personRoomNr[P_HOWARD] = r_nr;
							}
							switchRoom(r_nr);
						}
					}
				}
			}
		}
	}
}

} // namespace Rooms
} // namespace Chewy
