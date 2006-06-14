/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "common/savefile.h"
#include "common/system.h"

#include "gui/about.h"
#include "gui/message.h"

#include "simon/simon.h"
#include "simon/intern.h"

namespace Simon {

int SimonEngine::countSaveGames() {
	Common::InSaveFile *f;
	uint i = 1;
	bool marks[256];

	char *prefix = genSaveName(999);
	prefix[strlen(prefix)-3] = '\0';
	_saveFileMan->listSavefiles(prefix, marks, 256);

	while (i < 256) {
		if (marks[i] &&
		    (f = _saveFileMan->openForLoading(genSaveName(i)))) {
			i++;
			delete f;
		} else
			break;
	}
	return i;
}

int SimonEngine::displaySaveGameList(int curpos, bool load, char *dst) {
	int slot, last_slot;
	Common::InSaveFile *in;

	showMessageFormat("\xC");

	memset(dst, 0, 108);

	slot = curpos;

	while (curpos + 6 > slot) {
		if (!(in = _saveFileMan->openForLoading(genSaveName(slot))))
			break;

		in->read(dst, 18);
		delete in;

		last_slot = slot;
		if (slot < 10) {
			showMessageFormat(" ");
		} else if (_language == Common::HB_ISR) {
			last_slot = (slot % 10) * 10;
			last_slot += slot / 10;
		}

		if (_language == Common::HB_ISR && !(slot % 10))
			showMessageFormat("0");
		showMessageFormat("%d", last_slot);
		showMessageFormat(".%s\n", dst);
		dst += 18;
		slot++;
	}
	// while_break
	if (!load) {
		if (curpos + 6 == slot)
			slot++;
		else {
			if (slot < 10)
				showMessageFormat(" ");
			showMessageFormat("%d.\n", slot);
		}
	} else {
		if (curpos + 6 == slot) {
			if ((in = _saveFileMan->openForLoading(genSaveName(slot)))) {
				slot++;
				delete in;
			}
		}
	}

	return slot - curpos;
}

char *SimonEngine::genSaveName(int slot) {
	static char buf[15];

	if (getGameType() == GType_FF) {
		if (slot == 999) {
			// Restart state
			if (getPlatform() == Common::kPlatformWindows)
				sprintf(buf, "save.%.3d", slot);
			else
				sprintf(buf, "setup");
		} else {
			sprintf(buf, "feeble.%.3d", slot);
		}
	} else if (getGameType() == GType_SIMON2) {
		sprintf(buf, "simon2.%.3d", slot);
	} else {
		sprintf(buf, "simon1.%.3d", slot);
	}
	return buf;
}

void SimonEngine::quickLoadOrSave() {
	// The demo of Simon 1 (DOS Floppy) is missing too many segments
	// and the Feeble Files doesn't always allow a load or save
	if (getGameId() == GID_SIMON1DEMO || getGameType() == GType_FF)
		return;

	bool success;
	char buf[50];

	char *filename = genSaveName(_saveLoadSlot);
	if (_saveLoadType == 2) {
		Subroutine *sub;
		success = loadGame(_saveLoadSlot);
		if (!success) {
			sprintf(buf, "Failed to load game state to file:\n\n%s", filename);
		} else {
			// Redraw Inventory
			mouseOff();
			drawIconArray(2, me(), 0, 0);
			mouseOn();
			// Reset engine?
			setBitFlag(97, true);
			sub = getSubroutineByID(100);
			startSubroutine(sub);
		}
	} else {
		success = saveGame(_saveLoadSlot, _saveLoadName);
		if (!success)
			sprintf(buf, "Failed to save game state to file:\n\n%s", filename);
	}

	if (!success) {
		GUI::MessageDialog dialog(buf, "OK");
		dialog.runModal();

	} else if (_saveLoadType == 1) {
		sprintf(buf, "Successfully saved game state in file:\n\n%s", filename);
		GUI::TimedMessageDialog dialog(buf, 1500);
		dialog.runModal();

	}

	_saveLoadType = 0;
}

void SimonEngine::listSaveGames(char *buf) {
	int i;

	disableFileBoxes();

	i = displaySaveGameList(_saveLoadRowCurPos, _saveOrLoad, buf);

	_saveDialogFlag = true;

	if (i != 7) {
		i++;
		if (!_saveOrLoad)
			i++;
		_saveDialogFlag = false;
	}

	if (!--i)
		return;

	do {
		enableBox(208 + i - 1);
	} while (--i);
}


const byte hebrewKeyTable[96] = {
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 90, 45, 85, 47, 48, 49, 50,
	51, 52, 53, 54, 55, 56, 57, 83, 83, 90, 61, 85, 63, 35, 89, 80, 65, 66, 87,
	75, 82, 73, 79, 71, 76, 74, 86, 78, 77, 84, 47, 88, 67, 64, 69, 68, 44, 81,
	72, 70, 91, 92, 93, 94, 95, 96, 89, 80, 65, 66, 87, 75, 82, 73, 79, 71, 76,
	74, 86, 78, 77, 84, 47, 88, 67, 64, 69, 68, 44, 81, 72, 70,
	123, 124, 125, 126, 127,
};

void SimonEngine::userGame(bool load) {
	time_t save_time;
	int number_of_savegames;
	int i, name_len, result;
	WindowBlock *window;
	char *name;
	bool b;
	char buf[108];
	int maxChar = (_language == Common::HB_ISR) ? 155: 128;

	_saveOrLoad = load;

	save_time = time(NULL);

	_copyPartialMode = 1;

	number_of_savegames = countSaveGames();
	if (!load)
		number_of_savegames++;
	number_of_savegames -= 6;
	if (number_of_savegames < 0)
		number_of_savegames = 0;
	number_of_savegames++;
	_numSaveGameRows = number_of_savegames;

	_saveLoadRowCurPos = 1;
	if (!load)
		_saveLoadRowCurPos = number_of_savegames;

	_saveLoadEdit = false;

restart:;
	do {
		i = userGameGetKey(&b, buf);
	} while (!b);

	if (i == 205)
		goto get_out;
	if (!load) {
		// if_1
	if_1:;
		result = i;

		disableBox(208 + i);
		leaveHitAreaById(208 + i);

		window = _windowArray[5];

		window->textRow = result;

		// init x offset with a 2 character savegame number + a period (18 pix)
		if (_language == Common::HB_ISR) {
			window->textColumn = 3;
			window->textColumnOffset = 6;
			window->textLength = 3;
		} else {
			window->textColumn = 2;
			window->textColumnOffset = 2;
			window->textLength = 3;
		}

		name = buf + i * 18;

		// now process entire savegame name to get correct x offset for cursor
		name_len = 0;
		while (name[name_len]) {
			if (_language == Common::HB_ISR) {
				byte width = 6;
				if (name[name_len] >= 64 && name[name_len] < 91)
					width = _hebrewCharWidths [name[name_len] - 64];
				window->textLength++;
				window->textColumnOffset -= width;
				if (window->textColumnOffset < width) {
					window->textColumnOffset += 8;
					window->textColumn++;
				}
			} else {
				window->textLength++;
				window->textColumnOffset += 6;
				if (name[name_len] == 'i' || name[name_len] == 'l')
					window->textColumnOffset -= 2;
				if (window->textColumnOffset >= 8) {
					window->textColumnOffset -= 8;
					window->textColumn++;
				}
			}
			name_len++;
		}
		// while_1_end

		// do_3_start
		for (;;) {
			windowPutChar(window, 0x7f);

			_saveLoadEdit = true;

			// do_2
			do {
				i = userGameGetKey(&b, buf);

				if (b) {
					if (i == 205)
						goto get_out;
					enableBox(208 + result);
					if (_saveLoadEdit) {
						userGameBackSpace(_windowArray[5], 8);
					}
					goto if_1;
				}

				// is_not_b
				if (!_saveLoadEdit) {
					enableBox(208 + result);
					goto restart;
				}
			} while (i >= maxChar || i == 0);

			if (_language == Common::HB_ISR) {
				if (i >= 128)
					i -= 64;
				else if (i >= 32)
					i = hebrewKeyTable[i - 32];
			}

			// after_do_2
			userGameBackSpace(_windowArray[5], 8);
			if (i == 10 || i == 13)
				break;
			if (i == 8) {
				// do_backspace
				if (name_len != 0) {
					int x;
					byte m;

					name_len--;
					m = name[name_len];

					if (_language == Common::HB_ISR)
						x = 8;
					else
						x = (name[name_len] == 'i' || name[name_len] == 'l') ? 1 : 8;

					name[name_len] = 0;

					userGameBackSpace(_windowArray[5], x, m);
				}
			} else if (i >= 32 && name_len != 17) {
				name[name_len++] = i;

				windowPutChar(_windowArray[5], i);
			}
		}

		// do_save
		if (!saveGame(_saveLoadRowCurPos + result, buf + result * 18))
			fileError(_windowArray[5], true);
	} else {
		if (!loadGame(_saveLoadRowCurPos + i))
			fileError(_windowArray[5], false);
	}

get_out:;
	disableFileBoxes();

	_gameStoppedClock = time(NULL) - save_time + _gameStoppedClock;
	_copyPartialMode = 0;

	restoreBlock(94, 208, 46, 80);

	i = _timer4;
	do {
		delay(10);
	} while (i == _timer4);
}

int SimonEngine::userGameGetKey(bool *b, char *buf) {
	HitArea *ha;
	*b = true;

	if (!_saveLoadEdit) {
		listSaveGames(buf);
	}

	_keyPressed = 0;

	for (;;) {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;

		do {
			if (_saveLoadEdit && _keyPressed != 0) {
				*b = false;
				return _keyPressed;
			}
			delay(10);
		} while (_lastHitArea3 == 0);

		ha = _lastHitArea;

		if (ha == NULL || ha->id < 205) {
		} else if (ha->id == 205) {
			return ha->id;
		} else if (ha->id == 206) {
			if (_saveLoadRowCurPos != 1) {
				if (_saveLoadRowCurPos < 7)
					_saveLoadRowCurPos = 1;
				else
					_saveLoadRowCurPos -= 6;

				_saveLoadEdit = false;
				listSaveGames(buf);
			}
		} else if (ha->id == 207) {
			if (_saveDialogFlag) {
				_saveLoadRowCurPos += 6;
				if (_saveLoadRowCurPos >= _numSaveGameRows)
					_saveLoadRowCurPos = _numSaveGameRows;

				_saveLoadEdit = false;
				listSaveGames(buf);
			}
		} else if (ha->id < 214) {
			return ha->id - 208;
		}
	}
}

void SimonEngine::disableFileBoxes() {
	for (int i = 208; i != 214; i++)
		disableBox(i);
}

void SimonEngine::userGameBackSpace(WindowBlock *window, int x, byte b) {
	byte old_text;

	windowPutChar(window, x, b);
	old_text = window->text_color;
	window->text_color = window->fill_color;

	if (_language == Common::HB_ISR) {
		x = 128;
	} else {
		x += 120;
		if (x != 128)
			x = 129;
	}

	windowPutChar(window, x);

	window->text_color = old_text;
	windowPutChar(window, 8);
}

void SimonEngine::fileError(WindowBlock *window, bool save_error) {
	HitArea *ha;
	const char *string1, *string2;

	if (save_error) {
		switch (_language) {
		case Common::RU_RUS:
			if (getGameType() == GType_SIMON2) {
				string1 = "\r   Mf sowrap+fts+.";
				string2 = "\r  Nzjb#a ejs#a.";
			} else {
				string1 = "\r   Mf sowrap]fts].";
				string2 = "\r   Nzjb_a ejs_a.";
			}
			break;
		case Common::PL_POL:
			string1 = "\r      Blad zapisu.    ";
			string2 = "\rBlad dysku.                       ";
			break;
		case Common::ES_ESP:
			string1 = "\r     Error al salvar";
			string2 = "\r  Intenta con otro disco";
			break;
		case Common::IT_ITA:
			string1 = "\r  Salvataggio non riuscito";
			string2 = "\r    Prova un""\x27""altro disco";
			break;
		case Common::FR_FRA:
			string1 = "\r    Echec sauvegarde";
			string2 = "\rEssayez une autre disquette";
			break;
		case Common::DE_DEU:
			string1 = "\r  Sicherung erfolglos.";
			string2 = "\rVersuche eine andere     Diskette.";
			break;
		default:
			string1 = "\r       Save failed.";
			string2 = "\r       Disk error.";
			break;
		}
	} else {
		switch (_language) {
		case Common::RU_RUS:
			if (getGameType() == GType_SIMON2) {
				string1 = "\r  Mf ^adruhafts+.";
				string2 = "\r   Takm pf pakefp.";
			} else {
				string1 = "\r   Mf ^adruhafts].";
				string2 = "\r   Takm pf pakefp.";
			}
			break;
		case Common::PL_POL:
			string1 = "\r   Blad odczytu.    ";
			string2 = "\r  Nie znaleziono pliku.";
			break;
		case Common::ES_ESP:
			string1 = "\r     Error al cargar";
			string2 = "\r  Archivo no encontrado";
			break;
		case Common::IT_ITA:
			string1 = "\r  Caricamento non riuscito";
			string2 = "\r      File non trovato";
			break;
		case Common::FR_FRA:
			string1 = "\r    Echec chargement";
			string2 = "\r  Fichier introuvable";
			break;
		case Common::DE_DEU:
			string1 = "\r    Laden erfolglos.";
			string2 = "\r  Datei nicht gefunden.";
			break;
		default:
			string1 = "\r       Load failed.";
			string2 = "\r     File not found.";
			break;
		}
	}

	windowPutChar(window, 0xC);
	for (; *string1; string1++)
		windowPutChar(window, *string1);
	for (; *string2; string2++)
		windowPutChar(window, *string2);

	window->textColumn = (window->width / 2) - 3;
	window->textRow = window->height - 1;
	window->textLength = 0;

	string1 = "[ OK ]";
	for (; *string1; string1++)
		windowPutChar(window, *string1);

	ha = findEmptyHitArea();
	ha->x = ((window->width / 2) + (window->x - 3)) * 8;
	ha->y = (window->height * 8) + window->y - 8;
	ha->width = 48;
	ha->height = 8;
	ha->flags = kBFBoxInUse;
	ha->id = 0x7FFF;
	ha->priority = 0x3EF;

loop:;
	_lastHitArea = _lastHitArea3 = 0;

	do {
		delay(1);
	} while (_lastHitArea3 == 0);

	ha = _lastHitArea;
	if (ha == NULL || ha->id != 0x7FFF)
		goto loop;

	// Return
	undefineBox(0x7FFF);
}

bool SimonEngine::saveGame(uint slot, char *caption) {
	Common::WriteStream *f;
	uint item_index, num_item, i, j;
	TimeEvent *te;
	uint32 curTime = 0;
	uint32 gsc = _gameStoppedClock;

	_lockWord |= 0x100;

	f = _saveFileMan->openForSaving(genSaveName(slot));
	if (f == NULL) {
		warning("saveGame: Failed to save slot %d", slot);
		_lockWord &= ~0x100;
		return false;
	}

	if (getGameType() == GType_FF) {
		f->write(caption, 100);
		curTime = time(NULL);
	} else {
		f->write(caption, 18);
	}

	f->writeUint32BE(_itemArrayInited - 1);
	f->writeUint32BE(0xFFFFFFFF);
	f->writeUint32BE(0);
	f->writeUint32BE(0);

	i = 0;
	for (te = _firstTimeStruct; te; te = te->next)
		i++;
	f->writeUint32BE(i);

	if (_clockStopped)
		gsc += ((uint32)time(NULL) - _clockStopped);
	for (te = _firstTimeStruct; te; te = te->next) {
		f->writeUint32BE(te->time - curTime + gsc);
		f->writeUint16BE(te->subroutine_id);
	}

	item_index = 1;
	for (num_item = _itemArrayInited - 1; num_item; num_item--) {
		Item *item = _itemArrayPtr[item_index++];

		f->writeUint16BE(item->parent);
		f->writeUint16BE(item->sibling);
		f->writeUint16BE(item->state);
		f->writeUint16BE(item->classFlags);

		SubRoom *subRoom = (SubRoom *)findChildOfType(item, 1);
		if (subRoom) {
			f->writeUint16BE(subRoom->roomExitStates);
		}

		SubObject *subObject = (SubObject *)findChildOfType(item, 2);
		if (subObject) {
			f->writeUint32BE(subObject->objectFlags);
			i = subObject->objectFlags & 1;

			for (j = 1; j < 16; j++) {
				if (subObject->objectFlags & (1 << j)) {
					f->writeUint16BE(subObject->objectFlagValue[i++]);
				}
			}
		}

		SubUserFlag *subUserFlag = (SubUserFlag *)findChildOfType(item, 9);
		if (subUserFlag) {
			for (i = 0; i != 4; i++) {
				f->writeUint16BE(subUserFlag->userFlags[i]);
			}
		}
	}

	// write the 255 variables
	for (i = 0; i != 255; i++) {
		f->writeUint16BE(readVariable(i));
	}

	// write the items in array 6
	for (i = 0; i != 10; i++) {
		f->writeUint16BE(itemPtrToID(_itemStore[i]));
	}

	// Write the bits in array 1
	for (i = 0; i != 16; i++)
		f->writeUint16BE(_bitArray[i]);

	// Write the bits in array 2
	for (i = 0; i != 16; i++)
		f->writeUint16BE(_bitArrayTwo[i]);

	// Write the bits in array 3
	if (getGameType() == GType_FF) {
		for (i = 0; i != 16; i++)
			f->writeUint16BE(_bitArrayThree[i]);
	}

	f->flush();
	bool result = !f->ioFailed();

	delete f;
	_lockWord &= ~0x100;

	return result;
}

bool SimonEngine::loadGame(uint slot) {
	char ident[100];
	Common::SeekableReadStream *f = NULL;
	uint num, item_index, i, j;

	_lockWord |= 0x100;

	if (getGameType() == GType_FF && slot == 999) {
		// Load restart state
		Common::File *file = new Common::File();
		file->open(genSaveName(slot), Common::File::kFileReadMode);
		if (!file->isOpen()) {
			delete file;
		} else {
			f = file;
		}
	} else {
		f = _saveFileMan->openForLoading(genSaveName(slot));
	}

	if (f == NULL) {
		warning("loadGame: Failed to load slot %d", slot);
		_lockWord &= ~0x100;
		return false;
	}

	if (getGameType() == GType_FF) {
		f->read(ident, 100);
	} else {
		f->read(ident, 18);
	}

	num = f->readUint32BE();

	if (f->readUint32BE() != 0xFFFFFFFF || num != _itemArrayInited - 1) {
		delete f;
		_lockWord &= ~0x100;
		return false;
	}

	f->readUint32BE();
	f->readUint32BE();
	_noParentNotify = true;


	// add all timers
	killAllTimers();
	for (num = f->readUint32BE(); num; num--) {
		uint32 timeout = f->readUint32BE();
		uint16 func_to_call = f->readUint16BE();
		addTimeEvent(timeout, func_to_call);
	}

	item_index = 1;
	for (num = _itemArrayInited - 1; num; num--) {
		Item *item = _itemArrayPtr[item_index++], *parent_item;

		uint parent = f->readUint16BE();
		uint sibling = f->readUint16BE();

		parent_item = derefItem(parent);

		setItemParent(item, parent_item);

		if (parent_item == NULL) {
			item->parent = parent;
			item->sibling = sibling;
		}

		item->state = f->readUint16BE();
		item->classFlags = f->readUint16BE();

		SubRoom *subRoom = (SubRoom *)findChildOfType(item, 1);
		if (subRoom != NULL) {
			subRoom->roomExitStates = f->readUint16BE();
		}

		SubObject *subObject = (SubObject *)findChildOfType(item, 2);
		if (subObject != NULL) {
			subObject->objectFlags = f->readUint32BE();
			i = subObject->objectFlags & 1;

			for (j = 1; j < 16; j++) {
				if (subObject->objectFlags & (1 << j)) {
					subObject->objectFlagValue[i++] = f->readUint16BE();
				}
			}
		}

		SubUserFlag *subUserFlag = (SubUserFlag *) findChildOfType(item, 9);
		if (subUserFlag) {
			for (i = 0; i != 4; i++) {
				subUserFlag->userFlags[i] = f->readUint16BE();
			}
		}
	}


	// read the 255 variables
	for (i = 0; i != 255; i++) {
		writeVariable(i, f->readUint16BE());
	}

	// read the items in array 6
	for (i = 0; i != 10; i++) {
		_itemStore[i] = derefItem(f->readUint16BE());
	}

	// Read the bits in array 1
	for (i = 0; i != 16; i++)
		_bitArray[i] = f->readUint16BE();

	// Read the bits in array 2
	for (i = 0; i != 16; i++)
		_bitArrayTwo[i] = f->readUint16BE();

	// Read the bits in array 3
	if (getGameType() == GType_FF) {
		for (i = 0; i != 16; i++)
			_bitArrayThree[i] = f->readUint16BE();
	}

	if (f->ioFailed()) {
		error("load failed");
	}

	delete f;

	_noParentNotify = false;

	_lockWord &= ~0x100;

	return true;
}

} // End of namespace Simon
