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

CGameLoader::CGameLoader() {
	_interactionController = new CInteractionController();

	// g_gameLoader = this;  // FIXME

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

bool CObject::loadFile(const char *fname) {
	MfcArchive file;

	if (!file.open(fname))
		return false;

	return load(file);
}

bool CGameLoader::load(MfcArchive &file) {
	_gameName = file.readPascalString();
	debug(6, "_gameName: %s", _gameName);

	_gameProject = new GameProject();

	_gameProject->load(file);

	if (g_gameProjectVersion < 12) {
		error("Old gameProjectVersion: %d", g_gameProjectVersion);
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

		debug(0, "sc: %d", it->_sceneId);

		_sc2array[i].loadFile((const char *)tmp);
	}

	_preloadItems.load(file);

	_field_FA = file.readUint16LE();
	_field_F8 = file.readUint16LE();

	_gameVar = (CGameVar *)file.readClass();

	return true;
}

GameProject::GameProject() {
	_field_4 = 0;
	_headerFilename = 0;
	_field_10 = 12;
}

bool GameProject::load(MfcArchive &file) {
	_field_4 = 0;
	_headerFilename = 0;
	_field_10 = 12;

	g_gameProjectVersion = file.readUint32LE();
	g_gameProjectValue = file.readUint16LE();
	g_scrollSpeed = file.readUint32LE();

	_headerFilename = file.readPascalString();

	debug(1, "_gameProjectVersion = %d", g_gameProjectVersion);
	debug(1, "_gameProjectValue = %d", g_gameProjectValue);
	debug(1, "_scrollSpeed = %d", g_scrollSpeed);
	debug(1, "_headerFilename = %s", _headerFilename);

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

	for (int i = 0; i < numEntries; i++) {
		SceneTag *t = new SceneTag();
		t->load(file);
		push_back(*t);
	}

	return true;
}

SceneTag::SceneTag() {
	_field_4 = 0;
	_scene = 0;
}

bool SceneTag::load(MfcArchive &file) {
	_field_4 = 0;
	_scene = 0;

	_sceneId = file.readUint16LE();

	_tag = file.readPascalString();

	debug(6, "sceneId: %d  tag: %s", _sceneId, _tag);

	return true;
}

SceneTag::~SceneTag() {
	free(_tag);
}

bool CInventory::load(MfcArchive &file) {
	_sceneId = file.readUint16LE();
	int numInvs = file.readUint32LE();

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

	for (int i = 0; i < numInvs; i++) {
		InventoryItem *t = new InventoryItem();
		t->itemId = file.readUint16LE();
		t->count = file.readUint16LE();
		_inventoryItems.push_back(*t);
	}

	return true;
}

bool CInteractionController::load(MfcArchive &file) {
	return _interactions.load(file);
}

bool CObList::load(MfcArchive &file) {
	int count = file.readCount();

	for (int i = 0; i < count; i++) {
		CObject *t = file.readClass();

		push_back(*t);
	}

	return true;
}

CInputController::CInputController() {
	// TODO
}

CInteraction::CInteraction() {
	_objectId1 = 0;
	_objectId2 = 0;
	_staticsId1 = 0;
	_objectId3 = 0;
	_objectState2 = 0;
	_objectState1 = 0;
	_messageQueue = 0;
	_flags = 0;
	_yOffs = 0;
	_xOffs = 0;
	_staticsId2 = 0;
	_field_28 = 0;
	_sceneId = -1;
}

bool CInteraction::load(MfcArchive &file) {
	_objectId1 = file.readUint16LE();
	_objectId2 = file.readUint16LE();
	_staticsId1 = file.readUint16LE();
	_staticsId2 = file.readUint16LE();
	_objectId3 = file.readUint16LE();
	_objectState2 = file.readUint32LE();
	_objectState1 = file.readUint32LE();
	_xOffs = file.readUint32LE();
	_yOffs = file.readUint32LE();
	_sceneId = file.readUint32LE();
	_flags = file.readUint32LE();
	_stringObj = file.readPascalString();

	_messageQueue = (MessageQueue *)file.readClass();

	return true;
}

MessageQueue::MessageQueue() {
	_field_14 = 0;
	_parId = 0;
	_dataId = 0;
	_id = 0;
	_isFinished = 0;
	_flags = 0;
}

bool MessageQueue::load(MfcArchive &file) {
	_dataId = file.readUint16LE();

	int count = file.readUint16LE();

	_stringObj = file.readPascalString();

	for (int i = 0; i < count; i++) {
		CObject *tmp = file.readClass();

		_exCommands.push_back(tmp);
	}

	_id = -1;
	_field_14 = 0;
	_parId = 0;
	_isFinished = 0;

	return true;
}

ExCommand::ExCommand() {
	_field_3C = 1;
	_messageNum = 0;
	_flags = 0;
	_parId = 0;
}

bool ExCommand::load(MfcArchive &file) {
	_msg._parentId = file.readUint16LE();
	_msg._messageKind = file.readUint32LE();
	_msg._x = file.readUint32LE();
	_msg._y = file.readUint32LE();
	_msg._field_14 = file.readUint32LE();
	_msg._sceneClickX = file.readUint32LE();
	_msg._sceneClickY = file.readUint32LE();
	_msg._field_20 = file.readUint32LE();
	_msg._field_24 = file.readUint32LE();
	_msg._param28 = file.readUint32LE();
	_msg._field_2C = file.readUint32LE();
	_msg._field_30 = file.readUint32LE();
	_msg._field_34 = file.readUint32LE();

	_messageNum = file.readUint32LE();

	_field_3C = 0;

	if (g_gameProjectVersion >= 12) {
		_flags = file.readUint32LE();
		_parId = file.readUint32LE();
	}

	return true;
}

Message::Message() {
	_messageKind = 0;
	_parentId = 0;
	_x = 0;
	_y = 0;
	_field_14 = 0;
	_sceneClickX = 0;
	_sceneClickY = 0;
	_field_20 = 0;
	_field_24 = 0;
	_param28 = 0;
	_field_2C = 0;
	_field_30 = 0;
	_field_34 = 0;
}

CObjstateCommand::CObjstateCommand() {
	_value = 0;
}

bool CObjstateCommand::load(MfcArchive &file) {
	_cmd.load(file);

	_value = file.readUint32LE();

	_stringObj = file.readPascalString();

	return true;
}

bool CObArray::load(MfcArchive &file) {
	int count = file.readCount();

	resize(count);

	for (int i = 0; i < count; i++) {
		CObject *t = file.readClass();

		push_back(*t);
	}

	return true;
}

bool PreloadItems::load(MfcArchive &file) {
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
	_stringObj = file.readPascalString();
	_varType = file.readUint32LE();

	debugN(6, "[%03d] ", file.getLevel());
	for (int i = 0; i < file.getLevel(); i++)
		debugN(6, " ");

	debugN(6, "<%s>: ", _stringObj);

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

Sc2::Sc2() {
	_sceneId = 0;
	_field_2 = 0;
	//_scene = 0;
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
	_sceneId = file.readUint16LE();

	_motionController = (CMotionController *)file.readClass();

	return true;
}


bool CMotionController::load(MfcArchive &file) {
	// Is originally empty	file.readClass();


	return true;
}

bool CMctlCompound::load(MfcArchive &file) {
	int count = file.readUint32LE();

	debug(0, "CMctlCompund::count = %d", count);

	for (int i = 0; i < count; i++) {
	  CMctlCompoundArrayItem *obj = (CMctlCompoundArrayItem *)file.readClass();

	  _motionControllers.push_back(*obj);
	}

	return true;
}

bool CMctlCompoundArray::load(MfcArchive &file) {
	int count = file.readUint32LE();

	debug(0, "CMctlCompundArray::count = %d", count);

	return true;
}

} // End of namespace Fullpipe
