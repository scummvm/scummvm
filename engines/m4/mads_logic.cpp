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
 * $URL$
 * $Id$
 *
 */

#include "m4/m4.h"
#include "m4/mads_logic.h"
#include "m4/scene.h"

namespace M4 {

/*--------------------------------------------------------------------------*/

const char *MadsSceneLogic::formAnimName(char sepChar, int suffixNum) {
	return MADSResourceManager::getResourceName(sepChar, _sceneNumber, EXTTYPE_NONE, NULL, suffixNum);
}

void MadsSceneLogic::getSceneSpriteSet() {
	char *setName = _madsVm->scene()->_playerSpriteName;
	char oldName[100];
	strcpy(oldName, setName);

	// Room change sound
	_madsVm->_sound->playSound(5);

	// Set up sprite set prefix to use
	if ((_sceneNumber <= 103) || (_sceneNumber == 111)) {
		if (_madsVm->globals()->_globals[0] == SEX_FEMALE)
			strcpy(setName, "ROX");
		else
			strcpy(setName, "RXM");
	} else if (_sceneNumber <= 110) {
		strcpy(setName, "RXSW");
		_madsVm->globals()->_globals[0] = SEX_UNKNOWN;
	} else if (_sceneNumber == 112)
		strcpy(setName, "");

	if (strcmp(setName, oldName) != 0)
		_madsVm->globals()->playerSpriteChanged = true;

	if ((_sceneNumber == 105)/* || ((_sceneNumber == 109) && (word_84800 != 0))*/)
		_madsVm->globals()->playerSpriteChanged = true;

	_vm->_palette->setEntry(16, 0x38, 0xFF, 0xFF);
	_vm->_palette->setEntry(17, 0x38, 0xb4, 0xb4);
}

void MadsSceneLogic::getAnimName() {
	const char *newName = MADSResourceManager::getAAName(
		((_sceneNumber <= 103) || (_sceneNumber > 111)) ? 0 : 1);
	strcpy(_madsVm->scene()->_aaName, newName);
}

/*--------------------------------------------------------------------------*/

/**
 * FIXME:
 * Currently I'm only working at providing manual implementation of the first Rex Nebular scene.
 * It will make more sense to convert the remaining game logic from the games into some
 * kind of bytecode scripts
 */

void MadsSceneLogic::selectScene(int sceneNum) {
	assert(sceneNum == 101);
	_sceneNumber = sceneNum;


}

void MadsSceneLogic::setupScene() {
	// FIXME: This is the hardcoded logic for Rex scene 101 only
	const char *animName = formAnimName('A', -1);
warning("anim - %s\n", animName);	
//	sub_1e754(animName, 3);

	getSceneSpriteSet();
	getAnimName();
}

void MadsSceneLogic::enterScene() {

}

void MadsSceneLogic::doAction() {

}

}
