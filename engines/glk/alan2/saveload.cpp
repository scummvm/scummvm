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
 */

#include "common/savefile.h"
#include "glk/alan2/acode.h"
#include "glk/alan2/alan2.h"
#include "glk/alan2/saveload.h"
#include "glk/alan2/types.h"

namespace Glk {
namespace Alan2 {

void SaveLoad::save() {
	uint i;
	Common::SaveFileManager *saveFileMan = _vm->getSaveFileManager();
	Common::OutSaveFile *saveFile = nullptr;
	Common::String str;
	AtrElem *atr;

	// TODO
#if 0
	frefid_t fref = glk_fileref_create_by_prompt(fileusage_SavedGame, filemode_Write, 0);
	if (fref == NULL)
		_vm->printError(M_SAVEFAILED);

	strcpy(str, garglk_fileref_get_name(fref));
	glk_fileref_destroy(fref);
#endif

	if (str.empty())
		str = _prevSaveName;

	_vm->col = 1;

	// TODO
#if 0
	if ((savfil = fopen(str, READ_MODE)) != NULL)
		// It already existed
		if (!confirm(M_SAVEOVERWRITE))
			_vm->printError(MSGMAX);            // Return to player without saying anything

	if ((savfil = fopen(str, WRITE_MODE)) == NULL)
		_vm->printError(M_SAVEFAILED);
#endif

	saveFile = saveFileMan->openForSaving(str);

	_prevSaveName = str;

	// Save version of interpreter and name of game
	saveFile->write(_vm->header->vers, sizeof(Aword));
	saveFile->writeString(_gameName);
	saveFile->writeByte('\0');
	// Save current values
	saveFile->write(&_vm->cur, sizeof(_vm->cur));

	// Save actors
	for (i = ACTMIN; i <= ACTMAX; i++) {
		saveFile->writeUint32LE(_actors[i - ACTMIN].loc);
		saveFile->writeUint32LE(_actors[i - ACTMIN].script);
		saveFile->writeUint32LE(_actors[i - ACTMIN].step);
		saveFile->writeUint32LE(_actors[i - ACTMIN].count);
		if (_actors[i - ACTMIN].atrs)
			for (atr = (AtrElem *)addrTo(_actors[i - ACTMIN].atrs); !endOfTable(atr); atr++)
				saveFile->writeUint32LE(atr->val);
	}

	// Save locations
	for (i = LOCMIN; i <= LOCMAX; i++) {
		saveFile->writeUint32LE(_locations[i - LOCMIN].describe);
		if (_locations[i - LOCMIN].atrs)
			for (atr = (AtrElem *)addrTo(_locations[i - LOCMIN].atrs); !endOfTable(atr); atr++)
				saveFile->writeUint32LE(atr->val);
	}

	// Save objects
	for (i = OBJMIN; i <= OBJMAX; i++) {
		saveFile->writeUint32LE(_objects[i - OBJMIN].loc);
		if (_objects[i - OBJMIN].atrs)
			for (atr = (AtrElem *)addrTo(_objects[i - OBJMIN].atrs); !endOfTable(atr); atr++)
				saveFile->writeUint32LE(atr->val);
	}

	// Save the event queue
	_events[*_eventTop].time = 0;        // Mark the top
	for (i = 0; i < (uint)*_eventTop + 1; i++)
		saveFile->write(&_events[i], sizeof(_events[0]));

	// Save scores
	for (i = 0; _vm->scores[i] != EOF; i++)
		saveFile->writeUint32LE(_vm->scores[i]);

	delete saveFile;
}

void SaveLoad::restore() {
	uint i;
	Common::SaveFileManager *saveFileMan = _vm->getSaveFileManager();
	Common::InSaveFile *saveFile = nullptr;
	Common::String str;
	AtrElem *atr;
	char savedVersion[4];
	char savedName[256];

	// TODO
#if 0
	frefid_t fref = glk_fileref_create_by_prompt(fileusage_SavedGame, filemode_Read, 0);
	if (fref == NULL)
		_vm->printError(M_SAVEFAILED);

	strcpy(str, garglk_fileref_get_name(fref));
	glk_fileref_destroy(fref);
#endif

	_vm->col = 1;

	if (str.empty())
		str = _prevSaveName;	// Use the name temporarily

	if ((saveFile = saveFileMan->openForLoading(str)) == nullptr) {
		_vm->printError(M_SAVEMISSING);
		return;
	}

	_prevSaveName = str;		// Save it for future use

	saveFile->read(savedVersion, sizeof(Aword));

	// 4f - save file version check doesn't seem to work on PC's!
	if (strncmp(savedVersion, _vm->header->vers, 4)) {
		delete saveFile;
		_vm->printError(M_SAVEVERS);
		return;
	}

	i = 0;

	while ((savedName[i++] = saveFile->readByte()) != '\0');

	if (savedName != _gameName) {
		delete saveFile;
		_vm->printError(M_SAVENAME);
		return;
	}

	// Restore current values
	saveFile->read(&_vm->cur, sizeof(_vm->cur));

	// Restore actors
	for (i = ACTMIN; i <= ACTMAX; i++) {
		_actors[i - ACTMIN].loc = saveFile->readUint32LE();
		_actors[i - ACTMIN].script = saveFile->readUint32LE();
		_actors[i - ACTMIN].step = saveFile->readUint32LE();
		_actors[i - ACTMIN].count = saveFile->readUint32LE();

		if (_actors[i - ACTMIN].atrs)
			for (atr = (AtrElem *)addrTo(_actors[i - ACTMIN].atrs); !endOfTable(atr); atr++)
				atr->val = saveFile->readUint32LE();
	}

	// Restore locations
	for (i = LOCMIN; i <= LOCMAX; i++) {
		_locations[i - LOCMIN].describe = saveFile->readUint32LE();
		if (_locations[i - LOCMIN].atrs)
			for (atr = (AtrElem *)addrTo(_locations[i - LOCMIN].atrs); !endOfTable(atr); atr++)
				atr->val = saveFile->readUint32LE();
	}

	// Restore objects
	for (i = OBJMIN; i <= OBJMAX; i++) {
		_objects[i - OBJMIN].loc = saveFile->readUint32LE();
		if (_objects[i - OBJMIN].atrs)
			for (atr = (AtrElem *)addrTo(_objects[i - OBJMIN].atrs); !endOfTable(atr); atr++)
				atr->val = saveFile->readUint32LE();
	}

	// Restore the event queue
	*_eventTop = 0;
	do {
		saveFile->read(&_events[*_eventTop], sizeof(_events[0]));
		(*_eventTop)++;
	} while (_events[*_eventTop - 1].time != 0);

	(*_eventTop)--;

	// Restore scores
	for (i = 0; _vm->scores[i] != EOF; i++)
		_vm->scores[i] = saveFile->readUint32LE();

	delete saveFile;
}

bool SaveLoad::endOfTable(AtrElem *addr) {
	Aword *x = (Aword *)addr;
	return *x == EOF;
}

} // End of namespace Alan2
} // End of namespace Glk
