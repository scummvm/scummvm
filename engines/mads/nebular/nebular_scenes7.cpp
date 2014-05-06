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
#include "mads/nebular/nebular_scenes7.h"

namespace MADS {

namespace Nebular {

void Scene7xx::setAAName() {
	_game._aaName = Resources::formatAAName(5);
}

void Scene7xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);

	Common::String oldName = _game._player._spritesPrefix;

	if ((_scene->_nextSceneId == 703) || (_scene->_nextSceneId == 704) || (_scene->_nextSceneId == 705)
	 || (_scene->_nextSceneId == 707) || (_scene->_nextSceneId == 710) || (_scene->_nextSceneId == 711))
		_game._player._spritesPrefix = "";
	else if (_globals[kSexOfRex] == REX_MALE)
		_game._player._spritesPrefix = "RXM";
	else
		_game._player._spritesPrefix = "ROX";


	_game._player._scalingVelocity = true;

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void Scene7xx::sceneEntrySound() {
	if (!_vm->_musicFlag) {
		_vm->_sound->command(2);
		return;
	}

	switch (_scene->_nextSceneId) {
	case 701:
	case 702:
	case 704:
	case 705:
	case 751:
		_vm->_sound->command(38);
		break;
	case 703:
		if (_globals[kMonsterAlive] == 0)
			_vm->_sound->command(24);
		else
			_vm->_sound->command(27);
		break;
	case 706:
	case 707:
	case 710:
	case 711:
		_vm->_sound->command(25);
		break;
	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

void Scene707::setup() {
	_game._player._spritesPrefix = "";
	// The original calls Scene7xx::setAAName()
	_game._aaName = Resources::formatAAName(5);
}

void Scene707::enter() {
	_handSpriteId = _scene->_sprites.addSprites("*REXHAND");
	teleporterEnter();

	// The original uses Scene7xx_sceneEntrySound
	if (!_vm->_musicFlag)
		_vm->_sound->command(2);
	else 
		_vm->_sound->command(25);
}

void Scene707::step() {
	teleporterStep();
}

void Scene707::actions() {
	if (teleporterActions()) {
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, 0x181) || _action.isAction(0x103, 0x181))
		_vm->_dialogs->show(70710);
	else if (_action.isAction(VERB_LOOK, 0xC4))
		_vm->_dialogs->show(70711);
	else if (_action.isAction(VERB_LOOK, 0x1CC))
		_vm->_dialogs->show(70712);
	else if (_action.isAction(VERB_LOOK, 0x1D0) || _action.isAction(VERB_LOOK, 0x1D1)
	 || _action.isAction(VERB_LOOK, 0x1D2) || _action.isAction(VERB_LOOK, 0x1D3)
	 || _action.isAction(VERB_LOOK, 0x1D4) || _action.isAction(VERB_LOOK, 0x1D5)
	 || _action.isAction(VERB_LOOK, 0x1D6) || _action.isAction(VERB_LOOK, 0x1D7)
	 || _action.isAction(VERB_LOOK, 0x1D8) || _action.isAction(VERB_LOOK, 0x1D9)
	 || _action.isAction(VERB_LOOK, 0x1DB) || _action.isAction(VERB_LOOK, 0x7A)
	 || _action.isAction(VERB_LOOK, 0x1DA))
		_vm->_dialogs->show(70713);
	else if (_action.isAction(VERB_LOOK, 0x1CF) || _action._lookFlag)
		_vm->_dialogs->show(70714);
	else
		return;

	_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene710::setup() {
	_game._player._spritesPrefix = "";
	setAAName();
}

void Scene710::enter() {
	_game._scene._userInterface.setup(kInputLimitedSentences);

	if (_game._objects[OBJ_VASE]._roomNumber == 706) {
		_game._globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('g', -1));
		_globals._sequenceIndexes[1] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[1], false, 6, 0, 0, 0);
	}

	_game._player._visible = false;
	_game._scene._sequences.addTimer(600, 70);

	sceneEntrySound();
}

void Scene710::step() {
	if (_game._trigger == 70) {
		if (_game._globals[kCityFlooded])
			_game._scene._nextSceneId = 701;
		else
			_game._scene._nextSceneId = 751;
	}
}

void Scene710::actions() {
	if (_game._scene._action.isAction(0x46F, 0x27)) {
		_game._player._stepEnabled = false;

		if (_game._globals[kCityFlooded])
			_game._scene._nextSceneId = 701;
		else
			_game._scene._nextSceneId = 751;

		_action._inProgress = false;
	}
}

/*------------------------------------------------------------------------*/

void Scene711::setup() {
	// The original was calling Scene7xx::setPlayerSpreitesPrefix()
	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;
	_game._player._spritesPrefix = "";
	_game._player._scalingVelocity = true;

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);

	// The original was calling Scene7xx::setAAName()
	_game._aaName = Resources::formatAAName(5);

	_game._player._spritesPrefix = "";
}

void Scene711::enter() {
	if (_globals[kSexOfRex] == REX_FEMALE)
		_handSpriteId = _scene->_sprites.addSprites("*ROXHAND");
	else
		_handSpriteId = _scene->_sprites.addSprites("*REXHAND");

	teleporterEnter();

	// The original was using Scene7xx_SceneEntrySound()
	if (!_vm->_musicFlag)
		_vm->_sound->command(2);
	else 
		_vm->_sound->command(25);
}

void Scene711::step() {
	teleporterStep();
}

void Scene711::actions() {
	if (teleporterActions())
		_action._inProgress = false;
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
