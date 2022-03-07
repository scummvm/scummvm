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

#include "common/textconsole.h"
#include "chewy/mouse.h"
#include "chewy/events.h"
#include "chewy/globals.h"

namespace Chewy {

bool _cursorMoveFl;

InputMgr::InputMgr() {
}

InputMgr::~InputMgr() {
}

int16 InputMgr::mouseVector(int16 x, int16 y, const int16 *tbl, int16 nr) {
	int16 i = -1;
	for (int16 j = 0; (j < nr * 4) && (i == -1); j += 4) {
		if ((x >= tbl[j]) && (x <= tbl[j + 2]) && (y >= tbl[j + 1]) && (y <= tbl[j + 3]))
			i = j / 4;
	}

	return i;
}

int16 InputMgr::getSwitchCode() {
	int16 switch_code = 0;

	if (_G(minfo)._button == 2) {
		switch_code = Common::KEYCODE_ESCAPE;
	} else if (_G(minfo)._button == 1)
		switch_code = MOUSE_LEFT;
	else if (_G(minfo)._button == 4)
		switch_code = MOUSE_CENTER;

	if (g_events->_kbInfo._keyCode != 0)
		switch_code = (int16)g_events->_kbInfo._keyCode;

	return switch_code;
}

} // namespace Chewy
