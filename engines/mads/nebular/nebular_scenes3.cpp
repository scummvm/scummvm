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
#include "mads/nebular/nebular_scenes3.h"

namespace MADS {

namespace Nebular {

void Scene3xx::setAAName() {
	_game._aaName = Resources::formatAAName(4);
}

void Scene3xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);

	Common::String oldName = _game._player._spritesPrefix;

	if (_globals[kSexOfRex] == REX_MALE)
		_game._player._spritesPrefix = "RXM";
	else
		_game._player._spritesPrefix = "ROX";

	if ((_scene->_nextSceneId == 313) || (_scene->_nextSceneId == 366)
	|| ((_scene->_nextSceneId >= 301) && (_scene->_nextSceneId <= 303))
	|| ((_scene->_nextSceneId == 304) && (_scene->_currentSceneId == 303))
	|| ((_scene->_nextSceneId == 311) && (_scene->_currentSceneId == 304))
	|| ((_scene->_nextSceneId >= 308) && (_scene->_nextSceneId <= 310))
	|| ((_scene->_nextSceneId >= 319) && (_scene->_nextSceneId <= 322))
	|| ((_scene->_nextSceneId >= 387) && (_scene->_nextSceneId <= 391))) {
		_game._player._spritesPrefix = "";
		_game._player._spritesChanged = true;
	} 

	_game._player._scalingVelocity = true;
	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void Scene3xx::sceneEntrySound() {
	if (!_vm->_musicFlag) {
		_vm->_sound->command(2);
		return;
	}

	switch (_scene->_nextSceneId) {
	case 301:
	case 302:
	case 303:
	case 304:
	case 308:
	case 309:
	case 310:
		_vm->_sound->command(11);
		break;

	case 311:
		if (_scene->_priorSceneId == 304)
			_vm->_sound->command(11);
		else
			_vm->_sound->command(10);
		break;

	case 313:
	case 316:
	case 320:
	case 322:
	case 357:
	case 358:
	case 359:
	case 360:
	case 361:
	case 387:
	case 388:
	case 389:
	case 390:
	case 391:
	case 399:
		_vm->_sound->command(10);
		break;

	case 318:
		if ((_scene->_priorSceneId == 357) || (_scene->_priorSceneId == 407))
			_vm->_sound->command(10);
		else if (_scene->_priorSceneId == 319)
			_vm->_sound->command(16);
		else
			_vm->_sound->command(3);

		_vm->_sound->command(50);
		break;

	case 319:
		_vm->_sound->command(16);
		break;

	case 321:
		_vm->_sound->command(18);
		break;

	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

void Scene301::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene301::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);

	_globals[kMeteorologistStatus] = METEOROLOGIST_GONE;
	_globals[kTeleporterCommand] = TELEPORTER_NONE;

	_game._player._stepEnabled = false;
	_game._player._visible = false;
	_scene->loadAnimation(formAnimName('a', -1), 60);

	sceneEntrySound();
}

void Scene301::step() {
	if (_game._trigger == 60)
		_scene->_nextSceneId = 302;
}

/*------------------------------------------------------------------------*/
	
} // End of namespace Nebular
} // End of namespace MADS
