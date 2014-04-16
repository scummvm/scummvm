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

void Scene302::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene302::enter() {
	_game._player._stepEnabled = false;
	_game._player._visible = false;

	_scene->loadAnimation(formAnimName('a',-1), 71);
	sceneEntrySound();
}

void Scene302::step() {
	if (_game._trigger == 71)
		_scene->_nextSceneId = 303;

	if ((_scene->_activeAnimation != nullptr) && (_scene->_activeAnimation->getCurrentFrame() != _oldFrame)) {
		_oldFrame = _scene->_activeAnimation->getCurrentFrame();
		if (_oldFrame == 147) {
			_game._objects.setRoom(OBJ_POISON_DARTS, 1);
			_game._objects.setRoom(OBJ_BLOWGUN, 1);
			_game._objects.setRoom(OBJ_REBREATHER, 1);
			_game._objects.setRoom(OBJ_STUFFED_FISH, 1);
			_game._objects.setRoom(OBJ_DEAD_FISH, 1);
			_game._objects.setRoom(OBJ_BURGER, 1);

			int count = (int)_game._objects._inventoryList.size();
			for (int idx = 0; idx < count; idx++) {
				if (_game._objects.isInInventory(idx))
					_game._objects.setRoom(idx, 50);
			}
		}
	}
}

/*------------------------------------------------------------------------*/

void Scene303::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene303::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', 1));

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 10, 0, 50, 120);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 10, 0, 0, 0);

	_game._player._visible = false;
	_game._player._stepEnabled = false;
	_scene->loadAnimation(formAnimName('a', -1), 60);

	sceneEntrySound();
}

void Scene303::step() {
	if (_game._trigger == 60)
		_scene->_nextSceneId = 304;
}

/*------------------------------------------------------------------------*/

void Scene304::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene304::enter() {
	if (_scene->_priorSceneId == 303) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->loadAnimation(formAnimName('a',-1), 60);
	} else {
		if (_globals[kSexOfRex] == REX_MALE)
			_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 0));
		else
			_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('a', 2));

		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 1));
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('b', 0));

		_globals._sequenceIndexes[3] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[3], false, 150, 0, 3, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 2);
		_vm->_palette->setEntry(252, 45, 63, 45);
		_vm->_palette->setEntry(253, 20, 45, 20);

		if (_globals[kSexOfRex] == REX_MALE)
			_game._player._playerPos = Common::Point(111, 117);
		else
			_game._player._playerPos = Common::Point(113, 116);

		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 11, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], -1, -1);
		_scene->_sequences.addTimer(48, 70);
	}

	sceneEntrySound();
	_game.loadQuoteSet(0xEB, 0xEC, 0);
}

void Scene304::step() {
	if (_game._trigger == 60)
		_scene->_nextSceneId = 311;

	if (_game._trigger >= 70) { 
		switch (_game._trigger) {
		case 70: {
			_game._player._visible = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 2, 4);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			if (_globals[kSexOfRex] == REX_MALE)
				_explosionSpriteId = _globals._spriteIndexes[1];
			else
				_explosionSpriteId = _globals._spriteIndexes[4];

			int sprIdx = _scene->_sequences.addSpriteCycle(_explosionSpriteId, false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(sprIdx, -1, 4);
			_scene->_sequences.setDepth(sprIdx, 1);
			_scene->_sequences.addSubEntry(sprIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 71);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 74);
			}
			break;

		case 71:
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 60, _game.getQuote(0xEB));
			_scene->_sequences.addTimer(1, 72);
			break;

		case 72: {
			_vm->_sound->command(43);
			int sprIdx = _scene->_sequences.addSpriteCycle(_explosionSpriteId, false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(sprIdx, 5, -2);
			_scene->_sequences.setDepth(sprIdx, 1);
			_scene->_sequences.addSubEntry(sprIdx, SEQUENCE_TRIGGER_EXPIRE, 0, 73);
			if (_game._storyMode == STORYMODE_NICE)
				_scene->_sequences.addSubEntry(sprIdx, SEQUENCE_TRIGGER_SPRITE, 8, 78);
			}
			break;

		case 73: {
			int sprIdx = _scene->_sequences.addSpriteCycle(_explosionSpriteId, false, 8, 0, 0, 0);
			_scene->_sequences.setAnimRange(sprIdx, -2, -2);
			_scene->_sequences.setDepth(sprIdx, 1);
			}
			break;

		case 74:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 5, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
			break;

		case 75: 
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 2, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 76);
			break;

		case 76:
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 0, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 2, 2);
			_scene->_sequences.addTimer(48, 77);
			break;

		case 77:
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(211, 45), 0xFDFC, 32, 0, 180, _game.getQuote(0xEC));
			_scene->_sequences.addTimer(120, 78);
			break;

		case 78: 
			_scene->_nextSceneId = 316;
			break;

		default:
			break;
		}
	}
}

/*------------------------------------------------------------------------*/

} // End of namespace Nebular
} // End of namespace MADS
