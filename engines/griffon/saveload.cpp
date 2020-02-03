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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 *              Originally written by Syn9 in FreeBASIC with SDL
 *              http://syn9.thehideoutgames.com/index_backup.php
 *
 *            Ported to plain C for GCW-Zero handheld by Dmitry Smagin
 *                http://github.com/dmitrysmagin/griffon_legend
 *
 *
 *                 Programming/Graphics: Daniel "Syn9" Kennedy
 *                     Music/Sound effects: David Turner
 *
 *                   Beta testing and gameplay design help:
 *                    Deleter, Cha0s, Aether Fox, and Kiz
 *
 */

#include "griffon/griffon.h"

#include "common/savefile.h"
#include "engines/metaengine.h"

namespace Griffon {

#define PRINT(A,B)                      \
	do {                                \
		char line[256];                 \
		sprintf(line, A "\n", B);       \
		file->write(line, strlen(line)); \
	} while(0)

#define INPUT(A, B)                 \
	do {                            \
		Common::String line;        \
		line = file->readLine();     \
		sscanf(line.c_str(), A, B); \
	} while(0)

Common::String GriffonEngine::getSaveStateName(int slot) const {
	return (_targetName + Common::String::format(".s%02d", slot));
}

Common::Error GriffonEngine::loadGameState(int slot) {
	Common::Error result = Engine::loadGameState(slot);
	if (result.getCode() == Common::kNoError) {
		_saveSlot = slot;
		_gameMode = kGameModeLoadGame;
	}

	return result;
}

Common::Error GriffonEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::String saveDesc = Common::String::format("Level: %d Map: %d", _player.level, _curMap);
	return Engine::saveGameState(slot, isAutosave ? desc : saveDesc, isAutosave);
}

Common::Error GriffonEngine::loadGameStream(Common::SeekableReadStream *file) {
	INPUT("%i", &_player.level);

	if (_player.level > 0) {
		char temp[256];

		INPUT("%i", &_secStart);
		INPUT("%s", temp);

		INPUT("%f", &_player.px);
		INPUT("%f", &_player.py);
		INPUT("%f", &_player.opx);
		INPUT("%f", &_player.opy);
		INPUT("%i", &_player.walkDir);
		INPUT("%f", &_player.walkFrame);
		INPUT("%f", &_player.walkSpeed);
		INPUT("%f", &_player.attackFrame);
		INPUT("%f", &_player.attackSpeed);
		INPUT("%i", &_player.hp);
		INPUT("%i", &_player.maxHp);
		INPUT("%f", &_player.hpflash);
		INPUT("%i", &_player.level);
		INPUT("%i", &_player.maxLevel);
		INPUT("%i", &_player.sword);
		INPUT("%i", &_player.shield);
		INPUT("%i", &_player.armour);
		for (int i = 0; i < 5; i++) {
			INPUT("%i", &_player.foundSpell[i]);
			INPUT("%f", &_player.spellCharge[i]);
		}
		for (int a = 0; a < 5; a++) {
			INPUT("%i", &_player.inventory[a]);
		}
		INPUT("%f", &_player.attackStrength);
		INPUT("%i", &_player.spellDamage);
		INPUT("%i", &_player.swordDamage);
		INPUT("%i", &_player.exp);
		INPUT("%i", &_player.nextLevel);
		for (int a = 0; a <= 99; a++) {
			for (int b = 0; b <= 9; b++) {
				INPUT("%i", &_scriptFlag[a][b]);
			}
		}
		INPUT("%i", &_curMap);

		for (int a = 0; a <= 999; a++) {
			for (int b = 0; b <= 20; b++) {
				for (int c = 0; c <= 14; c++) {
					INPUT("%i", &_objectMapFull[a][b][c]);
				}
			}
		}

		for (int a = 0; a <= 200; a++) {
			INPUT("%i", &_roomLocks[a]);
		}

		INPUT("%f", &_player.spellStrength);

		return Common::kNoError;
	}

	return Common::kReadingFailed;
}

/* fill PLAYERTYPE _playera; */
int GriffonEngine::loadPlayer(int slotnum) {
	Common::String filename = getSaveStateName(slotnum);
	Common::InSaveFile *file;

	_playera.level = 0;

	if (!(file = _saveFileMan->openForLoading(filename)))
		return 0;

	INPUT("%i", &_playera.level);

	if (_playera.level > 0) {
		char temp[256];

		INPUT("%i", &_asecstart);
		INPUT("%s", temp);

		INPUT("%f", &_playera.px);
		INPUT("%f", &_playera.py);
		INPUT("%f", &_playera.opx);
		INPUT("%f", &_playera.opy);
		INPUT("%i", &_playera.walkDir);
		INPUT("%f", &_playera.walkFrame);
		INPUT("%f", &_playera.walkSpeed);
		INPUT("%f", &_playera.attackFrame);
		INPUT("%f", &_playera.attackSpeed);
		INPUT("%i", &_playera.hp);
		INPUT("%i", &_playera.maxHp);
		INPUT("%f", &_playera.hpflash);
		INPUT("%i", &_playera.level);
		INPUT("%i", &_playera.maxLevel);
		INPUT("%i", &_playera.sword);
		INPUT("%i", &_playera.shield);
		INPUT("%i", &_playera.armour);
		for (int i = 0; i < 5; i++) {
			INPUT("%i", &_playera.foundSpell[i]);
			INPUT("%f", &_playera.spellCharge[i]);
		}
		for (int a = 0; a < 5; a++) {
			INPUT("%i", &_playera.inventory[a]);
		}
		INPUT("%f", &_playera.attackStrength);
		INPUT("%i", &_playera.spellDamage);
		INPUT("%i", &_playera.swordDamage);
		INPUT("%i", &_playera.exp);
		INPUT("%i", &_playera.nextLevel);

		return 1; // success
	}

	return 0; // fail
}

Common::Error GriffonEngine::saveGameStream(Common::WriteStream *file, bool) {
	PRINT("%i", _player.level);

	if (_player.level > 0) {
		PRINT("%i", (_secStart + _secsInGame));
		PRINT("%s", "a");

		PRINT("%f", _player.px);
		PRINT("%f", _player.py);
		PRINT("%f", _player.opx);
		PRINT("%f", _player.opy);
		PRINT("%i", _player.walkDir);
		PRINT("%f", _player.walkFrame);
		PRINT("%f", _player.walkSpeed);
		PRINT("%f", _player.attackFrame);
		PRINT("%f", _player.attackSpeed);
		PRINT("%i", _player.hp);
		PRINT("%i", _player.maxHp);
		PRINT("%f", _player.hpflash);
		PRINT("%i", _player.level);
		PRINT("%i", _player.maxLevel);
		PRINT("%i", _player.sword);
		PRINT("%i", _player.shield);
		PRINT("%i", _player.armour);
		for (int i = 0; i < 5; i++) {
			PRINT("%i", _player.foundSpell[i]);
			PRINT("%f", _player.spellCharge[i]);
		}
		for (int a = 0; a < 5; a++) {
			PRINT("%i", _player.inventory[a]);
		}
		PRINT("%f", _player.attackStrength);
		PRINT("%i", _player.spellDamage);
		PRINT("%i", _player.swordDamage);
		PRINT("%i", _player.exp);
		PRINT("%i", _player.nextLevel);
		for (int a = 0; a <= 99; a++) {
			for (int b = 0; b <= 9; b++) {
				PRINT("%i", _scriptFlag[a][b]);
			}
		}
		PRINT("%i", _curMap);

		for (int a = 0; a <= 999; a++) {
			for (int b = 0; b <= 20; b++) {
				for (int c = 0; c <= 14; c++) {
					PRINT("%i", _objectMapFull[a][b][c]);
				}
			}
		}

		for (int a = 0; a <= 200; a++) {
			PRINT("%i", _roomLocks[a]);
		}

		PRINT("%f", _player.spellStrength);
	}

	drawView();

	return Common::kNoError;
}

} // end of namespace Griffon
