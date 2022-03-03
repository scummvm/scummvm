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
#include "chewy/rooms/room81.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room81::entry() {
	g_engine->_sound->playSound(0, 0);
	g_engine->_sound->playSound(0);
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(zoom_horizont) = 140;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(spieler).DiaAMov = 0;
	if (_G(spieler).r88DestRoom == 84) {
		_G(det)->showStaticSpr(0);
		_G(atds)->del_steuer_bit(488, ATS_AKTIV_BIT, ATS_DATA);
	}

	proc3();

	if (_G(flags).LoadGame || !_G(spieler).flags30_1)
		return;

	setPersonPos(155, 146, P_CHEWY, P_LEFT);
	setPersonPos(103, 115, P_HOWARD, P_RIGHT);
	setPersonPos(62, 112, P_NICHELLE, P_RIGHT);
	_G(spieler).flags30_1 = false;
	_G(maus_links_click) = false;
	_G(spieler).scrollx = 0;
}

void Room81::xit(int16 eib_nr) {
	_G(spieler).R79Val[P_CHEWY] = 1;
	_G(spieler).R79Val[P_HOWARD] = 1;
	_G(spieler)._personRoomNr[P_HOWARD] = 80;
	_G(spieler).R79Val[P_NICHELLE] = 1;
	_G(spieler)._personRoomNr[P_NICHELLE] = 80;
}

void Room81::proc1() {
	int diaNr = -1;
	hideCur();
	autoMove(1, P_CHEWY);
	if (!_G(spieler).flags30_2)
		diaNr = 458;
	else if (_G(spieler).flags30_4)
		diaNr = 460;
	else {
		_G(spieler_mi)[P_CHEWY].Mode = true;
		goAutoXy(222, 97, P_CHEWY, ANI_WAIT);
		goAutoXy(100, 96, P_CHEWY, ANI_WAIT);
		goAutoXy(171, 93, P_CHEWY, ANI_WAIT);
		goAutoXy(143, 62, P_CHEWY, ANI_WAIT);
		goAutoXy(112, 60, P_CHEWY, ANI_WAIT);
		startAadWait(461);
		goAutoXy(143, 62, P_CHEWY, ANI_WAIT);
		_G(spieler)._personHide[P_CHEWY] = true;
		startSetailWait(0, 1, ANI_FRONT);
		_G(det)->startDetail(1, 255, false);
		startAadWait(459);
		_G(det)->stop_detail(1);
		startSetailWait(0, 1, ANI_GO);
		_G(spieler)._personHide[P_CHEWY] = false;
		goAutoXy(171, 93, P_CHEWY, ANI_WAIT);
		goAutoXy(100, 96, P_CHEWY, ANI_WAIT);
		goAutoXy(222, 97, P_CHEWY, ANI_WAIT);
		autoMove(1, P_CHEWY);
		_G(spieler_mi)[P_CHEWY].Mode = false;
		invent_2_slot(DIARY_INV);
		_G(spieler).R84GoonsPresent = true;
		_G(spieler).gottenDiary = true;
	}
	
	if (diaNr != -1) {
		start_spz(CH_TALK12, 255, false, P_CHEWY);
		startAadWait(diaNr);
	}

	showCur();
}

int Room81::proc2() {
	int retVal = 0;
	int diaNr = -1;
	int aniId = -1;

	hideCur();
	if (isCurInventory(110)) {
		autoMove(1, P_CHEWY);
		start_spz_wait(13, 1, false, P_CHEWY);
		aniId = 5;
		diaNr = 462;
		delInventory(_G(spieler).AkInvent);
		_G(spieler).flags30_2 = true;
	} else if (isCurInventory(104)) {
		aniId = 4;
		diaNr = 463;
	} else if (isCurInventory(102)) {
		aniId = 4;
		diaNr = 464;
	}

	if (diaNr != -1) {
		retVal = 1;
		start_spz(aniId, 255, false, P_CHEWY);
		startAadWait(diaNr);
		proc3();
	}

	showCur();
	return retVal;
}

void Room81::proc3() {
	if (!_G(spieler).flags30_2) {
		_G(det)->startDetail(2, 255, false);
		return;
	}

	for (int i = 0; i < 3; ++i)
		_G(det)->startDetail(3 + i, 255, false);

	_G(det)->stop_detail(2);
	_G(atds)->del_steuer_bit(486, ATS_AKTIV_BIT, ATS_DATA);
	_G(atds)->set_steuer_bit(490, ATS_AKTIV_BIT, ATS_DATA);
}

} // namespace Rooms
} // namespace Chewy
