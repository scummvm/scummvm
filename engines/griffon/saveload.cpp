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

Common::String GriffonEngine::makeSaveGameName(int slot) {
	return (_targetName + Common::String::format(".%02d", slot));
}

int GriffonEngine::state_load(int slotnum) {
	Common::String filename = makeSaveGameName(slotnum);
	Common::InSaveFile *file;
	if (!(file = _saveFileMan->openForLoading(filename)))
		return 0;

	INPUT("%i", &_player.level);

	if (_player.level > 0) {
		char temp[256];

		INPUT("%i", &_secstart);
		INPUT("%s", temp);

		INPUT("%f", &_player.px);
		INPUT("%f", &_player.py);
		INPUT("%f", &_player.opx);
		INPUT("%f", &_player.opy);
		INPUT("%i", &_player.walkdir);
		INPUT("%f", &_player.walkframe);
		INPUT("%f", &_player.walkspd);
		INPUT("%f", &_player.attackframe);
		INPUT("%f", &_player.attackspd);
		INPUT("%i", &_player.hp);
		INPUT("%i", &_player.maxhp);
		INPUT("%f", &_player.hpflash);
		INPUT("%i", &_player.level);
		INPUT("%i", &_player.maxlevel);
		INPUT("%i", &_player.sword);
		INPUT("%i", &_player.shield);
		INPUT("%i", &_player.armour);
		for (int i = 0; i < 5; i++) {
			INPUT("%i", &_player.foundspell[i]);
			INPUT("%f", &_player.spellcharge[i]);
		}
		for (int a = 0; a < 5; a++) {
			INPUT("%i", &_player.inventory[a]);
		}
		INPUT("%f", &_player.attackstrength);
		INPUT("%i", &_player.spelldamage);
		INPUT("%i", &_player.sworddamage);
		INPUT("%i", &_player.exp);
		INPUT("%i", &_player.nextlevel);
		for (int a = 0; a <= 99; a++) {
			for (int b = 0; b <= 9; b++) {
				INPUT("%i", &_scriptflag[a][b]);
			}
		}
		INPUT("%i", &_curmap);

		for (int a = 0; a <= 999; a++) {
			for (int b = 0; b <= 20; b++) {
				for (int c = 0; c <= 14; c++) {
					INPUT("%i", &_objmapf[a][b][c]);
				}
			}
		}

		for (int a = 0; a <= 200; a++) {
			INPUT("%i", &_roomLocks[a]);
		}

		INPUT("%f", &_player.spellstrength);

		return 1; // success
	}

	return 0; // failure
}

/* fill PLAYERTYPE _playera; */
int GriffonEngine::state_load_player(int slotnum) {
	Common::String filename = makeSaveGameName(slotnum);
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
		INPUT("%i", &_playera.walkdir);
		INPUT("%f", &_playera.walkframe);
		INPUT("%f", &_playera.walkspd);
		INPUT("%f", &_playera.attackframe);
		INPUT("%f", &_playera.attackspd);
		INPUT("%i", &_playera.hp);
		INPUT("%i", &_playera.maxhp);
		INPUT("%f", &_playera.hpflash);
		INPUT("%i", &_playera.level);
		INPUT("%i", &_playera.maxlevel);
		INPUT("%i", &_playera.sword);
		INPUT("%i", &_playera.shield);
		INPUT("%i", &_playera.armour);
		for (int i = 0; i < 5; i++) {
			INPUT("%i", &_playera.foundspell[i]);
			INPUT("%f", &_playera.spellcharge[i]);
		}
		for (int a = 0; a < 5; a++) {
			INPUT("%i", &_playera.inventory[a]);
		}
		INPUT("%f", &_playera.attackstrength);
		INPUT("%i", &_playera.spelldamage);
		INPUT("%i", &_playera.sworddamage);
		INPUT("%i", &_playera.exp);
		INPUT("%i", &_playera.nextlevel);

		return 1; // success
	}

	return 0; // fail
}

int GriffonEngine::state_save(int slotnum) {
	Common::String filename = makeSaveGameName(slotnum);
	Common::OutSaveFile *file;

	if (!(file = _saveFileMan->openForSaving(filename))) {
		warning("Cannot open %s for saving", filename.c_str());
		return 0;
	}

	PRINT("%i", _player.level);

	if (_player.level > 0) {
		PRINT("%i", (_secstart + _secsingame));
		PRINT("%s", "a");

		PRINT("%f", _player.px);
		PRINT("%f", _player.py);
		PRINT("%f", _player.opx);
		PRINT("%f", _player.opy);
		PRINT("%i", _player.walkdir);
		PRINT("%f", _player.walkframe);
		PRINT("%f", _player.walkspd);
		PRINT("%f", _player.attackframe);
		PRINT("%f", _player.attackspd);
		PRINT("%i", _player.hp);
		PRINT("%i", _player.maxhp);
		PRINT("%f", _player.hpflash);
		PRINT("%i", _player.level);
		PRINT("%i", _player.maxlevel);
		PRINT("%i", _player.sword);
		PRINT("%i", _player.shield);
		PRINT("%i", _player.armour);
		for (int i = 0; i < 5; i++) {
			PRINT("%i", _player.foundspell[i]);
			PRINT("%f", _player.spellcharge[i]);
		}
		for (int a = 0; a < 5; a++) {
			PRINT("%i", _player.inventory[a]);
		}
		PRINT("%f", _player.attackstrength);
		PRINT("%i", _player.spelldamage);
		PRINT("%i", _player.sworddamage);
		PRINT("%i", _player.exp);
		PRINT("%i", _player.nextlevel);
		for (int a = 0; a <= 99; a++) {
			for (int b = 0; b <= 9; b++) {
				PRINT("%i", _scriptflag[a][b]);
			}
		}
		PRINT("%i", _curmap);

		for (int a = 0; a <= 999; a++) {
			for (int b = 0; b <= 20; b++) {
				for (int c = 0; c <= 14; c++) {
					PRINT("%i", _objmapf[a][b][c]);
				}
			}
		}

		for (int a = 0; a <= 200; a++) {
			PRINT("%i", _roomLocks[a]);
		}

		PRINT("%f", _player.spellstrength);
	}

	file->finalize();

	return 1; // success
}

} // end of namespace Griffon
