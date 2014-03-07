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
#include "mads/nebular/nebular_scenes8.h"

namespace MADS {

namespace Nebular {

void Scene8xx::setup1() {
	_vm->_sound->command(5);
	if ((_globals[178] && !_globals[179]) ||
			_scene->_nextSceneId == 804 || _scene->_nextSceneId == 805 ||
			_scene->_nextSceneId == 808 || _scene->_nextSceneId == 810) {
		_vm->_game->_player._spritesPrefix = "";
	} else {
		_vm->_game->_player._spritesPrefix = _globals[0] == SEX_FEMALE ? "ROX" : "RXM";
	}

	_vm->_palette->setEntry(16, 0x0A, 0x3F, 0x3F);
	_vm->_palette->setEntry(17, 0x0A, 0x2D, 0x2D);
}

void Scene8xx::setup2() {
	_vm->_game->_aaName = Resources::formatAAName(5);
}

void Scene8xx::enter1() {
	if (_vm->_musicFlag) {
		switch (_scene->_nextSceneId) {
		case 801:
		case 802:
		case 803:
		case 804:
		case 806:
		case 807:
		case 808:
			_vm->_sound->command(20);
			break;
		case 805:
			_vm->_sound->command(23);
			break;
		case 810:
			_vm->_sound->command(10);
			break;
		default:
			break;
		}
	}
}

/*------------------------------------------------------------------------*/

void Scene804::setup() {
	_globals._chairHotspotIndex = 0;
	_globals._v1 = _globals._v2 = 0;
	_globals._v3 = _globals._v4 = 0;
	_globals._v5 = -1;
	_globals._v6 = _globals._v7 = 0;
	_globals._v8 = 0;
	if (_globals[5]) {
		// Copy protection failed
		_globals[165] = 0xFFFF;
		_globals[164] = 0;
	}

	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('f', 1));

	_vm->_game->loadQuoteSet(791, 0);

	if (_globals[165]) {
		if (_globals[164]) {
			_globals._spriteIndexes[19] = _scene->_sequences.startCycle(
				_globals._spriteIndexes[4], 0, 1);
			_scene->_sequences.addTimer(60, 100);
		} else {
			_globals._spriteIndexes[20] = _scene->_sequences.startCycle(
				_globals._spriteIndexes[5], false, 1);
			_globals._spriteIndexes[21] = _scene->_sequences.startReverseCycle(
				_globals._spriteIndexes[6], false, 4, 0, 0, 0);
			_scene->_sequences.addTimer(160, 70);
			_vm->_game->_player._stepEnabled = false;
		}
	} else {
		if (_globals[167] == 0) {
			_globals._spriteIndexes[22] = _scene->_sequences.startCycle(
				_globals._spriteIndexes[7], false, 1);
		}

		if (_globals[164] == 0) {
			_globals._spriteIndexes[22] = _scene->_sequences.startCycle(
				_globals._spriteIndexes[18], false, 1);
		}

		_globals._spriteIndexes[0] = _scene->_sequences.startCycle(
			_globals._spriteIndexes[0], false, 1);
		_scene->_sequences.setMsgPosition(_globals._spriteIndexes[15], Common::Point(133, 139));
		_scene->_sequences.setDepth(_globals._spriteIndexes[15], 8);
	}

	// TODO: More setup
}

void Scene804::enter() {
}

void Scene804::step() {
}

void Scene804::preActions() {
}

void Scene804::actions() {
}

void Scene804::postActions() {
}

} // End of namespace Nebular

} // End of namespace MADS
