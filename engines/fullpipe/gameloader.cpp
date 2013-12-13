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
 */

#include "fullpipe/fullpipe.h"

#include "fullpipe/gameloader.h"
#include "fullpipe/scene.h"
#include "fullpipe/input.h"
#include "fullpipe/statics.h"
#include "fullpipe/interaction.h"
#include "fullpipe/motion.h"

namespace Fullpipe {

Inventory2 *getGameLoaderInventory() {
	return &g_fullpipe->_gameLoader->_inventory;
}

MctlCompound *getSc2MctlCompoundBySceneId(int16 sceneId) {
	for (uint i = 0; i < g_fullpipe->_gameLoader->_sc2array.size(); i++)
		if (g_fullpipe->_gameLoader->_sc2array[i]._sceneId == sceneId)
			return (MctlCompound *)g_fullpipe->_gameLoader->_sc2array[i]._motionController;

	return 0;
}

InteractionController *getGameLoaderInteractionController() {
	return g_fullpipe->_gameLoader->_interactionController;
}

GameLoader::GameLoader() {
	_interactionController = new InteractionController();
	_inputController = new InputController();

	_gameProject = 0;
	_gameName = 0;

	addMessageHandlerByIndex(global_messageHandler2, 0, 0);
	insertMessageHandler(global_messageHandler3, 0, 128);
	insertMessageHandler(global_messageHandler4, 0, 1);

	_field_FA = 0;
	_field_F8 = 0;
	_sceneSwitcher = 0;
	_preloadCallback = 0;
	_readSavegameCallback = 0;
	_gameVar = 0;
	_preloadSceneId = 0;
	_preloadEntranceId = 0;
	_updateCounter = 0;

	g_fullpipe->_msgX = 0;
	g_fullpipe->_msgY = 0;
	g_fullpipe->_msgObjectId2 = 0;
	g_fullpipe->_msgId = 0;
}

GameLoader::~GameLoader() {
	free(_gameName);
	delete _gameProject;
	delete _interactionController;
	delete _inputController;
}

bool GameLoader::load(MfcArchive &file) {
	debug(5, "GameLoader::load()");

	_gameName = file.readPascalString();
	debug(6, "_gameName: %s", _gameName);

	_gameProject = new GameProject();

	_gameProject->load(file);

	g_fullpipe->_gameProject = _gameProject;

	if (g_fullpipe->_gameProjectVersion < 12) {
		error("Old gameProjectVersion: %d", g_fullpipe->_gameProjectVersion);
	}

	_gameName = file.readPascalString();
	debug(6, "_gameName: %s", _gameName);

	_inventory.load(file);

	_interactionController->load(file);

	debug(6, "sceneTag count: %d", _gameProject->_sceneTagList->size());

	_sc2array.resize(_gameProject->_sceneTagList->size());

	int i = 0;
	for (SceneTagList::const_iterator it = _gameProject->_sceneTagList->begin(); it != _gameProject->_sceneTagList->end(); ++it, i++) {
		char tmp[12];

		snprintf(tmp, 11, "%04d.sc2", it->_sceneId);

		debug(2, "sc: %s", tmp);

		_sc2array[i].loadFile((const char *)tmp);
	}

	_preloadItems.load(file);

	_field_FA = file.readUint16LE();
	_field_F8 = file.readUint16LE();

	_gameVar = (GameVar *)file.readClass();

	return true;
}

bool GameLoader::loadScene(int sceneId) {
	SceneTag *st;

	int idx = getSceneTagBySceneId(sceneId, &st);

	if (idx < 0)
		return false;

	if (!st->_scene)
		st->loadScene();

	if (st->_scene) {
		st->_scene->init();

		applyPicAniInfos(st->_scene, _sc2array[idx]._defPicAniInfos, _sc2array[idx]._defPicAniInfosCount);
		applyPicAniInfos(st->_scene, _sc2array[idx]._picAniInfos, _sc2array[idx]._picAniInfosCount);

		_sc2array[idx]._scene = st->_scene;
		_sc2array[idx]._isLoaded = 1;

		return true;
	}

	return false;
}

bool GameLoader::gotoScene(int sceneId, int entranceId) {
	SceneTag *st;

	int sc2idx = getSceneTagBySceneId(sceneId, &st);

	if (sc2idx < 0)
		return false;

	if (!_sc2array[sc2idx]._isLoaded)
		return false;

	if (_sc2array[sc2idx]._entranceDataCount < 1) {
		g_fullpipe->_currentScene = st->_scene;
		return true;
	}

	if (_sc2array[sc2idx]._entranceDataCount <= 0)
		return false;

	int entranceIdx = 0;
	if (sceneId != 726) // WORKAROUND
		for (entranceIdx = 0; _sc2array[sc2idx]._entranceData[entranceIdx]->_field_4 != entranceId; entranceIdx++) {
			if (entranceIdx >= _sc2array[sc2idx]._entranceDataCount)
				return false;
		}

	GameVar *sg = _gameVar->getSubVarByName("OBJSTATES")->getSubVarByName("SAVEGAME");

	if (sg || (sg = _gameVar->getSubVarByName("OBJSTATES")->addSubVarAsInt("SAVEGAME", 0)) != 0)
		sg->setSubVarAsInt("Entrance", entranceId);

	if (!g_fullpipe->sceneSwitcher(_sc2array[sc2idx]._entranceData[entranceIdx]))
		return false;

	g_fullpipe->_msgObjectId2 = 0;
	g_fullpipe->_msgY = -1;
	g_fullpipe->_msgX = -1;

	g_fullpipe->_currentScene = st->_scene;

	MessageQueue *mq1 = g_fullpipe->_currentScene->getMessageQueueById(_sc2array[sc2idx]._entranceData[entranceIdx]->_messageQueueId);
	if (mq1) {
		MessageQueue *mq = new MessageQueue(mq1, 0, 0);

		StaticANIObject *stobj = g_fullpipe->_currentScene->getStaticANIObject1ById(_field_FA, -1);
		if (stobj) {
			stobj->_flags &= 0x100;

			ExCommand *ex = new ExCommand(stobj->_id, 34, 256, 0, 0, 0, 1, 0, 0, 0);

			ex->_field_14 = 256;
			ex->_messageNum = 0;
			ex->_excFlags |= 3;

			mq->addExCommandToEnd(ex);
		}

		mq->setFlags(mq->getFlags() | 1);

		if (!mq->chain(0)) {
			delete mq;

			return false;
		}
	} else {
		StaticANIObject *stobj = g_fullpipe->_currentScene->getStaticANIObject1ById(_field_FA, -1);
		if (stobj)
			stobj->_flags &= 0xfeff;
	}

	return true;
}

bool preloadCallback(const PreloadItem &pre, int flag) {
	warning("STUB: preloadCallback");

	return true;
}

bool GameLoader::preloadScene(int sceneId, int entranceId) {
	debug(0, "preloadScene(%d, %d), ", sceneId, entranceId);

	if (_preloadSceneId != sceneId || _preloadEntranceId != entranceId) {
		_preloadSceneId = sceneId;
		_preloadEntranceId = entranceId;
		return true;
	}

	int idx = -1;

	for (uint i = 0; i < _preloadItems.size(); i++)
		if (_preloadItems[i]->preloadId1 == sceneId && _preloadItems[i]->preloadId2 == entranceId) {
			idx = i;
			break;
		}

	if (idx == -1) {
		_preloadSceneId = 0;
		_preloadEntranceId = 0;
		return false;
	}

	if (_preloadCallback) {
		if (!_preloadCallback(*_preloadItems[idx], 0))
			return false;
	}

	if (g_fullpipe->_currentScene && g_fullpipe->_currentScene->_sceneId == sceneId)
		g_fullpipe->_currentScene = 0;

	saveScenePicAniInfos(sceneId);
	clearGlobalMessageQueueList1();
	unloadScene(sceneId);

	if (_preloadCallback)
		_preloadCallback(*_preloadItems[idx], 50);

	loadScene(_preloadItems[idx]->sceneId);

	ExCommand *ex = new ExCommand(_preloadItems[idx]->sceneId, 17, 62, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags = 2;
	ex->_keyCode = _preloadItems[idx]->keyCode;

	_preloadSceneId = 0;
	_preloadEntranceId = 0;

	if (_preloadCallback)
		_preloadCallback(*_preloadItems[idx], 100);

	ex->postMessage();

	return true;
}

bool GameLoader::unloadScene(int sceneId) {
	SceneTag *tag;
	int sceneTag = getSceneTagBySceneId(sceneId, &tag);

	if (sceneTag < 0)
		return false;

	if (_sc2array[sceneTag]._isLoaded)
		saveScenePicAniInfos(sceneId);

	_sc2array[sceneTag]._motionController->freeItems();

	delete tag->_scene;
	tag->_scene = 0;

	_sc2array[sceneTag]._isLoaded = 0;
	_sc2array[sceneTag]._scene = 0;

   return true;
}

int GameLoader::getSceneTagBySceneId(int sceneId, SceneTag **st) {
	if (_sc2array.size() > 0 && _gameProject->_sceneTagList->size() > 0) {
		for (uint i = 0; i < _sc2array.size(); i++) {
			if (_sc2array[i]._sceneId == sceneId) {
				int num = 0;
				for (SceneTagList::iterator s = _gameProject->_sceneTagList->begin(); s != _gameProject->_sceneTagList->end(); ++s, num++) {
					if (s->_sceneId == sceneId) {
						*st = &(*s);
						return num;
					}
				}
			}
		}
	}

	*st = 0;
	return -1;
}

void GameLoader::applyPicAniInfos(Scene *sc, PicAniInfo **picAniInfo, int picAniInfoCount) {
	if (picAniInfoCount <= 0)
		return;

	debug(0, "GameLoader::applyPicAniInfos(sc, ptr, %d)", picAniInfoCount);

	PictureObject *pict;
	StaticANIObject *ani;

	for (int i = 0; i < picAniInfoCount; i++) {
		debug(7, "PicAniInfo: id: %d type: %d", picAniInfo[i]->objectId, picAniInfo[i]->type);
		if (picAniInfo[i]->type & 2) {
			pict = sc->getPictureObjectById(picAniInfo[i]->objectId, picAniInfo[i]->field_8);
			if (pict) {
				pict->setPicAniInfo(picAniInfo[i]);
				continue;
			}
			pict = sc->getPictureObjectById(picAniInfo[i]->objectId, 0);
			if (pict) {
				PictureObject *pictNew = new PictureObject(pict);

				sc->_picObjList.push_back(pictNew);
				pictNew->setPicAniInfo(picAniInfo[i]);
				continue;
			}
		} else {
			if (!(picAniInfo[i]->type & 1))
				continue;

			Scene *scNew = g_fullpipe->accessScene(picAniInfo[i]->sceneId);
			if (!scNew)
				continue;

			ani = sc->getStaticANIObject1ById(picAniInfo[i]->objectId, picAniInfo[i]->field_8);
			if (ani) {
				ani->setPicAniInfo(picAniInfo[i]);
				continue;
			}

			ani = scNew->getStaticANIObject1ById(picAniInfo[i]->objectId, 0);
			if (ani) {
				StaticANIObject *aniNew = new StaticANIObject(ani);

				sc->addStaticANIObject(aniNew, 1);

				aniNew->setPicAniInfo(picAniInfo[i]);
				continue;
			}
		}
	}
}

void GameLoader::saveScenePicAniInfos(int sceneId) {
	warning("STUB: GameLoader::saveScenePicAniInfos(%d)", sceneId);
}

void GameLoader::updateSystems(int counterdiff) {
	if (g_fullpipe->_currentScene) {
		g_fullpipe->_currentScene->update(counterdiff);

		_exCommand._messageKind = 17;
		_updateCounter++;
		_exCommand._messageNum = 33;
		_exCommand._excFlags = 0;
		_exCommand.postMessage();
	}

	processMessages();

	if (_preloadSceneId) {
		processMessages();
		preloadScene(_preloadSceneId, _preloadEntranceId);
	}
}

Sc2::Sc2() {
	_sceneId = 0;
	_field_2 = 0;
	_scene = 0;
	_motionController = 0;
	_data1 = 0;
	_count1 = 0;
	_defPicAniInfos = 0;
	_defPicAniInfosCount = 0;
	_picAniInfos = 0;
	_picAniInfosCount = 0;
	_isLoaded = 0;
	_entranceData = 0;
	_entranceDataCount = 0;
}

bool Sc2::load(MfcArchive &file) {
	debug(5, "Sc2::load()");

	_sceneId = file.readUint16LE();

	_motionController = (MotionController *)file.readClass();

	_count1 = file.readUint32LE();
	debug(4, "count1: %d", _count1);
	if (_count1 > 0) {
		_data1 = (int32 *)malloc(_count1 * sizeof(int32));

		for (int i = 0; i < _count1; i++) {
			_data1[i] = file.readUint32LE();
		}
	} else {
		_data1 = 0;
	}

	_defPicAniInfosCount = file.readUint32LE();
	debug(4, "defPicAniInfos: %d", _defPicAniInfosCount);
	if (_defPicAniInfosCount > 0) {
		_defPicAniInfos = (PicAniInfo **)malloc(_defPicAniInfosCount * sizeof(PicAniInfo *));

		for (int i = 0; i < _defPicAniInfosCount; i++) {
			_defPicAniInfos[i] = new PicAniInfo();

			_defPicAniInfos[i]->load(file);
		}
	} else {
		_defPicAniInfos = 0;
	}

	_picAniInfos = 0;
	_picAniInfosCount = 0;

	_entranceDataCount = file.readUint32LE();
	debug(4, "_entranceData: %d", _entranceDataCount);

	if (_entranceDataCount > 0) {
		_entranceData = (EntranceInfo **)malloc(_entranceDataCount * sizeof(EntranceInfo *));

		for (int i = 0; i < _entranceDataCount; i++) {
			_entranceData[i] = new EntranceInfo();
			_entranceData[i]->load(file);
		}
	} else {
		_entranceData = 0;
	}

	if (file.size() - file.pos() > 0)
		error("Sc2::load(): (%d bytes left)", file.size() - file.pos());

	return true;
}

bool PreloadItems::load(MfcArchive &file) {
	debug(5, "PreloadItems::load()");

	int count = file.readCount();

	clear();

	for (int i = 0; i < count; i++) {
		PreloadItem *t = new PreloadItem();
		t->preloadId1 = file.readUint32LE();
		t->preloadId2 = file.readUint32LE();
		t->sceneId = file.readUint32LE();
		t->keyCode = file.readUint32LE();

		push_back(t);
	}

	return true;
}

GameVar *FullpipeEngine::getGameLoaderGameVar() {
	if (_gameLoader)
		return _gameLoader->_gameVar;
	else
		return 0;
}

InputController *FullpipeEngine::getGameLoaderInputController() {
	if (_gameLoader)
		return _gameLoader->_inputController;
	else
		return 0;
}

} // End of namespace Fullpipe
