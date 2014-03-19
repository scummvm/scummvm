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
#include "mads/nebular/nebular_scenes2.h"

namespace MADS {

namespace Nebular {

void Scene2xx::setAAName() {
	int idx = (_scene->_nextSceneId == 216) ? 4 : 2;
	_game._aaName = Resources::formatAAName(idx);
}

void Scene2xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;

	switch(_scene->_nextSceneId) {
	case 213:
	case 216:
		_game._player._spritesPrefix = "";
		break;
	default:
		if (_globals[0] == SEX_MALE) {
			_game._player._spritesPrefix = "ROX";
		} else {
			_game._player._spritesPrefix = "RXM";
		}
		break;
	}

	if (_scene->_nextSceneId > 212)
		_game._player._unk4 = false;
	else
		_game._player._unk4 = true;

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;
	
	if ((_scene->_nextSceneId == 203 || _scene->_nextSceneId == 204) && _globals[34])
		_game._v3 = 0;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void Scene2xx::sceneEntrySound() {
	if (_vm->_musicFlag) {
		switch (_scene->_nextSceneId) {
		case 201:
			if ((_globals[39] == 2) || (_globals[39] == 4) || (_globals[33] != 1))
				_vm->_sound->command(17);
			else
				_vm->_sound->command(9);
			break;
		case 202:
		case 203:
		case 204:
		case 205:
		case 208:
		case 209:
		case 212:
			_vm->_sound->command(9);
			break;
		case 206:
		case 211:
		case 215:
			_vm->_sound->command(10);
			break;
		case 207:
		case 214:
			_vm->_sound->command(11);
			break;
		case 210:
			if (_globals[44] == 0)
				_vm->_sound->command(15);
			else
				_vm->_sound->command(10);
			break;
		case 213:
			if (_globals[38] == 0)
				_vm->_sound->command(1);
			else
				_vm->_sound->command(9);
				break;
		case 216:
			_vm->_sound->command(16);
			break;
		default:
			_vm->_sound->command(10);
			break;
		}
	} else
		_vm->_sound->command(2);
}

/*------------------------------------------------------------------------*/

void Scene201::setup() {
	setPlayerSpritesPrefix();
	setAAName();

	Scene &scene = _vm->_game->_scene;
	scene.addActiveVocab(NOUN_15F);
	scene.addActiveVocab(NOUN_487);
	scene.addActiveVocab(NOUN_D);
}

void Scene201::enter() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0), 0);
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1), 0);
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('m', -1), 0);
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('b', -1), 0);
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*SC002Z1");
	_globals._spriteIndexes[16] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 0, 1, 0);
	_globals._spriteIndexes[17] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 15, 0, 0, 50);
	_globals._spriteIndexes[18] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 4, 0, 0, 0);
	_globals._spriteIndexes[19] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 0);
	_scene->_sequences.setDepth(_globals._spriteIndexes[19], 8);
	_scene->_sequences.setMsgPosition(_globals._spriteIndexes[19], Common::Point(185, 46));

	int idx = _scene->_dynamicHotspots.add(1159, 209, _globals._spriteIndexes[19], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, 186, 81, 8);

	_globals._v0 = 0;

	if ((_scene->_priorSceneId == 202) || (_scene->_priorSceneId == -1)) {
		_game._player._playerPos = Common::Point(165, 152);
	} else {
		_game._player._playerPos = Common::Point(223, 149);
		_game._player._direction = 2;
	}

	if (_globals[39] != 0) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		int sepChar = (_globals[0] == SEX_UNKNOWN) ? 't' : 'u';
		// Guess values. What is the default value used by the compiler?
		int suffixNum = -1;
		int abortTimers = -1; 
		switch(_globals[39]) {
		case 1:
			suffixNum = 3;
			abortTimers = 76;
			_globals[41] = -1;
			break;
		case 2:
			suffixNum = 1;
			abortTimers = 77;
			break;
		case 3:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			suffixNum = -1;
			break;
		case 4:
			suffixNum = 2;
			abortTimers = 78;
			break;
		}
		_globals[39] = 0;
		if (suffixNum >= 0)
			_scene->loadAnimation(formAnimName(sepChar, suffixNum), abortTimers);
	}
	
	if ((_scene->_priorSceneId == 202) && (_globals[33] == 1) && !_scene->_roomChanged) {
		_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('a', 0), 0);
		_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('a', 1), 0);
		_game.loadQuoteSet(90, 91, 0);
		_game._player._stepEnabled = false;
		_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[21], -1, 12);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[21], SM_FRAME_INDEX, 12, 70);
		_scene->_sequences.setDepth(_globals._spriteIndexes[21], 1);
		_globals._frameTime = 0;
		_game._player.startWalking(Common::Point(157, 143), 8);
		_vm->_palette->setEntry(252, 45, 63, 45);
		_vm->_palette->setEntry(253, 20, 45, 20);
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 2, 0, 120, _game.getQuote(90));
	} else
		_globals._frameTime = 0xFFFF;

	if (_globals[41] != 0)
		_scene->_hotspots.activate(438, false);

	sceneEntrySound();
}

void Scene201::step() {
	if ((_globals._frameTime) && (_vm->getRandomNumber(5000) == 9)) {
		_globals._spriteIndexes[20] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 5, 1, 6, 0);
		int idx = _scene->_dynamicHotspots.add(351, 13, _globals._spriteIndexes[20], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, 270, 80, 6);
		_scene->_sequences.setDepth(_globals._spriteIndexes[20], 8);
		_vm->_sound->command(14);
		_globals._frameTime = 0;
	}

	if (_game._abortTimers == 70) {
		_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 9, 1, 0, 0);
		_game._player._visible = false;
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[21], 12, 16);
		_globals._spriteIndexes[22] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 9, 1, 0, 0);
		_vm->_sound->command(42);
		_scene->_sequences.setDepth(_globals._spriteIndexes[21], 1);
		_scene->_sequences.setDepth(_globals._spriteIndexes[22], 1);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[22], SM_FRAME_INDEX, 3, 81);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[22], SM_0, 0, 71);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[21], SM_0, 0, 73);
	}

	if (_game._abortTimers == 81) {
		_scene->_kernelMessages.reset();
	}

	if (_game._abortTimers == 71) {
		_globals._spriteIndexes[22] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 9, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[22], -2, -2);
		_scene->_sequences.setDepth(_globals._spriteIndexes[22], 1);
	}

	if (_game._abortTimers == 73) {
		_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 9, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[21], 17, -2);
		_scene->_sequences.addSubEntry(_globals._spriteIndexes[21], SM_0, 0, 74);
		_scene->_sequences.setDepth(_globals._spriteIndexes[21], 1);
	}

	if (_game._abortTimers == 74) {
		_vm->_sound->command(40);

		_scene->_kernelMessages.add(Common::Point(125, 56), 0xFDFC, 32, 82, 180, _game.getQuote(91));
		_globals._spriteIndexes[21] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 9, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._spriteIndexes[21], 1);
		_scene->_sequences.setAnimRange(_globals._spriteIndexes[21], -2, -2);
		_scene->_sequences.addTimer(180, 75);
	}

	if (_game._abortTimers == 75) {
		_globals[37] = 0;
		_scene->_nextSceneId = 202;
	}

	if (_game._abortTimers == 76) {
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
	}

	if (_game._abortTimers == 77) {
		_globals[39] = 1;
		_scene->_nextSceneId = _globals[40];
		_scene->_reloadSceneFlag = true;
	}

	if (_game._abortTimers == 78) {
		_vm->_sound->command(40);
		Dialog::show(0x4E92);
		_scene->_reloadSceneFlag = true;
	}
}

void Scene201::actions() {
	MADSAction *action = _game._player._action;
	if (action->_lookFlag == false) {
		if (action->isAction(0x18C, 0x83, 0))
			_scene->_nextSceneId = 202;
		else if ((action->isAction(0x50, 0x156, 0)) || (action->isAction(0x188, 0x16C, 0)) || (action->isAction(0x188, 0x1B6, 0))) {
			if (_game._abortTimers != 0) { 
				if (_game._abortTimers != 1)
					_scene->_nextSceneId = 213;
			} else {
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				int sepChar = (_globals[0] == SEX_UNKNOWN) ? 't' : 'u';
				_scene->loadAnimation(formAnimName(sepChar, 0), 1);
			}
		} else if (action->isAction(0x3, 0x1A6, 0)) {
			Dialog::show(0x4E85);
		} else if (action->isAction(0x3, 0x129, 0)) {
			Dialog::show(0x4E86);
		} else if (action->isAction(0x3, 0x16F, 0)) {
			Dialog::show(0x4E87);
		} else if (action->isAction(0x3, 0x142, 0)) {
			Dialog::show(0x4E88);
		} else if (action->isAction(0x3, 0x18F, 0)) {
			Dialog::show(0x4E89);
		} else if (action->isAction(0x3, 0x1B9, 0)) {
			Dialog::show(0x4E8A);
		} else if (action->isAction(0x3, 0x192, 0)) {
			Dialog::show(0x4E8B);
		} else if (action->isAction(0x3, 0x1BA, 0)) {
			Dialog::show(0x4E8C);
		} else if (action->isAction(0x3, 0x83, 0)) {
			Dialog::show(0x4E8E);
		} else if (action->isAction(0x3, 0x1B6, 0)) {
			if (_globals[37])
				Dialog::show(0x4E90);
			else
				Dialog::show(0x4E8D);
		} else if (action->isAction(0x3, 0x16C, 0)) {
			Dialog::show(0x4E91);
		} else
			return;
	} else {
		Dialog::show(0x4E8F);
	}
	action->_inProgress = false;
}

/*------------------------------------------------------------------------*/

void Scene202::setup() {
}

void Scene202::enter() {
}

void Scene202::step() {
}

void Scene202::preActions() {
}

void Scene202::actions() {
	MADSAction *action = _game._player._action;
	if (action->_lookFlag == false) {
		if (action->isAction(0x4E, 0xC7, 0)) {
			action->_inProgress = false;
			return;
		} else if (action->isAction(0x18C, 0x83, 0)) {
			_scene->_nextSceneId = 203;
		} else if (action->isAction(0x18C, 0x82, 0)) {
			if (_globals[33] != 2) {
				if (_scene->_activeAnimation)
					_globals[33] = 1;
				else
					_globals[33] = 0;
			}
			_scene->_nextSceneId = 201;
		} else if (action->isAction(0x4, 0x2C, 0)) {
			if (action->_actionMode2 == 4) {
				if (_game._abortTimers == 0) {
					if (_game._objects.isInInventory(OBJ_BONES)) {
						Dialog::show(0x4EFB);
					} else {
						_game._player._stepEnabled = false;
						_game._player._visible = false;
						_globals._spriteIndexes[22] = _scene->_sequences.startReverseCycle(_globals._spriteIndexes[6], false, 3, 2, 0, 0);
						warning("TODO: sub71312");
						_scene->_sequences.addSubEntry(_globals._spriteIndexes[22], SM_FRAME_INDEX, 6, 1);
						_scene->_sequences.addSubEntry(_globals._spriteIndexes[22], SM_0, 0, 2);
					}
				} else if (_game._abortTimers == 1) {
					if ((_game._player._playerPos.x == 132) && (_game._player._playerPos.y == 97)) {
						_scene->_sequences.remove(_globals._spriteIndexes[16]);
						_globals[32] |= 1;
					} else {
						_scene->_sequences.remove(_globals._spriteIndexes[21]);
						_globals[32] |= 2;
					}
				} else if (_game._abortTimers == 2) {
					if (_game._objects.isInInventory(OBJ_BONE)) {
						warning("TODO: _game._objects.removeFromInventory(OBJ_BONE, 1);");
						warning("TODO: _game._objects.addToInventory(OBJ_BONES);");
						warning("TODO: Dialog::showPicture(OBJ_BONES, 0x4EFA, 0);");
					} else {
						warning("TODO: _game._objects.addToInventory(OBJ_BONE);");
						warning("TODO: Dialog::showPicture(OBJ_BONE, 0x4EFA, 0);");
					}
					warning("TODO: sub1EB6E(_globals[32];");
					_game._player._stepEnabled = true;
					_game._player._visible = true;
				} else {
					action->_inProgress = false;
					return;
				}
			}
		} else if ((action->isAction(0x50, 0xC7, 0)) && (_globals[31] == 0)) {
			if (_game._abortTimers == 0) {
				_vm->_sound->command(29);
				warning("TODO: word_8425E = _scene->_frameStartTime;");
				_game._player._visible = false;
				_game._player._stepEnabled = false;

				int idx = _scene->_dynamicHotspots.add(199, 79, -1, Common::Rect(241, 68, 12, 54));
				warning("CHECKME: _globals._frameTime = _scene->_dynamicHotspots.setPosition(idx, 246, 124, 8);");
				_globals._frameTime = _scene->_dynamicHotspots.setPosition(idx, 246, 124, 8);
				_globals._spriteIndexes[23] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 1, 0, 0);
				_scene->_sequences.setDepth(_globals._spriteIndexes[23], 1);
				_scene->_sequences.addSubEntry(_globals._spriteIndexes[23], SM_0, 0, 1);
			} else if (_game._abortTimers == 1) {
				_globals._spriteIndexes[24] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], true, 1);
				_scene->_sequences.setMsgPosition(_globals._spriteIndexes[24], Common::Point(247, 82));
				_scene->_sequences.setDepth(_globals._spriteIndexes[24], 1);
				warning("TODO: TimerList_getTimeout(_globals._spriteIndexes[23], _globals._spriteIndexes[24]);");
				_globals._v4 = -1;
				_game._player._stepEnabled = true;
				int msgIndex = _scene->_kernelMessages.add(Common::Point(248, 15), 0x1110, 32, 0, 60, _game.getQuote(97));
				_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
			} else {
				action->_inProgress = false;
				return;
			}
		} else if (((action->isAction(0x3, 0x27, 0x82)) || (action->isAction(0x3, 0x27, 0x1B6))) && (_globals[0] == SEX_MALE)) {
			if (_globals._v4 == 0) {
				if (_game._abortTimers == 0) {
					_game._player._stepEnabled = false;
					_game._player._visible= false;
					_globals._spriteIndexes[25] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6, 1, 0, 0);
					_scene->_sequences.setAnimRange(_globals._spriteIndexes[25], 1, 6);
					_scene->_sequences.setMsgPosition(_globals._spriteIndexes[25], Common::Point(172, 123));
					_scene->_sequences.setDepth(_globals._spriteIndexes[25], 1);
					warning("TODO: TimerList_getTimeout(-1, _globals._spriteIndexes[25]);");
					_scene->_sequences.addSubEntry(_globals._spriteIndexes[25], SM_0, 0, 1);
				} else if (_game._abortTimers == 1) {
					_globals._spriteIndexes[25] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 6);
					_scene->_sequences.setDepth(_globals._spriteIndexes[25], 1);
					_scene->_sequences.setMsgPosition(_globals._spriteIndexes[25], Common::Point(172, 123));
					if (_scene->_activeAnimation) {
						_globals._v5 = -1;
						_globals[38] = 1;
					} else {
						_scene->_sequences.addTimer(120, 2);
					}
				} else if (_game._abortTimers == 2) {
					if (!_scene->_activeAnimation && (_globals._abortVal == 0)) {
						Dialog::show(0x4EFE);
					}
					_scene->_sequences.remove(_globals._spriteIndexes[25]);
					_globals._spriteIndexes[25] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[9], false, 6, 1, 0, 0);
					_scene->_sequences.setDepth(_globals._spriteIndexes[25], 1);
					_scene->_sequences.setAnimRange(_globals._spriteIndexes[25], 1, 6);
					_scene->_sequences.setMsgPosition(_globals._spriteIndexes[25], Common::Point(172, 123));
					_scene->_sequences.addSubEntry(_globals._spriteIndexes[25], SM_0, 0, 3);
				} else if (_game._abortTimers == 3) {
					warning("TODO: TimerList_getTimeout(-1, _globals._spriteIndexes[25]);");
					_game._player._stepEnabled = true;
					_game._player._visible = true;
				} else {
					action->_inProgress = false;
					return;
				}
			} else {
				if (_game._abortTimers == 0) {
					warning("TODO: word_84268 = 1;");
					_game._player._stepEnabled = false;
					_scene->_sequences.remove(_globals._spriteIndexes[24]);
					_globals._spriteIndexes[24] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], true, 6, 1, 0, 0);
					_scene->_sequences.setMsgPosition(_globals._spriteIndexes[24], Common::Point(247, 82));
					_scene->_sequences.setDepth(_globals._spriteIndexes[24], 1);
					_scene->_sequences.addSubEntry(_globals._spriteIndexes[24], SM_0, 0, 1);
				} else if (_game._abortTimers == 1) {
					_globals._spriteIndexes[25] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], true, -2);
					_scene->_sequences.setMsgPosition(_globals._spriteIndexes[25], Common::Point(247, 82));
					_scene->_sequences.setDepth(_globals._spriteIndexes[25], 1);
					if (_scene->_activeAnimation) {
						if (_scene->_activeAnimation->getCurrentFrame() > 200) {
							_scene->_sequences.addTimer(120, 2);
						} else {
							_globals._v5 = -1;
							_globals[38] = 2;
							if ((_scene->_activeAnimation->getCurrentFrame() >= 44) && (_scene->_activeAnimation->getCurrentFrame() <= 75)) {
								_scene->_kernelMessages.reset();
								int msgIndex = _scene->_kernelMessages.add(Common::Point(248, 15), 0x1110, 32, 0, 60, _game.getQuote(100));
								_scene->_kernelMessages.setQuoted(msgIndex, 4, false);
							} else {
								action->_inProgress = false;
								return;
							}
						}
					} else {
						_scene->_sequences.addTimer(120, 2);
					}
				} else if (_game._abortTimers == 2) {
					if (!_scene->_activeAnimation)
						Dialog::show(0x4EFE);
					_globals._abortVal = 0;
					_scene->_sequences.remove(_globals._spriteIndexes[25]);
					_globals._spriteIndexes[24] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[9], false, 6, 1, 0, 0);
					_scene->_sequences.setMsgPosition(_globals._spriteIndexes[24], Common::Point(247, 82));
					_scene->_sequences.setDepth(_globals._spriteIndexes[24], 1);
					_scene->_sequences.addSubEntry(_globals._spriteIndexes[24], SM_0, 0, 3);
				} else if (_game._abortTimers == 3) {
					_globals._spriteIndexes[24] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], true, 1);
					_scene->_sequences.setMsgPosition(_globals._spriteIndexes[24], Common::Point(247, 82));
					_scene->_sequences.setDepth(_globals._spriteIndexes[24], 1);
					_game._player._stepEnabled = true;
				} else {
					action->_inProgress = false;
					return;
				}
			}
		} else if (action->isAction(0x188, 0xAA, 0)) {
			warning("TODO - sub_370C8");
		} else if (action->isAction(0x3, 0x129, 0)) {
			Dialog::show(0x4EEA);
		} else if (action->isAction(0x3, 0x86, 0)) {
			Dialog::show(0x4EEB);
		} else if (action->isAction(0x3, 0x19C, 0)) {
			Dialog::show(0x4EEC);
		} else if (action->isAction(0x3, 0x82, 0)) {
			if ((_globals[33] == 0) || (_globals[33] == 2)) {
				Dialog::show(0x4EED);
			} else if (_globals[33] == 1) {
				Dialog::show(0x4EFC);
			} else {
				action->_inProgress = false;
				return;
			} 
		} else if (action->isAction(0x3, 0x18E, 0)) {
			Dialog::show(0x4EEE);
		} else if (action->isAction(0x3, 0x164, 0)) {
			Dialog::show(0x4EEF);
		} else if (action->isAction(0x3, 0x175, 0)) {
			Dialog::show(0x4EF0);
		} else if (action->isAction(0x3, 0x174, 0)) {
			Dialog::show(0x4EF1);
		} else if (action->isAction(0x3, 0x142, 0)) {
			Dialog::show(0x4EF2);
		} else if (action->isAction(0x3, 0xAA, 0)) {
			if ((_game._player._playerPos == Common::Point(77, 105)) && (_game._player._direction == 8))
				Dialog::show(0x4EF4);
			else
				Dialog::show(0x4EF3);
		} else if (action->isAction(0x3, 0x186, 0)) {
			Dialog::show(0x4EF5);
		} else if (action->isAction(0x3, 0x1B5, 0)) {
			Dialog::show(0x4EF6);
		} else if (action->isAction(0x3, 0x140, 0)) {
			Dialog::show(0x4EF7);
		} else if (action->isAction(0x4, 0x140, 0)) {
			Dialog::show(0x4EF8);
		} else if (action->isAction(0x3, 0x2D, 0)) {
			if (action->_actionMode == 4)
				Dialog::show(0x4EF9);
			else
				return;
		} else {
			return;			
		}
	} else {
		Dialog::show(0x4EFB);
	}
	action->_inProgress = false;
}

} // End of namespace Nebular
} // End of namespace MADS
