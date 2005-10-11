/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/file.h"

#include "saga/saga.h"
#include "saga/actor.h"
#include "saga/events.h"
#include "saga/interface.h"
#include "saga/isomap.h"
#include "saga/music.h"
#include "saga/render.h"
#include "saga/resnames.h"
#include "saga/scene.h"
#include "saga/script.h"

#define CURRENT_SAGA_VER 4

namespace Saga {

static SaveFileData emptySlot = {
	 "", 0
};

//TODO:
// - delete savegame

char* SagaEngine::calcSaveFileName(uint slotNumber) {
	static char name[MAX_FILE_NAME];
	sprintf(name, "%s.s%02d", _targetName.c_str(), slotNumber);
	return name;
}

SaveFileData *SagaEngine::getSaveFile(uint idx) {
	if (idx >= _saveFilesMaxCount) {
		error("getSaveFileName wrong idx");
	}
	if (isSaveListFull()) {
		return &_saveFiles[_saveFilesCount - idx - 1];
	} else {
		if (!emptySlot.name[0])
			strcpy(emptySlot.name, getTextString(kTextNewSave));

		return (idx == 0) ? &emptySlot : &_saveFiles[_saveFilesCount - idx];
	}
}

bool SagaEngine::locateSaveFile(char *saveName, uint &titleNumber) {
	uint i;
	for (i = 0; i < _saveFilesCount; i++) {
		if (strcmp(saveName, _saveFiles[i].name) == 0) {
			if (isSaveListFull()) {
				titleNumber = _saveFilesCount - i - 1;
			} else {
				titleNumber = _saveFilesCount - i;
			}
			return true;
		}
	}
	return false;
}

uint SagaEngine::getNewSaveSlotNumber() {
	uint i, j;
	bool found;
	if (isSaveListFull()) {
		error("getNewSaveSlotNumber save list is full");
	}
	for (i = 0; i < MAX_SAVES; i++) {
		if (_saveMarks[i]) {
			found = false;
			for (j = 0; j < _saveFilesCount; j++) {
				if (_saveFiles[j].slotNumber == i) {
					found = true;
					break;
				}
			}
			if (!found) {
				return i;
			}
		}
	}

	error("getNewSaveSlotNumber save list is full");
}

void SagaEngine::fillSaveList() {
	int i;
	Common::InSaveFile *in;
	char *name;

	name = calcSaveFileName(MAX_SAVES);
	name[strlen(name) - 2] = 0;
	_saveFileMan->listSavefiles(name, _saveMarks, MAX_SAVES);

	_saveFilesMaxCount = 0;
	for (i = 0; i < MAX_SAVES; i++) {
		if (_saveMarks[i]) {
			_saveFilesMaxCount++;
		}
		_saveFiles[i].name[0] = 0;
		_saveFiles[i].slotNumber = (uint)-1;
	}

	_saveFilesCount = 0;

	i = 0;
	while (i < MAX_SAVES) {
		if (_saveMarks[i]) {
			name = calcSaveFileName(i);
			if ((in = _saveFileMan->openForLoading(name)) != NULL) {
				in->read(&_saveHeader, sizeof(_saveHeader));

				if (_saveHeader.type != MKID('SAGA')) {
					error("SagaEngine::load wrong format");
				}
				strcpy(_saveFiles[_saveFilesCount].name, _saveHeader.name);
				_saveFiles[_saveFilesCount].slotNumber = i;
				delete in;
				_saveFilesCount++;
			}
		}
		i++;
	}
/* 4debug
	for (i = 0; i < 14; i++) {
		sprintf(_saveFiles[i].name,"test%i", i);
		_saveFiles[i].slotNumber = i;
	}
	_saveFilesCount = 14;
	_saveFilesMaxCount = 14;
	*/
}


void SagaEngine::save(const char *fileName, const char *saveName) {
	Common::OutSaveFile *out;

	if (!(out = _saveFileMan->openForSaving(fileName))) {
		return;
	}

	_saveHeader.type = MKID('SAGA');
	_saveHeader.size = 0;
	_saveHeader.version = TO_LE_32(CURRENT_SAGA_VER);
	strncpy(_saveHeader.name, saveName, SAVE_TITLE_SIZE);

	out->write(&_saveHeader, sizeof(_saveHeader));

	// Surrounding scene
	out->writeSint32LE(_scene->getOutsetSceneNumber());

	// Inset scene
	out->writeSint32LE(_scene->currentSceneNumber());

	if (getGameType() != GType_ITE) {
		out->writeUint32LE(_globalFlags);
		for (int i = 0; i < ARRAYSIZE(_ethicsPoints); i++)
			out->writeSint16LE(_ethicsPoints[i]);
	}

	_interface->saveState(out);

	_actor->saveState(out);

	out->writeSint16LE(_script->_commonBufferSize);

	out->write(_script->_commonBuffer, _script->_commonBufferSize);

	out->writeSint16LE(_isoMap->getMapPosition().x);
	out->writeSint16LE(_isoMap->getMapPosition().y);

	delete out;
}

void SagaEngine::load(const char *fileName) {
	Common::InSaveFile *in;
	int  commonBufferSize;
	int sceneNumber, insetSceneNumber;
	int mapx, mapy;

	if (!(in = _saveFileMan->openForLoading(fileName))) {
		return;
	}

	in->read(&_saveHeader, sizeof(_saveHeader));

	_saveHeader.size = FROM_LE_32(_saveHeader.size);
	_saveHeader.version = FROM_LE_32(_saveHeader.version);

	if (_saveHeader.version < 4)
		warning("This savegame is not endian-safe. There may be problems");

	if (_saveHeader.type != MKID('SAGA')) {
		error("SagaEngine::load wrong format");
	}

	// Surrounding scene
	sceneNumber = in->readSint32LE();

	// Inset scene
	insetSceneNumber = in->readSint32LE();

	if (getGameType() != GType_ITE) {
		_globalFlags = in->readUint32LE();
		for (int i = 0; i < ARRAYSIZE(_ethicsPoints); i++)
			_ethicsPoints[i] = in->readSint16LE();
	}

	_interface->loadState(in);

	_actor->loadState(in);

	commonBufferSize = in->readSint16LE();
	in->read(_script->_commonBuffer, commonBufferSize);

	mapx = in->readSint16LE();
	mapy = in->readSint16LE();

	delete in;

	// Mute volume to prevent outScene music play
	int volume = _music->getVolume();
	_music->setVolume(0);

	_isoMap->setMapPosition(mapx, mapy);

	_scene->clearSceneQueue();
	_scene->changeScene(sceneNumber, ACTOR_NO_ENTRANCE, kTransitionNoFade);

	_events->handleEvents(0); //dissolve backgrounds

	if (insetSceneNumber != sceneNumber) {
		_render->setFlag(RF_DISABLE_ACTORS);
		_render->drawScene();
		_render->clearFlag(RF_DISABLE_ACTORS);
		_scene->changeScene(insetSceneNumber, ACTOR_NO_ENTRANCE, kTransitionNoFade);
	}

	_music->setVolume(volume);

	_interface->draw();
}

} // End of namespace Saga
