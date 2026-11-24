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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DUNE_POINTER_BUTTONS_H
#define DUNE_POINTER_BUTTONS_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/rect.h"

namespace Dune {

inline bool panelControlContains(const Common::Point &at, int left, int top, int right, int bottom) {
	// D6E2..D6F5 excludes both X edges and the top. DEC BX before the
	// final comparison includes the bottom, unlike D6FE's portrait boxes.
	return at.x > left && at.x < right && at.y > top && at.y <= bottom;
}

inline Common::Point cursorHotspot(const byte *data) {
	// DB7D and DNVGA1888 subtract the first word from DX (screen X),
	// then the second from BX (screen Y). The mask header is X,Y, not Y,X.
	return Common::Point(READ_LE_INT16(data), READ_LE_INT16(data + 2));
}

class PointerButtons {
public:
	// DF79..DF94 folds current keyboard left with physical left masked by
	// the PREVIOUS keyboard sample (DC57). DB4C returns held plus edge bits.
	byte sample(bool keyboard, bool mouse) {
		const bool pressed = keyboard || (mouse && !_keyboard);
		_buttons = (pressed ? 1 : 0) | (pressed != down() ? 4 : 0);
		_keyboard = keyboard;
		return _buttons;
	}
	byte buttons() const { return _buttons; }
	bool down() const { return (_buttons & 1) != 0; }
	void reset(bool mouse = false) { _keyboard = false; _buttons = mouse ? 1 : 0; }

private:
	bool _keyboard = false;
	byte _buttons = 0;
};

} // namespace Dune

#endif
