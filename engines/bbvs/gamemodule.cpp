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

#include "bbvs/gamemodule.h"
#include "engines/util.h"

namespace Bbvs {

GameModule::GameModule()
	: _bgSpriteCount(0), _bgSpriteIndices(0), _bgSpritePriorities(0), _walkRectsCount(0),
	_walkRects(0), _sceneExitsCount(0), _sceneExits(0), _bgObjectsCount(0), _bgObjects(0),
	_animationsCount(0), _animations(0), _sceneObjectDefsCount(0), _sceneObjectDefs(0),
	_sceneObjectInitsCount(0), _sceneObjectInits(0), _actionsCount(0), _actions(0),
	_sceneSoundsCount(0), _sceneSounds(0), _preloadSoundsCount(0), _preloadSounds(0) {
}

GameModule::~GameModule() {
	unload();
}

void GameModule::load(const char *filename) {
	debug(0, "GameModule::load()");

	unload();

	Common::File fd;

	if (!fd.open(filename))
		error("GameModule::load() Could not open %s", filename);

	loadBgSprites(fd);
	loadCameraInits(fd);
	loadWalkRects(fd);
	loadSceneExits(fd);
	loadBgObjects(fd);
	loadAnimations(fd);
	loadSceneObjectDefs(fd);
	loadSceneObjectInits(fd);
	loadActions(fd);
	loadGuiSpriteIndices(fd);
	loadInventoryItemSpriteIndices(fd);
	loadInventoryItemInfos(fd);
	loadDialogItemSpriteIndices(fd);
	loadSceneSounds(fd);
	loadPreloadSounds(fd);

	fd.seek(0xC);
	_fieldC = fd.readUint32LE();

	fd.seek(0x1A8);
	_buttheadObjectIndex = fd.readUint32LE();

	fd.close();

	debug(0, "GameModule::load() OK");
}

int GameModule::getFieldC() {
	return _fieldC;
}

int GameModule::getButtheadObjectIndex() {
	return _buttheadObjectIndex;
}

int GameModule::getGuiSpriteIndex(int index) {
	assert(index < kGuiSpriteCount);
	return _guiSpriteIndices[index];
}

int GameModule::getInventoryItemSpriteIndex(int index) {
	assert(index < kInventoryItemSpriteCount);
	return _inventoryItemSpriteIndices[index];
}

int GameModule::getDialogItemSpriteIndex(int index) {
	assert(index < kDialogItemSpriteCount);
	return _dialogItemSpriteIndices[index];
}

int GameModule::getActionsCount() {
	return _actionsCount;
}

Action *GameModule::getAction(int index) {
	assert(index < _actionsCount);
	return &_actions[index];
}

InventoryItemInfo *GameModule::getInventoryItemInfo(int index) {
	assert(index < kInventoryItemCount);
	return &_inventoryItemInfos[index];
}

CameraInit *GameModule::getCameraInit(int cameraNum) {
	assert(cameraNum < kCameraInitsCount);
	return &_cameraInits[cameraNum];
}

int GameModule::getSceneExitsCount() {
	return _sceneExitsCount;
}

SceneExit *GameModule::getSceneExit(int index) {
	assert(index < _sceneExitsCount);
	return &_sceneExits[index];
}

int GameModule::getWalkRectsCount() {
	return _walkRectsCount;
}

Common::Rect *GameModule::getWalkRects() {
	return _walkRects;
}

int GameModule::getSceneObjectDefsCount() {
	return _sceneObjectDefsCount;
}

SceneObjectDef *GameModule::getSceneObjectDef(int index) {
	assert(index < _sceneObjectDefsCount);
	return &_sceneObjectDefs[index];
}

int GameModule::getSceneObjectInitsCount() {
	return _sceneObjectInitsCount;
}

SceneObjectInit *GameModule::getSceneObjectInit(int index) {
	assert(index < _sceneObjectInitsCount);
	return &_sceneObjectInits[index];
}

int GameModule::getBgObjectsCount() {
	return _bgObjectsCount;
}

BgObject *GameModule::getBgObject(int index) {
	assert(index < _bgObjectsCount);
	return &_bgObjects[index];
}

int GameModule::getBgSpritesCount() {
	return _bgSpriteCount;
}

int GameModule::getBgSpriteIndex(int index) {
	assert(index < _bgSpriteCount);
	return _bgSpriteIndices[index];
}

int GameModule::getBgSpritePriority(int index) {
	assert(index < _bgSpriteCount);
	return _bgSpritePriorities[index];
}

int GameModule::getSceneSoundsCount() {
	return _sceneSoundsCount;
}

SceneSound *GameModule::getSceneSound(int index) {
	assert(index < _sceneSoundsCount);
	return &_sceneSounds[index];
}

uint GameModule::getSceneSoundIndex(uint soundNum) {
	for (int i = 0; i < getSceneSoundsCount(); ++i)
		if (getSceneSound(i)->soundNum == soundNum)
			return i;
	return 0;
}

uint GameModule::getPreloadSoundsCount() {
	return _preloadSoundsCount;
}

uint GameModule::getPreloadSound(uint index) {
	assert(index < _preloadSoundsCount);
	return _preloadSounds[index];
}

Animation *GameModule::getAnimation(int index) {
	assert(index < _animationsCount);
	return &_animations[index];
}

Common::Point GameModule::readPoint(Common::SeekableReadStream &s) {
	Common::Point p;
	p.x = s.readUint16LE();
	p.y = s.readUint16LE();
	return p;
}

Common::Rect GameModule::readRect(Common::SeekableReadStream &s) {
	Common::Rect r;
	r.left = s.readUint16LE();
	r.top = s.readUint16LE();
	r.setWidth(s.readUint16LE());
	r.setHeight(s.readUint16LE());
	return r;
}

Conditions GameModule::readConditions(Common::SeekableReadStream &s) {
	Conditions c;
	for (int i = 0; i < 8; ++i) {
		c.conditions[i].cond = s.readByte();
		c.conditions[i].value1 = s.readByte();
		c.conditions[i].value2 = s.readUint16LE();
	}
	return c;
}

void GameModule::unload() {
	delete[] _bgSpriteIndices;
	delete[] _bgSpritePriorities;
	delete[] _walkRects;
	delete[] _sceneExits;
	delete[] _bgObjects;
	delete[] _animations;
	delete[] _sceneObjectDefs;
	delete[] _sceneObjectInits;
	delete[] _actions;
	delete[] _sceneSounds;
	delete[] _preloadSounds;
	_bgSpriteIndices = 0;
	_bgSpritePriorities = 0;
	_walkRects = 0;
	_sceneExits = 0;
	_bgObjects = 0;
	_animations = 0;
	_sceneObjectDefs = 0;
	_sceneObjectInits = 0;
	_actions = 0;
	_sceneSounds = 0;
	_preloadSounds = 0;
}

void GameModule::loadBgSprites(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadBgSprites()");

	s.seek(0x14);
	_bgSpriteCount = s.readUint32LE();
	uint32 bgSpriteIndicesOffs = s.readUint32LE();
	uint32 bgSpritePrioritiesOffs = s.readUint32LE();
	_bgSpriteIndices = new int[_bgSpriteCount];
	_bgSpritePriorities = new int16[_bgSpriteCount];
	s.seek(bgSpriteIndicesOffs);
	for (int i = 0; i < _bgSpriteCount; ++i)
		_bgSpriteIndices[i] = s.readUint32LE();
	s.seek(bgSpritePrioritiesOffs);
	for (int i = 0; i < _bgSpriteCount; ++i)
		_bgSpritePriorities[i] = s.readUint16LE();

}

void GameModule::loadCameraInits(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadCameraInits()");

	s.seek(0x20);
	for (int i = 0; i < kCameraInitsCount; ++i) {
		CameraInit &cameraInit = _cameraInits[i];
		cameraInit.cameraPos = readPoint(s);
		for (int j = 0; j < 8; ++j)
			cameraInit.cameraLinks[j] = s.readByte();
		for (int j = 0; j < 8; ++j)
			cameraInit.rects[j] = readRect(s);
	}
}

void GameModule::loadWalkRects(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadWalkRects()");

	s.seek(0x150);
	_walkRectsCount = s.readUint32LE();
	uint32 offs = s.readUint32LE();
	_walkRects = new Common::Rect[_walkRectsCount];
	s.seek(offs);
	for (int i = 0; i < _walkRectsCount; ++i)
		_walkRects[i] = readRect(s);
}

void GameModule::loadSceneExits(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadSceneExits()");

	s.seek(0x158);
	_sceneExitsCount = s.readUint32LE();
	uint32 offs = s.readUint32LE();
	_sceneExits = new SceneExit[_sceneExitsCount];
	s.seek(offs);
	for (int i = 0; i < _sceneExitsCount; ++i) {
		_sceneExits[i].rect = readRect(s);
		_sceneExits[i].newModuleNum = s.readUint32LE();
	}
}

void GameModule::loadBgObjects(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadBgObjects()");

	s.seek(0x160);
	_bgObjectsCount = s.readUint32LE();
	uint32 offs = s.readUint32LE();
	_bgObjects = new BgObject[_bgObjectsCount];
	s.seek(offs);
	for (int i = 0; i < _bgObjectsCount; ++i) {
		s.read(_bgObjects[i].name, 20);
		_bgObjects[i].rect = readRect(s);
	}
}

void GameModule::loadAnimations(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadAnimations()");

	s.seek(0x168);
	_animationsCount = s.readUint32LE();
	uint32 offs = s.readUint32LE();
	_animations = new Animation[_animationsCount];
	for (int i = 0; i < _animationsCount; ++i) {
		Animation &anim = _animations[i];
		s.seek(offs + i * 20);
		anim.frameCount = s.readUint32LE();
		uint32 frameSpriteIndicesOffs = s.readUint32LE();
		uint32 frameTicksOffs = s.readUint32LE();
		uint32 frameRects1Offs = s.readUint32LE();
		uint32 frameRects2Offs = s.readUint32LE();
		anim.frameSpriteIndices = new int[anim.frameCount];
		s.seek(frameSpriteIndicesOffs);
		for (int j = 0; j < anim.frameCount; ++j)
			anim.frameSpriteIndices[j] = s.readUint32LE();
		anim.frameTicks = new int16[anim.frameCount];
		s.seek(frameTicksOffs);
		for (int j = 0; j < anim.frameCount; ++j)
			anim.frameTicks[j] = s.readUint16LE();
		anim.frameRects1 = new Common::Rect[anim.frameCount];
		s.seek(frameRects1Offs);
		for (int j = 0; j < anim.frameCount; ++j)
			anim.frameRects1[j] = readRect(s);
		anim.frameRects2 = new Common::Rect[anim.frameCount];
		s.seek(frameRects2Offs);
		for (int j = 0; j < anim.frameCount; ++j)
			anim.frameRects2[j] = readRect(s);
	}
}

void GameModule::loadSceneObjectDefs(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadSceneObjectDefs()");

	s.seek(0x170);
	_sceneObjectDefsCount = s.readUint32LE();
	uint32 offs = s.readUint32LE();
	_sceneObjectDefs = new SceneObjectDef[_sceneObjectDefsCount];
	s.seek(offs);
	for (int i = 0; i < _sceneObjectDefsCount; ++i) {
		s.read(_sceneObjectDefs[i].name, 20);
		_sceneObjectDefs[i].walkSpeed = s.readUint32LE();
		for (int j = 0; j < 16; ++j)
			_sceneObjectDefs[i].animIndices[j] = s.readUint32LE();
	}
}

void GameModule::loadSceneObjectInits(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadSceneObjectInits()");

	s.seek(0x178);
	_sceneObjectInitsCount = s.readUint32LE();
	uint32 offs = s.readUint32LE();
	_sceneObjectInits = new SceneObjectInit[_sceneObjectInitsCount];
	s.seek(offs);
	for (int i = 0; i < _sceneObjectInitsCount; ++i) {
		_sceneObjectInits[i].conditions = readConditions(s);
		_sceneObjectInits[i].sceneObjectIndex = s.readUint32LE();
		_sceneObjectInits[i].animIndex = s.readUint32LE();
		_sceneObjectInits[i].x = s.readUint16LE();
		_sceneObjectInits[i].y = s.readUint16LE();
	}
}

void GameModule::loadActions(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadActions()");

	s.seek(0x180);
	_actionsCount = s.readUint32LE();
	uint32 offs = s.readUint32LE();
	_actions = new Action[_actionsCount];
	for (int i = 0; i < _actionsCount; ++i) {
		s.seek(offs + i * 72);
		debug(0, "Action(%d) offs: %08X", i, offs + i * 72);
		_actions[i].conditions = readConditions(s);
		for (int j = 0; j < 8; ++j) {
			_actions[i].results.actionResults[j].kind = s.readByte();
			_actions[i].results.actionResults[j].value1 = s.readByte();
			_actions[i].results.actionResults[j].value2 = s.readUint16LE();
		}
		const int actionListCount = s.readUint32LE();
		const uint32 actionListOffs = s.readUint32LE();
		s.seek(actionListOffs);
		for (int j = 0; j < actionListCount; ++j) {
			ActionCommand actionCommand;
			actionCommand.cmd = s.readUint16LE();
			actionCommand.sceneObjectIndex = s.readUint16LE();
			actionCommand.timeStamp = s.readUint32LE();
			actionCommand.walkDest = readPoint(s);
			actionCommand.param = s.readUint32LE();
			_actions[i].actionCommands.push_back(actionCommand);
		}
	}
}

void GameModule::loadGuiSpriteIndices(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadGuiSpriteIndices()");

	s.seek(0x188);
	uint32 offs = s.readUint32LE();
	s.seek(offs);
	for (int i = 0; i < kGuiSpriteCount; ++i)
		_guiSpriteIndices[i] = s.readUint32LE();
}

void GameModule::loadInventoryItemSpriteIndices(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadInventoryItemSpriteIndices()");

	s.seek(0x18C);
	uint32 offs = s.readUint32LE();
	s.seek(offs);
	for (int i = 0; i < kInventoryItemSpriteCount; ++i)
		_inventoryItemSpriteIndices[i] = s.readUint32LE();
}

void GameModule::loadInventoryItemInfos(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadInventoryItemInfos()");

	s.seek(0x190);
	uint32 offs = s.readUint32LE();
	s.seek(offs);
	for (int i = 0; i < kInventoryItemCount; ++i) {
		_inventoryItemInfos[i].xOffs = s.readUint16LE();
		_inventoryItemInfos[i].yOffs = s.readUint16LE();
		_inventoryItemInfos[i].width = s.readUint16LE();
		_inventoryItemInfos[i].height = s.readUint16LE();
		s.skip(8); // Unused
	}
}

void GameModule::loadDialogItemSpriteIndices(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadDialogItemSpriteIndices()");

	s.seek(0x194);
	uint32 offs = s.readUint32LE();
	s.seek(offs);
	for (int i = 0; i < kDialogItemSpriteCount; ++i) {
		_dialogItemSpriteIndices[i] = s.readUint32LE();
	}
}

void GameModule::loadSceneSounds(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadSceneSounds()");

	s.seek(0x1A0);
	_sceneSoundsCount = s.readUint32LE();
	uint32 offs = s.readUint32LE();
	_sceneSounds = new SceneSound[_sceneSoundsCount];
	s.seek(offs);
	for (int i = 0; i < _sceneSoundsCount; ++i) {
		_sceneSounds[i].conditions = readConditions(s);
		_sceneSounds[i].soundNum = s.readUint32LE();
	}
}

void GameModule::loadPreloadSounds(Common::SeekableReadStream &s) {
	debug(0, "GameModule::loadPreloadSounds()");

	s.seek(0x198);
	_preloadSoundsCount = s.readUint32LE();
	uint32 offs = s.readUint32LE();
	_preloadSounds = new uint[_preloadSoundsCount];
	s.seek(offs);
	for (uint i = 0; i < _preloadSoundsCount; ++i)
		_preloadSounds[i] = s.readUint32LE();
}

} // End of namespace Bbvs
