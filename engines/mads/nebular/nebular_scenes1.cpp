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

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"
#include "mads/nebular/nebular_scenes1.h"

namespace MADS {

namespace Nebular {

void Scene1xx::sceneEntrySound() {
	if (_vm->_musicFlag) {
		switch (_scene->_nextSceneId) {
		case 101:
			_vm->_sound->command(11);
			break;
		case 102:
			_vm->_sound->command(12);
			break;
		case 103:
			_vm->_sound->command(3);
			_vm->_sound->command(25);
			break;
		case 109:
			_vm->_sound->command(13);
			break;
		case 110:
			_vm->_sound->command(10);
			break;
		case 111:
			_vm->_sound->command(3);
			break;
		case 112:
			_vm->_sound->command(15);
			break;
		default:
			if (_scene->_priorSceneId < 104 || _scene->_priorSceneId > 108)
				_vm->_sound->command(10);
			break;
		}
	}
}

/*------------------------------------------------------------------------*/

void Scene101::setup() {
}

void Scene101::enter() {
}

void Scene101::step() {
}

void Scene101::preActions() {
}

void Scene101::actions() {
}

void Scene101::postActions() {
}

/*------------------------------------------------------------------------*/

void Scene103::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene103::enter() {
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 5));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', -1));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('h', -1));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('m', -1));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('t', -1));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('r', -1));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('c', -1));

	_globals._spriteIndexes[12] = _scene->_sprites.addSprites("*RXMBD_2");
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites("*RXMRD_3");
	_globals._spriteIndexes[15] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[0], false, 7, 0, 1, 0);
	_globals._spriteIndexes[16] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[1], false, 6, 0, 2, 0);
	_scene->_sequences.setDepth(_globals._spriteIndexes[16], 0);
	_globals._spriteIndexes[17] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[2], false, 6, 0, 0, 25);	
	_globals._spriteIndexes[18] = _scene->_sequences.addSubEntry(
		_globals._spriteIndexes[17], SM_FRAME_INDEX, 2, 72);
	_globals._spriteIndexes[19] = _scene->_sequences.addSubEntry(
		_globals._spriteIndexes[17], SM_FRAME_INDEX, 2, 72);
	_scene->_sequences.addSubEntry(_globals._spriteIndexes[17], 
		SM_FRAME_INDEX, 2, 73);

	_globals._spriteIndexes[23] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[8], false, 8);
	_globals._spriteIndexes[22] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[7], false, 6);
	_globals._spriteIndexes[19] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[4], false, 6);
	_globals._spriteIndexes[20] = _scene->_sequences.addSpriteCycle(
		_globals._spriteIndexes[5], false, 6);

	if (_game._objects.isInRoom(OBJ_TIMER_MODULE)) {
		_vm->_game->_scene._hotspots.activate(371, false);
	} else {
		_globals._spriteIndexes[26] = _scene->_sequences.addSpriteCycle(
			_globals._spriteIndexes[11], false, 6);
	}

	if (_game._objects.isInRoom(OBJ_REBREATHER)) {
		_vm->_game->_scene._hotspots.activate(289, false);
	} else {
		_globals._spriteIndexes[25] = _scene->_sequences.addSpriteCycle(
			_globals._spriteIndexes[10], false, 6);
	}
	
	if (_globals[11]) {
		_globals._spriteIndexes[24] = _scene->_sequences.addSpriteCycle(
			_globals._spriteIndexes[9], false, 6);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[24], -2, -2);
		_scene->_hotspots.activate(362, false);
	}

	if (_scene->_priorSceneId != -2)
		_game._player._playerPos = Common::Point(237, 74);
	if (_scene->_priorSceneId == 102) {
		_game._player._stepEnabled = false;

		_globals._spriteIndexes[21] = _scene->_sequences.addReverseSpriteCycle(
			_globals._spriteIndexes[6], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[21], SM_0, 0, 70);
	}

	sceneEntrySound();
	_vm->_game->loadQuoteSet(70, 51, 71, 7, 73, 0);

	if (!_game._visitedScenes._sceneRevisited) {
		int msgIndex = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 
			34, 0, 120, _game.getQuote(70));
		_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
	}

	if (_scene->_priorSceneId == 102)
		_vm->_sound->command(20);

	_vm->_palette->setEntry(252, 63, 63, 10);
	_vm->_palette->setEntry(253, 45, 45, 10);
	_globals._v0 = 0;
	_globals._frameTime = _scene->_frameStartTime;
}

void Scene103::step() {
}

void Scene103::preActions() {
}

void Scene103::actions() {
}

void Scene103::postActions() {
}

} // End of namespace Nebular

} // End of namespace MADS
