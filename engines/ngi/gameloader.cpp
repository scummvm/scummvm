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

#include "ngi/ngi.h"

#include "ngi/gameloader.h"
#include "ngi/scene.h"
#include "ngi/input.h"
#include "ngi/statics.h"
#include "ngi/interaction.h"
#include "ngi/motion.h"
#include "ngi/constants.h"
#include "ngi/scenes.h"
#include "ngi/floaters.h"

namespace NGI {

Inventory2 *getGameLoaderInventory() {
	return &g_nmi->_gameLoader->_inventory;
}

static MotionController *getMotionControllerBySceneId(int16 sceneId) {
	for (uint i = 0; i < g_nmi->_gameLoader->_sc2array.size(); i++) {
		if (g_nmi->_gameLoader->_sc2array[i]._sceneId == sceneId) {
			return g_nmi->_gameLoader->_sc2array[i]._motionController;
		}
	}

	return nullptr;
}

MovGraph *getSc2MovGraphBySceneId(int16 sceneId) {
	MotionController *mc = getMotionControllerBySceneId(sceneId);
	if (mc) {
		assert(mc->_objtype == kObjTypeMovGraph);
		return static_cast<MovGraph *>(mc);
	}
	return nullptr;
}

MctlCompound *getSc2MctlCompoundBySceneId(int16 sceneId) {
	MotionController *mc = getMotionControllerBySceneId(sceneId);
	if (mc) {
		assert(mc->_objtype == kObjTypeMctlCompound);
		return static_cast<MctlCompound *>(mc);
	}
	return nullptr;
}

InteractionController *getGameLoaderInteractionController() {
	return g_nmi->_gameLoader->_interactionController;
}

GameLoader::GameLoader() {
	_interactionController = new InteractionController();
	_inputController = new InputController();

	addMessageHandlerByIndex(global_messageHandler2, 0, 0);
	insertMessageHandler(global_messageHandler3, 0, 128);
	insertMessageHandler(global_messageHandler4, 0, 1);

	_field_FA = 0;
	_field_F8 = 0;
	_sceneSwitcher = 0;
	_preloadCallback = 0;
	_savegameCallback = 0;
	_gameVar = 0;
	_preloadSceneId = 0;
	_preloadEntranceId = 0;
	_updateCounter = 0;

	g_nmi->_msgX = 0;
	g_nmi->_msgY = 0;
	g_nmi->_msgObjectId2 = 0;
	g_nmi->_msgId = 0;
}

GameLoader::~GameLoader() {
	delete _interactionController;
	delete _inputController;
	delete _gameVar;
}

bool GameLoader::load(MfcArchive &file) {
	debugC(1, kDebugLoading, "GameLoader::load()");

	_gameName = file.readPascalString();
	debugC(1, kDebugLoading, "_gameName: %s", _gameName.c_str());

	_gameProject.reset(new GameProject());

	if (!_gameProject->load(file))
		error("Cannot load project");

	g_nmi->_gameProject = _gameProject.get();

	if (g_nmi->_gameProjectVersion < 12) {
		error("GameLoader::load(): old gameProjectVersion: %d", g_nmi->_gameProjectVersion);
	}

	_gameName = file.readPascalString();
	debugC(1, kDebugLoading, "_gameName: %s", _gameName.c_str());

	_inventory.load(file);

	_interactionController->load(file);

	debugC(1, kDebugLoading, "sceneTag count: %d", _gameProject->_sceneTagList->size());

	_sc2array.resize(_gameProject->_sceneTagList->size());

	int i = 0;
	for (SceneTagList::const_iterator it = _gameProject->_sceneTagList->begin(); it != _gameProject->_sceneTagList->end(); ++it, i++) {
		char tmp[12];

		snprintf(tmp, 11, "%04d.sc2", it->_sceneId);

		debugC(1, kDebugLoading, "sc: %s", tmp);

		_sc2array[i].loadFile(tmp);
	}

	_preloadItems.load(file);

	_field_FA = file.readUint16LE();
	_field_F8 = file.readUint16LE();

	debugC(1, kDebugLoading, "_field_FA: %d\n_field_F8: %d", _field_FA, _field_F8);

	_gameVar = file.readClass<GameVar>();

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

		applyPicAniInfos(st->_scene, _sc2array[idx]._defPicAniInfos);
		applyPicAniInfos(st->_scene, _sc2array[idx]._picAniInfos);

		_sc2array[idx]._scene = st->_scene;
		_sc2array[idx]._isLoaded = true;

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

	if (_sc2array[sc2idx]._entranceData.size() < 1) {
		g_nmi->_currentScene = st->_scene;
		return true;
	}

	if (!_sc2array[sc2idx]._entranceData.size())
		return false;

	uint entranceIdx = 0;
	if (sceneId != 726) // WORKAROUND
		for (entranceIdx = 0; _sc2array[sc2idx]._entranceData[entranceIdx]._field_4 != entranceId; entranceIdx++) {
			if (entranceIdx >= _sc2array[sc2idx]._entranceData.size())
				return false;
		}

	GameVar *sg = _gameVar->getSubVarByName("OBJSTATES")->getSubVarByName("SAVEGAME");

	if (sg || (sg = _gameVar->getSubVarByName("OBJSTATES")->addSubVarAsInt("SAVEGAME", 0)) != 0)
		sg->setSubVarAsInt("Entrance", entranceId);

	if (!g_nmi->sceneSwitcher(_sc2array[sc2idx]._entranceData[entranceIdx]))
		return false;

	g_nmi->_msgObjectId2 = 0;
	g_nmi->_msgY = -1;
	g_nmi->_msgX = -1;

	g_nmi->_currentScene = st->_scene;

	MessageQueue *mq1 = g_nmi->_currentScene->getMessageQueueById(_sc2array[sc2idx]._entranceData[entranceIdx]._messageQueueId);
	if (mq1) {
		MessageQueue *mq = new MessageQueue(mq1, 0, 0);

		StaticANIObject *stobj = g_nmi->_currentScene->getStaticANIObject1ById(_field_FA, -1);
		if (stobj) {
			stobj->_flags &= 0x100;

			ExCommand *ex = new ExCommand(stobj->_id, 34, 256, 0, 0, 0, 1, 0, 0, 0);

			ex->_z = 256;
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
		StaticANIObject *stobj = g_nmi->_currentScene->getStaticANIObject1ById(_field_FA, -1);
		if (stobj)
			stobj->_flags &= 0xfeff;
	}

	return true;
}

bool preloadCallback(PreloadItem &pre, int flag) {
	if (flag) {
		if (flag == 50)
			g_nmi->_aniMan->preloadMovements(g_nmi->_movTable.get());

		StaticANIObject *pbar = g_nmi->_loaderScene->getStaticANIObject1ById(ANI_PBAR, -1);

		if (pbar) {
			int sz;

			if (pbar->_movement->_currMovement)
				sz = pbar->_movement->_currMovement->_dynamicPhases.size();
			else
				sz = pbar->_movement->_dynamicPhases.size();

			pbar->_movement->setDynamicPhaseIndex(flag * (sz - 1) / 100);
		}

		g_nmi->updateMap(&pre);

		g_nmi->_currentScene = g_nmi->_loaderScene;

		g_nmi->_loaderScene->draw();

		g_nmi->_system->updateScreen();
	} else {
		if (g_nmi->_scene2) {
			g_nmi->_aniMan = g_nmi->_scene2->getAniMan();
			g_nmi->_scene2 = 0;
			setInputDisabled(1);
		}

		g_nmi->_floaters->stopAll();

		if (g_nmi->_soundEnabled) {
			g_nmi->_currSoundListCount = 1;
			g_nmi->_currSoundList1[0] = g_nmi->accessScene(SC_COMMON)->_soundList.get();
		}

		g_vars->scene18_inScene18p1 = false;

		if ((pre.preloadId1 != SC_18 || pre.sceneId != SC_19) && (pre.preloadId1 != SC_19 || (pre.sceneId != SC_18 && pre.sceneId != SC_19))) {
			if (g_nmi->_scene3) {
				if (pre.preloadId1 != SC_18)
					g_nmi->_gameLoader->unloadScene(SC_18);

				g_nmi->_scene3 = 0;
			}
		} else {
			scene19_setMovements(g_nmi->accessScene(pre.preloadId1), pre.param);

			g_vars->scene18_inScene18p1 = true;

			if (pre.preloadId1 == SC_18) {
				g_nmi->_gameLoader->saveScenePicAniInfos(SC_18);

				scene18_preload();
			}
		}

		if (((pre.sceneId == SC_19 && pre.param == TrubaRight) || (pre.sceneId == SC_18 && pre.param == TrubaRight)) && !pre.preloadId2) {
			pre.sceneId = SC_18;
			pre.param = TrubaLeft;
		}

		if (!g_nmi->_loaderScene) {
			g_nmi->_gameLoader->loadScene(SC_LDR);
			g_nmi->_loaderScene = g_nmi->accessScene(SC_LDR);
		}

		StaticANIObject *pbar = g_nmi->_loaderScene->getStaticANIObject1ById(ANI_PBAR, -1);

		if (pbar) {
			pbar->show1(ST_EGTR_SLIMSORROW, ST_MAN_GOU, MV_PBAR_RUN, 0);
			pbar->startAnim(MV_PBAR_RUN, 0, -1);
		}

		g_nmi->_inventoryScene = 0;
		g_nmi->_updateCursorCallback = 0;

		g_nmi->_sceneRect.translate(-g_nmi->_sceneRect.left, -g_nmi->_sceneRect.top);

		g_nmi->_system->delayMillis(10);

		Scene *oldsc = g_nmi->_currentScene;

		g_nmi->_currentScene = g_nmi->_loaderScene;

		g_nmi->_loaderScene->draw();

		g_nmi->_system->updateScreen();

		g_nmi->_currentScene = oldsc;
	}

	return true;
}

void GameLoader::addPreloadItem(const PreloadItem &item) {
	_preloadItems.push_back(item);
}

bool GameLoader::preloadScene(int sceneId, int entranceId) {
	debugC(0, kDebugLoading, "preloadScene(%d, %d), ", sceneId, entranceId);

	if (_preloadSceneId != sceneId || _preloadEntranceId != entranceId) {
		_preloadSceneId = sceneId;
		_preloadEntranceId = entranceId;
		return true;
	}

	int idx = -1;

	for (uint i = 0; i < _preloadItems.size(); i++)
		if (_preloadItems[i].preloadId1 == sceneId && _preloadItems[i].preloadId2 == entranceId) {
			idx = i;
			break;
		}

	if (idx == -1) {
		_preloadSceneId = 0;
		_preloadEntranceId = 0;
		return false;
	}

	if (_preloadCallback) {
		if (!_preloadCallback(_preloadItems[idx], 0))
			return false;
	}

	if (g_nmi->_currentScene && g_nmi->_currentScene->_sceneId == sceneId)
		g_nmi->_currentScene = 0;

	saveScenePicAniInfos(sceneId);
	clearGlobalMessageQueueList1();
	unloadScene(sceneId);

	if (_preloadCallback)
		_preloadCallback(_preloadItems[idx], 50);

	loadScene(_preloadItems[idx].sceneId);

	ExCommand *ex = new ExCommand(_preloadItems[idx].sceneId, 17, 62, 0, 0, 0, 1, 0, 0, 0);
	ex->_excFlags = 2;
	ex->_param = _preloadItems[idx].param;

	_preloadSceneId = 0;
	_preloadEntranceId = 0;

	if (_preloadCallback)
		_preloadCallback(_preloadItems[idx], 100);

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

	_sc2array[sceneTag]._motionController->detachAllObjects();

	delete tag->_scene;
	tag->_scene = nullptr;

	_sc2array[sceneTag]._isLoaded = false;
	_sc2array[sceneTag]._scene = nullptr;

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

void GameLoader::applyPicAniInfos(Scene *sc, const PicAniInfoList &picAniInfo) {
	if (!picAniInfo.size())
		return;

	debugC(0, kDebugAnimation, "GameLoader::applyPicAniInfos(sc, ptr, %d)", picAniInfo.size());

	PictureObject *pict;
	StaticANIObject *ani;

	for (uint i = 0; i < picAniInfo.size(); i++) {
		const PicAniInfo &info = picAniInfo[i];
		debugC(7, kDebugAnimation, "PicAniInfo: id: %d type: %d", info.objectId, info.type);
		if (info.type & 2) {
			pict = sc->getPictureObjectById(info.objectId, info.field_8);
			if (pict) {
				pict->setPicAniInfo(info);
				continue;
			}
			pict = sc->getPictureObjectById(info.objectId, 0);
			if (pict) {
				PictureObject *pictNew = new PictureObject(pict);

				sc->_picObjList.push_back(pictNew);
				pictNew->setPicAniInfo(info);
				continue;
			}
		} else {
			if (!(info.type & 1))
				continue;

			Scene *scNew = g_nmi->accessScene(info.sceneId);
			if (!scNew)
				continue;

			ani = sc->getStaticANIObject1ById(info.objectId, info.field_8);
			if (ani) {
				ani->setPicAniInfo(picAniInfo[i]);
				continue;
			}

			ani = scNew->getStaticANIObject1ById(info.objectId, 0);
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
	SceneTag *st;

	int idx = getSceneTagBySceneId(sceneId, &st);

	if (idx < 0)
		return;

	if (!_sc2array[idx]._isLoaded)
		return;

	if (!st->_scene)
		return;

	_sc2array[idx]._picAniInfos = savePicAniInfos(st->_scene, 0, 128);
}

PicAniInfoList GameLoader::savePicAniInfos(Scene *sc, int flag1, int flag2) {
	if (!sc)
		return PicAniInfoList();

	if (!sc->_picObjList.size())
		return PicAniInfoList();

	int numInfos = sc->_staticANIObjectList1.size() + sc->_picObjList.size() - 1;
	if (numInfos < 1)
		return PicAniInfoList();

	PicAniInfoList res;
	res.reserve(numInfos);

	for (uint i = 0; i < sc->_picObjList.size(); i++) {
		PictureObject *obj = sc->_picObjList[i];

		if (obj && ((obj->_flags & flag1) == flag1) && ((obj->_field_8 & flag2) == flag2)) {
			res.push_back(PicAniInfo());
			obj->getPicAniInfo(res.back());
		}
	}

	for (uint i = 0; i < sc->_staticANIObjectList1.size(); i++) {
		StaticANIObject *obj = sc->_staticANIObjectList1[i];

		if (obj && ((obj->_flags & flag1) == flag1) && ((obj->_field_8 & flag2) == flag2)) {
			res.push_back(PicAniInfo());
			obj->getPicAniInfo(res.back());
			res.back().type &= 0xFFFF;
		}
	}

	debugC(4, kDebugBehavior | kDebugAnimation, "savePicAniInfos: Stored %d infos", res.size());

	return res;
}

void GameLoader::updateSystems(int counterdiff) {
	if (g_nmi->_currentScene) {
		g_nmi->_currentScene->update(counterdiff);

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

Sc2::Sc2() :
	_sceneId(0),
	_field_2(0),
	_scene(nullptr),
	_isLoaded(false),
	_motionController(nullptr) {}

Sc2::~Sc2() {
	delete _motionController;
}

bool Sc2::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "Sc2::load()");

	_sceneId = file.readUint16LE();

	delete _motionController;
	_motionController = file.readClass<MotionController>();

	const uint count1 = file.readUint32LE();
	debugC(4, kDebugLoading, "count1: %d", count1);
	if (count1) {
		_data1.reserve(count1);
		for (uint i = 0; i < count1; i++) {
			_data1.push_back(file.readUint32LE());
		}
	}

	const uint defPicAniInfosCount = file.readUint32LE();
	debugC(4, kDebugLoading, "defPicAniInfos: %d", defPicAniInfosCount);
	if (defPicAniInfosCount) {
		_defPicAniInfos.resize(defPicAniInfosCount);
		for (uint i = 0; i < defPicAniInfosCount; i++) {
			_defPicAniInfos[i].load(file);
		}
	}

	const uint entranceDataCount = file.readUint32LE();
	debugC(4, kDebugLoading, "_entranceData: %d", entranceDataCount);

	if (entranceDataCount) {
		_entranceData.resize(entranceDataCount);
		for (uint i = 0; i < entranceDataCount; i++) {
			_entranceData[i].load(file);
		}
	}

	if (file.size() - file.pos() > 0)
		error("Sc2::load(): (%d bytes left)", file.size() - file.pos());

	return true;
}

bool PreloadItems::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "PreloadItems::load()");

	int count = file.readCount();

	clear();

	resize(count);
	for (int i = 0; i < count; i++) {
		PreloadItem &t = (*this)[i];
		t.preloadId1 = file.readUint32LE();
		t.preloadId2 = file.readUint32LE();
		t.sceneId = file.readUint32LE();
		t.param = file.readSint32LE();
	}

	return true;
}

const char *getSavegameFile(int saveGameIdx) {
	static char buffer[20];
	sprintf(buffer, "fullpipe.s%02d", saveGameIdx);
	return buffer;
}

void GameLoader::restoreDefPicAniInfos() {
	for (uint i = 0; i < _sc2array.size(); i++) {
		_sc2array[i]._picAniInfos.clear();

		if (_sc2array[i]._scene)
			applyPicAniInfos(_sc2array[i]._scene, _sc2array[i]._defPicAniInfos);
	}
}

GameVar *NGIEngine::getGameLoaderGameVar() {
	if (_gameLoader)
		return _gameLoader->_gameVar;
	else
		return 0;
}

InputController *NGIEngine::getGameLoaderInputController() {
	if (_gameLoader)
		return _gameLoader->_inputController;
	else
		return 0;
}

MctlCompound *getCurrSceneSc2MotionController() {
	return getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId);
}

} // End of namespace NGI
