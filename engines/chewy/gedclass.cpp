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
#include "chewy/gedclass.h"

namespace Chewy {

void GedClass::load_ged_pool(GedChunkInfo *Gh, int16 ch_nr, byte *speicher) {
	Common::File f;
	if (!f.open(EPISODE1_GEP))
		error("load_ged_pool error");

	f.seek(6, SEEK_SET); // skip header (4 bytes ID + 2 bytes number)

	// Scan for the correct index entry
	int i = 0;
	uint32 length = 0;

	do {
		length = f.readUint32LE();

		Gh->pos.x = f.readSint16LE();
		Gh->pos.y = f.readSint16LE();
		Gh->level = f.readSint16LE();

		if (i != ch_nr) {
			// Skip over the entry's data
			f.seek(length, SEEK_CUR);
		}
	} while (++i <= ch_nr);

	if (f.read(speicher, length) != length) {
		error("load_ged_chunk error");
	}
}

int16 GedClass::getBarrierId(int16 x, int16 y, int16 x_anz, byte *speicher) {
	return getBarrierId(((y / 8) * x_anz) + (x / 8), speicher);
}

int16 GedClass::getBarrierId(int16 g_idx, byte *speicher) {
	int16 idx_nr = speicher[g_idx];

	switch (idx_nr) {
	case 40:
		switch (_G(gameState)._personRoomNr[P_CHEWY]) {
		case 8:
			if (_G(gameState).R8GTuer)
				idx_nr = 0;
			break;

		case 9:
			if (!_G(gameState).R9Gitter)
				idx_nr = 0;
			break;

		case 16:
			if (!_G(gameState).R16F5Exit)
				idx_nr = 0;
			break;

		case 17:
			if (_G(gameState).R17Location != 1)
				idx_nr = 0;
			break;

		case 21:
			if (!_G(gameState).R21Laser2Weg)
				idx_nr = 0;
			break;

		case 31:
			if (!_G(gameState).R31KlappeZu)
				idx_nr = 0;
			break;

		case 41:
			if (!_G(gameState).R41LolaOk)
				idx_nr = 0;
			break;

		case 52:
			if (!_G(gameState).R52LichtAn)
				idx_nr = 2;
			else
				idx_nr = 4;
			break;

		case 71:
			idx_nr = _G(gameState).R71LeopardVined ? 1 : 0;
			break;

		case 76:
			idx_nr = _G(gameState).flags29_4 ? 4 : 0;
			break;

		case 84:
			if (!_G(gameState).R88UsedMonkey)
				_G(gameState).R84GoonsPresent = true;
			break;

		case 86:
			if (!_G(gameState).flags32_2)
				idx_nr = 0;
			break;

		case 94:
			if (!_G(gameState).flags35_10)
				idx_nr = 0;
			break;

		case 97:
			if (_G(gameState).flags35_80)
				idx_nr = 0;
			break;

		default:
			break;
		}
		break;

	case 41:
		switch (_G(gameState)._personRoomNr[P_CHEWY]) {
		case 17:
			if (_G(gameState).R17Location != 2)
				idx_nr = 0;
			break;

		case 21:
			if (!_G(gameState).R21Laser1Weg) {
				idx_nr = 0;
			} else
				idx_nr = 3;
			break;

		case 37:
			if (!_G(gameState).R37Kloppe)
				idx_nr = 0;
			break;

		case 52:
			if (!_G(gameState).R52TuerAuf)
				idx_nr = 2;
			else
				idx_nr = 4;
			break;

		case 97:
			if (_G(gameState).flags36_20)
				idx_nr = 0;
			break;

		default:
			break;
		}
		break;

	case 42:
		if (_G(gameState)._personRoomNr[P_CHEWY] == 97) {
			if (!_G(gameState).flags37_1)
				idx_nr = 0;
		}
		break;

	default:
		break;
	}

	return idx_nr;
}

} // namespace Chewy
