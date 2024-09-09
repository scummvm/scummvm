/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "common/file.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "gui/saveload.h"

#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "wage/wage.h"
#include "wage/world.h"
#include "wage/entities.h"
#include "wage/gui.h"

#define SAVEGAME_CURRENT_VERSION 1

//
// Original saves format is supported.
// ScummVM adds flags, description and thumbnail
// in the end of the file (shouldn't make saves incompatible).
//
// Version 0 (original/ScummVM):  first ScummVM version
//

namespace Wage {

//TODO: make sure these are calculated right: (we add flag, description, etc)
#define VARS_INDEX 0x005E
#define SCENES_INDEX 0x0232

#define SCENE_SIZE 0x0010
#define CHR_SIZE 0x0016
#define OBJ_SIZE 0x0010

#define GET_HEX_OFFSET(ptr, baseOffset, entrySize) ((ptr) == nullptr ? -1 : ((baseOffset) + (entrySize) * (ptr)->_index))
#define GET_HEX_CHR_OFFSET(ptr) GET_HEX_OFFSET((ptr), chrsHexOffset, CHR_SIZE)
#define GET_HEX_OBJ_OFFSET(ptr) GET_HEX_OFFSET((ptr), objsHexOffset, OBJ_SIZE)
#define GET_HEX_SCENE_OFFSET(ptr) ((ptr) == nullptr ? -1 : \
	((ptr) == _world->_storageScene ? 0 : (SCENES_INDEX + getSceneIndex(_world->_player->_currentScene) * SCENE_SIZE)))

int WageEngine::getSceneIndex(Scene *scene) const {
	assert(scene);
	Common::Array<Scene *> &orderedScenes = _world->_orderedScenes;
	for (uint32 i = 0; i < orderedScenes.size(); ++i) {
		if (orderedScenes[i] == scene) return i-1;
	}

	warning("Scene's index not found");
	return -1;
}

Obj *WageEngine::getObjByOffset(int offset, int objBaseOffset) const {
	int objIndex = -1;

	if (offset != 0xFFFF) {
		objIndex = (offset - objBaseOffset) / CHR_SIZE;
	}

	if (objIndex >= 0 && (uint)objIndex < _world->_orderedObjs.size()) {
		return _world->_orderedObjs[objIndex];
	}

	return nullptr;
}

Chr *WageEngine::getChrById(int resId) const {
	Common::Array<Chr *> &orderedChrs = _world->_orderedChrs;
	for (uint32 i = 0; i < orderedChrs.size(); ++i) {
		if (orderedChrs[i]->_resourceId == resId)
			return orderedChrs[i];
	}

	return nullptr;
}

Chr *WageEngine::getChrByOffset(int offset, int chrBaseOffset) const {
	int chrIndex = -1;

	if (offset != 0xFFFF) {
		chrIndex = (offset - chrBaseOffset) / CHR_SIZE;
	}

	if (chrIndex >= 0 && (uint)chrIndex < _world->_orderedChrs.size()) {
		return _world->_orderedChrs[chrIndex];
	}

	return nullptr;
}

Scene *WageEngine::getSceneById(int resId) const {
	Common::Array<Scene *> &orderedScenes = _world->_orderedScenes;
	for (uint32 i = 0; i < orderedScenes.size(); ++i) {
		if (orderedScenes[i]->_resourceId == resId)
			return orderedScenes[i];
	}

	return nullptr;
}

Scene *WageEngine::getSceneByOffset(int offset) const {
	int sceneIndex = -1;

	if (offset != 0xFFFF) {
		if (offset == 0)
			sceneIndex = 0;
		else
			sceneIndex = 1 + (offset - SCENES_INDEX) / SCENE_SIZE;
	}

	if (sceneIndex >= 0 && (uint)sceneIndex < _world->_orderedScenes.size()) {
		if (sceneIndex == 0) return _world->_storageScene;
		return _world->_orderedScenes[sceneIndex];
	}

	return nullptr;
}

int WageEngine::saveGame(const Common::String &fileName, const Common::String &descriptionString) {
	Common::OutSaveFile *out;
	int result = 0;

	debug(9, "WageEngine::saveGame(%s, %s)", fileName.c_str(), descriptionString.c_str());
	if (!(out = _saveFileMan->openForSaving(fileName))) {
		warning("Can't create file '%s', game not saved", fileName.c_str());
		return -1;
	} else {
		debug(9, "Successfully opened %s for writing", fileName.c_str());
	}

	// Counters
	out->writeSint16LE(_world->_scenes.size()); //numScenes
	out->writeSint16LE(_world->_chrs.size()); //numChars
	out->writeSint16LE(_world->_objs.size()); //numObjs

	// Hex Offsets
	int chrsHexOffset = SCENES_INDEX + _world->_scenes.size() * SCENE_SIZE; //chrs start after scenes
	int objsHexOffset = chrsHexOffset + _world->_chrs.size() * CHR_SIZE; //objs start after chrs
	out->writeSint32LE(chrsHexOffset);
	out->writeSint32LE(objsHexOffset);

	// Unique 8-byte World Signature
	out->writeSint32LE(_world->_signature); //8-byte ints? seriously? (uses 4 bytes in java code too)

	Chr *player = _world->_player;
	Context &playerContext = player->_context;

	// More Counters
	out->writeSint32LE(playerContext._visits); //visitNum
	out->writeSint32LE(_loopCount); //loopNum
	out->writeSint32LE(playerContext._kills); //killNum

	// Hex offset to player character
	out->writeSint32LE(GET_HEX_CHR_OFFSET(player)); //getPlayerHexOffset() == getHexOffsetForChr(player)

	// character in this scene?
	out->writeSint32LE(GET_HEX_CHR_OFFSET(_monster)); //getPresCharHexOffset() == getHexOffsetForChr(monster)

	// Hex offset to current scene
	out->writeSint32LE(GET_HEX_SCENE_OFFSET(player->_currentScene)); //getCurSceneHexOffset()

	// wearing a helmet?
	out->writeSint32LE(GET_HEX_OBJ_OFFSET(player->_armor[Chr::HEAD_ARMOR])); //helmetIndex

	// holding a shield?
	out->writeSint32LE(GET_HEX_OBJ_OFFSET(player->_armor[Chr::SHIELD_ARMOR])); //shieldIndex

	// wearing chest armor?
	out->writeSint32LE(GET_HEX_OBJ_OFFSET(player->_armor[Chr::BODY_ARMOR])); //chestArmIndex

	// wearing spiritual armor?
	out->writeSint32LE(GET_HEX_OBJ_OFFSET(player->_armor[Chr::MAGIC_ARMOR])); //sprtArmIndex

	// TODO:
	out->writeUint16LE(0xffff);	// ???? - always FFFF
	out->writeUint16LE(0xffff);	// ???? - always FFFF
	out->writeUint16LE(0xffff);	// ???? - always FFFF
	out->writeUint16LE(0xffff);	// ???? - always FFFF

	// did a character just escape?
	out->writeSint32LE(GET_HEX_CHR_OFFSET(_running)); //getRunCharHexOffset() == getHexOffsetForChr(running)

	// players experience points
	out->writeSint32LE(playerContext._experience);

	out->writeSint16LE(_aim); //aim
	out->writeSint16LE(_opponentAim); //opponentAim

	// TODO:
	out->writeSint16LE(0x0000);	// always 0
	out->writeSint16LE(0x0000);	// always 0
	out->writeSint16LE(0x0000);	// always 0

	// Base character stats
	out->writeByte(playerContext._statVariables[PHYS_STR_BAS]); //state.getBasePhysStr()
	out->writeByte(playerContext._statVariables[PHYS_HIT_BAS]); //state.getBasePhysHp()
	out->writeByte(playerContext._statVariables[PHYS_ARM_BAS]); //state.getBasePhysArm()
	out->writeByte(playerContext._statVariables[PHYS_ACC_BAS]); //state.getBasePhysAcc()
	out->writeByte(playerContext._statVariables[SPIR_STR_BAS]); //state.getBaseSprtStr()
	out->writeByte(playerContext._statVariables[SPIR_HIT_BAS]); //state.getBaseSprtHp()
	out->writeByte(playerContext._statVariables[SPIR_ARM_BAS]); //state.getBaseSprtArm()
	out->writeByte(playerContext._statVariables[SPIR_ACC_BAS]); //state.getBaseSprtAcc()
	out->writeByte(playerContext._statVariables[PHYS_SPE_BAS]); //state.getBaseRunSpeed()

	// TODO:
	out->writeByte(0x0A);		// ???? - always seems to be 0x0A

	// write user vars
	for (uint32 i = 0; i < 26 * 9; ++i)
		out->writeSint16LE(playerContext._userVariables[i]);

	// write updated info for all scenes
	Common::Array<Scene *> &orderedScenes = _world->_orderedScenes;
	for (uint32 i = 0; i < orderedScenes.size(); ++i) {
		Scene *scene = orderedScenes[i];
		if (scene != _world->_storageScene) {
			out->writeSint16LE(scene->_resourceId);
			out->writeSint16LE(scene->_worldY);
			out->writeSint16LE(scene->_worldX);
			out->writeByte(scene->_blocked[NORTH] ? 0x01 : 0x00);
			out->writeByte(scene->_blocked[SOUTH] ? 0x01 : 0x00);
			out->writeByte(scene->_blocked[EAST] ? 0x01 : 0x00);
			out->writeByte(scene->_blocked[WEST] ? 0x01 : 0x00);
			out->writeSint16LE(scene->_soundFrequency);
			out->writeByte(scene->_soundType);
			// the following two bytes are currently unknown
			out->writeByte(0);
			out->writeByte(0);
			out->writeByte(scene->_visited ? 0x01 : 0x00);
		}
	}

	// write updated info for all characters
	Common::Array<Chr *> &orderedChrs = _world->_orderedChrs;
	for (uint32 i = 0; i < orderedChrs.size(); ++i) {
		Chr *chr = orderedChrs[i];
		out->writeSint16LE(chr->_resourceId);
		out->writeSint16LE(chr->_currentScene->_resourceId);
		Context &chrContext = chr->_context;
		out->writeByte(chrContext._statVariables[PHYS_STR_CUR]);
		out->writeByte(chrContext._statVariables[PHYS_HIT_CUR]);
		out->writeByte(chrContext._statVariables[PHYS_ARM_CUR]);
		out->writeByte(chrContext._statVariables[PHYS_ACC_CUR]);
		out->writeByte(chrContext._statVariables[SPIR_STR_CUR]);
		out->writeByte(chrContext._statVariables[SPIR_HIT_CUR]);
		out->writeByte(chrContext._statVariables[SPIR_ARM_CUR]);
		out->writeByte(chrContext._statVariables[SPIR_ACC_CUR]);
		out->writeByte(chrContext._statVariables[PHYS_SPE_CUR]);
		out->writeByte(chr->_rejectsOffers);
		out->writeByte(chr->_followsOpponent);
		// bytes 16-20 are unknown
		out->writeByte(0);
		out->writeByte(0);
		out->writeByte(0);
		out->writeByte(0);
		out->writeByte(0);
		out->writeByte(chr->_weaponDamage1);
		out->writeByte(chr->_weaponDamage2);
	}

	// write updated info for all objects
	Common::Array<Obj *> &orderedObjs = _world->_orderedObjs;
	for (uint32 i = 0; i < orderedObjs.size(); ++i) {
		Obj *obj = orderedObjs[i];
		Scene *location = obj->_currentScene;
		Chr *owner = obj->_currentOwner;

		out->writeSint16LE(obj->_resourceId);
		out->writeSint16LE(location == nullptr ? 0 : location->_resourceId);
		out->writeSint16LE(owner == nullptr ? 0 : owner->_resourceId);

		// bytes 7-9 are unknown (always = 0)
		out->writeByte(0);
		out->writeByte(0);
		out->writeByte(0);

		out->writeByte(obj->_accuracy);
		out->writeByte(obj->_value);
		out->writeByte(obj->_type);
		out->writeByte(obj->_damage);
		out->writeByte(obj->_attackType);
		out->writeSint16LE(obj->_numberOfUses);
	}

	// the following is appended by ScummVM
	g_engine->getMetaEngine()->appendExtendedSave(out, g_engine->getTotalPlayTime(), descriptionString, fileName.contains("auto"));

	out->finalize();
	if (out->err()) {
		warning("Can't write file '%s'. (Disk full?)", fileName.c_str());
		result = -1;
	} else
		debug(9, "Saved game %s in file %s", descriptionString.c_str(), fileName.c_str());

	delete out;
	return result;
}

int WageEngine::loadGame(int slotId) {
	Common::InSaveFile *data;
	Common::String fileName = getSaveStateName(slotId);

	debug(9, "WageEngine::loadGame(%d)", slotId);
	if (!(data = _saveFileMan->openForLoading(fileName))) {
		warning("Can't open file '%s', game not loaded", fileName.c_str());
		return -1;
	} else {
		debug(9, "Successfully opened %s for reading", fileName.c_str());
	}

	// Counters
	int numScenes = data->readSint16LE();
	int numChars = data->readSint16LE();
	int numObjs = data->readSint16LE();

	// Hex Offsets
	int chrsHexOffset = data->readSint32LE();
	int objsHexOffset = data->readSint32LE();

	// Unique 8-byte World Signature
	int signature = data->readSint32LE();
	if (_world->_signature != signature) {
		warning("This saved game is for a different world, please select another one");
		warning("World signature = %d, save signature = %d", _world->_signature, signature);
		delete data;
		return -1;
	}

	// More Counters
	int visitNum = data->readSint32LE(); //visitNum
	int loopNum = data->readSint32LE(); //loopNum
	int killNum = data->readSint32LE(); //killNum

	// Hex offset to player character
	int playerOffset = data->readSint32LE();

	// character in this scene?
	int presCharOffset = data->readSint32LE();

	// Hex offset to current scene
	int currentSceneOffset = data->readSint32LE();

	// find player and current scene
	Chr *player = getChrByOffset(playerOffset, chrsHexOffset);
	if (player == nullptr) {
		warning("Invalid Character!  Aborting load.");
		delete data;
		return -1;
	}

	Scene *currentScene = getSceneByOffset(currentSceneOffset);
	if (currentScene == nullptr) {
		warning("Invalid Scene!  Aborting load.");
		delete data;
		return -1;
	}

	// set player character
	_world->_player = player;

	// set current scene
	player->_currentScene = currentScene;

	// clear the players inventory list
	player->_inventory.clear();

	// wearing a helmet?
	int helmetOffset = data->readSint32LE(); //helmetIndex

	// holding a shield?
	int shieldOffset = data->readSint32LE(); //shieldIndex

	// wearing chest armor?
	int armorOffset = data->readSint32LE(); //chestArmIndex

	// wearing spiritual armor?
	int spiritualArmorOffset = data->readSint32LE(); //sprtArmIndex

	data->readSint16LE();	// FFFF
	data->readSint16LE();	// FFFF
	data->readSint16LE();	// FFFF
	data->readSint16LE();	// FFFF

	/* int runCharOffset = */ data->readSint32LE();

	// players experience points
	int exp = data->readSint32LE(); // @ playerContext._experience

	int aim = data->readSint16LE(); //aim
	int opponentAim = data->readSint16LE(); //opponentAim

	data->readSint16LE(); // 0000
	data->readSint16LE(); // 0000
	data->readSint16LE(); // 0000

	// Base character stats
	int basePhysStr = data->readByte();
	int basePhysHp = data->readByte();
	int basePhysArm = data->readByte();
	int basePhysAcc = data->readByte();
	int baseSprtStr = data->readByte();
	int baseSprtHp = data->readByte();
	int baseSprtArm = data->readByte();
	int baseSprtAcc = data->readByte();
	int baseRunSpeed = data->readByte();

	// set player stats
	Context &playerContext = player->_context;
	// I'm setting player fields also, because those are used as base values in Chr::resetState()
	playerContext._statVariables[PHYS_STR_BAS] = player->_physicalStrength = basePhysStr;
	playerContext._statVariables[PHYS_HIT_BAS] = player->_physicalHp = basePhysHp;
	playerContext._statVariables[PHYS_ARM_BAS] = player->_naturalArmor = basePhysArm;
	playerContext._statVariables[PHYS_ACC_BAS] = player->_physicalAccuracy = basePhysAcc;
	playerContext._statVariables[SPIR_STR_BAS] = player->_spiritualStength = baseSprtStr;
	playerContext._statVariables[SPIR_HIT_BAS] = player->_spiritialHp = baseSprtHp;
	playerContext._statVariables[SPIR_ARM_BAS] = player->_resistanceToMagic = baseSprtArm;
	playerContext._statVariables[SPIR_ACC_BAS] = player->_spiritualAccuracy = baseSprtAcc;
	playerContext._statVariables[PHYS_SPE_BAS] = player->_runningSpeed = baseRunSpeed;

	// set visit#
	playerContext._visits = visitNum;

	// set monsters killed
	playerContext._kills = killNum;

	// set experience
	playerContext._experience = exp;

	// if a character is present, move it to this scene
	// TODO: This is done in the engine object, would it be cleaner
	// to move it here?
	// well, it's actually down there now, now sure if that's "cleaner"
	// when it's up there or down there

	// if a character just ran away, let our engine know
	// TODO: The current engine doesn't have a case for this, we
	// should update it
	// yep, I don't see such code anywhere in java, so not added it here

	data->readByte(); // 0x0A?

	// set all user variables
	for (uint32 i = 0; i < 26 * 9; ++i) {
		playerContext._userVariables[i] = data->readSint16LE();
	}

	// update all scene stats
	Common::Array<Scene *> &orderedScenes = _world->_orderedScenes;
	if ((uint)numScenes != orderedScenes.size()) {
		warning("scenes number in file (%d) differs from the one in world (%d)", numScenes, orderedScenes.size());
	}
	for (uint32 i = 0; i < orderedScenes.size(); ++i) {
		Scene *scene = orderedScenes[i];
		if (scene == _world->_storageScene) {
			scene->_chrs.clear();
			scene->_objs.clear();
		} else {
			int id = data->readSint16LE();

			if (scene->_resourceId != id) {
				warning("loadGame(): updating scenes: expected %d but got %d", scene->_resourceId, id);
				data->skip(14); //2,2,1,1,1,1,2,1,1,1,1 down there
				continue;
			}

			scene->_worldY = data->readSint16LE();
			scene->_worldX = data->readSint16LE();
			scene->_blocked[NORTH] = data->readByte() != 0;
			scene->_blocked[SOUTH] = data->readByte() != 0;
			scene->_blocked[EAST] = data->readByte() != 0;
			scene->_blocked[WEST] = data->readByte() != 0;
			scene->_soundFrequency = data->readSint16LE();
			scene->_soundType = data->readByte();
			// the following two bytes are currently unknown
			data->readByte();
			data->readByte();
			scene->_visited = data->readByte() != 0;
		}
	}

	// update all char locations and stats
	Common::Array<Chr *> &orderedChrs = _world->_orderedChrs;
	if ((uint)numChars != orderedChrs.size()) {
		warning("characters number in file (%d) differs from the one in world (%d)", numChars, orderedChrs.size());
	}
	for (uint32 i = 0; i < orderedChrs.size(); ++i) {
		int resourceId = data->readSint16LE();
		int sceneResourceId = data->readSint16LE();

		int strength = data->readByte();
		int hp = data->readByte();
		int armor = data->readByte();
		int accuracy = data->readByte();
		int spirStrength = data->readByte();
		int spirHp = data->readByte();
		int spirArmor = data->readByte();
		int spirAccuracy = data->readByte();
		int speed = data->readByte();
		int rejectsOffers = data->readByte();
		int followsOpponent = data->readByte();

		// bytes 16-20 are unknown
		data->readByte();
		data->readByte();
		data->readByte();
		data->readByte();
		data->readByte();

		int weaponDamage1 = data->readByte();
		int weaponDamage2 = data->readByte();

		Chr *chr = orderedChrs[i];
		if (chr->_resourceId != resourceId) {
			warning("loadGame(): updating chrs: expected %d but got %d", chr->_resourceId, resourceId);
			continue;
		}

		chr->_currentScene = getSceneById(sceneResourceId);
		Context &chrContext = chr->_context;
		chrContext._statVariables[PHYS_STR_CUR] = strength;
		chrContext._statVariables[PHYS_HIT_CUR] = hp;
		chrContext._statVariables[PHYS_ARM_CUR] = armor;
		chrContext._statVariables[PHYS_ACC_CUR] = accuracy;
		chrContext._statVariables[SPIR_STR_CUR] = spirStrength;
		chrContext._statVariables[SPIR_HIT_CUR] = spirHp;
		chrContext._statVariables[SPIR_ARM_CUR] = spirArmor;
		chrContext._statVariables[SPIR_ACC_CUR] = spirAccuracy;
		chrContext._statVariables[PHYS_SPE_CUR] = speed;
		chr->_rejectsOffers = rejectsOffers;
		chr->_followsOpponent = followsOpponent;
		chr->_weaponDamage1 = weaponDamage1;
		chr->_weaponDamage2 = weaponDamage2;
	}

	// update all object locations and stats
	Common::Array<Obj *> &orderedObjs = _world->_orderedObjs;
	if ((uint)numObjs != orderedObjs.size()) {
		warning("objects number in file (%d) differs from the one in world (%d)", numObjs, orderedObjs.size());
	}
	for (uint32 i = 0; i < orderedObjs.size(); ++i) {
		int resourceId = data->readSint16LE();
		int locationResourceId = data->readSint16LE();
		int ownerResourceId = data->readSint16LE();

		// bytes 7-9 are unknown (always = 0)
		data->readByte();
		data->readByte();
		data->readByte();

		int accuracy = data->readByte();
		int value = data->readByte();
		int type = data->readByte();
		int damage = data->readByte();
		int attackType= data->readByte();
		int numberOfUses = data->readSint16LE();

		Obj *obj = orderedObjs[i];
		if (obj->_resourceId != resourceId) {
			warning("loadGame(): updating objs: expected %d but got %d", obj->_resourceId, resourceId);
			continue;
		}

		if (ownerResourceId != 0) {
			obj->setCurrentOwner(getChrById(ownerResourceId));
			if (obj->_currentOwner == nullptr)
				warning("loadGame(): updating objs: owner not found - char with id %d", ownerResourceId);
		} else {
			obj->setCurrentScene(getSceneById(locationResourceId));
			if (obj->_currentScene == nullptr)
				warning("loadGame(): updating objs: scene with id %d not found", ownerResourceId);
		}

		obj->_accuracy = accuracy;
		obj->_value = value;
		obj->_type = type;
		obj->_damage = damage;
		obj->_attackType = attackType;
		obj->_numberOfUses = numberOfUses;
	}

	// update inventories and scene contents
	for (uint32 i = 0; i < orderedObjs.size(); ++i) {
		Obj *obj = orderedObjs[i];
		Chr *chr = obj->_currentOwner;
		if (chr != nullptr) {
			chr->_inventory.push_back(obj);
		} else {
			Scene *scene = obj->_currentScene;
			scene->_objs.push_back(obj);
		}
	}

	// update scene chrs
	for (uint32 i = 0; i < orderedChrs.size(); ++i) {
		Chr *chr = orderedChrs[i];
		Scene *scene = chr->_currentScene;
		scene->_chrs.push_back(chr);
		if (chr != player) {
			wearObjs(chr);
		}
	}

	// move all worn helmets, shields, chest armors and spiritual
	// armors to player
	for (int type = 0; type < Chr::NUMBER_OF_ARMOR_TYPES; ++type) {
		Obj *armor;

		if (type == Chr::HEAD_ARMOR)
			armor = getObjByOffset(helmetOffset, objsHexOffset);
		else if (type == Chr::SHIELD_ARMOR)
			armor = getObjByOffset(shieldOffset, objsHexOffset);
		else if (type == Chr::BODY_ARMOR)
			armor = getObjByOffset(armorOffset, objsHexOffset);
		else
			armor = getObjByOffset(spiritualArmorOffset, objsHexOffset);

		if (armor != nullptr) {
			_world->move(armor, player);
			player->_armor[type] = armor;
		}
	}

	//TODO: make sure that armor in the inventory gets put on if we are wearing it

	_loopCount = loopNum;

	// let the engine know if there is a npc in the current scene
	if (presCharOffset != 0xffff) {
		_monster = getChrByOffset(presCharOffset, chrsHexOffset);
	}

	// java engine calls clearOutput(); here
	// processTurn("look", NULL); called in Wage right after this loadGame()

	// TODO: as you may see, aim, opponentAim or runCharOffset are not used anywhere
	// I'm fixing the first two, as those are clearly not even mentioned anywhere
	// the runCharOffset is mentioned up there as "not implemented case"
	_aim = aim;
	_opponentAim = opponentAim;

	// Load the savaegame header
	ExtendedSavegameHeader header;
	if (!MetaEngine::readSavegameHeader(data, &header, true))
		error("Invalid savegame");

	_defaultSaveDescritpion = header.description;

	delete data;
	return 0;
}

Common::Error WageEngine::loadGameState(int slot) {
	warning("LOADING %d", slot);
	if (loadGame(slot) == 0) {
		if (slot != getAutosaveSlot()) {
			_defaultSaveSlot = slot;
			// save description is set inside of loadGame()
			_gui->enableSave();
		}

		return Common::kNoError;
	} else {
		return Common::kUnknownError;
	}
}

Common::Error WageEngine::saveGameState(int slot, const Common::String &description, bool isAutosave) {
	Common::String saveLoadSlot = getSaveStateName(slot);
	if (saveGame(saveLoadSlot, description) == 0) {
		if (slot != getAutosaveSlot()) {
			_defaultSaveSlot = slot;
			_defaultSaveDescritpion = description;

			_gui->enableSave();
		}

		return Common::kNoError;
	} else {
		return Common::kUnknownError;
	}
}

bool WageEngine::scummVMSaveLoadDialog(bool isSave) {
	if (!isSave) {
		// do loading
		GUI::SaveLoadChooser dialog = GUI::SaveLoadChooser(_("Load game:"), _("Load"), false);
		int slot = dialog.runModalWithCurrentTarget();

		if (slot < 0)
			return true;

		return loadGameState(slot).getCode() == Common::kNoError;
	}

	// do saving
	GUI::SaveLoadChooser dialog = GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
	int slot = dialog.runModalWithCurrentTarget();
	Common::String desc = dialog.getResultString();

	if (desc.empty()) {
		// create our own description for the saved game, the user didn't enter it
		desc = dialog.createDefaultSaveDescription(slot);
	}

	if (desc.size() > 28)
		desc = Common::String(desc.c_str(), 28);

	if (slot < 0)
		return true;

	return saveGameState(slot, desc).getCode() == Common::kNoError;
}

} // End of namespace Agi
