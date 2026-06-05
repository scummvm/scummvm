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
 */

#include "common/scummsys.h"
#include "math/utils.h"
#include "mads/madsv2/nebular/nebular.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

void Scene322::setup() {
	_game._player._spritesPrefix = "";
	// The original is calling scene3xx_setAAName()
	_game._aaName = Resources::formatAAName(4);
}

void Scene322::enter() {
	if (_globals[kSexOfRex] == REX_MALE)
		_handSpriteId = _scene->_sprites.addSprites("*REXHAND");
	else
		_handSpriteId = _scene->_sprites.addSprites("*ROXHAND");

	teleporterEnter();

	// The original is using scene3xx_sceneEntrySound()
	if (!_vm->_musicFlag)
		_vm->_sound->command(2);
	else
		_vm->_sound->command(10);
}

void Scene322::step() {
	teleporterStep();
}

void Scene322::actions() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(32214);
		_action._inProgress = false;
		return;
	}

	if (teleporterActions()) {
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_VIEWPORT) || _action.isAction(VERB_PEER_THROUGH, NOUN_VIEWPORT))
		_vm->_dialogs->show(32210);
	else if (_action.isAction(VERB_LOOK, NOUN_KEYPAD))
		_vm->_dialogs->show(32211);
	else if (_action.isAction(VERB_LOOK, NOUN_DISPLAY))
		_vm->_dialogs->show(32212);
	else if (_action.isAction(VERB_LOOK, NOUN_0_KEY) || _action.isAction(VERB_LOOK, NOUN_1_KEY)
		|| _action.isAction(VERB_LOOK, NOUN_2_KEY) || _action.isAction(VERB_LOOK, NOUN_3_KEY)
		|| _action.isAction(VERB_LOOK, NOUN_4_KEY) || _action.isAction(VERB_LOOK, NOUN_5_KEY)
		|| _action.isAction(VERB_LOOK, NOUN_6_KEY) || _action.isAction(VERB_LOOK, NOUN_7_KEY)
		|| _action.isAction(VERB_LOOK, NOUN_8_KEY) || _action.isAction(VERB_LOOK, NOUN_9_KEY)
		|| _action.isAction(VERB_LOOK, NOUN_SMILE_KEY) || _action.isAction(VERB_LOOK, NOUN_ENTER_KEY)
		|| _action.isAction(VERB_LOOK, NOUN_FROWN_KEY))
		_vm->_dialogs->show(32213);
	else if (_action.isAction(VERB_LOOK, NOUN_DEVICE))
		_vm->_dialogs->show(32214);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
