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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"
#include "mads/nebular/nebular_scenes5.h"

namespace MADS {

namespace Nebular {

void Scene5xx::setAAName() {
	_game._aaName = Resources::formatAAName(5);
}

void Scene5xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);

	Common::String oldName = _game._player._spritesPrefix;

	if ((_scene->_nextSceneId == 502) || (_scene->_nextSceneId == 504) || (_scene->_nextSceneId == 505) || (_scene->_nextSceneId == 515))
		_game._player._spritesPrefix = "";
	else if (_globals[kSexOfRex] == REX_MALE)
		_game._player._spritesPrefix = "RXM";
	else if ((_scene->_nextSceneId == 501) || (_scene->_nextSceneId == 503) || (_scene->_nextSceneId == 551))
		_game._player._spritesPrefix = "ROX";

	_game._player._scalingVelocity = true;

	if ((_scene->_nextSceneId == 512) || (_scene->_nextSceneId == 507))
		_game._player._scalingVelocity = false;

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void Scene5xx::sceneEntrySound() {
	if (!_vm->_musicFlag) {
		_vm->_sound->command(2);
		return;
	}

	switch (_scene->_nextSceneId) {
	case 501:
	case 502:
	case 504:
	case 505:
	case 506:
	case 507:
	case 508:
	case 511:
	case 512:
	case 513:
	case 515:
	case 551:
		if (_scene->_priorSceneId == 503)
			_vm->_sound->command(38);
		else
			_vm->_sound->command(29);
		break;
	case 503:
		_vm->_sound->command(41);
		break;
	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

void Scene502::setup() {
	_game._player._spritesPrefix = "";
	// The original is using Scene5xx_setAAName()
	_game._aaName = Resources::formatAAName(5);
}

void Scene502::enter() {
	if (_globals[kSexOfRex] == REX_MALE)
		_handSpriteId = _scene->_sprites.addSprites("*REXHAND");
	else
		_handSpriteId = _scene->_sprites.addSprites("*ROXHAND");

	teleporterEnter();

	// The original uses scene5xx_sceneEntrySound
	if (!_vm->_musicFlag)
		_vm->_sound->command(2);
	else if (_scene->_priorSceneId == 503)
		_vm->_sound->command(38);
	else
		_vm->_sound->command(29);
}

void Scene502::step() {
	teleporterStep();
}

void Scene502::actions() {
	if (teleporterActions()) {
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x181) || _action.isAction(0x103, 0x181))
		_vm->_dialogs->show(50210);
	else if (_action.isAction(VERB_LOOK, 0xC4))
		_vm->_dialogs->show(50211);
	else if (_action.isAction(VERB_LOOK, 0x1CC))
		_vm->_dialogs->show(50212);
	else if (_action.isAction(VERB_LOOK, 0x1D0) || _action.isAction(VERB_LOOK, 0x1D1)
	  || _action.isAction(VERB_LOOK, 0x1D2) || _action.isAction(VERB_LOOK, 0x1D3)
	  || _action.isAction(VERB_LOOK, 0x1D4) || _action.isAction(VERB_LOOK, 0x1D5)
	  || _action.isAction(VERB_LOOK, 0x1D6) || _action.isAction(VERB_LOOK, 0x1D7)
	  || _action.isAction(VERB_LOOK, 0x1D8) || _action.isAction(VERB_LOOK, 0x1D9)
	  || _action.isAction(VERB_LOOK, 0x1DB) || _action.isAction(VERB_LOOK, 0x7A)
	  || _action.isAction(VERB_LOOK, 0x1DA))
		_vm->_dialogs->show(50213);
	else if (_action.isAction(VERB_LOOK, 0x1CF) || _action._lookFlag)
		_vm->_dialogs->show(50214);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
