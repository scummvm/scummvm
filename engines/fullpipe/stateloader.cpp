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

#include "fullpipe/utils.h"
#include "fullpipe/objects.h"

namespace Fullpipe {

bool FullpipeEngine::loadGam(const char *fname) {
	g_gameLoader = new CGameLoader();

	if (g_gameLoader->loadFile(fname)) {
		// TODO
	} else
		return false;

	return true;
}

bool CGameLoader::loadFile(const char *fname) {
	MfcArchive file;

	if (!file.open(fname))
		return false;

	_gameName = file.readPascalString();
	debug(0, "_gameName: %s", _gameName);

	_gameProject = new GameProject();

	_gameProject->load(file);

	if (g_gameProjectVersion < 12) {
		error("Old gameProjectVersion: %d", g_gameProjectVersion);
	}

	_gameName = file.readPascalString();
	debug(0, "_gameName: %s", _gameName);

	_inventory.load(file);

	debug(0, "%x", file.pos());

	return true;
}

CGameLoader::~CGameLoader() {
	free(_gameName);
	delete _gameProject;
}

GameProject::GameProject() {
	_field_4 = 0;
	_headerFilename = 0;
	_field_10 = 12;
}

bool GameProject::load(MfcArchive &file) {
	g_gameProjectVersion = file.readUint32LE();
	g_gameProjectValue = file.readUint16LE();
	g_scrollSpeed = file.readUint32LE();

	_headerFilename = file.readPascalString();

	debug(0, "_gameProjectVersion = %d", g_gameProjectVersion);
	debug(0, "_gameProjectValue = %d", g_gameProjectValue);
	debug(0, "_scrollSpeed = %d", g_scrollSpeed);
	debug(0, "_headerFilename = %s", _headerFilename);

	_sceneTagList = new SceneTagList();

	_sceneTagList->load(file);

	if (g_gameProjectVersion >= 3)
		_field_4 = file.readUint32LE();

	if (g_gameProjectVersion >= 5) {
		file.readUint32LE();
		file.readUint32LE();
	}

	return true;
}

GameProject::~GameProject() {
	free(_headerFilename);
}

bool SceneTagList::load(MfcArchive &file) {
	int numEntries = file.readUint16LE();

	debug(0, "numEntries: %d", numEntries);

	for (int i = 0; i < numEntries; i++) {
		SceneTag *t = new SceneTag();
		t->load(file);
		_list.push_back(*t);
	}

	return true;
}

SceneTag::SceneTag() {
	_field_4 = 0;
	_scene = 0;
}

bool SceneTag::load(MfcArchive &file) {
	_sceneId = file.readUint16LE();

	_tag = file.readPascalString();

	debug(0, "sceneId: %d  tag: %s", _sceneId, _tag);

	return true;
}

SceneTag::~SceneTag() {
	free(_tag);
}

bool CInventory::load(MfcArchive &file) {
	_sceneId = file.readUint16LE();
	int numInvs = file.readUint32LE();

	debug(0, "numInvs: %d %x", numInvs, numInvs);

	for (int i = 0; i < numInvs; i++) {
		InventoryPoolItem *t = new InventoryPoolItem();
		t->_id = file.readUint16LE();
		t->_pictureObjectNormalId = file.readUint16LE();
		t->_pictureObjectId1 = file.readUint16LE();
		t->_pictureObjectMouseInsideId = file.readUint16LE();
		t->_pictureObjectId3 = file.readUint16LE();
		t->_flags = file.readUint32LE();
		t->_field_C = 0;
		t->_field_A = -536;
		_itemsPool.push_back(*t);
	}

	return true;
}

CInventory2::CInventory2() {
	_selectedId = -1;
	_field_48 = -1;
	_sceneObj = 0;
	_picture = 0;
	_isInventoryOut = 0;
	_isLocked = 0;
	_topOffset = -65;
}

bool CInventory2::load(MfcArchive &file) {
	return _inventory.load(file);
}

bool CInventory2::loadPartial(MfcArchive &file) { // CInventory2_SerializePartially
	int numInvs = file.readUint32LE();

	debug(0, "numInvs: %d", numInvs);

	for (int i = 0; i < numInvs; i++) {
		InventoryItem *t = new InventoryItem();
		t->itemId = file.readUint16LE();
		t->count = file.readUint16LE();
		_inventoryItems.push_back(*t);
	}

	return true;
}

} // End of namespace Fullpipe
