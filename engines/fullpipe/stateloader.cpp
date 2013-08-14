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

#include "common/file.h"
#include "common/array.h"
#include "common/list.h"

#include "fullpipe/objects.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/scene.h"
#include "fullpipe/statics.h"
#include "fullpipe/interaction.h"

#include "fullpipe/constants.h"

namespace Fullpipe {

bool FullpipeEngine::loadGam(const char *fname, int scene) {
	_gameLoader = new CGameLoader();

	if (!_gameLoader->loadFile(fname))
		return false;

	_currSoundListCount = 0;
	initObjectStates();
	// set_g_messageQueueCallback1(messageQueueCallback1); // substituted with direct call

	addMessageHandlerByIndex(global_messageHandler1, 0, 4);

	_inventory = getGameLoaderInventory();
	_inventory->setItemFlags(ANI_INV_MAP, 0x10003);
	_inventory->addItem(ANI_INV_MAP, 1);

	_inventory->rebuildItemRects();

	for (CPtrList::iterator p = _inventory->getScene()->_picObjList.begin(); p != _inventory->getScene()->_picObjList.end(); ++p) {
		((MemoryObject *)((PictureObject *)*p)->_picture)->load();
	}

	// _sceneSwitcher = sceneSwitcher; // substituted with direct call
	// _preloadCallback = gameLoaderPreloadCallback
	// _readSavegameCallback = gameLoaderReadSavegameCallback;

	_aniMan = accessScene(SC_COMMON)->getAniMan();
	_scene2 = 0;

	_movTable = _aniMan->countMovements();

	_aniMan->setSpeed(1);

	PictureObject *pic = accessScene(SC_INV)->getPictureObjectById(PIC_INV_MENU, 0);

	pic->setFlags(pic->_flags & 0xFFFB);

	// Not used in full game
	//_evalVersionPic = accessScene(SC_COMMON)->getPictureObjectById(PIC_CMN_EVAL, 0);

	initMap();
	initCursors();

	setMusicAllowed(_gameLoader->_gameVar->getSubVarAsInt("MUSIC_ALLOWED"));

	if (scene) {
		_gameLoader->loadScene(scene);
		_gameLoader->gotoScene(scene, TrubaLeft);
	} else {
		if (_flgPlayIntro) {
			_gameLoader->loadScene(SC_INTRO1);
			_gameLoader->gotoScene(SC_INTRO1, TrubaUp);
		} else {
			_gameLoader->loadScene(SC_1);
			_gameLoader->gotoScene(SC_1, TrubaLeft);
		}
	}

	if (!_currentScene)
		return false;

	return true;
}

GameProject::GameProject() {
	_field_4 = 0;
	_headerFilename = 0;
	_field_10 = 12;
}

bool GameProject::load(MfcArchive &file) {
	debug(5, "GameProject::load()");

	_field_4 = 0;
	_headerFilename = 0;
	_field_10 = 12;

	g_fullpipe->_gameProjectVersion = file.readUint32LE();
	g_fullpipe->_pictureScale = file.readUint16LE();
	g_fullpipe->_scrollSpeed = file.readUint32LE();

	_headerFilename = file.readPascalString();

	debug(1, "_gameProjectVersion = %d", g_fullpipe->_gameProjectVersion);
	debug(1, "_pictureScale = %d", g_fullpipe->_pictureScale);
	debug(1, "_scrollSpeed = %d", g_fullpipe->_scrollSpeed);
	debug(1, "_headerFilename = %s", _headerFilename);

	_sceneTagList = new SceneTagList();

	_sceneTagList->load(file);

	if (g_fullpipe->_gameProjectVersion >= 3)
		_field_4 = file.readUint32LE();

	if (g_fullpipe->_gameProjectVersion >= 5) {
		file.readUint32LE();
		file.readUint32LE();
	}

	return true;
}

GameProject::~GameProject() {
	free(_headerFilename);
}

bool PreloadItems::load(MfcArchive &file) {
	debug(5, "PreloadItems::load()");

	int count = file.readCount();

	resize(count);

	for (int i = 0; i < count; i++) {
		PreloadItem *t = new PreloadItem();
		t->preloadId1 = file.readUint32LE();
		t->preloadId2 = file.readUint32LE();
		t->sceneId = file.readUint32LE();
		t->field_C = file.readUint32LE();

		push_back(*t);
	}

	return true;
}

CGameVar::CGameVar() {
	_subVars = 0;
	_parentVarObj = 0;
	_nextVarObj = 0;
	_prevVarObj = 0;
	_field_14 = 0;
	_varType = 0;
	_value.floatValue = 0;
}

bool CGameVar::load(MfcArchive &file) {
	_varName = file.readPascalString();
	_varType = file.readUint32LE();

	debugN(6, "[%03d] ", file.getLevel());
	for (int i = 0; i < file.getLevel(); i++)
		debugN(6, " ");

	debugN(6, "<%s>: ", transCyrillic((byte *)_varName));

	switch (_varType) {
	case 0:
		_value.intValue = file.readUint32LE();
		debug(6, "d --> %d", _value.intValue);
		break;
	case 1:
		_value.intValue = file.readUint32LE(); // FIXME
		debug(6, "f --> %f", _value.floatValue);
		break;
	case 2:
		_value.stringValue = file.readPascalString();
		debug(6, "s --> %s", _value.stringValue);
		break;
	default:
		error("Unknown var type: %d (0x%x)", _varType, _varType);
	}

	file.incLevel();
	_parentVarObj = (CGameVar *)file.readClass();
	_prevVarObj = (CGameVar *)file.readClass();
	_nextVarObj = (CGameVar *)file.readClass();
	_field_14 = (CGameVar *)file.readClass();
	_subVars = (CGameVar *)file.readClass();
	file.decLevel();

	return true;
}

CGameVar *CGameVar::getSubVarByName(const char *name) {
	CGameVar *sv = 0;

	if (_subVars != 0) {
		sv = _subVars;
		for (;sv && scumm_stricmp(sv->_varName, name); sv = sv->_nextVarObj)
			;
	}
	return sv;
}

bool CGameVar::setSubVarAsInt(const char *name, int value) {
	CGameVar *var = getSubVarByName(name);

	if (var) {
		if (var->_varType == 0) {
			var->_value.intValue = value;

			return true;
		}
		return false;
	}

	var = new CGameVar();
	var->_varType = 0;
	var->_value.intValue = value;
	var->_varName = (char *)calloc(strlen(name) + 1, 1);
	strcpy(var->_varName, name);

	return addSubVar(var);
}

int CGameVar::getSubVarAsInt(const char *name) {
	CGameVar *var = getSubVarByName(name);

	if (var)
		return var->_value.intValue;
	else
		return 0;
}

CGameVar *CGameVar::addSubVarAsInt(const char *name, int value) {
	if (getSubVarByName(name)) {
		return 0;
	} else {
		CGameVar *var = new CGameVar();

		var->_varType = 0;
		var->_value.intValue = value;

		var->_varName = (char *)calloc(strlen(name) + 1, 1);
		strcpy(var->_varName, name);

		return (addSubVar(var) != 0) ? var : 0;
	}
}

bool CGameVar::addSubVar(CGameVar *subvar) {
	CGameVar *var = _subVars;

	if (var) {
		for (CGameVar *i = var->_nextVarObj; i; i = i->_nextVarObj)
			var = i;

		var->_nextVarObj = subvar;
		subvar->_prevVarObj = var;
		subvar->_parentVarObj = this;

		return true;
	} else {
		_subVars = subvar;
		subvar->_parentVarObj = this;

		return true;
	}

	return false;
}

int CGameVar::getSubVarsCount() {
	int res;
	CGameVar *sub = _subVars;

	for (res = 0; sub; res++)
		sub = sub->_nextVarObj;

	return res;
}

CGameVar *CGameVar::getSubVarByIndex(int idx) {
	CGameVar *sub = _subVars;

	while (idx--) {
		sub = sub->_nextVarObj;

		if (!sub)
			return 0;
	}

	return sub;
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

	_motionController = (CMotionController *)file.readClass();

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
		_entranceData = (EntranceInfo **)malloc(_defPicAniInfosCount * sizeof(EntranceInfo *));

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

bool PicAniInfo::load(MfcArchive &file) {
	debug(5, "PicAniInfo::load()");

	type = file.readUint32LE();
	objectId = file.readUint16LE();
	field_6 = file.readUint16LE();
	field_8 = file.readUint32LE();
	sceneId = file.readUint16LE();
	field_E = file.readUint16LE();
	ox = file.readUint32LE();
	oy = file.readUint32LE();
	priority = file.readUint32LE();
	staticsId = file.readUint16LE();
	movementId = file.readUint16LE();
	dynamicPhaseIndex = file.readUint16LE();
	flags = file.readUint16LE();
	field_24 = file.readUint32LE();
	someDynamicPhaseIndex = file.readUint32LE();

	return true;
}

} // End of namespace Fullpipe
