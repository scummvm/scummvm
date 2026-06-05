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

Scene319::Scene319(RexNebularEngine *vm) : Scene3xx(vm) {
	_animMode = -1;
	_animFrame = -1;
	_nextAction1 = -1;
	_nextAction2 = -1;
	_slacheMode = -1;
	_slacheTopic = -1;
	_slachePosY = -1;

	_slacheTalkingFl = false;
	_slacheReady = false;
	_slacheInitFl = false;

	_subQuote2 = "";
}

void Scene319::synchronize(Common::Serializer &s) {
	Scene3xx::synchronize(s);

	s.syncAsUint32LE(_animMode);
	s.syncAsUint32LE(_animFrame);
	s.syncAsUint32LE(_nextAction1);
	s.syncAsUint32LE(_nextAction2);
	s.syncAsUint32LE(_slacheMode);
	s.syncAsUint32LE(_slacheTopic);
	s.syncAsUint32LE(_slachePosY);

	s.syncAsByte(_slacheTalkingFl);
	s.syncAsByte(_slacheReady);
	s.syncAsByte(_slacheInitFl);

	s.syncString(_subQuote2);
}

void Scene319::setup() {
	setPlayerSpritesPrefix();
	setAAName();
}

void Scene319::handleRexDialogues(int quote) {
	_scene->_kernelMessages.reset();

	Common::String curQuote = _game.getQuote(quote);
	if (_vm->_font->getWidth(curQuote, _scene->_textSpacing) > 200) {
		Common::String subQuote1;
		_game.splitQuote(curQuote, subQuote1, _subQuote2);
		_scene->_kernelMessages.add(Common::Point(160, 106), 0x1110, 32, 0, 120, subQuote1);
		_scene->_kernelMessages.add(Common::Point(160, 120), 0x1110, 32, 1, 120, _subQuote2);
	} else
		_scene->_kernelMessages.add(Common::Point(160, 120), 0x1110, 32, 1, 120, curQuote);
}

void Scene319::handleSlacheDialogs(int quoteId, int counter, uint32 timer) {
	int curQuote = quoteId;
	int posY = 5 + (_slachePosY * 14);

	for (int count = 0; count < counter; count++, curQuote++) {
		_scene->_kernelMessages.add(Common::Point(8, posY), 0xFDFC, 0, 0, timer, _game.getQuote(curQuote));
		posY += 14;
	}
}

void Scene319::enter() {
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('e', 0));
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 3));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('k', -1));

	if (!_game._objects.isInInventory(OBJ_SCALPEL)) {
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
	}

	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 18, 0, 0, 300);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 67, 0, 0, 377);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 173, 0, 0, 233);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

	_globals._sequenceIndexes[0] = _scene->_sequences.startCycle(_globals._spriteIndexes[0], false, 1);
	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);

	_dialog1.setup(0x43, 0x165, 0x166, 0x167, 0x168, 0x169, 0x16A, 0);
	_dialog2.setup(0x44, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0);
	_dialog3.setup(0x45, 0x17D, 0x17E, 0x17F, 0x180, 0x181, 0x182, 0x183, 0);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_dialog1.set(0x165, 0x166, 0x167, 0x168, 0);
		_dialog2.set(0x171, 0x172, 0x173, 0x174, 0);
		_dialog3.set(0x17D, 0x17E, 0x17F, 0x180, 0);
	}

	_game.loadQuoteSet(0x15F, 0x160, 0x161, 0x162, 0x163, 0x164, 0x16B, 0x16C, 0x16D,
		0x16E, 0x16F, 0x170, 0x177, 0x178, 0x178, 0x17A, 0x17B, 0x17C, 0x165, 0x166,
		0x167, 0x168, 0x169, 0x16A, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0x17D,
		0x17E, 0x17F, 0x180, 0x181, 0x182, 0x183, 0x184, 0x185, 0x186, 0x187, 0x188,
		0x189, 0x18A, 0x18B, 0);

	_vm->_palette->setEntry(252, 63, 30, 2);
	_vm->_palette->setEntry(253, 45, 15, 1);

	_slachePosY = 0;
	_slacheInitFl = false;
	_slacheTalkingFl = false;
	_slacheReady = false;
	_animFrame = 0;

	_scene->loadAnimation(formAnimName('b', 0));

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_animMode = 1;
		_nextAction1 = 2;
		_nextAction2 = 2;
		_slacheMode = 1;
		_slacheTopic = 1;
		_slacheInitFl = true;

		if (_globals[kRexHasMetSlache]) {
			handleSlacheDialogs(VERB_WALK_OUTSIDE, 2, INDEFINITE_TIMEOUT);
			_slachePosY = 3;
		} else {
			handleSlacheDialogs(0x186, 4, INDEFINITE_TIMEOUT);
			_slachePosY = 5;
		}
	}

	switch (_slacheTopic) {
	case 1:
		handleSlacheDialogs(0x15F, 2, INDEFINITE_TIMEOUT);
		_dialog1.start();
		break;

	case 2:
		handleSlacheDialogs(0x16B, 2, INDEFINITE_TIMEOUT);
		_dialog2.start();
		break;

	case 3:
		handleSlacheDialogs(0x177, 2, INDEFINITE_TIMEOUT);
		_dialog3.start();
		break;

	default:
		break;
	}

	_slachePosY = 0;
	sceneEntrySound();
}

void Scene319::step() {
	if (_scene->_animation[0] == nullptr)
		return;

	if (_animFrame != _scene->_animation[0]->getCurrentFrame()) {
		_animFrame = _scene->_animation[0]->getCurrentFrame();
		int nextFrame = -1;
		if (_animMode == 1) {
			switch (_animFrame) {
			case 6:
				_slacheTalkingFl = true;
				break;

			case 29:
				_slacheReady = true;
				break;

			case 39:
				if (_slacheInitFl) {
					_slacheInitFl = false;
					if (_nextAction1 == 2)
						nextFrame = 0;
				} else if (_nextAction1 == 2)
					_nextAction1 = 1;
				break;

			case 50:
			case 60:
			case 70:
			case 85:
				if (_nextAction1 == 2)
					nextFrame = 0;
				else if (_nextAction1 == 3) {
					nextFrame = 85;
					_slacheTalkingFl = true;
				} else if (_animFrame == 85) {
					if (!_game._player._stepEnabled)
						_slacheTalkingFl = true;
					nextFrame = 40;
				}
				break;

			case 115:
				_slacheReady = true;
				break;

			case 129:
				if (_nextAction1 == 3) {
					nextFrame = 115;
					if (!_game._player._stepEnabled)
						_slacheTalkingFl = true;
				}
				break;

			case 145:
				nextFrame = 40;
				break;

			default:
				break;
			}

			if ((_animFrame > 40) && (_animFrame < 85) && (nextFrame < 0)) {
				switch (_nextAction1) {
				case 4:
					_animFrame = 0;
					_scene->freeAnimation();
					_animMode = 2;
					_scene->loadAnimation(formAnimName('b', 3), 70);
					break;

				case 5:
					_animFrame = 0;
					_scene->freeAnimation();
					_animMode = 3;
					_scene->loadAnimation(formAnimName('b', 4), 71);
					break;

				case 6:
					_animFrame = 0;
					_scene->freeAnimation();
					_animMode = 4;
					_scene->loadAnimation(formAnimName('b', 5), 72);
					break;

				default:
					break;
				}

				if (!_animFrame) {
					_scene->_sequences.remove(_globals._sequenceIndexes[0]);
					_scene->_sequences.remove(_globals._sequenceIndexes[1]);

					for (int i = 0; i <= 1; i++) {
						_globals._sequenceIndexes[i] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[i], false, 8, 1, 0, 0);
						_scene->_sequences.setAnimRange(_globals._sequenceIndexes[i], 1, 7);
					}
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
				}
			}
		}

		if (_animMode == 2) {
			if (_animFrame == 13)
				_vm->_screen->_shakeCountdown = 40;

			if (_animFrame == 16)
				_vm->_screen->_shakeCountdown = 1;
		}

		if (_animMode == 3) {
			if (_animFrame == 11)
				_vm->_screen->_shakeCountdown = 60;

			if (_animFrame == 18)
				_vm->_screen->_shakeCountdown = 1;
		}

		if ((_animMode == 4) && (_animFrame == 16))
			_vm->_screen->_shakeCountdown = 80;

		if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
			_scene->_animation[0]->setCurrentFrame(nextFrame);
			_animFrame = nextFrame;
		}
	}

	switch (_game._trigger) {
	case 70:
	case 71:
	{
		_animMode = 1;
		_nextAction1 = _nextAction2;
		_animFrame = 0;
		_scene->freeAnimation();
		_scene->loadAnimation(formAnimName('b', 0));
		if (_nextAction1 == 3)
			_scene->_animation[0]->setCurrentFrame(85);
		else if (_nextAction1 == 1)
			_scene->_animation[0]->setCurrentFrame(40);

		_animFrame = _scene->_animation[0]->getCurrentFrame();
		_slacheTalkingFl = true;
		_vm->_screen->_shakeCountdown = 1;

		for (int i = 0; i <= 1; i++) {
			int oldIdx = _globals._sequenceIndexes[i];
			_scene->_sequences.remove(_globals._sequenceIndexes[i]);
			_globals._sequenceIndexes[i] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[i], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[i], 8, 13);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[i], oldIdx);
		}
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 74);

		// WORKAROUND: This fixes the game sometimes going into an endless waiting
		// loop even after the doctor has finished hitting Rex. Note sure if it's due
		// to a bug in room script or in the engine, but this at least fixes it
		int seqIndex = _scene->_sequences.findByTrigger(2);
		_scene->_sequences[seqIndex]._doneFlag = false;
		break;
	}

	case 72:
		_vm->_palette->setColorFlags(0xFF, 0, 0);
		_vm->_palette->setColorValues(0, 0, 0);
		_vm->_palette->fadeOut(_vm->_palette->_mainPalette, nullptr, 18, 228,
			248, 0, 1, 16);
		_vm->_screen->_shakeCountdown = 1;
		_scene->_reloadSceneFlag = true;
		break;

	case 73:
		for (int i = 0; i <= 1; i++) {
			int oldIdx = _globals._sequenceIndexes[i];
			_scene->_sequences.remove(_globals._sequenceIndexes[i]);
			_globals._sequenceIndexes[i] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[i], false, 8, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[i], 6, 7);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[i], oldIdx);
		}
		break;

	case 74:
		for (int i = 0; i <= 1; i++) {
			int oldIdx = _globals._sequenceIndexes[i];
			_scene->_sequences.remove(_globals._sequenceIndexes[i]);
			_globals._sequenceIndexes[i] = _scene->_sequences.startCycle(_globals._spriteIndexes[i], false, 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[i], oldIdx);
		}
		break;

	default:
		break;
	}
}

void Scene319::actions() {
	if (_game._trigger == 0) {
		_game._player._stepEnabled = false;
		handleRexDialogues(_action._activeAction._verbId);
	} else {
		if ((_action._activeAction._verbId == 0x165) || (_action._activeAction._verbId == 0x166)) {
			if (_game._trigger == 1) {
				_nextAction1 = 3;
				_slacheTalkingFl = false;
				_slacheMode = 1;
				_slacheTopic = 2;
			}

			if (!_slacheTalkingFl) {
				_scene->_sequences.addTimer(4, 2);
			} else {
				handleSlacheDialogs(0x16B, 2, INDEFINITE_TIMEOUT);
				_dialog2.start();
				_game._player._stepEnabled = true;
			}
		}

		if ((_action._activeAction._verbId == 0x171) || (_action._activeAction._verbId == 0x172)) {
			if (_game._trigger == 1) {
				_nextAction1 = 2;
				_slacheTalkingFl = false;
				_slacheMode = 1;
				_slacheTopic = 3;
			}

			if (!_slacheTalkingFl) {
				_scene->_sequences.addTimer(4, 2);
			} else {
				handleSlacheDialogs(0x177, 2, INDEFINITE_TIMEOUT);
				_dialog3.start();
				_game._player._stepEnabled = true;
			}
		}

		if ((_action._activeAction._verbId == 0x17D) || (_action._activeAction._verbId == 0x17E)) {
			if (_game._trigger == 1) {
				_nextAction1 = 3;
				_slacheTalkingFl = false;
				_slacheReady = false;
				_slacheMode = 1;
				_slacheTopic = 1;
			}

			if (!_slacheTalkingFl) {
				_scene->_sequences.addTimer(4, 2);
			} else {
				if (_game._trigger == 2)
					handleSlacheDialogs(0x184, 2, 180);

				if (!_slacheReady) {
					_scene->_sequences.addTimer(120, 3);
				} else {
					_globals[kRexHasMetSlache] = true;
					_scene->_nextSceneId = 318;
				}
			}
		}

		if ((_action._activeAction._verbId == 0x168) || (_action._activeAction._verbId == 0x174) ||
			(_action._activeAction._verbId == 0x180) || (_action._activeAction._verbId == 0x169) ||
			(_action._activeAction._verbId == 0x175) || (_action._activeAction._verbId == 0x181) ||
			(_action._activeAction._verbId == 0x16A) || (_action._activeAction._verbId == 0x176) ||
			(_action._activeAction._verbId == 0x182) || (_action._activeAction._verbId == 0x183) ||
			(_action._activeAction._verbId == 0x167) || (_action._activeAction._verbId == 0x173) ||
			(_action._activeAction._verbId == 0x17F)) {

			bool addDialogLine = !((_action._activeAction._verbId == 0x167) || (_action._activeAction._verbId == 0x173) ||
				(_action._activeAction._verbId == 0x17F) || (_action._activeAction._verbId == 0x16A) ||
				(_action._activeAction._verbId == 0x176) || (_action._activeAction._verbId == 0x182) ||
				(_action._activeAction._verbId == 0x183));

			int addVerbId = _action._activeAction._verbId + 1;
			if ((addVerbId == 0x182) && (_game._storyMode != STORYMODE_NAUGHTY))
				addVerbId = 0x183;

			if (_slacheMode == 1) {
				if (_game._trigger == 1) {
					_nextAction2 = _nextAction1;
					_nextAction1 = 4;
				}

				if (_nextAction1 != _nextAction2) {
					_scene->_sequences.addTimer(4, 2);
				} else {
					Conversation *curDialog;
					int nextDocQuote;
					if ((_action._activeAction._verbId == 0x168) || (_action._activeAction._verbId == 0x167)) {
						curDialog = &_dialog1;
						nextDocQuote = 0x161;
					} else if ((_action._activeAction._verbId == 0x174) || (_action._activeAction._verbId == 0x1753)) {
						nextDocQuote = 0x16D;
						curDialog = &_dialog2;
					} else {
						nextDocQuote = 0x179;
						curDialog = &_dialog3;
					}

					handleSlacheDialogs(nextDocQuote, 2, INDEFINITE_TIMEOUT);
					if (addDialogLine) {
						curDialog->write(_action._activeAction._verbId, false);
						curDialog->write(addVerbId, true);
					}

					curDialog->start();
					_game._player._stepEnabled = true;
					_slacheMode = 2;
				}
			} else if (_slacheMode == 2) {
				if (_game._trigger == 1) {
					_nextAction2 = _nextAction1;
					_nextAction1 = 5;
				}

				if (_nextAction1 != _nextAction2) {
					_scene->_sequences.addTimer(4, 2);
				} else {
					Conversation *curDialog;
					int nextDocQuote;
					if ((_action._activeAction._verbId == 0x168) || (_action._activeAction._verbId == 0x169) || (_action._activeAction._verbId == 0x167)) {
						curDialog = &_dialog1;
						nextDocQuote = 0x163;
					} else if ((_action._activeAction._verbId == 0x174) || (_action._activeAction._verbId == 0x175) || (_action._activeAction._verbId == 0x173)) {
						nextDocQuote = 0x16F;
						curDialog = &_dialog2;
					} else {
						nextDocQuote = 0x17B;
						curDialog = &_dialog3;
					}

					handleSlacheDialogs(nextDocQuote, 2, INDEFINITE_TIMEOUT);
					if (addDialogLine) {
						curDialog->write(_action._activeAction._verbId, false);
						curDialog->write(addVerbId, true);
					}

					curDialog->start();
					_game._player._stepEnabled = true;
					_slacheMode = 3;
				}
			} else {
				_nextAction2 = _nextAction1;
				_nextAction1 = 6;
			}
		}
	}

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
