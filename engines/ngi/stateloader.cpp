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

#include "ngi/constants.h"
#include "ngi/detection.h"
#include "ngi/gameloader.h"
#include "ngi/interaction.h"
#include "ngi/objects.h"
#include "ngi/scene.h"
#include "ngi/statics.h"

#include "common/file.h"
#include "common/array.h"
#include "common/list.h"
#include "common/memstream.h"

#include "graphics/thumbnail.h"

namespace NGI {

bool GameLoader::readSavegame(const char *fname) {
	SaveHeader header;
	Common::ScopedPtr<Common::InSaveFile> saveFile(g_system->getSavefileManager()->openForLoading(fname));

	if (!saveFile) {
		warning("Cannot open save %s for loading", fname);
		return false;
	}

	header.version = saveFile->readUint32LE();
	saveFile->read(header.magic, 32);
	header.updateCounter = saveFile->readUint32LE();
	header.unkField = saveFile->readUint32LE();
	header.encSize = saveFile->readUint32LE();

	debugC(3, kDebugLoading, "version: %d magic: %s updateCounter: %d unkField: %d encSize: %d, pos: %d",
			header.version, header.magic, header.updateCounter, header.unkField, header.encSize, saveFile->pos());

	if (header.version != 48)
		return false;

	_updateCounter = header.updateCounter;

	Common::Array<byte> data(header.encSize);
	saveFile->read(data.data(), header.encSize);

	Common::Array<byte> map(800);
	saveFile->read(map.data(), 800);

	FullpipeSavegameHeader header2;
	if (NGI::readSavegameHeader(saveFile.get(), header2)) {
		g_nmi->setTotalPlayTime(header2.playtime * 1000);
	}

	{
		Common::MemoryReadStream tempStream(map.data(), 800, DisposeAfterUse::NO);
		MfcArchive temp(&tempStream);

		if (_savegameCallback)
			_savegameCallback(&temp, false);
	}

	// Deobfuscate the data
	for (int i = 0; i < header.encSize; i++)
		data[i] -= i & 0x7f;

	Common::MemoryReadStream archiveStream(data.data(), header.encSize, DisposeAfterUse::NO);
	MfcArchive archive(&archiveStream);

	GameVar *var = archive.readClass<GameVar>();

	GameVar *v = _gameVar->getSubVarByName("OBJSTATES");

	if (!v) {
		v = _gameVar->addSubVarAsInt("OBJSTATES", 0);

		if (!v) {
			warning("No state to save");
			delete var;
			return false;
		}
	}

	addVar(var, v);

	getGameLoaderInventory()->loadPartial(archive);

	uint32 arrSize = archive.readUint32LE();

	debugC(3, kDebugLoading, "Reading %d infos", arrSize);

	for (uint i = 0; i < arrSize; i++) {

		const uint picAniInfosCount = archive.readUint32LE();
		if (picAniInfosCount)
			debugC(3, kDebugLoading, "Count %d: %d", i, picAniInfosCount);

		_sc2array[i]._picAniInfos.clear();
		_sc2array[i]._picAniInfos.resize(picAniInfosCount);

		for (uint j = 0; j < picAniInfosCount; j++) {
			_sc2array[i]._picAniInfos[j].load(archive);
		}

		_sc2array[i]._isLoaded = false;
	}

	getGameLoaderInventory()->rebuildItemRects();

	PreloadItem preloadItem;

	v = _gameVar->getSubVarByName("OBJSTATES")->getSubVarByName("SAVEGAME");

	if (v) {
		if (g_nmi->_currentScene)
			preloadItem.preloadId1 = g_nmi->_currentScene->_sceneId & 0xffff;
		else
			preloadItem.preloadId1 = 0;

		preloadItem.param = v->getSubVarAsInt("Entrance");
		preloadItem.preloadId2 = 0;
		preloadItem.sceneId = v->getSubVarAsInt("Scene");

		if (_preloadCallback) {
			if (!_preloadCallback(preloadItem, 0))
				return false;
		}

		clearGlobalMessageQueueList1();

		if (g_nmi->_currentScene)
			unloadScene(g_nmi->_currentScene->_sceneId);

		g_nmi->_currentScene = 0;

		if (_preloadCallback)
			_preloadCallback(preloadItem, 50);

		loadScene(preloadItem.sceneId);

		ExCommand *ex = new ExCommand(preloadItem.sceneId, 17, 62, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags = 2;
		ex->_param = preloadItem.param;

		if (_preloadCallback)
			_preloadCallback(preloadItem, 100);

		ex->postMessage();
	}

	return true;
}

void parseSavegameHeader(NGI::FullpipeSavegameHeader &header, SaveStateDescriptor &desc) {
	int day = (header.date >> 24) & 0xFF;
	int month = (header.date >> 16) & 0xFF;
	int year = header.date & 0xFFFF;
	desc.setSaveDate(year, month, day);
	int hour = (header.time >> 8) & 0xFF;
	int minutes = header.time & 0xFF;
	desc.setSaveTime(hour, minutes);
	desc.setPlayTime(header.playtime * 1000);

	desc.setDescription(header.description);
}

void fillDummyHeader(NGI::FullpipeSavegameHeader &header) {
	// This is wrong header, perhaps it is original savegame. Thus fill out dummy values
	header.date = (20 << 24) | (9 << 16) | 2016;
	header.time = (9 << 8) | 56;
	header.playtime = 0;
}

WARN_UNUSED_RESULT bool readSavegameHeader(Common::InSaveFile *in, FullpipeSavegameHeader &header, bool skipThumbnail) {
	uint oldPos = in->pos();

	in->seek(-4, SEEK_END);

	int headerOffset = in->readUint32LE();

	// Sanity check
	if (headerOffset >= in->pos() || headerOffset == 0) {
		in->seek(oldPos, SEEK_SET); // Rewind the file
		fillDummyHeader(header);
		return false;
	}

	in->seek(headerOffset, SEEK_SET);

	in->read(header.id, 6);

	// Validate the header Id
	if (strcmp(header.id, "SVMCR")) {
		in->seek(oldPos, SEEK_SET); // Rewind the file
		fillDummyHeader(header);
		return false;
	}

	header.version = in->readByte();
	header.date = in->readUint32LE();
	header.time = in->readUint16LE();
	header.playtime = in->readUint32LE();

	if (header.version > 1)
		header.description = in->readPascalString();

	// Generate savename
	SaveStateDescriptor desc;

	parseSavegameHeader(header, desc);

	header.saveName = Common::String::format("%s %s", desc.getSaveDate().c_str(), desc.getSaveTime().c_str());

	if (header.description.empty())
		header.description = header.saveName;

	// Get the thumbnail
	if (!Graphics::loadThumbnail(*in, header.thumbnail, skipThumbnail)) {
		in->seek(oldPos, SEEK_SET); // Rewind the file
		return false;
	}

	in->seek(oldPos, SEEK_SET); // Rewind the file

	return true;
}

void GameLoader::addVar(GameVar *var, GameVar *subvar) {
	if (var && subvar) {
		int type = var->_varType;
		if (type == subvar->_varType && (!type || type == 1))
			subvar->_value.intValue = var->_value.intValue;

		for (GameVar *v = var->_subVars; v; v = v->_nextVarObj) {
			GameVar *nv = subvar->getSubVarByName(v->_varName.c_str());
			if (!nv) {
				nv = new GameVar;
				nv->_varName = v->_varName;
				nv->_varType = v->_varType;

				subvar->addSubVar(nv);
			}

			addVar(v, nv);
		}
	}
}

void gameLoaderSavegameCallback(MfcArchive *archive, bool mode) {
	if (mode)
		for (int i = 0; i < 200; i++)
			archive->writeUint32LE(g_nmi->_mapTable[i]);
	else
		for (int i = 0; i < 200; i++)
			g_nmi->_mapTable[i] = archive->readUint32LE();
}

bool NGIEngine::loadGam(const char *fname, int scene) {
	_gameLoader.reset(new GameLoader());

	if (!_gameLoader->loadFile(fname))
		return false;

	_currSoundListCount = 0;
	initObjectStates();
	// set_g_messageQueueCallback1(messageQueueCallback1); // substituted with direct call

	addMessageHandlerByIndex(global_messageHandler1, 0, 4);

	_inventory = getGameLoaderInventory();

	if (isDemo() && getLanguage() == Common::RU_RUS) {
		_inventory->addItem(ANI_INV_HAMMER, 1);
	} else {
		_inventory->setItemFlags(ANI_INV_MAP, 0x10003);
		_inventory->addItem(ANI_INV_MAP, 1);
	}

	_inventory->rebuildItemRects();

	for (uint i = 0; i < _inventory->getScene()->_picObjList.size(); i++)
		_inventory->getScene()->_picObjList[i]->_picture->MemoryObject::load();

	// _sceneSwitcher = sceneSwitcher; // substituted with direct call
	_gameLoader->_preloadCallback = preloadCallback;
	_gameLoader->_savegameCallback = gameLoaderSavegameCallback;

	_aniMan = accessScene(SC_COMMON)->getAniMan();
	_scene2 = 0;

	_movTable.reset(_aniMan->countMovements());

	_aniMan->setSpeed(1);

	PictureObject *pic = accessScene(SC_INV)->getPictureObjectById(PIC_INV_MENU, 0);

	pic->setFlags(pic->_flags & 0xFFFB);

	// Not used in full game
	//_evalVersionPic = accessScene(SC_COMMON)->getPictureObjectById(PIC_CMN_EVAL, 0);

	initMap();
	initCursors();

	setMusicAllowed(_gameLoader->_gameVar->getSubVarAsInt("MUSIC_ALLOWED"));

	if (scene == -1)
		return true;

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
			if (g_nmi->isDemo() && g_nmi->getLanguage() == Common::RU_RUS) {
				_gameLoader->loadScene(SC_9);
				_gameLoader->gotoScene(SC_9, TrubaDown);
			} else {
				_gameLoader->loadScene(SC_1);
				_gameLoader->gotoScene(SC_1, TrubaLeft);
			}
		}
	}

	if (!_currentScene)
		return false;

	return true;
}

GameProject::GameProject() {
	_field_4 = 0;
	_field_10 = 12;
}

bool GameProject::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "GameProject::load()");

	_field_4 = 0;
	_field_10 = 12;

	if (g_nmi->getGameGID() == GID_MDREAM)
		g_nmi->_gameProjectVersion = 1;
	else
		g_nmi->_gameProjectVersion = file.readUint32LE();

	g_nmi->_pictureScale = file.readUint16LE();
	g_nmi->_scrollSpeed = file.readUint32LE();

	_headerFilename = file.readPascalString();

	debugC(1, kDebugLoading, "_gameProjectVersion = %d", g_nmi->_gameProjectVersion);
	debugC(1, kDebugLoading, "_pictureScale = %d", g_nmi->_pictureScale);
	debugC(1, kDebugLoading, "_scrollSpeed = %d", g_nmi->_scrollSpeed);
	debugC(1, kDebugLoading, "_headerFilename = %s", _headerFilename.c_str());

	_sceneTagList.reset(new SceneTagList());

	_sceneTagList->load(file);

	if (g_nmi->_gameProjectVersion >= 3)
		_field_4 = file.readUint32LE();

	if (g_nmi->_gameProjectVersion >= 5) {
		file.readUint32LE();
		file.readUint32LE();
	}

	return true;
}

GameVar::GameVar() {
	_subVars = 0;
	_parentVarObj = 0;
	_nextVarObj = 0;
	_prevVarObj = 0;
	_field_14 = 0;
	_varType = 0;
	_value.floatValue = 0;

	_objtype = kObjTypeGameVar;
}

GameVar::~GameVar() {
	if (_varType == 2)
		free(_value.stringValue);

	if (_parentVarObj && !_prevVarObj ) {
		if (_parentVarObj->_subVars == this) {
			_parentVarObj->_subVars = _nextVarObj;
		} else if (_parentVarObj->_field_14 == this) {
			_parentVarObj->_field_14 = _nextVarObj;
		} else {
			_parentVarObj = 0;
		}
	}

	if (_prevVarObj)
		_prevVarObj->_nextVarObj = _nextVarObj;

	if (_nextVarObj)
		_nextVarObj->_prevVarObj = _prevVarObj;

	_prevVarObj = 0;
	_nextVarObj = 0;

	GameVar *s = _subVars;

	while (s) {
		delete s;
		s = _subVars;
	}

	s = _field_14;

	while (s) {
		delete s;
		s = _field_14;
	}
}

bool GameVar::load(MfcArchive &file) {
	_varName = file.readPascalString();
	_varType = file.readUint32LE();

	debugCN(6, kDebugLoading, "[%03d] ", file.getLevel());
	for (int i = 0; i < file.getLevel(); i++)
		debugCN(6, kDebugLoading, " ");

	debugCN(6, kDebugLoading, "<%s>: ", transCyrillic(_varName));

	switch (_varType) {
	case 0:
		_value.intValue = file.readUint32LE();
		debugC(6, kDebugLoading, "d --> %d", _value.intValue);
		break;
	case 1:
		_value.intValue = file.readUint32LE(); // FIXME
		debugC(6, kDebugLoading, "f --> %f", _value.floatValue);
		break;
	case 2: {
		Common::String str = file.readPascalString();
		_value.stringValue = (char *)calloc(str.size() + 1, 1);
		Common::strlcpy(_value.stringValue, str.c_str(), str.size() + 1);
		debugC(6, kDebugLoading, "s --> %s", _value.stringValue);
		}
		break;
	default:
		error("Unknown var type: %d (0x%x)", _varType, _varType);
	}

	file.incLevel();
	_parentVarObj = file.readClass<GameVar>();
	_prevVarObj = file.readClass<GameVar>();
	_nextVarObj = file.readClass<GameVar>();
	_field_14 = file.readClass<GameVar>();
	_subVars = file.readClass<GameVar>();
	file.decLevel();

	return true;
}

GameVar *GameVar::getSubVarByName(const Common::String &name) {
	GameVar *sv = 0;

	if (_subVars != 0) {
		sv = _subVars;
		for (;sv && scumm_stricmp(sv->_varName.c_str(), name.c_str()); sv = sv->_nextVarObj)
			;
	}
	return sv;
}

bool GameVar::setSubVarAsInt(const Common::String &name, int value) {
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
	var->_varName = name;

	return addSubVar(var);
}

int GameVar::getSubVarAsInt(const Common::String &name) {
	GameVar *var = getSubVarByName(name);

	if (var)
		return var->_value.intValue;
	else
		return 0;
}

GameVar *GameVar::addSubVarAsInt(const Common::String &name, int value) {
	if (getSubVarByName(name)) {
		return 0;
	} else {
		GameVar *var = new GameVar();

		var->_varType = 0;
		var->_value.intValue = value;

		var->_varName = name;

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
	debugC(5, kDebugLoading, "PicAniInfo::load()");

	type = file.readUint32LE();
	objectId = file.readUint16LE();
	field_6 = file.readUint16LE();
	field_8 = file.readUint32LE();
	sceneId = file.readUint16LE();
	field_E = file.readUint16LE();
	ox = file.readSint32LE();
	oy = file.readSint32LE();
	priority = file.readUint32LE();
	staticsId = file.readUint16LE();
	movementId = file.readUint16LE();
	dynamicPhaseIndex = file.readUint16LE();
	flags = file.readUint16LE();
	field_24 = file.readUint32LE();
	someDynamicPhaseIndex = file.readUint32LE();

	return true;
}

} // End of namespace NGI
