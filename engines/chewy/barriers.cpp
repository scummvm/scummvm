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

#include "common/file.h"
#include "chewy/chewy.h"
#include "chewy/globals.h"
#include "chewy/barriers.h"

namespace Chewy {

int16 Barriers::getBarrierId(int16 x, int16 y) {
	const int16 width = _G(room)->_barriers->getWidth();
	const byte *data = _G(room)->_barriers->getData();
	const int16 result = getBarrierId(((y / 8) * width) + (x / 8), data + 6);
	delete[] data;
	return result;
}

int16 Barriers::getBarrierId(int16 g_idx, const byte *buffer) {
	int16 idx_nr = buffer[g_idx];

	switch (idx_nr) {
	case 40:
		switch (_G(gameState)._personRoomNr[P_CHEWY]) {
		case 8:
			if (_G(gameState).R8GTuer)
				return 0;
			break;

		case 9:
			if (!_G(gameState).R9Grid)
				return 0;
			break;

		case 16:
			if (!_G(gameState).R16F5Exit)
				return 0;
			break;

		case 17:
			if (_G(gameState).R17Location != 1)
				return 0;
			break;

		case 21:
			if (!_G(gameState).R21Laser2Weg)
				return 0;
			break;

		case 31:
			if (!_G(gameState).R31KlappeZu)
				return 0;
			break;

		case 41:
			if (!_G(gameState).R41LolaOk)
				return 0;
			break;

		case 52:
			if (!_G(gameState).R52LichtAn)
				return 2;
			else
				return 4;
			break;

		case 71:
			return _G(gameState).R71LeopardVined ? 1 : 0;

		case 76:
			return _G(gameState).flags29_4 ? 4 : 0;

		case 84:
			if (!_G(gameState).R88UsedMonkey)
				_G(gameState).R84GoonsPresent = true;
			break;

		case 86:
			if (!_G(gameState).flags32_2)
				return 0;
			break;

		case 94:
			if (!_G(gameState).flags35_10)
				return 0;
			break;

		case 97:
			if (_G(gameState).flags35_80)
				return 0;
			break;

		default:
			break;
		}
		break;

	case 41:
		switch (_G(gameState)._personRoomNr[P_CHEWY]) {
		case 17:
			if (_G(gameState).R17Location != 2)
				return 0;
			break;

		case 21:
			if (!_G(gameState).R21Laser1Weg)
				return 0;
			else
				return 3;

		case 37:
			if (!_G(gameState).R37RoosterFoughtWithDog)
				return 0;
			break;

		case 52:
			if (!_G(gameState).R52TuerAuf)
				return 2;
			else
				return 4;
			break;

		case 97:
			if (_G(gameState).flags36_20)
				return 0;
			break;

		default:
			break;
		}
		break;

	case 42:
		if (_G(gameState)._personRoomNr[P_CHEWY] == 97 && !_G(gameState).flags37_1)
			return 0;
		break;

	default:
		break;
	}

	return idx_nr;
}

} // namespace Chewy
