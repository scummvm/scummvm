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

Scene101::Scene101(MADSEngine *vm) : Scene1xx(vm) {

}

void Scene101::synchronize(Common::Serializer &s) {
	Scene1xx::synchronize(s);

}

void Scene101::setup() {
	//setPlayerSpritesPrefix();
	setAAName();
}

void Scene101::enter() {
	// TODO

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1));
		// TODO
		//_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	} else {
		// TODO
	}

	// TODO
}

void Scene101::step() {
	// TODO
}

void Scene101::preActions() {
	if (_action.isAction(VERB_EXIT_TO, NOUN_ORCHESTRA_PIT)) {
		// TODO: Handle Brie
		_game._player._walkOffScreenSceneId = 102;
	} else if (_action.isAction(VERB_EXIT_TO, NOUN_GRAND_FOYER)) {
		// TODO: Handle Brie
		_game._player._walkOffScreenSceneId = 202;
	} else if (_action.isAction(VERB_TAKE, NOUN_MONSIEUR_BRIE)) {
		_vm->_dialogs->show(10121);
	} else if (_action.isAction(VERB_TALK_TO, NOUN_MONSIEUR_BRIE)) {
		if (_globals[kBrieTalkStatus] == 2)
			_game._player._needToWalk = false;
	}

	// TODO
}

void Scene101::actions() {
	// TODO: Brie conversation

	// TODO: Look around

	if (_action.isAction(VERB_LOOK) || _action.isAction(VERB_LOOK_AT)) {
		if (_action.isObject(NOUN_AISLE)) {
			_vm->_dialogs->show(10112);
		} else if (_action.isObject(NOUN_CHANDELIER)) {
			_vm->_dialogs->show(10113);
		} else if (_action.isObject(NOUN_BACK_WALL)) {
			_vm->_dialogs->show(10114);
		} else if (_action.isObject(NOUN_SIDE_WALL)) {
			_vm->_dialogs->show(10115);
		} else if (_action.isObject(NOUN_SEATS)) {
			// TODO: Finish this
			_vm->_dialogs->show(10116);
		} else if (_action.isObject(NOUN_GRAND_FOYER)) {
			_vm->_dialogs->show(10117);
		} else if (_action.isObject(NOUN_ORCHESTRA_PIT)) {
			_vm->_dialogs->show(10118);
		} else if (_action.isObject(NOUN_MONSIEUR_BRIE)) {
			_vm->_dialogs->show(10120);
		}

		_game._player._stepEnabled = true;
	} else if (_action.isAction(VERB_TALK_TO, NOUN_MONSIEUR_BRIE)) {
		if (_globals[kBrieTalkStatus] == 2)
			_vm->_dialogs->show(10122);
		_game._player._stepEnabled = true;
	} else if (_action.isAction(VERB_TAKE, NOUN_MONSIEUR_BRIE)) {
		_game._player._stepEnabled = true;
	}
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

	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RAL86");

	if (_globals[kCurrentYear] == 1993) {
		_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('z', -1));
		// TODO
		//_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	} else {
		// TODO
	}

	if (_scene->_priorSceneId == 101) {
		_game._player._playerPos = Common::Point(97, 79);
		_game._player._facing = FACING_SOUTHEAST;
		// TODO
		_game._player.walk(Common::Point(83, 87), FACING_SOUTHEAST);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	} else if (_scene->_priorSceneId == 104) {
		// Player fell from pit -> death
		// TODO
	} else if (_scene->_priorSceneId == 103 || _scene->_priorSceneId != -1) {
		_game._player._playerPos = Common::Point(282, 145);
		_game._player._facing = FACING_WEST;
		_animRunningFl = true;
		// TODO: Door closing animation
	} else if (_scene->_priorSceneId == -1) {
		// TODO
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
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
			_scene->_nextSceneId = 103;		// FIXME: temporary HACK - remove!

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
