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

#include "stdafx.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/file.h"

#include "saga/saga.h"
#include "saga/actor.h"
#include "saga/isomap.h"
#include "saga/resnames.h"
#include "saga/script.h"
#include "saga/interface.h"
#include "saga/scene.h"
#include "saga/render.h"

#define CURRENT_SAGA_VER 1

namespace Saga {

struct SaveGameHeader {
	uint32 type;
	uint32 size;
	uint32 version;
	char name[SAVE_TITLE_SIZE];
};

static SaveFileData emptySlot = {
	 "[New Save Game]", 0
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
	uint i;
	uint saveCount;
	if (isSaveListFull()) {
		error("getNewSaveSlotNumber save list is full");
	}
	i = 0;
	saveCount = 0;
	while (saveCount < _saveFilesCount) {
		if (_saveMarks[i++]) {
			saveCount++;
		}
	}
	return i;
}

void SagaEngine::fillSaveList() {
	int i;
	Common::InSaveFile *in;
	SaveGameHeader header;
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
			if (!(in = _saveFileMan->openForLoading(name))) {
				break;
			}
			in->read(&header, sizeof(header));

			if (header.type != MKID('SAGA')) {
				error("SagaEngine::load wrong format");
			}
			strcpy(_saveFiles[_saveFilesCount].name, header.name);
			_saveFiles[_saveFilesCount].slotNumber = i;
			delete in;
			_saveFilesCount++;
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
	SaveGameHeader header;

	if (!(out = _saveFileMan->openForSaving(fileName))) {
		return;
	}

	header.type = MKID('SAGA');
	header.size = 0;
	header.version = CURRENT_SAGA_VER;
	strcpy(header.name, saveName);

	out->write(&header, sizeof(header));

	// Surrounding scene
	out->writeSint32LE(_scene->getOutsetSceneNumber());

	// Inset scene
	out->writeSint32LE(_scene->currentSceneNumber());

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
	SaveGameHeader header;

	if (!(in = _saveFileMan->openForLoading(fileName))) {
		return;
	}

	in->read(&header, sizeof(header));

	if (header.type != MKID('SAGA')) {
		error("SagaEngine::load wrong format");
	}
			
	// Surrounding scene
	sceneNumber = in->readSint32LE();

	// Inset scene
	insetSceneNumber = in->readSint32LE();

	_interface->loadState(in);

	_actor->loadState(in);
	
	commonBufferSize = in->readSint16LE();
	in->read(_script->_commonBuffer, commonBufferSize);

	mapx = in->readSint16LE();
	mapy = in->readSint16LE();

	delete in;

	_isoMap->setMapPosition(mapx, mapy);

	_scene->clearSceneQueue();
	_scene->changeScene(sceneNumber, ACTOR_NO_ENTRANCE, kTransitionNoFade);

	if (insetSceneNumber != sceneNumber) {
		_render->drawScene();
		_scene->clearSceneQueue();
		_scene->changeScene(insetSceneNumber, ACTOR_NO_ENTRANCE, kTransitionNoFade);
	}
}

} // End of namespace Saga
