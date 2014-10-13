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
#include "mads/phantom/phantom_scenes.h"
#include "mads/phantom/phantom_scenes1.h"

namespace MADS {

namespace Phantom {

void Scene1xx::setAAName() {
	// TODO
	//int idx = 0;
	//_game._aaName = Resources::formatAAName(idx);
}

void Scene1xx::sceneEntrySound() {
	// TODO
}

/*------------------------------------------------------------------------*/

Scene102::Scene102(MADSEngine *vm) : Scene1xx(vm) {
	_animRunningFl = false;
}

void Scene102::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

	s.syncAsByte(_animRunningFl);
}

void Scene102::setup() {
	//setPlayerSpritesPrefix();
	setAAName();
}

void Scene102::enter() {
	_animRunningFl = false;

	// TODO: Load sprite series

	if (_scene->_priorSceneId == 101) {
		_game._player._playerPos = Common::Point(97, 79);
		_game._player._facing = FACING_SOUTHEAST;
		// TODO
	} else if (_scene->_priorSceneId == 104) {
		// Player fell from pit -> death
		// TODO
	} else if (_scene->_priorSceneId == 103 || _scene->_priorSceneId != -1) {
		_game._player._playerPos = Common::Point(282, 145);
		_game._player._facing = FACING_WEST;
		// TODO: Door closing animation
		_animRunningFl = true;
	} else if (_scene->_priorSceneId == -1) {
		// TODO
	}

	sceneEntrySound();
}

void Scene102::step() {
	if (_game._trigger == 60) {		// Door closes
		// TODO
		_animRunningFl = false;
	} else if (_game._trigger == 65) {		// Death
		// TODO
		_scene->_currentSceneId = 104;
	}
}

void Scene102::preActions() {
	if (_action.isAction(VERB_OPEN, NOUN_ORCHESTRA_DOOR) || _action.isAction(VERB_PUSH, NOUN_ORCHESTRA_DOOR)) {
		_game._player.walk(Common::Point(282, 145), FACING_EAST);
	}
}

void Scene102::actions() {
	if (_action.isAction(VERB_WALK_DOWN, NOUN_AISLE)) {
		_scene->_nextSceneId = 101;
		_game._player._stepEnabled = true;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ORCHESTRA_DOOR) ||
		_action.isAction(VERB_PUSH, NOUN_ORCHESTRA_DOOR) ||
		_action.isAction(VERB_OPEN, NOUN_ORCHESTRA_DOOR)) {
		if (_animRunningFl) {
			// TODO
		} else {
			switch (_game._trigger) {
			case 70:	// try again
			case 0:
				// TODO
				break;
			case 1:
				_scene->_nextSceneId = 103;
				break;
			}
		}

		_game._player._stepEnabled = true;
	}

	// TODO: Look around

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_PIT))
			_vm->_dialogs->show(10211);
		else if (_action.isObject(NOUN_SEATS))
			if (_globals[kCurrentYear] == 1881)
				_vm->_dialogs->show(10212);
			else
				_vm->_dialogs->show(10230);
		else if (_action.isObject(NOUN_ORCHESTRA_DOOR))
			_vm->_dialogs->show(10213);
		else if (_action.isObject(NOUN_CONDUCTORS_STAND))
			_vm->_dialogs->show(10214);
		else if (_action.isObject(NOUN_MUSIC_STAND) || _action.isObject(NOUN_MUSIC_STANDS))
			_vm->_dialogs->show(10215);
		else if (_action.isObject(NOUN_PROMPTERS_BOX))
			_vm->_dialogs->show(10217);
		else if (_action.isObject(NOUN_STAGE))
			_vm->_dialogs->show(10218);
		else if (_action.isObject(NOUN_APRON))
			_vm->_dialogs->show(10219);
		else if (_action.isObject(NOUN_SIDE_WALL))
			_vm->_dialogs->show(10220);
		else if (_action.isObject(NOUN_FOLDING_CHAIRS))
			_vm->_dialogs->show(10221);
		else if (_action.isObject(NOUN_AISLE))
			_vm->_dialogs->show(10222);
		else if (_action.isObject(NOUN_PROSCENIUM_ARCH))
			_vm->_dialogs->show(10223);
		else if (_action.isObject(NOUN_ACT_CURTAIN))
			_vm->_dialogs->show(10224);
		else if (_action.isObject(NOUN_IN_ONE))
			_vm->_dialogs->show(10225);
		else if (_action.isObject(NOUN_IN_TWO))
			_vm->_dialogs->show(10226);
		else if (_action.isObject(NOUN_LEG))
			_vm->_dialogs->show(10227);
		else if (_action.isObject(NOUN_CHANDELIER))
			_vm->_dialogs->show(10231);
		else
			return;

		_game._player._stepEnabled = true;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_ORCHESTRA_DOOR)) {
		_vm->_dialogs->show(10228);
		_game._player._stepEnabled = true;
	}
}


/*------------------------------------------------------------------------*/

} // End of namespace Phantom
} // End of namespace MADS
