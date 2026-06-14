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
#include "mads/madsv2/nebular/rooms/forcefield.h"
#include "mads/madsv2/nebular/rooms/thunks.h"
#include "mads/madsv2/nebular/rooms/conversation.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _afterPeeingFl;
	bool _duringPeeingFl;
	bool _grateOpenedFl;
	bool _activePrisonerFl;
 	int32 _animationMode;
	int32 _prisonerMessageId;
	int32 _fieldCollisionCounter;
	int32 _lastFrameTime;
	int32 _guardTime;
	int32 _prisonerTimer;
	char _subQuote2[256];

	Conversation _dialog1;
	Conversation _dialog2;
	Forcefield _forcefield;
};

static Scratch local;


static void handleRexDialog(int quote) {
	Common::String curQuote = _game.getQuote(quote);
	if (_vm->_font->getWidth(curQuote, _scene->_textSpacing) > 200) {
		Common::String subQuote1, subQuote2;
		_game.splitQuote(curQuote, subQuote1, subQuote2);
		Common::strcpy_s(local._subQuote2, subQuote2.c_str());

		_scene->_kernelMessages.add(Common::Point(0, -14), 0x1110, 34, 0, 240, subQuote1);
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 1, 180, local._subQuote2);
	} else
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 1, 120, curQuote);
}

static void handlePrisonerSpeech(int firstQuoteId, int number, uint32 timeout) {
	int height = number * 14;
	int posY;

	if (height < 60)
		posY = 65 - height;
	else
		posY = 78 - (height / 2);

	_scene->_kernelMessages.reset();
	local._activePrisonerFl = true;

	int quoteId = firstQuoteId;
	for (int count = 0; count < number; count++) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_kernelMessages.add(Common::Point(5, posY), 0xFDFC, 0, 81, timeout, _game.getQuote(quoteId));
		posY += 14;
		quoteId++;
	}
}

static void setDialogNode(int node) {
	switch (node) {
	case 0:
		handlePrisonerSpeech(0x153, 2, 120);
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	case 1:
		_globals[kMetBuddyBeast] = true;
		handlePrisonerSpeech(0x10F, 2, INDEFINITE_TIMEOUT);
		local._dialog1.start();
		break;

	case 2:
		_globals[kMetBuddyBeast] = true;
		handlePrisonerSpeech(0x111, 2, INDEFINITE_TIMEOUT);
		local._dialog1.start();
		break;

	case 4:
		handlePrisonerSpeech(0x116, 1, 120);
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	case 5:
		_globals[kKnowsBuddyBeast] = true;
		handlePrisonerSpeech(0x117, 2, INDEFINITE_TIMEOUT);
		local._dialog2.start();
		break;

	case 6:
		handlePrisonerSpeech(0x123, 1, 120);
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	case 7:
		_globals[kKnowsBuddyBeast] = true;
		handlePrisonerSpeech(0x124, 10, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x11A, false);
		local._dialog2.write(0x11B, true);
		local._dialog2.write(0x120, true);
		local._dialog2.start();
		break;

	case 8:
		handlePrisonerSpeech(0x12E, 6, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x11A, false);
		local._dialog2.write(0x11B, false);
		local._dialog2.write(0x11C, true);
		local._dialog2.write(0x11D, true);
		local._dialog2.write(0x11F, true);
		local._dialog2.start();
		break;

	case 9:
		handlePrisonerSpeech(0x134, 4, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x11D, false);
		local._dialog2.start();
		break;

	case 10:
		handlePrisonerSpeech(0x138, 6, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x11E, false);
		local._dialog2.start();
		break;

	case 11:
		handlePrisonerSpeech(0x13E, 6, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x11F, false);
		local._dialog2.write(0x121, true);
		local._dialog2.start();
		break;

	case 12:
		handlePrisonerSpeech(0x144, 4, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x11C, false);
		local._dialog2.start();
		break;

	case 13:
		handlePrisonerSpeech(0x148, 7, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x120, false);
		local._dialog2.start();
		break;

	case 14:
		handlePrisonerSpeech(0x14F, 3, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x121, false);
		local._dialog2.start();
		break;

	case 15:
		handlePrisonerSpeech(0x152, 1, 120);
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	case 16:
		_globals[kKnowsBuddyBeast] = true;
		handlePrisonerSpeech(0x10C, 1, INDEFINITE_TIMEOUT);
		local._dialog2.start();
		break;

	default:
		break;
	}
}

static void handlePrisonerEncounter() {
	switch (_action._activeAction._verbId) {
	case 275:
		setDialogNode(5);
		break;

	case 277:
		setDialogNode(4);
		break;

	case 276:
		setDialogNode(6);
		break;

	default:
		break;
	}
}

static void handlePrisonerDialog() {
	switch (_action._activeAction._verbId) {
	case 0x11A:
		setDialogNode(7);
		break;

	case 0x11B:
		setDialogNode(8);
		break;

	case 0x11C:
		setDialogNode(12);
		break;

	case 0x11D:
		setDialogNode(9);
		break;

	case 0x11E:
		setDialogNode(10);
		break;

	case 0x11F:
		setDialogNode(11);
		break;

	case 0x120:
		setDialogNode(13);
		break;

	case 0x121:
		setDialogNode(14);
		break;

	case 0x122:
		setDialogNode(15);
		break;

	default:
		break;
	}
}

static void handleDialog() {
	if (_game._trigger == 0) {
		_scene->_kernelMessages.reset();
		_game._player._stepEnabled = false;
		handleRexDialog(_action._activeAction._verbId);
	} else {
		_game._player._stepEnabled = true;

		if (!_globals[kKnowsBuddyBeast]) {
			handlePrisonerEncounter();
		} else {
			handlePrisonerDialog();
		}
	}
}

static void room_307_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites("*SC003x0");
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites("*SC003x1");
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*SC003x2");
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 0));

	init_forcefield(&local._forcefield, true);

	_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);

	local._animationMode = 0;
	local._fieldCollisionCounter = 0;

	_scene->changeVariant(1);

	_game.loadQuoteSet(0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0x10C, 0x104, 0x106, 0x107, 0x108, 0x105,
		0x109, 0x10A, 0x10B, 0x10D, 0x10E, 0x10F, 0x110, 0x111, 0x112, 0x113, 0x114, 0x115, 0x116, 0x117,
		0x118, 0x119, 0x11A, 0x11B, 0x11C, 0x11D, 0x11E, 0x11F, 0x120, 0x121, 0x122, 0x123, 0x124, 0x125,
		0x126, 0x127, 0x128, 0x129, 0x12A, 0x12B, 0x12C, 0x12D, 0x12E, 0x12F, 0x130, 0x131, 0x132, 0x133,
		0x134, 0x135, 0x136, 0x137, 0x138, 0x139, 0x13A, 0x13B, 0x13C, 0x13D, 0x13E, 0x13F, 0x140, 0x141,
		0x142, 0x143, 0x144, 0x145, 0x146, 0x147, 0x148, 0x149, 0x14A, 0x14B, 0x14C, 0x14D, 0x14E, 0x14F,
		0x150, 0x151, 0x152, 0x153, 0);

	local._dialog1.setup(0x3F, 0x113, 0x114, 0x115, -1);
	local._dialog2.setup(0x40, 0x11A, 0x11B, 0x11C, 0x11D, 0x11E, 0x11F, 0x120, 0x121, 0x122, 0);

	if (!_game._visitedScenes._sceneRevisited)
		local._dialog2.set(0x11A, 0x122, 0);
	else if (_scene->_priorSceneId == 318)
		local._dialog2.write(0x11E, true);


	if (_scene->_priorSceneId == RETURNING_FROM_DIALOG) {
		if (local._grateOpenedFl)
			_vm->_sound->command(10);
		else
			_vm->_sound->command(3);
	} else {
		local._afterPeeingFl = false;
		local._duringPeeingFl = false;
		local._guardTime = 0;
		local._grateOpenedFl = false;
		local._activePrisonerFl = false;
		local._prisonerTimer = 0;
		local._prisonerMessageId = 0x104;

		if (_scene->_priorSceneId == 308) {
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			_game._player._playerPos = Common::Point(156, 113);
			_game._player._facing = FACING_NORTH;
			local._animationMode = 1;
			_vm->_sound->command(11);
			_scene->loadAnimation(formAnimName('a', -1), 60);
		} else if (_scene->_priorSceneId == 387) {
			_game._player._playerPos = Common::Point(129, 108);
			_game._player._facing = FACING_NORTH;
			_vm->_sound->command(3);
			local._grateOpenedFl = true;
		} else {
			_game._player._playerPos = Common::Point(159, 109);
			_game._player._facing = FACING_SOUTH;
			_vm->_sound->command(3);
		}
	}

	if (local._grateOpenedFl) {
		_scene->_hotspots.activate(17, false);

		int idx = _scene->_dynamicHotspots.add(17, VERB_CLIMB_INTO, -1, Common::Rect(117, 67, 117 + 19, 67 + 13));
		int hotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(129, 104), FACING_NORTH);
		_scene->_dynamicHotspots.setCursor(hotspotId, CURSOR_GO_UP);

		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);
	}

	_vm->_palette->setEntry(252, 63, 30, 20);
	_vm->_palette->setEntry(253, 45, 15, 12);

	section_3_music();

	if ((_scene->_priorSceneId == 318) || (_scene->_priorSceneId == 387))
		_scene->_kernelMessages.addQuote(0xF3, 0, 120);
}

static void room_307_daemon() {
	handle_forcefield(&local._forcefield, &_globals._spriteIndexes[0]);

	if ((local._animationMode == 1) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() == 126) {
			local._forcefield._flag = false;
			_vm->_sound->command(5);
		}

		if (_scene->_animation[0]->getCurrentFrame() == 194) {
			local._forcefield._flag = true;
			_vm->_sound->command(24);
		}
	}

	if ((local._animationMode == 2) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() == 54)
			local._forcefield._flag = false;

		if (_scene->_animation[0]->getCurrentFrame() == 150) {
			_game._player._visible = false;
			_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		}
	}

	if (_game._trigger == 60) {
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		local._animationMode = 0;
		_vm->_sound->command(9);
	}

	if ((local._lastFrameTime != _scene->_frameStartTime) && !local._duringPeeingFl) {
		int32 elapsedTime = local._lastFrameTime - _scene->_frameStartTime;
		if ((elapsedTime > 0) && (elapsedTime <= 4)) {
			local._guardTime += elapsedTime;
			local._prisonerTimer += elapsedTime;
		} else {
			local._guardTime++;
			local._prisonerTimer++;
		}
		local._lastFrameTime = _scene->_frameStartTime;

		if ((local._guardTime > 3000) && !local._duringPeeingFl && (_scene->_animation[0] == nullptr)
			&& (_game._screenObjects._inputMode != kInputConversation) && _globals[kMetBuddyBeast] && !local._activePrisonerFl) {
			if (!_game._objects.isInInventory(OBJ_SCALPEL) && !local._grateOpenedFl) {
				_game._player._stepEnabled = false;
				_game._player.walk(Common::Point(151, 119), FACING_SOUTHEAST);
				local._animationMode = 2;
				_vm->_sound->command(11);
				_scene->loadAnimation(formAnimName('b', -1), 70);
			}
			local._guardTime = 0;
		} else if ((local._prisonerTimer > 300) && (_game._screenObjects._inputMode != kInputConversation) && (_scene->_animation[0] == nullptr) && !local._activePrisonerFl) {
			if (!_globals[kMetBuddyBeast]) {
				if (local._prisonerMessageId == -1)
					local._prisonerMessageId = 0x104;

				int idx = _scene->_kernelMessages.add(Common::Point(5, 51), 0xFDFC, 0, 81, 120, _game.getQuote(local._prisonerMessageId));
				_scene->_kernelMessages.setQuoted(idx, 4, true);
				local._prisonerMessageId++;
				if (local._prisonerMessageId > 0x10A)
					local._prisonerMessageId = 0x104;
			} else if (_globals[kKnowsBuddyBeast] && (local._dialog2.read(0) > 1) && (_vm->getRandomNumber(1, 3) == 1)) {
				int idx = _scene->_kernelMessages.add(Common::Point(5, 51), 0xFDFC, 0, 81, 120, _game.getQuote(267));
				_scene->_kernelMessages.setQuoted(idx, 4, true);
			}
			local._prisonerTimer = 0;
		}
	}

	if (_game._trigger == 70)
		_scene->_nextSceneId = 318;

	if (_game._trigger == 81) {
		local._prisonerTimer = 0;
		if (local._activePrisonerFl && (local._guardTime > 2600))
			local._guardTime = 3000 - _vm->getRandomNumber(1, 800);

		local._activePrisonerFl = false;
	}
}

static void room_307_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(30715);
	else if (_game._screenObjects._inputMode == kInputConversation)
		handleDialog();
	else if (_action.isAction(VERB_TALKTO, NOUN_CELL_WALL) || _action.isAction(VERB_TALKTO, NOUN_WALL) || _action.isAction(VERB_TALKTO, NOUN_TOILET)) {
		int node, say;
		if (_globals[kKnowsBuddyBeast]) {
			say = 0x10E;
			node = 16;
		} else if (_globals[kMetBuddyBeast]) {
			say = 0x10E;
			node = 2;
		} else {
			say = 0x10D;
			node = 1;
		}

		switch (_game._trigger) {
		case 0:
			handleRexDialog(say);
			break;

		case 1:
			setDialogNode(node);
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PRY, NOUN_SCALPEL, NOUN_AIR_VENT)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(239));
			_scene->_sequences.addTimer(120, 1);
			break;

		case 1:
			_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXCL_8");
			_game._player._visible = false;
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, 3);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
		{
			int oldIdx = _globals._sequenceIndexes[5];
			_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 12, 6, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 2, 3);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], oldIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		}
		break;

		case 3:
		{
			int oldIdx = _globals._sequenceIndexes[5];
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], oldIdx);
			_scene->_sequences.addTimer(48, 4);
		}
		break;

		case 4:
			_vm->_sound->command(26);
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);
			_scene->_sequences.addTimer(90, 5);
			break;

		case 5:
			_vm->_sound->command(10);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(241));
			_scene->_sequences.addTimer(120, 6);
			break;

		case 6:
		{
			_game._player._visible = true;
			_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
			_scene->_sequences.remove(_globals._sequenceIndexes[5]);
			local._grateOpenedFl = true;
			_scene->_hotspots.activate(17, false);
			int idx = _scene->_dynamicHotspots.add(17, VERB_CLIMB_INTO, -1, Common::Rect(117, 67, 117 + 19, 67 + 13));
			int hotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(129, 104), FACING_NORTH);
			_scene->_dynamicHotspots.setCursor(hotspotId, CURSOR_GO_UP);
			_game._objects.removeFromInventory(OBJ_SCALPEL, NOWHERE);
			_scene->_kernelMessages.addQuote(0xF2, 7, 120);
		}
		break;

		case 7:
			_scene->_sprites.remove(_globals._spriteIndexes[5]);
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_CLIMB_INTO, NOUN_AIR_VENT)) {
		if (local._grateOpenedFl) {
			switch (_game._trigger) {
			case 0:
				_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXCL_8");
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[4]);
				_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 60, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 3, -2);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);
				_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 18, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], -1, 4);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 15);
				break;

			case 2:
			{
				int oldIdx = _globals._sequenceIndexes[5];
				_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 12, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 4, 10);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], oldIdx);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			}
			break;

			case 3:
				_scene->_sequences.remove(_globals._sequenceIndexes[4]);
				_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 3);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
				_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 11);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[5], Common::Point(129, 102));
				_scene->_sequences.addTimer(48, 4);
				break;

			case 4:
				_scene->_sequences.remove(_globals._sequenceIndexes[4]);
				_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 2);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 12, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 12, 14);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[5], Common::Point(129, 102));
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
				break;

			case 5:
				_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 15);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[5], Common::Point(129, 102));
				_scene->_sequences.addTimer(48, 6);
				break;

			case 6:
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_scene->_sequences.remove(_globals._sequenceIndexes[4]);
				_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 1);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
				_scene->_sequences.addTimer(48, 7);
				break;

			case 7:
				_scene->_nextSceneId = 313;
				break;

			default:
				break;
			}
		}
	} else if (_action.isAction(VERB_USE, NOUN_TOILET) && (_game._storyMode != STORYMODE_NAUGHTY))
		_vm->_dialogs->show(30723);
	else if (_action.isAction(VERB_USE, NOUN_TOILET)) {
		if (!local._afterPeeingFl) {
			switch (_game._trigger) {
			case 0:
				_vm->_sound->command(25);
				_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 0));
				local._duringPeeingFl = true;
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 9, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, 2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 9, 5, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 3, -2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 9);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);
				_game._player._visible = true;
				_scene->_sequences.addTimer(48, 3);
				break;

			case 3:
			{
				_scene->_sprites.remove(_globals._spriteIndexes[3]);
				_scene->_kernelMessages.reset();
				int idx = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 4, 120, _game.getQuote(237));
				_scene->_kernelMessages.setQuoted(idx, 4, true);
			}
			break;

			case 4:
				_game._player._stepEnabled = true;
				local._duringPeeingFl = false;
				local._afterPeeingFl = true;
				break;

			default:
				break;
			}
		} else {
			_scene->_kernelMessages.reset();
			int idx = _scene->_kernelMessages.add(Common::Point(85, 39), 0x1110, 0, 0, 180, _game.getQuote(238));
			_scene->_kernelMessages.setQuoted(idx, 4, true);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_AIR_VENT)) {
		if (!local._grateOpenedFl)
			_vm->_dialogs->show(30710);
		else
			_vm->_dialogs->show(30711);
	} else if (_action.isAction(VERB_LOOK, NOUN_BED))
		_vm->_dialogs->show(30712);
	else if (_action.isAction(VERB_LOOK, NOUN_SINK))
		_vm->_dialogs->show(30713);
	else if (_action.isAction(VERB_LOOK, NOUN_TOILET))
		_vm->_dialogs->show(30714);
	else if (_action.isAction(VERB_SHARPEN, NOUN_SCALPEL))
		_vm->_dialogs->show(30716);
	else if (_action.isAction(VERB_LOOK, NOUN_CELL_WALL))
		_vm->_dialogs->show(30717);
	else if (_action.isAction(VERB_LOOK, NOUN_LIGHT))
		_vm->_dialogs->show(30718);
	else if (_action.isAction(VERB_WALK_INTO, NOUN_CORRIDOR)) {
		switch (local._fieldCollisionCounter) {
		case 0:
			_vm->_dialogs->show(30719);
			local._fieldCollisionCounter = 1;
			break;

		case 1:
			_vm->_dialogs->show(30720);
			local._fieldCollisionCounter = 2;
			break;

		case 2:
			_vm->_dialogs->show(30721);
			local._fieldCollisionCounter = 3;
			break;

		case 3:
			_vm->_dialogs->show(30722);
			break;

		default:
			break;
		}
	} else
		return;

	_action._inProgress = false;
}

void room_307_synchronize(Common::Serializer &s) {
	local._forcefield.synchronize(s);

	s.syncAsByte(local._afterPeeingFl);
	s.syncAsByte(local._duringPeeingFl);
	s.syncAsByte(local._grateOpenedFl);
	s.syncAsByte(local._activePrisonerFl);

	s.syncAsSint32LE(local._animationMode);
	s.syncAsSint32LE(local._prisonerMessageId);
	s.syncAsSint32LE(local._fieldCollisionCounter);

	s.syncAsUint32LE(local._lastFrameTime);
	s.syncAsUint32LE(local._guardTime);
	s.syncAsUint32LE(local._prisonerTimer);

	Common::String subQuote2 = local._subQuote2;
	s.syncString(subQuote2);
	if (s.isLoading())
		Common::strcpy_s(local._subQuote2, subQuote2.c_str());
}

void room_307_preload() {
	local._forcefield.init();

	room_init_code_pointer = room_307_init;
	room_parser_code_pointer = room_307_parser;
	room_daemon_code_pointer = room_307_daemon;

	section_3_walker();
	section_3_interface();
	_scene->addActiveVocab(NOUN_AIR_VENT);
	_scene->addActiveVocab(VERB_CLIMB_INTO);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
