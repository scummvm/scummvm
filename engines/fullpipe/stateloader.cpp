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

#include "fullpipe/fullpipe.h"

#include "common/file.h"
#include "common/array.h"
#include "common/list.h"

#include "fullpipe/objects.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/scene.h"
#include "fullpipe/statics.h"
#include "fullpipe/interaction.h"
#include "fullpipe/gameloader.h"

#include "fullpipe/constants.h"

namespace Fullpipe {

bool FullpipeEngine::loadGam(const char *fname, int scene) {
	_gameLoader = new GameLoader();

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

	for (uint i = 0; i < _inventory->getScene()->_picObjList.size(); i++)
		((MemoryObject *)_inventory->getScene()->_picObjList[i]->_picture)->load();

	// _sceneSwitcher = sceneSwitcher; // substituted with direct call
	_gameLoader->_preloadCallback = preloadCallback;
	// _readSavegameCallback = gameLoaderReadSavegameCallback; // TODO

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
		_gameLoader->loadScene(726);
		_gameLoader->gotoScene(726, TrubaLeft);

		if (scene != 726)
			_gameLoader->preloadScene(726, getSceneEntrance(scene));
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

	_sceneTagList = 0;
}

bool GameProject::load(MfcArchive &file) {
	debug(5, "GameProject::load()");

	_field_4 = 0;
	_headerFilename = 0;
	_field_10 = 12;

	g_fp->_gameProjectVersion = file.readUint32LE();
	g_fp->_pictureScale = file.readUint16LE();
	g_fp->_scrollSpeed = file.readUint32LE();

	_headerFilename = file.readPascalString();

	debug(1, "_gameProjectVersion = %d", g_fp->_gameProjectVersion);
	debug(1, "_pictureScale = %d", g_fp->_pictureScale);
	debug(1, "_scrollSpeed = %d", g_fp->_scrollSpeed);
	debug(1, "_headerFilename = %s", _headerFilename);

	_sceneTagList = new SceneTagList();

	_sceneTagList->load(file);

	if (g_fp->_gameProjectVersion >= 3)
		_field_4 = file.readUint32LE();

	if (g_fp->_gameProjectVersion >= 5) {
		file.readUint32LE();
		file.readUint32LE();
	}

	return true;
}

GameProject::~GameProject() {
	free(_headerFilename);

	delete _sceneTagList;
}

GameVar::GameVar() {
	_subVars = 0;
	_parentVarObj = 0;
	_nextVarObj = 0;
	_prevVarObj = 0;
	_field_14 = 0;
	_varType = 0;
	_value.floatValue = 0;
	_varName = 0;
}

GameVar::~GameVar() {
	warning("STUB: GameVar::~GameVar()");
}

bool GameVar::load(MfcArchive &file) {
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
	_parentVarObj = (GameVar *)file.readClass();
	_prevVarObj = (GameVar *)file.readClass();
	_nextVarObj = (GameVar *)file.readClass();
	_field_14 = (GameVar *)file.readClass();
	_subVars = (GameVar *)file.readClass();
	file.decLevel();

	return true;
}

GameVar *GameVar::getSubVarByName(const char *name) {
	GameVar *sv = 0;

	if (_subVars != 0) {
		sv = _subVars;
		for (;sv && scumm_stricmp(sv->_varName, name); sv = sv->_nextVarObj)
			;
	}
	return sv;
}

bool GameVar::setSubVarAsInt(const char *name, int value) {
	GameVar *var = getSubVarByName(name);

	if (var) {
		if (var->_varType == 0) {
			var->_value.intValue = value;

			return true;
		}
		return false;
	}

	var = new GameVar();
	var->_varType = 0;
	var->_value.intValue = value;
	var->_varName = (char *)calloc(strlen(name) + 1, 1);
	strcpy(var->_varName, name);

	return addSubVar(var);
}

int GameVar::getSubVarAsInt(const char *name) {
	GameVar *var = getSubVarByName(name);

	if (var)
		return var->_value.intValue;
	else
		return 0;
}

GameVar *GameVar::addSubVarAsInt(const char *name, int value) {
	if (getSubVarByName(name)) {
		return 0;
	} else {
		GameVar *var = new GameVar();

		var->_varType = 0;
		var->_value.intValue = value;

		var->_varName = (char *)calloc(strlen(name) + 1, 1);
		strcpy(var->_varName, name);

		return (addSubVar(var) != 0) ? var : 0;
	}
}

bool GameVar::addSubVar(GameVar *subvar) {
	GameVar *var = _subVars;

	if (var) {
		for (GameVar *i = var->_nextVarObj; i; i = i->_nextVarObj)
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

int GameVar::getSubVarsCount() {
	int res;
	GameVar *sub = _subVars;

	for (res = 0; sub; res++)
		sub = sub->_nextVarObj;

	return res;
}

GameVar *GameVar::getSubVarByIndex(int idx) {
	GameVar *sub = _subVars;

	while (idx--) {
		sub = sub->_nextVarObj;

		if (!sub)
			return 0;
	}

	return sub;
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
