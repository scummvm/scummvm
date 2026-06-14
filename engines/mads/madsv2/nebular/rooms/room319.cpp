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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section3.h"
#include "mads/madsv2/nebular/rooms/conversation.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _animMode;
	int32 _animFrame;
	int32 _nextAction1;
	int32 _nextAction2;
	int32 _slacheMode;
	int32 _slacheTopic;
	int32 _slachePosY;
	bool _slacheTalkingFl;
	bool _slacheReady;
	bool _slacheInitFl;
	char _subQuote2[256];
	Conversation _dialog1;
	Conversation _dialog2;
	Conversation _dialog3;
};

static Scratch local;


static void handleRexDialogues(int quote) {
	_scene->_kernelMessages.reset();

	Common::String curQuote = _game.getQuote(quote);
	if (_vm->_font->getWidth(curQuote, _scene->_textSpacing) > 200) {
		Common::String subQuote1, subQuote2;
		_game.splitQuote(curQuote, subQuote1, subQuote2);
		Common::strcpy_s(local._subQuote2, subQuote2.c_str());

		_scene->_kernelMessages.add(Common::Point(160, 106), 0x1110, 32, 0, 120, subQuote1);
		_scene->_kernelMessages.add(Common::Point(160, 120), 0x1110, 32, 1, 120, local._subQuote2);
	} else
		_scene->_kernelMessages.add(Common::Point(160, 120), 0x1110, 32, 1, 120, curQuote);
}

static void handleSlacheDialogs(int quoteId, int counter, uint32 timer) {
	int curQuote = quoteId;
	int posY = 5 + (local._slachePosY * 14);

	for (int count = 0; count < counter; count++, curQuote++) {
		_scene->_kernelMessages.add(Common::Point(8, posY), 0xFDFC, 0, 0, timer, _game.getQuote(curQuote));
		posY += 14;
	}
}

static void room_319_init() {
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

	local._dialog1.setup(0x43, 0x165, 0x166, 0x167, 0x168, 0x169, 0x16A, 0);
	local._dialog2.setup(0x44, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0);
	local._dialog3.setup(0x45, 0x17D, 0x17E, 0x17F, 0x180, 0x181, 0x182, 0x183, 0);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		local._dialog1.set(0x165, 0x166, 0x167, 0x168, 0);
		local._dialog2.set(0x171, 0x172, 0x173, 0x174, 0);
		local._dialog3.set(0x17D, 0x17E, 0x17F, 0x180, 0);
	}

	_game.loadQuoteSet(0x15F, 0x160, 0x161, 0x162, 0x163, 0x164, 0x16B, 0x16C, 0x16D,
		0x16E, 0x16F, 0x170, 0x177, 0x178, 0x178, 0x17A, 0x17B, 0x17C, 0x165, 0x166,
		0x167, 0x168, 0x169, 0x16A, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0x17D,
		0x17E, 0x17F, 0x180, 0x181, 0x182, 0x183, 0x184, 0x185, 0x186, 0x187, 0x188,
		0x189, 0x18A, 0x18B, 0);

	_vm->_palette->setEntry(252, 63, 30, 2);
	_vm->_palette->setEntry(253, 45, 15, 1);

	local._slachePosY = 0;
	local._slacheInitFl = false;
	local._slacheTalkingFl = false;
	local._slacheReady = false;
	local._animFrame = 0;

	_scene->loadAnimation(formAnimName('b', 0));

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		local._animMode = 1;
		local._nextAction1 = 2;
		local._nextAction2 = 2;
		local._slacheMode = 1;
		local._slacheTopic = 1;
		local._slacheInitFl = true;

		if (_globals[kRexHasMetSlache]) {
			handleSlacheDialogs(VERB_WALK_OUTSIDE, 2, INDEFINITE_TIMEOUT);
			local._slachePosY = 3;
		} else {
			handleSlacheDialogs(0x186, 4, INDEFINITE_TIMEOUT);
			local._slachePosY = 5;
		}
	}

	switch (local._slacheTopic) {
	case 1:
		handleSlacheDialogs(0x15F, 2, INDEFINITE_TIMEOUT);
		local._dialog1.start();
		break;

	case 2:
		handleSlacheDialogs(0x16B, 2, INDEFINITE_TIMEOUT);
		local._dialog2.start();
		break;

	case 3:
		handleSlacheDialogs(0x177, 2, INDEFINITE_TIMEOUT);
		local._dialog3.start();
		break;

	default:
		break;
	}

	local._slachePosY = 0;
	section_3_music();
}

static void room_319_daemon() {
	if (_scene->_animation[0] == nullptr)
		return;

	if (local._animFrame != _scene->_animation[0]->getCurrentFrame()) {
		local._animFrame = _scene->_animation[0]->getCurrentFrame();
		int nextFrame = -1;
		if (local._animMode == 1) {
			switch (local._animFrame) {
			case 6:
				local._slacheTalkingFl = true;
				break;

			case 29:
				local._slacheReady = true;
				break;

			case 39:
				if (local._slacheInitFl) {
					local._slacheInitFl = false;
					if (local._nextAction1 == 2)
						nextFrame = 0;
				} else if (local._nextAction1 == 2)
					local._nextAction1 = 1;
				break;

			case 50:
			case 60:
			case 70:
			case 85:
				if (local._nextAction1 == 2)
					nextFrame = 0;
				else if (local._nextAction1 == 3) {
					nextFrame = 85;
					local._slacheTalkingFl = true;
				} else if (local._animFrame == 85) {
					if (!_game._player._stepEnabled)
						local._slacheTalkingFl = true;
					nextFrame = 40;
				}
				break;

			case 115:
				local._slacheReady = true;
				break;

			case 129:
				if (local._nextAction1 == 3) {
					nextFrame = 115;
					if (!_game._player._stepEnabled)
						local._slacheTalkingFl = true;
				}
				break;

			case 145:
				nextFrame = 40;
				break;

			default:
				break;
			}

			if ((local._animFrame > 40) && (local._animFrame < 85) && (nextFrame < 0)) {
				switch (local._nextAction1) {
				case 4:
					local._animFrame = 0;
					_scene->freeAnimation();
					local._animMode = 2;
					_scene->loadAnimation(formAnimName('b', 3), 70);
					break;

				case 5:
					local._animFrame = 0;
					_scene->freeAnimation();
					local._animMode = 3;
					_scene->loadAnimation(formAnimName('b', 4), 71);
					break;

				case 6:
					local._animFrame = 0;
					_scene->freeAnimation();
					local._animMode = 4;
					_scene->loadAnimation(formAnimName('b', 5), 72);
					break;

				default:
					break;
				}

				if (!local._animFrame) {
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

		if (local._animMode == 2) {
			if (local._animFrame == 13)
				_vm->_screen->_shakeCountdown = 40;

			if (local._animFrame == 16)
				_vm->_screen->_shakeCountdown = 1;
		}

		if (local._animMode == 3) {
			if (local._animFrame == 11)
				_vm->_screen->_shakeCountdown = 60;

			if (local._animFrame == 18)
				_vm->_screen->_shakeCountdown = 1;
		}

		if ((local._animMode == 4) && (local._animFrame == 16))
			_vm->_screen->_shakeCountdown = 80;

		if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
			_scene->_animation[0]->setCurrentFrame(nextFrame);
			local._animFrame = nextFrame;
		}
	}

	switch (_game._trigger) {
	case 70:
	case 71:
	{
		local._animMode = 1;
		local._nextAction1 = local._nextAction2;
		local._animFrame = 0;
		_scene->freeAnimation();
		_scene->loadAnimation(formAnimName('b', 0));
		if (local._nextAction1 == 3)
			_scene->_animation[0]->setCurrentFrame(85);
		else if (local._nextAction1 == 1)
			_scene->_animation[0]->setCurrentFrame(40);

		local._animFrame = _scene->_animation[0]->getCurrentFrame();
		local._slacheTalkingFl = true;
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
		_vm->_palette->fadeOut(master_palette, nullptr, 18, 228,
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

static void room_319_parser() {
	if (_game._trigger == 0) {
		_game._player._stepEnabled = false;
		handleRexDialogues(_action._activeAction._verbId);
	} else {
		if ((_action._activeAction._verbId == 0x165) || (_action._activeAction._verbId == 0x166)) {
			if (_game._trigger == 1) {
				local._nextAction1 = 3;
				local._slacheTalkingFl = false;
				local._slacheMode = 1;
				local._slacheTopic = 2;
			}

			if (!local._slacheTalkingFl) {
				_scene->_sequences.addTimer(4, 2);
			} else {
				handleSlacheDialogs(0x16B, 2, INDEFINITE_TIMEOUT);
				local._dialog2.start();
				_game._player._stepEnabled = true;
			}
		}

		if ((_action._activeAction._verbId == 0x171) || (_action._activeAction._verbId == 0x172)) {
			if (_game._trigger == 1) {
				local._nextAction1 = 2;
				local._slacheTalkingFl = false;
				local._slacheMode = 1;
				local._slacheTopic = 3;
			}

			if (!local._slacheTalkingFl) {
				_scene->_sequences.addTimer(4, 2);
			} else {
				handleSlacheDialogs(0x177, 2, INDEFINITE_TIMEOUT);
				local._dialog3.start();
				_game._player._stepEnabled = true;
			}
		}

		if ((_action._activeAction._verbId == 0x17D) || (_action._activeAction._verbId == 0x17E)) {
			if (_game._trigger == 1) {
				local._nextAction1 = 3;
				local._slacheTalkingFl = false;
				local._slacheReady = false;
				local._slacheMode = 1;
				local._slacheTopic = 1;
			}

			if (!local._slacheTalkingFl) {
				_scene->_sequences.addTimer(4, 2);
			} else {
				if (_game._trigger == 2)
					handleSlacheDialogs(0x184, 2, 180);

				if (!local._slacheReady) {
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

			if (local._slacheMode == 1) {
				if (_game._trigger == 1) {
					local._nextAction2 = local._nextAction1;
					local._nextAction1 = 4;
				}

				if (local._nextAction1 != local._nextAction2) {
					_scene->_sequences.addTimer(4, 2);
				} else {
					Conversation *curDialog;
					int nextDocQuote;
					if ((_action._activeAction._verbId == 0x168) || (_action._activeAction._verbId == 0x167)) {
						curDialog = &local._dialog1;
						nextDocQuote = 0x161;
					} else if ((_action._activeAction._verbId == 0x174) || (_action._activeAction._verbId == 0x1753)) {
						nextDocQuote = 0x16D;
						curDialog = &local._dialog2;
					} else {
						nextDocQuote = 0x179;
						curDialog = &local._dialog3;
					}

					handleSlacheDialogs(nextDocQuote, 2, INDEFINITE_TIMEOUT);
					if (addDialogLine) {
						curDialog->write(_action._activeAction._verbId, false);
						curDialog->write(addVerbId, true);
					}

					curDialog->start();
					_game._player._stepEnabled = true;
					local._slacheMode = 2;
				}
			} else if (local._slacheMode == 2) {
				if (_game._trigger == 1) {
					local._nextAction2 = local._nextAction1;
					local._nextAction1 = 5;
				}

				if (local._nextAction1 != local._nextAction2) {
					_scene->_sequences.addTimer(4, 2);
				} else {
					Conversation *curDialog;
					int nextDocQuote;
					if ((_action._activeAction._verbId == 0x168) || (_action._activeAction._verbId == 0x169) || (_action._activeAction._verbId == 0x167)) {
						curDialog = &local._dialog1;
						nextDocQuote = 0x163;
					} else if ((_action._activeAction._verbId == 0x174) || (_action._activeAction._verbId == 0x175) || (_action._activeAction._verbId == 0x173)) {
						nextDocQuote = 0x16F;
						curDialog = &local._dialog2;
					} else {
						nextDocQuote = 0x17B;
						curDialog = &local._dialog3;
					}

					handleSlacheDialogs(nextDocQuote, 2, INDEFINITE_TIMEOUT);
					if (addDialogLine) {
						curDialog->write(_action._activeAction._verbId, false);
						curDialog->write(addVerbId, true);
					}

					curDialog->start();
					_game._player._stepEnabled = true;
					local._slacheMode = 3;
				}
			} else {
				local._nextAction2 = local._nextAction1;
				local._nextAction1 = 6;
			}
		}
	}

	_action._inProgress = false;
}

void room_319_synchronize(Common::Serializer &s) {
	s.syncAsUint32LE(local._animMode);
	s.syncAsUint32LE(local._animFrame);
	s.syncAsUint32LE(local._nextAction1);
	s.syncAsUint32LE(local._nextAction2);
	s.syncAsUint32LE(local._slacheMode);
	s.syncAsUint32LE(local._slacheTopic);
	s.syncAsUint32LE(local._slachePosY);

	s.syncAsByte(local._slacheTalkingFl);
	s.syncAsByte(local._slacheReady);
	s.syncAsByte(local._slacheInitFl);

	Common::String subQuote2 = local._subQuote2;
	s.syncString(subQuote2);
	if (s.isLoading())
		Common::strcpy_s(local._subQuote2, subQuote2.c_str());
}


void room_319_preload() {
	room_init_code_pointer = room_319_init;
	room_parser_code_pointer = room_319_parser;
	room_daemon_code_pointer = room_319_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
