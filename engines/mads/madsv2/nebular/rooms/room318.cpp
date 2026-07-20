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
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section3.h"
#include "mads/madsv2/nebular/rooms/thunks.h"
#include "mads/madsv2/nebular/rooms/dialog.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _dropTimer;
	int32 _lastFrame;
	int32 _animMode;
	int32 _internCounter;
	int32 _counter;
	bool _dialogFl;
	bool _internTalkingFl;
	bool _internWalkingFl;
	bool _internVisibleFl;
	bool _explosionFl;
	int32 _lastFrameCounter = 0;
	Dialog _dialog1;
};

static Scratch local;


static void handleRexDialogs(int quote) {
	_scene->_kernelMessages.reset();

	const char *curQuote = _game.getQuote(quote);
	if (_vm->_font->getWidth(curQuote, _scene->_textSpacing) > 200) {
		static char subQuote1[34], subQuote2[34];
		_game.splitQuote(curQuote, subQuote1, subQuote2);

		_scene->_kernelMessages.add(Common::Point(138, 59), 0x1110, 32, 0, 240, subQuote1);
		_scene->_kernelMessages.add(Common::Point(138, 73), 0x1110, 32, 1, 180, subQuote2);
	} else
		_scene->_kernelMessages.add(Common::Point(138, 73), 0x1110, 32, 1, 120, curQuote);
}

static void handleInternDialog(int quoteId, int quoteNum, uint32 timeout) {
	int height = quoteNum * 14;
	int posY;
	if (height < 85)
		posY = 87 - height;
	else
		posY = 2;

	int curQuoteId = quoteId;

	int maxWidth = 0;
	for (int i = 0; i < quoteNum; i++) {
		maxWidth = MAX(maxWidth, _vm->_font->getWidth(_game.getQuote(curQuoteId), -1));
		curQuoteId++;
	}

	int posX = MIN(319 - maxWidth, 178 - (maxWidth >> 1));
	curQuoteId = quoteId;

	_scene->_kernelMessages.reset();
	local._internTalkingFl = true;

	// WORKAROUND: In case the player launches multiple talk selections with the
	// intern before previous ones have finished, take care of removing any
	int seqIndex;
	while ((seqIndex = _scene->_sequences.findByTrigger(63)) != -1)
		_scene->_sequences.remove(seqIndex);

	for (int i = 0; i < quoteNum; i++) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(180, 63);
		_scene->_kernelMessages.add(Common::Point(posX, posY), 0xFDFC, 0, 0, timeout, _game.getQuote(curQuoteId));
		posY += 14;
		curQuoteId++;
	}
}

static void handleDialog() {
	if (!_game._trigger) {
		_game._player._stepEnabled = false;
		handleRexDialogs(_action._activeAction._verbId);
	} else if (_game._trigger == 2) {
		int synxIdx = _globals._sequenceIndexes[2];
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(142, 121));
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], synxIdx);
		_vm->_sound->command(3);
		_scene->_userInterface.setup(kInputBuildingSentences);
		_game._player._stepEnabled = true;
	} else {
		if (_action._activeAction._verbId < 0x19C)
			local._dialog1.write(_action._activeAction._verbId, false);

		switch (_action._activeAction._verbId) {
		case 0x191:
			handleInternDialog(0x19E, 2, INDEFINITE_TIMEOUT);
			local._dialog1.write(0x192, true);
			break;

		case 0x192:
			handleInternDialog(0x1A0, 5, INDEFINITE_TIMEOUT);
			local._dialog1.write(0x193, true);
			break;

		case 0x193:
			handleInternDialog(0x1A5, 4, INDEFINITE_TIMEOUT);
			local._dialog1.write(0x194, true);
			break;

		case 0x194:
			handleInternDialog(0x1A9, 6, INDEFINITE_TIMEOUT);
			local._dialog1.write(0x195, true);
			local._dialog1.write(0x196, true);
			local._dialog1.write(0x19D, false);
			break;

		case 0x195:
			handleInternDialog(0x1AF, 7, INDEFINITE_TIMEOUT);
			if (!local._dialog1.read(0x196))
				local._dialog1.write(0x197, true);
			break;

		case 0x196:
			handleInternDialog(0x1B6, 5, INDEFINITE_TIMEOUT);
			if (!local._dialog1.read(0x195))
				local._dialog1.write(0x197, true);
			break;

		case 0x197:
			handleInternDialog(0x1BB, 5, INDEFINITE_TIMEOUT);
			break;

		case 0x198:
			handleInternDialog(0x1C0, 5, INDEFINITE_TIMEOUT);
			local._dialog1.write(0x19A, true);
			break;

		case 0x199:
			handleInternDialog(0x1C5, 3, INDEFINITE_TIMEOUT);
			break;

		case 0x19A:
			handleInternDialog(0x1C8, 5, INDEFINITE_TIMEOUT);
			local._dialog1.write(0x19B, true);
			break;

		case 0x19B:
			handleInternDialog(0x1CD, 3, INDEFINITE_TIMEOUT);
			break;

		case 0x19C:
		case 0x19D:
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(142, 121));
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 6, 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);

			local._dialogFl = false;
			handleInternDialog(0x1D0, 1, 120);
			if (local._dialog1.read(0) || (_action._activeAction._verbId == 0x19D)) {
				local._explosionFl = true;
				local._internCounter = 3420;
			}
			break;

		default:
			break;
		}

		if (_action._activeAction._verbId < 0x19C) {
			local._dialog1.start();
			_game._player._stepEnabled = true;
		}

	}
}

static void room_318_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('k', -1));

	if (_globals[kAfterHavoc]) {
		_scene->loadAnimation(formAnimName('f', -1));
		_scene->_animation[0]->_repeatFlag = true;
	} else if (!_globals[kHasSeenProfPyro]) {
		_scene->_hotspots.activate(words_professors_gurney, false);
		_scene->_hotspots.activate(words_professor, false);
		_scene->_hotspots.activate(words_tape_player, false);
	}

	if (_game._objects.isInRoom(OBJ_SCALPEL)) {
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 0, 120);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 4);
		_scene->_dynamicHotspots.add(words_scalpel, words_take, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
	}

	if (_scene->_priorSceneId == 357)
		_game._player._playerPos = Common::Point(15, 110);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(214, 152);

	local._dialog1.setup(0x47, 0x191, 0x192, 0x193, 0x194, 0x195, 0x196, 0x197, 0x198, 0x199, 0x19A, 0x19B, 0x19C, 0x19D, 0);

	if (!_game._visitedScenes._sceneRevisited) {
		local._dialog1.set(0x191, 0x198, 0x199, 0x19C, 0);
		if (_game._widepipeCtr >= 2)
			local._dialog1.write(0x19D, true);
	}

	if (_scene->_priorSceneId == 307) {
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->loadAnimation(formAnimName('a', -1), 60);
		local._animMode = 1;
	}

	local._lastFrame = 0;
	_scene->_hotspots.activate(words_intern, false);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		local._dialogFl = false;
		local._internWalkingFl = false;
		local._counter = 0;
		local._internCounter = 0;
		local._internVisibleFl = true;
		local._explosionFl = false;
	}

	_game.loadQuoteSet(0x18C, 0x18D, 0x18E, 0x18F, 0x191, 0x192, 0x193, 0x194, 0x195, 0x196,
		0x197, 0x198, 0x199, 0x19A, 0x19B, 0x19C, 0x19E, 0x19F, 0x1A0, 0x1A1, 0x1A2, 0x1A3,
		0x1A4, 0x1A5, 0x1A6, 0x1A7, 0x1A8, 0x1A9, 0x1AA, 0x1AB, 0x1AC, 0x1AD, 0x1AE, 0x1AF,
		0x1B0, 0x1B1, 0x1B2, 0x1B3, 0x1B4, 0x1B5, 0x1B6, 0x1B7, 0x1B8, 0x1B9, 0x1BA, 0x1BB,
		0x1BC, 0x1BD, 0x1BE, 0x1BF, 0x1C0, 0x1C1, 0x1C2, 0x1C3, 0x1C4, 0x1C5, 0x1C6, 0x1C7,
		0x1C8, 0x1C9, 0x1CA, 0x1CB, 0x1CC, 0x1CD, 0x1CE, 0x1CF, 0x1D0, 0x1D1, 0x1D2, 0x1D3,
		0x190, 0x19D, 0);

	if ((_scene->_priorSceneId == RETURNING_FROM_DIALOG) || (((_scene->_priorSceneId == 318) ||
		(_scene->_priorSceneId == RETURNING_FROM_LOADING)) && (!_globals[kAfterHavoc]))) {
		if (!_globals[kAfterHavoc]) {
			_game._player._visible = false;
			_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('g', -1));
			local._animMode = 2;

			if (_game._visitedScenes.exists(319) || !local._internVisibleFl) {
				local._internVisibleFl = false;
				local._dialogFl = false;
			} else {
				_scene->loadAnimation(formAnimName('b', -1), 61);
				_scene->_hotspots.activate(words_intern, true);
			}

			if (local._dialogFl) {
				local._dialog1.start();
				_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 8);
			} else
				_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);

			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(142, 121));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		}
	}

	if (_scene->_priorSceneId == 319) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		local._animMode = 4;
		if (!_globals[kHasSeenProfPyro]) {
			_scene->loadAnimation(formAnimName('d', -1), 64);
			_globals[kHasSeenProfPyro] = true;
		} else {
			_scene->loadAnimation(formAnimName('e', -1), 64);
		}
	}

	local._internTalkingFl = false;
	_vm->_palette->setEntry(252, 63, 63, 10);
	_vm->_palette->setEntry(253, 45, 45, 05);

	local._dropTimer = _vm->_game->_scene._frameStartTime;
	section_3_music();

	if (local._dialogFl)
		_vm->_sound->command(15);
}

static void room_318_daemon() {
	if ((_scene->_animation[0] != nullptr) && (local._animMode == 2)) {
		if (local._lastFrame != _scene->_animation[0]->getCurrentFrame()) {
			local._lastFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			switch (local._lastFrame) {
			case 20:
			case 30:
			case 40:
			case 50:
			case 60:
			case 70:
			case 80:
			case 90:
			case 100:
			case 110:
			case 120:
			case 130:
			case 140:
			case 149:
				if (local._internWalkingFl) {
					nextFrame = 149;
				} else if (local._internTalkingFl) {
					nextFrame = 149;
				} else if (local._lastFrame == 149) {
					nextFrame = 4;
				}
				break;

			case 151:
				if (local._internWalkingFl)
					nextFrame = 183;
				break;

			case 167:
			case 184:
				if (local._internWalkingFl) {
					nextFrame = 184;
				} else if (!local._internTalkingFl) {
					nextFrame = 0;
				} else if (_vm->getRandomNumber(1, 100) <= 50) {
					nextFrame = 151;
				} else {
					nextFrame = 167;
				}

				if (nextFrame == 184) {
					handleInternDialog(0x1D1, 3, 240);
					_scene->_hotspots.activate(words_intern, false);
					local._internVisibleFl = false;
				}
				break;

			default:
				break;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				local._lastFrame = nextFrame;
			}
		}
	}

	switch (_game._trigger) {
	case 60:
		_vm->_sound->command(3);
		local._animMode = 2;
		_scene->_reloadSceneFlag = true;
		break;

	case 61:
		local._counter = 0;
		break;

	case 62:
		_scene->_nextSceneId = 319;
		break;

	case 63:
		local._internTalkingFl = false;
		break;

	case 64:
		_vm->_sound->command(3);
		_scene->_nextSceneId = 307;
		break;

	default:
		break;
	}

	uint32 tmpFrame = _vm->_events->getFrameCounter();
	long diffFrame = tmpFrame - local._lastFrameCounter;
	local._lastFrameCounter = tmpFrame;

	if ((local._animMode == 2) && !local._internVisibleFl && _game._player._stepEnabled) {
		if ((diffFrame >= 0) && (diffFrame <= 4))
			local._counter += diffFrame;
		else
			local._counter++;

		int extraCounter = _game._objects.isInInventory(OBJ_SCALPEL) ? 900 : 0;

		if (local._counter + extraCounter >= 1800) {
			_scene->freeAnimation();
			_game._player._stepEnabled = false;
			_scene->loadAnimation(formAnimName('c', -1), 62);
			local._animMode = 3;
		}
	} else if ((local._animMode == 2) && local._explosionFl && local._internVisibleFl && !local._dialogFl
		&& !local._internWalkingFl && (_game._screenObjects._inputMode != kInputConversation)) {
		if ((diffFrame >= 0) && (diffFrame <= 4))
			local._internCounter += diffFrame;
		else
			local._internCounter++;

		if (local._internCounter >= 3600) {
			_vm->_sound->command(59);
			_vm->_screen->_shakeCountdown = 20;
			local._internWalkingFl = true;
		}
	}

	if ((_vm->_game->_scene._frameStartTime - local._dropTimer) > 600) {
		_vm->_sound->command(51);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 14, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		local._dropTimer = _vm->_game->_scene._frameStartTime;
	}
}

static void room_318_pre_parser() {
	if (_game._player._needToWalk)
		_game._player._needToWalk = _game._player._visible;

	if (player_said_2(walk_down, corridor_to_west))
		_game._player._walkOffScreenSceneId = 357;
}

static void room_318_parser() {
	if (_game._screenObjects._inputMode == kInputConversation) {
		handleDialog();
		_action._inProgress = false;
		return;
	}

	if (player_said_2(talkto, intern)) {
		switch (_game._trigger) {
		case 0:
		{
			local._dialogFl = true;
			_vm->_sound->command(15);
			_game._player._stepEnabled = false;
			handleRexDialogs(_vm->getRandomNumber(0x18C, 0x18E));

			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 80);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(142, 121));
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 6, 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		}
		break;

		case 1:
			_game._player._stepEnabled = true;
			handleInternDialog(0x18F, 1, INDEFINITE_TIMEOUT);
			local._dialog1.start();
			break;

		case 2:
		{
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 8);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(142, 121));
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
		}
		break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (player_said_2(take, scalpel) && (_game._objects.isInRoom(OBJ_SCALPEL) || _game._trigger)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 8, 2, 0, 80);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(142, 121));
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 2, 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 5, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			if (local._internVisibleFl)
				handleInternDialog(0x190, 1, 120);
			else {
				_game._objects.addToInventory(OBJ_SCALPEL);
				_vm->_dialogs->showItem(OBJ_SCALPEL, 0x7C5D);
				_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			}
			break;

		case 2:
		{
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(142, 121));
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.addTimer(60, 3);
		}
		break;

		case 3:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_game._player._visible) {
		if (player_said_2(walk_down, corridor_to_south)) {
			_scene->_nextSceneId = 407;
			_action._inProgress = false;
			return;
		}

		if (player_said_2(take, tape_player)) {
			if (_game._objects.isInRoom(OBJ_AUDIO_TAPE)) {
				_vm->_dialogs->showItem(OBJ_AUDIO_TAPE, 0x7C5B);
				_game._objects.addToInventory(OBJ_AUDIO_TAPE);
			} else
				_vm->_dialogs->show(31834);

			_action._inProgress = false;
			return;
		}

		if (player_said_2(look, tape_player)) {
			if (_game._objects.isInRoom(OBJ_AUDIO_TAPE))
				_vm->_dialogs->show(31833);
			else
				_vm->_dialogs->show(31834);

			_action._inProgress = false;
			return;
		}

		if (player_said_2(walk_into, doctors_office)) {
			_vm->_dialogs->show(31831);
			_action._inProgress = false;
			return;
		}

		if (player_said_2(look, gurney)) {
			_vm->_dialogs->show(31823);
			_action._inProgress = false;
			return;
		}

		if (player_said_2(look, instrument_table)) {
			_vm->_dialogs->show(31825);
			_action._inProgress = false;
			return;
		}
	} else { // Not visible
		if (player_said_2(look, gurney)) {
			_vm->_dialogs->show(31822);
			_action._inProgress = false;
			return;
		}

		if (player_said_2(look, instrument_table)) {
			_vm->_dialogs->show(31824);
			_action._inProgress = false;
			return;
		}
	}

	if (player_said_2(look, wall))
		_vm->_dialogs->show(31810);
	else if (player_said_2(look, floor))
		_vm->_dialogs->show(31811);
	else if (player_said_2(look, corridor_to_west))
		_vm->_dialogs->show(31812);
	else if (player_said_2(look, corridor_to_south))
		_vm->_dialogs->show(31813);
	else if (player_said_2(look, faucet))
		_vm->_dialogs->show(31814);
	else if (player_said_2(look, sink))
		_vm->_dialogs->show(31815);
	else if (player_said_2(look, conveyor_belt))
		_vm->_dialogs->show(31816);
	else if (player_said_2(look, large_blade))
		_vm->_dialogs->show(31817);
	else if (player_said_2(look, monitor))
		_vm->_dialogs->show(31818);
	else if (player_said_2(look, cabinets))
		_vm->_dialogs->show(31819);
	else if (player_said_2(look, equipment))
		_vm->_dialogs->show(31820);
	else if (player_said_2(look, shelf))
		_vm->_dialogs->show(31821);
	else if (player_said_2(open, cabinets))
		_vm->_dialogs->show(31829);
	else if (player_said_2(look, intern))
		_vm->_dialogs->show(31830);
	else if (player_said_2(look, professor))
		_vm->_dialogs->show(31832);
	else if (player_said_2(look, professors_gurney))
		_vm->_dialogs->show(31836);
	else if (_action._lookFlag) {
		if (_game._player._visible || _game._objects.isInInventory(OBJ_SCALPEL))
			_vm->_dialogs->show(31828);
		else if (local._internVisibleFl)
			_vm->_dialogs->show(31826);
		else
			_vm->_dialogs->show(31827);
	} else
		return;

	_action._inProgress = false;
}

void room_318_synchronize(Common::Serializer &s) {
	s.syncAsUint32LE(local._dropTimer);
	s.syncAsSint32LE(local._lastFrame);
	s.syncAsSint32LE(local._animMode);
	s.syncAsSint32LE(local._internCounter);
	s.syncAsSint32LE(local._counter);

	s.syncAsByte(local._dialogFl);
	s.syncAsByte(local._internTalkingFl);
	s.syncAsByte(local._internWalkingFl);
	s.syncAsByte(local._internVisibleFl);
	s.syncAsByte(local._explosionFl);
	s.syncAsUint32LE(local._lastFrameCounter);
}

void room_318_preload() {
	room_init_code_pointer = room_318_init;
	room_pre_parser_code_pointer = room_318_pre_parser;
	room_parser_code_pointer = room_318_parser;
	room_daemon_code_pointer = room_318_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
