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
#include "chewy/rooms/room24.h"

#include "chewy/cursor.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

static const uint8 KRISTALL_SPR[3][3] = {
	{ 14, 20, 13 },
	{ 20, 13, 14 },
	{ 13, 14, 20 },
};

void Room24::entry() {
	_G(flags).MainInput = false;
	_G(gameState)._personHide[P_CHEWY] = true;
	setPersonPos(0, 0, P_CHEWY, -1);
	_G(gameState).scrollx = 0;
	_G(gameState).scrolly = 0;
	_G(curblk).sprite = _G(room_blk)._detImage;
	_G(cur)->setAnimation(7, 10, -1);
	_G(menu_item) = CUR_USER;
	cursorChoice(CUR_USER);

	if (_G(gameState).R16F5Exit)
		_G(det)->showStaticSpr(10);
	else
		_G(det)->hideStaticSpr(10);

	calc_hebel_spr();
	calc_animation(255);

	for (int16 i = 0; i < 3; i++) {
		if (KRISTALL_SPR[i][_G(gameState).R24Hebel[i]] == 20)
			_G(det)->startDetail(5 + i * 4, 255, ANI_BACK);
	}
}

void Room24::xit() {
	_G(gameState)._personHide[P_CHEWY] = false;
	_G(menu_item) = CUR_WALK;
	cursorChoice(_G(menu_item));
	setPersonPos(263, 144, P_CHEWY, -1);
	_G(gameState).scrollx = 88;
	_G(mouseLeftClick) = false;
	_G(flags).MainInput = true;
}

void Room24::use_hebel(int16 txt_nr) {
	if (!_G(gameState).R24Hebel[txt_nr - 161] ||
		_G(gameState).R24Hebel[txt_nr - 161] == 2) {
		_G(gameState).R24Hebel[txt_nr - 161] = 1;
		_G(gameState).R24HebelDir[txt_nr - 161] ^= 1;
	} else {
		if (_G(gameState).R24HebelDir[txt_nr - 161])
			_G(gameState).R24Hebel[txt_nr - 161] = 0;
		else
			_G(gameState).R24Hebel[txt_nr - 161] = 2;
	}
	calc_hebel_spr();
	calc_animation(txt_nr - 161);

	if (_G(gameState).R24Hebel[0] == 1 && _G(gameState).R24Hebel[1] == 0 && _G(gameState).R24Hebel[2] == 2) {
		_G(gameState).R16F5Exit = true;
		g_engine->_sound->playSound(1, 0);
		g_engine->_sound->stopSound(1);
		_G(det)->startDetail(1, 1, ANI_FRONT);
		_G(det)->showStaticSpr(10);
		_G(atds)->set_ats_str(164, TXT_MARK_NAME, 1, ATS_DATA);

	} else if (_G(gameState).R16F5Exit) {
		_G(det)->hideStaticSpr(10);
		g_engine->_sound->playSound(1, 1);
		g_engine->_sound->stopSound(0);
		_G(det)->startDetail(1, 1, ANI_BACK);
		_G(gameState).R16F5Exit = false;
		_G(atds)->set_ats_str(164, TXT_MARK_NAME, 0, ATS_DATA);
	}
}

void Room24::calc_hebel_spr() {
	if (!_G(gameState).R24FirstEntry) {
		_G(gameState).R24FirstEntry = true;
		_G(gameState).R24Hebel[0] = 2;
		_G(gameState).R24HebelDir[0] = 0;
		_G(gameState).R24Hebel[1] = 1;
		_G(gameState).R24HebelDir[1] = 0;
		_G(gameState).R24Hebel[2] = 0;
		_G(gameState).R24HebelDir[2] = 1;
	}

	for (int16 i = 0; i < 3; i++) {
		for (int16 j = 0; j < 3; j++)
			_G(det)->hideStaticSpr(1 + j + i * 3);

		_G(det)->showStaticSpr(1 + _G(gameState).R24Hebel[i] + i * 3);
		_G(atds)->set_ats_str(166 + i, TXT_MARK_NAME, _G(gameState).R24Hebel[i], ATS_DATA);
	}
}

void Room24::calc_animation(int16 kristall_nr) {
	if (kristall_nr != 255) {
		hideCur();

		if (KRISTALL_SPR[kristall_nr][_G(gameState).R24Hebel[kristall_nr]] == 20) {
			int16 ani_nr = _G(gameState).R24KristallLast[kristall_nr] == 13 ? 7 : 8;
			g_engine->_sound->playSound(ani_nr + kristall_nr * 4, 0);
			g_engine->_sound->stopSound(0);
			_G(det)->hideStaticSpr(_G(gameState).R24KristallLast[kristall_nr] + kristall_nr * 2);
			startSetAILWait(ani_nr + kristall_nr * 4, 1, ANI_BACK);
			startSetAILWait(6 + kristall_nr * 4, 1, ANI_BACK);
			_G(det)->startDetail(5 + kristall_nr * 4, 255, ANI_BACK);

		} else if (_G(gameState).R24KristallLast[kristall_nr] == 20) {
			int16 ani_nr = KRISTALL_SPR[kristall_nr][_G(gameState).R24Hebel[kristall_nr]] == 13 ? 7 : 8;
			g_engine->_sound->stopSound(0);
			g_engine->_sound->playSound(5 + ani_nr + kristall_nr * 4, 0);
			_G(det)->stop_detail(5 + kristall_nr * 4);
			startSetAILWait(6 + kristall_nr * 4, 1, ANI_FRONT);
			startSetAILWait(ani_nr + kristall_nr * 4, 1, ANI_FRONT);
		}

		showCur();
	}

	for (int16 i = 0; i < 6; i++)
		_G(det)->hideStaticSpr(13 + i);

	for (int16 i = 0; i < 3; i++) {
		_G(det)->showStaticSpr(KRISTALL_SPR[i][_G(gameState).R24Hebel[i]] + i * 2);
		_G(gameState).R24KristallLast[i] = KRISTALL_SPR[i][_G(gameState).R24Hebel[i]];
	}
}

} // namespace Rooms
} // namespace Chewy
