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

#include "common/events.h"

#include "freescape/freescape.h"

namespace Freescape {

int FreescapeEngine::decodeAmigaAtariKey(int index) {
	switch (index) {
	case 0x30:
		return 0; // shoot?
	case 0x44:
		return Common::KEYCODE_d;
	case 0x4c:
		return Common::KEYCODE_l;
	case 0x50:
		return Common::KEYCODE_p;
	case 0x55:
		return 0; // turn around
	case 0x96:
		return Common::KEYCODE_UP;
	case 0x97:
		return Common::KEYCODE_DOWN;
	case 0x98:
		return Common::KEYCODE_w;
	case 0x99:
		return Common::KEYCODE_q;
	default:
		error("Invalid key index: %x", index);
	}
	return 0;
}

int FreescapeEngine::decodeDOSKey(int index) {
	switch (index) {
	case 1:
		return 	Common::KEYCODE_r;
	case 2:
		return Common::KEYCODE_f;
	case 3:
		return Common::KEYCODE_UP;
	case 4:
		return Common::KEYCODE_DOWN;
	case 5:
		return Common::KEYCODE_q;
	case 6:
		return Common::KEYCODE_w;
	case 7:
		return Common::KEYCODE_p;
	case 8:
		return Common::KEYCODE_l;
	case 30:
		return Common::KEYCODE_SPACE;
	case 40:
		return Common::KEYCODE_d;
	default:
		error("Invalid key index: %x", index);
	}
	return 0;
}

} // End of namespace Freescape
