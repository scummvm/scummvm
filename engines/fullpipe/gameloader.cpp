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

namespace Fullpipe {

CInventory2 *getGameLoaderInventory() {
	return &g_fullpipe->_gameLoader->_inventory;
}

CInteractionController *getGameLoaderInteractionController() {
	return g_fullpipe->_gameLoader->_interactionController;
}

CGameLoader::CGameLoader() {
	_interactionController = new CInteractionController();

	_gameProject = 0;
	//_gameName = "untitled";

	//addMessageHandler2(CGameLoader_messageHandler1, 0, 0);
	//insertMessageHandler(CGameLoader_messageHandler2, 0, 128);
	//insertMessageHandler(CGameLoader_messageHandler3, 0, 1);

	_field_FA = 0;
	_field_F8 = 0;
	_sceneSwitcher = 0;
	_preloadCallback = 0;
	_readSavegameCallback = 0;
	_gameVar = 0;
	_preloadId1 = 0;
	_preloadId2 = 0;
	_updateCounter = 0;

	//g_x = 0;
	//g_y = 0;
	//dword_478480 = 0;
	//g_objectId2 = 0;
	//g_id = 0;
}

CGameLoader::~CGameLoader() {
	free(_gameName);
	delete _gameProject;
}

bool CGameLoader::load(MfcArchive &file) {
	debug(5, "CGameLoader::load()");

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

	_gameVar = (CGameVar *)file.readClass();

	return true;
}

bool CGameLoader::loadScene(int num) {
	SceneTag *st;

	int idx = getSceneTagBySceneId(num, &st);

	if (st->_scene)
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

int CGameLoader::getSceneTagBySceneId(int num, SceneTag **st) {
	warning("STUB: CGameLoader::getSceneTagBySceneId()");

	return 0;
}

void CGameLoader::applyPicAniInfos(Scene *sc, PicAniInfo **picAniInfo, int picAniInfoCount) {
	warning("STUB: CGameLoader::applyPicAniInfo()");
}

} // End of namespace Fullpipe
