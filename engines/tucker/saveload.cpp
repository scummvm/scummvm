/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/savefile.h"

#include "tucker/tucker.h"

namespace Tucker {

enum {
	kCurrentGameStateVersion = 1
};

void TuckerEngine::generateGameStateFileName(int num, char *dst, int len, bool prefixOnly) const {
	if (prefixOnly) {
		snprintf(dst, len, "%s.*", _targetName.c_str());
	} else {
		snprintf(dst, len, "%s.%d", _targetName.c_str(), num);
	}
	dst[len] = 0;
}

static void saveOrLoadInt(Common::WriteStream &stream, int &i) {
	stream.writeSint32LE(i);
}

static void saveOrLoadInt(Common::ReadStream &stream, int &i) {
	i = stream.readSint32LE();
}

template <class S>
void TuckerEngine::saveOrLoadGameStateData(S &s) {
	for (int i = 0; i < 300; ++i) {
		saveOrLoadInt(s, _flagsTable[i]);
	}
	for (int i = 0; i < 40; ++i) {
		saveOrLoadInt(s, _inventoryObjectsList[i]);
	}
	for (int i = 0; i < 50; ++i) {
		saveOrLoadInt(s, _inventoryItemsState[i]);
	}
	for (int i = 0; i < 50; ++i) {
		saveOrLoadInt(s, _panelObjectsOffsetTable[i]);
	}
	saveOrLoadInt(s, _mainSpritesBaseOffset);
	saveOrLoadInt(s, _selectedObject.xPos);
	saveOrLoadInt(s, _selectedObject.yPos);
	saveOrLoadInt(s, _locationNum);
	saveOrLoadInt(s, _xPosCurrent);
	saveOrLoadInt(s, _yPosCurrent);
	saveOrLoadInt(s, _inventoryObjectsCount);
	saveOrLoadInt(s, _inventoryObjectsOffset);
}

void TuckerEngine::loadGame(int slot) {
	char gameStateFileName[64];
	generateGameStateFileName(slot, gameStateFileName, 63);
	Common::InSaveFile *f = _saveFileMan->openForLoading(gameStateFileName);
	if (f) {
		uint16 version = f->readUint16LE();
		if (version < kCurrentGameStateVersion) {
			warning("Unsupported gamestate version %d (slot %d)", version, slot);
		} else {
			f->skip(2);
			saveOrLoadGameStateData(*f);
			if (f->ioFailed()) {
				warning("Can't read file '%s'", gameStateFileName);
			} else {
				_nextLocationNum = _locationNum;
				setBlackPalette();
				loadBudSpr(0);
			}
		}
		delete f;
	}
}

void TuckerEngine::saveGame(int slot) {
	char gameStateFileName[64];
	generateGameStateFileName(slot, gameStateFileName, 63);
	Common::OutSaveFile *f = _saveFileMan->openForSaving(gameStateFileName);
	if (f) {
		f->writeUint16LE(kCurrentGameStateVersion);
		f->writeUint16LE(0);
		saveOrLoadGameStateData(*f);
		f->finalize();
		if (f->ioFailed()) {
			warning("Can't write file '%s'", gameStateFileName);
		}
		delete f;
	}
}

} // namespace Tucker
