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

#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section4.h"
#include "mads/nebular/rooms/dialog.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _lightOn;
	bool _blowingSmoke;
	bool _leftWomanMoving;
	bool _rightWomanMoving;
	bool _firstTalkToGirlInChair;
	bool _waitingGinnyMove;
	bool _ginnyLooking;
	bool _bigBeatFl;
	bool _roxOnStool;
	bool _bartenderSteady;
	bool _bartenderHandsHips;
	bool _bartenderLooksLeft;
	bool _bartenderReady;
	bool _bartenderTalking;
	bool _bartenderCalled;
	bool _conversationFl;
	bool _activeTeleporter;
	bool _activeArrows;
	bool _activeArrow1;
	bool _activeArrow2;
	bool _activeArrow3;
	bool _cutSceneReady;
	bool _cutSceneNeeded;
	bool _helgaReady;
	bool _refuseAlienLiquor;

	int32 _drinkTimer;
	int32 _beatCounter;
	int32 _bartenderMode;
	int32 _bartenderDialogNode;
	int32 _bartenderCurrentQuestion;
	int32 _helgaTalkMode;
	int32 _roxMode;
	int32 _rexMode;
	int32 _talkTimer;

	Dialog _dialog1;
	Dialog _dialog2;
	Dialog _dialog3;
	Dialog _dialog4;
};

static Scratch local;


static void setDialogNode(int node) {
	if (node > 0)
		local._bartenderDialogNode = node;

	_game._player._stepEnabled = true;

	switch (node) {
	case 0:
		_scene->_userInterface.setup(kInputBuildingSentences);
		local._conversationFl = false;
		local._bartenderDialogNode = 0;
		break;

	case 1:
		local._dialog1.start();
		local._bartenderDialogNode = 1;
		break;

	case 2:
		local._dialog2.start();
		local._bartenderDialogNode = 2;
		break;

	case 3:
		local._dialog3.start();
		local._bartenderDialogNode = 3;
		break;

	case 4:
		local._dialog4.start();
		local._bartenderDialogNode = 4;
		break;

	default:
		break;
	}
}

static void handleConversation1() {
	switch (_action._activeAction._verbId) {
	case 0x214:
	{
		int quoteId = 0;
		int quotePosX = 0;
		switch (_vm->getRandomNumber(1, 3)) {
		case 1:
			quoteId = 0x1E4;
			local._bartenderCurrentQuestion = 4;
			quotePosX = 205;
			break;

		case 2:
			quoteId = 0x1E5;
			local._bartenderCurrentQuestion = 5;
			quotePosX = 203;
			break;

		case 3:
			quoteId = 0x1E6;
			local._bartenderCurrentQuestion = 6;
			quotePosX = 260;
			break;

		default:
			break;
		}
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(quotePosX, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(quoteId));
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(1, 100);
		local._talkTimer = 120;
		setDialogNode(2);
	}
	break;

	case 0x215:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(260, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1EC));
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(1, 100);
		local._talkTimer = 120;
		local._bartenderCurrentQuestion = 1;
		setDialogNode(3);
		break;

	case 0x237:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(208, 41), 0xFDFC, 0, 0, 100, _game.getQuote(0x1FD));
		setDialogNode(0);
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(1, 100);
		local._talkTimer = 1120;
		break;

	default:
		break;
	}
}

static void handleConversation2() {
	switch (_action._activeAction._verbId) {
	case 0x216:
		local._dialog2.write(0x216, false);
		local._dialog2.write(0x21D, true);
		break;

	case 0x219:
		local._dialog2.write(0x219, false);
		local._dialog2.write(0x220, true);
		break;

	case 0x21A:
		local._dialog2.write(0x21A, false);
		local._dialog2.write(0x223, true);
		break;

	case 0x21B:
		local._dialog2.write(0x21B, false);
		local._dialog2.write(0x224, true);
		break;

	case 0x21D:
		local._dialog2.write(0x21D, false);
		local._dialog2.write(0x227, true);
		break;

	case 0x220:
		local._dialog2.write(0x220, false);
		local._dialog2.write(0x22A, true);
		break;

	case 0x223:
		local._dialog2.write(0x223, false);
		local._dialog2.write(0x22D, true);
		break;

	case 0x224:
		local._dialog2.write(0x224, false);
		local._dialog2.write(0x230, true);
		break;

	case 0x227:
		local._dialog2.write(0x227, false);
		break;

	case 0x22A:
		local._dialog2.write(0x22A, false);
		break;

	case 0x22D:
		local._dialog2.write(0x22D, false);
		break;

	case 0x230:
		local._dialog2.write(0x230, false);
		break;

	case 0x21C:
		setDialogNode(0);
		break;

	default:
		break;

	}

	if (_action._activeAction._verbId != 0x21C) {
		switch (_vm->getRandomNumber(1, 3)) {
		case 1:
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addTimer(1, 100);
			local._talkTimer = 180;
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(198, 27), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1E7));
			_scene->_kernelMessages.add(Common::Point(201, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1E8));
			local._bartenderCurrentQuestion = 7;
			break;

		case 2:
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addTimer(1, 100);
			local._talkTimer = 180;
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(220, 27), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1E9));
			_scene->_kernelMessages.add(Common::Point(190, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1EA));
			local._bartenderCurrentQuestion = 8;
			break;

		case 3:
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addTimer(1, 100);
			local._talkTimer = 150;
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(196, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1EB));
			local._bartenderCurrentQuestion = 9;
			break;

		default:
			break;
		}
		local._dialog2.start();
	} else {
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(208, 41), 0xFDFC, 0, 0, 100, _game.getQuote(0x1FD));
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(1, 100);
		local._talkTimer = 1120;
	}
}

static void handleConversation3() {
	switch (_action._activeAction._verbId) {
	case 0x233:
	case 0x234:
	case 0x235:
	case 0x236:
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(1, 86);
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(188, 27), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1ED));
		_scene->_kernelMessages.add(Common::Point(199, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1EE));
		setDialogNode(4);
		local._bartenderCurrentQuestion = 2;
		break;

	case 0x237:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(208, 41), 0xFDFC, 0, 0, 100, _game.getQuote(0x1FD));
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(1, 100);
		local._talkTimer = 1120;
		setDialogNode(0);
		break;

	default:
		break;
	}
}

static void handleConversation4() {
	switch (_action._activeAction._verbId) {
	case 0x238:
		_scene->_kernelMessages.reset();
		setDialogNode(0);
		_game._player._stepEnabled = false;
		_scene->_kernelMessages.add(Common::Point(196, 13), 0xFDFC, 0, 0, 180, _game.getQuote(0x1F0));
		_scene->_kernelMessages.add(Common::Point(184, 27), 0xFDFC, 0, 0, 180, _game.getQuote(0x1F1));
		_scene->_kernelMessages.add(Common::Point(200, 41), 0xFDFC, 0, 0, 180, _game.getQuote(0x1F2));
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(1, 100);
		local._talkTimer = 1100;
		local._dialog4.write(0x238, false);
		local._bartenderMode = 22;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(100, 95);
		local._refuseAlienLiquor = true;
		break;

	case 0x239:
		_game._player._stepEnabled = false;
		local._roxMode = 21;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(1, 92);
		_scene->_userInterface.setup(kInputBuildingSentences);
		local._conversationFl = false;
		break;

	case 0x23A:
		setDialogNode(0);
		_game._player._stepEnabled = false;
		_scene->_kernelMessages.add(Common::Point(193, 27), 0xFDFC, 0, 0, 150, _game.getQuote(0x1F4));
		_scene->_kernelMessages.add(Common::Point(230, 41), 0xFDFC, 0, 0, 150, _game.getQuote(0x1F5));
		local._dialog4.write(0x23A, false);
		_globals[kHasSaidTimer] = true;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(1, 100);
		local._talkTimer = 1100;
		local._bartenderMode = 22;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(100, 95);
		local._refuseAlienLiquor = true;
		break;

	case 0x23D:
		setDialogNode(0);
		_game._player._stepEnabled = false;
		_scene->_kernelMessages.add(Common::Point(153, 27), 0xFDFC, 0, 0, 150, _game.getQuote(0x1F6));
		_scene->_kernelMessages.add(Common::Point(230, 41), 0xFDFC, 0, 0, 150, _game.getQuote(0x1F7));
		local._dialog4.write(0x23D, false);
		_globals[kHasSaidBinocs] = true;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(1, 100);
		local._talkTimer = 1100;
		local._bartenderMode = 22;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(100, 95);
		local._refuseAlienLiquor = true;
		break;

	case 0x23E:
		_scene->_kernelMessages.reset();
		setDialogNode(0);
		_game._player._stepEnabled = false;
		_scene->_kernelMessages.add(Common::Point(205, 41), 0xFDFC, 0, 0, 100, _game.getQuote(0x1F8));
		local._bartenderMode = 22;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(1, 100);
		local._talkTimer = 1050;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(50, 95);
		local._refuseAlienLiquor = true;
		break;

	default:
		break;
	}
}

static void handleDialogs() {
	if (_game._trigger == 0) {
		_scene->_kernelMessages.reset();
		_game._player._stepEnabled = false;
		const char *curQuote = _game.getQuote(_action._activeAction._verbId);
		if (_vm->_font->getWidth(curQuote, _scene->_textSpacing) > 200) {
			static char subQuote1[34], subQuote2[34];
			_game.splitQuote(curQuote, subQuote1, subQuote2);
			_scene->_kernelMessages.add(Common::Point(230, 42), 0x1110, 32, 0, 140, subQuote1);
			_scene->_kernelMessages.add(Common::Point(230, 56), 0x1110, 32, 0, 140, subQuote2);
			_scene->_sequences.addTimer(160, 120);
		} else {
			_scene->_kernelMessages.add(Common::Point(230, 56), 0x1110, 32, 1, 140, curQuote);
			_scene->_sequences.addTimer(160, 120);
		}
	} else if (_game._trigger == 120) {
		_game._player._stepEnabled = true;
		switch (local._bartenderDialogNode) {
		case 1:
			handleConversation1();
			break;

		case 2:
			handleConversation2();
			break;

		case 3:
			handleConversation3();
			break;

		case 4:
			handleConversation4();
			break;

		default:
			break;
		}
	}
}

static void room_402_init() {
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('n', -1));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('g', 0));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('g', 1));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[15] = _scene->_sprites.addSprites(formAnimName('x', 5));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('b', 1));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('b', 2));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('b', 3));
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[14] = _scene->_sprites.addSprites(formAnimName('l', 0));
	_globals._spriteIndexes[16] = _scene->_sprites.addSprites(formAnimName('h', 0));
	_globals._spriteIndexes[17] = _scene->_sprites.addSprites(formAnimName('z', 0));
	_globals._spriteIndexes[18] = _scene->_sprites.addSprites(formAnimName('z', 1));
	_globals._spriteIndexes[19] = _scene->_sprites.addSprites(formAnimName('z', 2));
	_globals._spriteIndexes[20] = _scene->_sprites.addSprites(formAnimName('x', 6));
	_globals._spriteIndexes[21] = _scene->_sprites.addSprites("*ROXRC_9");
	_globals._spriteIndexes[22] = _scene->_sprites.addSprites("*ROXCL_8");

	if (_scene->_priorSceneId == 401) {
		_game._player._playerPos = Common::Point(160, 150);
		_game._player._facing = FACING_NORTH;
		local._roxOnStool = false;
		local._bartenderDialogNode = 1;
		local._conversationFl = false;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(160, 150);
		_game._player._facing = FACING_NORTH;
		_game._objects.addToInventory(OBJ_CREDIT_CHIP);
		_game._objects.addToInventory(OBJ_BINOCULARS);
		_game._objects.addToInventory(OBJ_TIMER_MODULE);
		local._roxOnStool = false;
		local._bartenderDialogNode = 1;
		local._conversationFl = false;
	}

	_game.loadQuoteSet(0x1D7, 0x1D8, 0x1D9, 0x1DA, 0x1DB, 0x1DC, 0x1DD, 0x1DE, 0x1DF, 0x1E2, 0x1E3, 0x1E6, 0x1E5, 0x1E7,
		0x1E8, 0x1E9, 0x1EA, 0x1EF, 0x1F0, 0x1F1, 0x1F2, 0x1F3, 0x1F4, 0x1F5, 0x1F6, 0x1F7, 0x1F8, 0x1F9, 0x1FA, 0x1FB,
		0x1FC, 0x1EB, 0x1EC, 0x1ED, 0x1EE, 0x1E4, 0x1FD, 0x1E0, 0x1E1, 0x1FE, 0x1FF, 0x200, 0x201, 0x202, 0x203, 0x204,
		0x205, 0x206, 0x207, 0x208, 0x209, 0x20A, 0x20B, 0x20C, 0x20F, 0x20D, 0x20E, 0x210, 0x211, 0x212, 0x213, 0x214,
		0x215, 0x237, 0x216, 0x219, 0x21A, 0x21B, 0x21C, 0x21D, 0x220, 0x223, 0x224, 0x227, 0x22A, 0x22D, 0x230, 0x233,
		0x234, 0x235, 0x236, 0x238, 0x239, 0x23A, 0x23D, 0x23E, 0x23F, 0);

	_vm->_palette->setEntry(250, 47, 41, 40);
	_vm->_palette->setEntry(251, 50, 63, 55);
	_vm->_palette->setEntry(252, 38, 34, 25);
	_vm->_palette->setEntry(253, 45, 41, 35);

	local._dialog1.setup(0x60, 0x214, 0x215, 0x237, 0);
	local._dialog2.setup(0x61, 0x216, 0x219, 0x21A, 0x21B, 0x21D, 0x220, 0x223, 0x224, 0x227, 0x22A, 0x22D, 0x230, 0x21C, 0);
	local._dialog3.setup(0x62, 0x233, 0x234, 0x235, 0x236, 0x237, -1);
	local._dialog4.setup(0x63, 0x238, 0x239, 0x23A, 0x23D, 0x23E, 0);

	if (!_game._visitedScenes._sceneRevisited) {
		local._dialog2.set(0x61, 0x216, 0x219, 0x21A, 0x21B, 0x21C, 0);
		local._dialog4.set(0x63, 0x238, 0x23E, 0);
		local._dialog1.set(0x60, 0x214, 0x215, 0x237, 0);
	}

	if (_game._objects.isInInventory(OBJ_CREDIT_CHIP))
		local._dialog4.write(0x239, true);
	else
		local._dialog4.write(0x239, false);

	if (_game._objects.isInInventory(OBJ_BINOCULARS) && !_globals[kHasSaidBinocs])
		local._dialog4.write(0x23D, true);
	else
		local._dialog4.write(0x23D, false);

	if (_game._objects.isInInventory(OBJ_TIMER_MODULE) && !_globals[kHasSaidTimer])
		local._dialog4.write(0x23A, true);
	else
		local._dialog4.write(0x23A, false);

	if (local._dialog2.read(0) <= 1)
		local._dialog1.write(0x214, false);

	if (local._conversationFl) {
		switch (local._bartenderDialogNode) {
		case 0:
			_scene->_userInterface.setup(kInputBuildingSentences);
			local._bartenderDialogNode = 1;
			break;

		case 1:
			local._dialog1.start();
			break;

		case 2:
			local._dialog2.start();
			break;

		case 3:
			local._dialog3.start();
			break;

		case 4:
			local._dialog4.start();
			break;

		default:
			break;
		}

		switch (local._bartenderCurrentQuestion) {
		case 1:
			_scene->_kernelMessages.add(Common::Point(260, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1EC));
			break;

		case 2:
			_scene->_kernelMessages.add(Common::Point(188, 27), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1ED));
			_scene->_kernelMessages.add(Common::Point(199, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1EE));
			break;

		case 3:
			_scene->_kernelMessages.add(Common::Point(177, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1EF));
			break;

		case 4:
			_scene->_kernelMessages.add(Common::Point(205, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1E4));
			break;

		case 5:
			_scene->_kernelMessages.add(Common::Point(203, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1E5));
			break;

		case 6:
			_scene->_kernelMessages.add(Common::Point(260, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1E6));
			break;

		case 7:
			_scene->_kernelMessages.add(Common::Point(198, 27), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1E7));
			_scene->_kernelMessages.add(Common::Point(201, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1E8));
			break;

		case 8:
			_scene->_kernelMessages.add(Common::Point(220, 27), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1E9));
			_scene->_kernelMessages.add(Common::Point(190, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1EA));
			break;

		case 9:
			_scene->_kernelMessages.add(Common::Point(196, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1EB));
			break;

		case 10:
			_scene->_kernelMessages.add(Common::Point(198, 27), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1E2));
			_scene->_kernelMessages.add(Common::Point(199, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1E3));
			break;

		default:
			break;
		}
	}

	local._firstTalkToGirlInChair = false;
	local._lightOn = false;
	local._blowingSmoke = false;
	local._leftWomanMoving = false;
	local._rightWomanMoving = false;
	local._ginnyLooking = false;
	local._beatCounter = 0;
	local._waitingGinnyMove = false;
	local._bigBeatFl = true;
	local._bartenderHandsHips = false;
	local._bartenderSteady = true;
	local._bartenderLooksLeft = false;
	local._activeTeleporter = false;
	local._activeArrows = false;
	local._activeArrow1 = false;
	local._activeArrow2 = false;
	local._activeArrow3 = false;
	local._cutSceneReady = false;
	local._cutSceneNeeded = false;
	local._helgaReady = true;
	local._bartenderReady = true;
	local._drinkTimer = 0;
	local._bartenderTalking = false;
	local._bartenderCalled = false;
	local._helgaTalkMode = 0;
	local._rexMode = 0;
	local._refuseAlienLiquor = false;

	_scene->loadAnimation(Resources::formatName(402, 'd', 1, EXT_AA, ""));
	_scene->_animation[0]->_repeatFlag = true;

	_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);

	_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, 2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);

	if (!_game._objects.isInInventory(OBJ_REPAIR_LIST)) {
		_globals._sequenceIndexes[14] = _scene->_sequences.startCycle(_globals._spriteIndexes[14], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[14], 7);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[14], Common::Point(210, 80));
		int idx = _scene->_dynamicHotspots.add(words_repair_list, words_look_at, _globals._sequenceIndexes[14], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), FACING_NONE);
	}

	{
		_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		int idx = _scene->_dynamicHotspots.add(words_bartender, words_walkto, _globals._sequenceIndexes[9], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(228, 83), FACING_SOUTH);
	}

	if (!_game._objects.isInRoom(OBJ_ALIEN_LIQUOR) && !_game._objects.isInInventory(OBJ_CREDIT_CHIP)) {
		_globals._sequenceIndexes[15] = _scene->_sequences.startCycle(_globals._spriteIndexes[15], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[15], 8);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[15], Common::Point(250, 80));
		int idx = _scene->_dynamicHotspots.add(words_credit_chip, words_take, _globals._sequenceIndexes[15], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), FACING_NONE);
	}

	_globals._sequenceIndexes[20] = _scene->_sequences.startCycle(_globals._spriteIndexes[20], false, 1);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[20], 7);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[20], Common::Point(234, 72));

	if (_globals[kBottleDisplayed]) {
		_globals._sequenceIndexes[8] = _scene->_sequences.startCycle(_globals._spriteIndexes[8], false, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 7);
	}

	if (local._roxOnStool) {
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 11);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 5);
		_game._player._visible = false;
	}

	section_4_music();
}

static void room_402_daemon() {
	if (_game._trigger == 104) {
		_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
		_game._player._visible = true;
		_scene->_sequences.remove(_globals._sequenceIndexes[15]);
		_game._objects.addToInventory(OBJ_CREDIT_CHIP);
		_vm->_dialogs->showItem(OBJ_CREDIT_CHIP, 40242);
		_game._player._stepEnabled = true;
	}

	if ((_vm->getRandomNumber(1, 1500) == 1) && (!local._activeTeleporter) && (_game._player._playerPos.x < 150)) {
		_vm->_sound->command(30);
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 13, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 1, 11);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 48);
		local._activeTeleporter = true;
		_globals[kSomeoneHasExploded] = true;
	}

	if (_game._trigger == 48)
		local._activeTeleporter = false;

	if (_game._trigger == 100) {
		local._bartenderReady = false;
		if (local._bartenderHandsHips) {
			_scene->_sequences.remove(_globals._sequenceIndexes[10]);
			local._bartenderHandsHips = false;
		} else if (local._bartenderLooksLeft) {
			_scene->_sequences.remove(_globals._sequenceIndexes[11]);
			local._bartenderLooksLeft = false;
		} else if (local._bartenderSteady) {
			_scene->_sequences.remove(_globals._sequenceIndexes[9]);
			local._bartenderSteady = false;
		}

		if (!local._bartenderTalking) {
			_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 7, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 3, 4);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 8);
			int idx = _scene->_dynamicHotspots.add(words_bartender, words_walkto, _globals._sequenceIndexes[10], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(228, 83), FACING_SOUTH);
			local._bartenderTalking = true;
			if (local._talkTimer > 1000)
				_scene->_sequences.addTimer(local._talkTimer - 1000, 101);
			else
				_scene->_sequences.addTimer(local._talkTimer, 101);
		}
	}

	if ((_game._trigger == 101) && local._bartenderTalking) {
		_scene->_sequences.remove(_globals._sequenceIndexes[10]);
		_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 1);
		int idx = _scene->_dynamicHotspots.add(words_bartender, words_walkto, _globals._sequenceIndexes[9], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(228, 83), FACING_SOUTH);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		local._bartenderSteady = true;
		local._bartenderTalking = false;
		if (local._talkTimer < 1000)
			local._bartenderReady = true;
	}

	if (_game._trigger == 28)
		_game._player._stepEnabled = true;

	switch (_game._trigger) {
	case 92:
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[6]);
		_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], 1, 2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 5);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 93);
		break;

	case 93:
	{
		int seqIdx = _globals._sequenceIndexes[7];
		switch (local._roxMode) {
		case 20:
			_vm->_sound->command(57);
			_scene->_sequences.remove(_globals._sequenceIndexes[15]);
			_game._objects.addToInventory(OBJ_CREDIT_CHIP);
			_vm->_dialogs->showItem(OBJ_CREDIT_CHIP, 40242);
			break;

		case 22:
			_vm->_sound->command(57);
			_scene->_sequences.remove(_globals._sequenceIndexes[8]);
			_game._objects.addToInventory(OBJ_ALIEN_LIQUOR);
			_globals[kBottleDisplayed] = false;
			_vm->_dialogs->showItem(OBJ_ALIEN_LIQUOR, 40241);
			break;

		case 21:
			_globals._sequenceIndexes[15] = _scene->_sequences.startCycle(_globals._spriteIndexes[15], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[15], 8);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[15], Common::Point(250, 80));
			break;

		default:
			break;
		}
		_globals._sequenceIndexes[7] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[7], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], 1, 3);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 5);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[7], seqIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 94);

		if (local._roxMode == 21) {
			if (_game._objects.isInInventory(OBJ_CREDIT_CHIP))
				_game._objects.setRoom(OBJ_CREDIT_CHIP, NOWHERE);

			local._bartenderMode = 20;
			_scene->_sequences.addTimer(60, 95);
		}
	}
	break;

	case 94:
	{
		int seqIdx = _globals._sequenceIndexes[7];
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 11);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 5);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[6], seqIdx);
		if (local._roxMode == 22) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(230, 56), 0x1110, 32, 0, 120, _game.getQuote(0x23F));
			local._bartenderMode = 21;
			_globals[kHasPurchased] = true;
			_scene->_sequences.addTimer(140, 95);
		} else if (local._roxMode == 20)
			_game._player._stepEnabled = true;

	}
	break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 95:
		local._bartenderReady = false;
		_game._player._stepEnabled = false;
		if (local._bartenderHandsHips || local._bartenderTalking) {
			_scene->_sequences.remove(_globals._sequenceIndexes[10]);
			local._bartenderHandsHips = false;
			local._bartenderTalking = false;
		}

		if (local._bartenderLooksLeft) {
			_scene->_sequences.remove(_globals._sequenceIndexes[11]);
			local._bartenderLooksLeft = false;
		}

		if (local._bartenderSteady) {
			_scene->_sequences.remove(_globals._sequenceIndexes[9]);
			local._bartenderSteady = false;
		}
		_globals._sequenceIndexes[12] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[12], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[12], 1, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 8);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 96);
		break;

	case 96:
	{
		int seqIdx = _globals._sequenceIndexes[12];
		_globals._sequenceIndexes[12] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[12], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[12], 6, 7);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 8);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[12], seqIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 97);
	}
	break;

	case 97:
	{
		int seqIdx = _globals._sequenceIndexes[12];
		switch (local._bartenderMode) {
		case 20:
			_scene->_sequences.remove(_globals._sequenceIndexes[15]);
			break;

		case 21:
		{
			_globals._sequenceIndexes[15] = _scene->_sequences.startCycle(_globals._spriteIndexes[15], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[15], 8);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[15], Common::Point(250, 80));
			int idx = _scene->_dynamicHotspots.add(words_credit_chip, words_take, _globals._sequenceIndexes[15], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(0, 0), FACING_NONE);
		}
		break;

		case 22:
			_scene->_sequences.remove(_globals._sequenceIndexes[8]);
			_globals[kBottleDisplayed] = false;
			break;

		default:
			break;
		}

		_globals._sequenceIndexes[12] = _scene->_sequences.startCycle(_globals._spriteIndexes[12], false, 6);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[12], seqIdx);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 8);
		_scene->_sequences.addTimer(10, 98);
	}
	break;

	case 98:
		_scene->_sequences.remove(_globals._sequenceIndexes[12]);
		_globals._sequenceIndexes[12] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[12], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[12], 1, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 8);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 99);
		break;

	case 99:
	{
		int seqIdx = _globals._sequenceIndexes[12];
		_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 1);
		local._bartenderSteady = true;
		int idx = _scene->_dynamicHotspots.add(words_bartender, words_walkto, _globals._sequenceIndexes[9], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(228, 83), FACING_SOUTH);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], seqIdx);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		if (local._bartenderMode == 20) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(210, 41), 0xFDFC, 0, 0, 100, _game.getQuote(0x1F3));
			_scene->_sequences.addTimer(5, 100);
			local._talkTimer = 180;
			local._roxMode = 22;
			_scene->_sequences.addTimer(65, 92);
		} else if ((local._bartenderMode == 21) || (local._bartenderMode == 22)) {
			_game._player._stepEnabled = true;
			local._bartenderReady = true;
		}

	}
	break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 86:
		local._bartenderReady = false;
		_game._player._stepEnabled = false;
		if ((local._bartenderHandsHips) || (local._bartenderTalking)) {
			_scene->_sequences.remove(_globals._sequenceIndexes[10]);
			local._bartenderHandsHips = false;
			local._bartenderTalking = false;
		} else if (local._bartenderLooksLeft) {
			_scene->_sequences.remove(_globals._sequenceIndexes[11]);
			local._bartenderLooksLeft = false;
		} else if (local._bartenderSteady) {
			_scene->_sequences.remove(_globals._sequenceIndexes[9]);
			local._bartenderSteady = false;
		}
		_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 1, 9);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 87);
		break;

	case 87:
	{
		int seqIdx = _globals._sequenceIndexes[9];
		_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 9);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], seqIdx);
		_scene->_sequences.addTimer(10, 89);

		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 7);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 88);
	}
	break;

	case 88:
		_globals._sequenceIndexes[8] = _scene->_sequences.startCycle(_globals._spriteIndexes[8], false, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 7);
		_globals[kBottleDisplayed] = true;
		break;

	case 89:
		_scene->_sequences.remove(_globals._sequenceIndexes[9]);
		_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 6);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		_scene->_sequences.addTimer(10, 90);
		break;

	case 90:
		_scene->_sequences.remove(_globals._sequenceIndexes[9]);
		_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		_scene->_sequences.addTimer(10, 91);
		break;

	case 91:
	{
		_scene->_sequences.remove(_globals._sequenceIndexes[9]);
		_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 1);
		int idx = _scene->_dynamicHotspots.add(words_bartender, words_walkto, _globals._sequenceIndexes[9], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(228, 83), FACING_SOUTH);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		local._bartenderSteady = true;
		_game._player._stepEnabled = true;
		local._bartenderReady = true;
	}
	break;

	default:
		break;
	}

	if (!local._waitingGinnyMove && !local._ginnyLooking) {
		local._waitingGinnyMove = true;
		++local._beatCounter;
		if (local._beatCounter >= 20) {
			local._ginnyLooking = true;
			local._beatCounter = 0;
			_scene->_sequences.addTimer(60, 54);
		} else {
			_scene->_sequences.addTimer(30, 75);
		}
	}

	switch (_game._trigger) {
	case 75:
		_scene->_sequences.remove(_globals._sequenceIndexes[5]);
		local._bigBeatFl = !local._bigBeatFl;

		if (local._bigBeatFl) {
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 5);
			_scene->_sequences.addTimer(8, 130);
		} else {
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 5);
			_scene->_sequences.addTimer(8, 53);
		}

		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		break;

	case 130:
		_scene->_sequences.remove(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 6);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		_scene->_sequences.addTimer(8, 131);
		break;

	case 131:
		_scene->_sequences.remove(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		_scene->_sequences.addTimer(8, 53);
		break;

	case 53:
		_scene->_sequences.remove(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		local._waitingGinnyMove = false;
		break;

	default:
		break;
	}

	if ((_game._trigger == 54) && local._ginnyLooking) {
		++local._beatCounter;
		if (local._beatCounter >= 10) {
			local._ginnyLooking = false;
			local._waitingGinnyMove = false;
			local._beatCounter = 0;
			local._bigBeatFl = true;
		} else {
			_scene->_sequences.remove(_globals._sequenceIndexes[5]);
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, _vm->getRandomNumber(1, 4));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
			_scene->_sequences.addTimer(60, 54);
		}
	}

	if (local._bartenderReady) {
		if (_vm->getRandomNumber(1, 250) == 1) {
			if (local._bartenderLooksLeft) {
				_scene->_sequences.remove(_globals._sequenceIndexes[11]);
				local._bartenderLooksLeft = false;
			} else if (local._bartenderHandsHips) {
				_scene->_sequences.remove(_globals._sequenceIndexes[10]);
				local._bartenderHandsHips = false;
			} else if (local._bartenderSteady) {
				_scene->_sequences.remove(_globals._sequenceIndexes[9]);
				local._bartenderSteady = false;
			}

			switch (_vm->getRandomNumber(1, 3)) {
			case 1:
			{
				_globals._sequenceIndexes[10] = _scene->_sequences.startCycle(_globals._spriteIndexes[10], false, 4);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 8);
				int idx = _scene->_dynamicHotspots.add(words_bartender, words_walkto, _globals._sequenceIndexes[10], Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots.setPosition(idx, Common::Point(228, 83), FACING_SOUTH);
				local._bartenderHandsHips = true;
			}
			break;

			case 2:
			{
				_globals._sequenceIndexes[11] = _scene->_sequences.startCycle(_globals._spriteIndexes[11], false, 2);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 8);
				int idx = _scene->_dynamicHotspots.add(words_bartender, words_walkto, _globals._sequenceIndexes[11], Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots.setPosition(idx, Common::Point(228, 83), FACING_SOUTH);
				local._bartenderLooksLeft = true;
			}
			break;

			case 3:
			{
				_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
				int idx = _scene->_dynamicHotspots.add(words_bartender, words_walkto, _globals._sequenceIndexes[9], Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots.setPosition(idx, Common::Point(228, 83), FACING_SOUTH);
				local._bartenderSteady = true;
			}
			break;

			default:
				break;
			}
		}
	}

	if (_game._trigger == 76) {
		int seqIdx = _globals._sequenceIndexes[6];
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 11);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 5);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[6], seqIdx);
		if (!_globals[kBeenThruHelgaScene]) {
			_game._player._stepEnabled = false;
			local._cutSceneNeeded = true;
		} else {
			_game._player._stepEnabled = true;
		}
		local._roxOnStool = true;
	}

	switch (_game._trigger) {
	case 77:
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[6]);
		_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], 10, 12);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 5);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 78);
		break;

	case 78:
	{
		_vm->_sound->command(57);
		int seqIdx = _globals._sequenceIndexes[7];
		_game._objects.addToInventory(OBJ_REPAIR_LIST);
		_scene->_sequences.remove(_globals._sequenceIndexes[14]);
		_globals._sequenceIndexes[7] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[7], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], 10, 12);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 5);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[7], seqIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 79);
	}
	break;

	case 79:
	{
		int seqIdx = _globals._sequenceIndexes[7];
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 11);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 5);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[6], seqIdx);
		_scene->_sequences.addTimer(20, 180);
	}
	break;

	case 180:
		_vm->_dialogs->showItem(OBJ_REPAIR_LIST, 40240);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}

	if (local._cutSceneNeeded && local._cutSceneReady) {
		local._cutSceneNeeded = false;
		_scene->_sequences.addTimer(20, 55);
		local._helgaReady = false;
		local._bartenderReady = false;
	}

	if (_vm->getRandomNumber(1, 25) == 1) {
		if (local._lightOn) {
			_scene->_sequences.remove(_globals._sequenceIndexes[0]);
			local._lightOn = false;
		} else {
			_globals._sequenceIndexes[0] = _scene->_sequences.startCycle(_globals._spriteIndexes[0], false, 1);
			local._lightOn = true;
		}
	}

	if (!local._blowingSmoke && (_vm->getRandomNumber(1, 300) == 1)) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 14);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 30);
		local._blowingSmoke = true;
	}

	switch (_game._trigger) {
	case 30:
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 14);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 31);
		break;

	case 31:
		local._blowingSmoke = false;
		break;

	default:
		break;
	}

	if (!local._leftWomanMoving) {
		if (_vm->getRandomNumber(1, 1000) == 1) {
			switch (_vm->getRandomNumber(1, 2)) {
			case 1:
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 12, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 8);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 33);
				local._leftWomanMoving = true;
				break;

			case 2:
				_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
				_scene->_sequences.addTimer(12, 35);
				local._leftWomanMoving = true;
				break;

			default:
				break;
			}
		}
	}

	switch (_game._trigger) {
	case 33:
	{
		int seqIdx = _globals._sequenceIndexes[2];
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 9);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], seqIdx);
		_scene->_sequences.addTimer(_vm->getRandomNumber(60, 250), 34);
	}
	break;

	case 34:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 8);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 38);
		break;

	case 35:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 2);
		_scene->_sequences.addTimer(_vm->getRandomNumber(60, 300), 36);
		break;

	case 36:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 1);
		_scene->_sequences.addTimer(12, 37);
		break;

	case 37:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		local._leftWomanMoving = false;
		break;

	case 38:
		local._leftWomanMoving = false;
		break;

	default:
		break;
	}

	if (!local._rightWomanMoving) {
		if (_vm->getRandomNumber(1, 300) == 1) {
			switch (_vm->getRandomNumber(1, 4)) {
			case 1:
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 32);
				local._rightWomanMoving = true;
				break;

			case 2:
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 2, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 4, 5);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 32);
				local._rightWomanMoving = true;
				break;

			case 3:
				_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 12, 2, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 4, 5);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 32);
				local._rightWomanMoving = true;
				break;

			case 4:
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 2, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 3, 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 32);
				local._rightWomanMoving = true;
				break;

			default:
				break;
			}
		}
	}

	if (_game._trigger == 32)
		local._rightWomanMoving = false;

	if (_scene->_animation[0]->getCurrentFrame() == 1) {
		switch (_vm->getRandomNumber(1, 50)) {
		case 1:
			_scene->_animation[0]->setCurrentFrame(2);
			break;

		case 2:
			_scene->_animation[0]->setCurrentFrame(7);
			break;

		case 3:
			_scene->_animation[0]->setCurrentFrame(11);
			break;

		default:
			_scene->_animation[0]->setCurrentFrame(0);
			break;
		}
	}

	if ((_scene->_animation[0]->getCurrentFrame() == 4) && (local._drinkTimer < 10)) {
		++local._drinkTimer;
		_scene->_animation[0]->setCurrentFrame(3);
	}

	if (local._drinkTimer == 10) {
		local._drinkTimer = 0;
		_scene->_animation[0]->setCurrentFrame(4);
		_scene->_animation[0]->_currentFrame = 5;
	}


	switch (_scene->_animation[0]->getCurrentFrame()) {
	case 6:
	case 10:
	case 14:
		_scene->_animation[0]->setCurrentFrame(0);
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 39:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(89, 67), 0xFDFC, 32, 0, 120, _game.getQuote(0x1D8));
		_scene->_sequences.addTimer(150, 40);
		break;

	case 40:
		_scene->_kernelMessages.add(Common::Point(89, 67), 0xFDFC, 32, 0, 120, _game.getQuote(0x1D9));
		_scene->_sequences.addTimer(150, 41);
		break;

	case 41:
		_scene->_kernelMessages.add(Common::Point(89, 67), 0xFDFC, 32, 0, 120, _game.getQuote(0x1DA));
		_game._player._stepEnabled = true;
		break;

	case 42:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(89, 67), 0xFDFC, 32, 0, 120, _game.getQuote(0x1DC));
		_scene->_sequences.addTimer(150, 43);
		break;

	case 43:
		_scene->_kernelMessages.add(Common::Point(89, 67), 0xFDFC, 32, 0, 120, _game.getQuote(0x1DD));
		_game._player._stepEnabled = true;
		break;

	case 44:
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}

	switch (_game._trigger) {
	case 55:
		if (local._bartenderHandsHips) {
			_scene->_sequences.remove(_globals._sequenceIndexes[10]);
			local._bartenderHandsHips = false;
			_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 1);
			int idx = _scene->_dynamicHotspots.add(words_bartender, words_walkto, _globals._sequenceIndexes[9], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(228, 83), FACING_SOUTH);
			local._bartenderSteady = true;
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		} else if (local._bartenderLooksLeft) {
			_scene->_sequences.remove(_globals._sequenceIndexes[11]);
			local._bartenderLooksLeft = false;
			_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 1);
			int idx = _scene->_dynamicHotspots.add(words_bartender, words_walkto, _globals._sequenceIndexes[9], Common::Rect(0, 0, 0, 0));
			_scene->_dynamicHotspots.setPosition(idx, Common::Point(228, 83), FACING_SOUTH);
			local._bartenderSteady = true;
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		}
		_game._player._stepEnabled = false;
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(180, 47), 0xFBFA, 0, 0, 100, _game.getQuote(0x1FE));
		_scene->_sequences.addTimer(120, 56);
		break;

	case 56:
		_scene->_sequences.remove(_globals._sequenceIndexes[13]);
		_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 2, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 57);
		break;

	case 57:
	{
		int seqIdx = _globals._sequenceIndexes[13];
		_scene->_sequences.remove(_globals._sequenceIndexes[20]);
		_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 6, 9);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], seqIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 58);
	}
	break;

	case 58:
	{
		int seqIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[13], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 5, 9);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], seqIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 59);
	}
	break;

	case 59:
	{
		int seqIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[13], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 1, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], seqIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 60);
		_globals._sequenceIndexes[20] = _scene->_sequences.startCycle(_globals._spriteIndexes[20], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[20], 8);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[20], Common::Point(234, 72));
	}
	break;

	case 60:
	{
		int seqIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, 2);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], seqIdx);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.addTimer(10, 61);
	}
	break;

	case 61:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(181, 33), 0xFBFA, 0, 0, 130, _game.getQuote(0x1FF));
		_scene->_kernelMessages.add(Common::Point(171, 47), 0xFBFA, 0, 0, 130, _game.getQuote(0x200));
		_scene->_sequences.addTimer(150, 63);
		_scene->_sequences.remove(_globals._sequenceIndexes[13]);
		_globals._sequenceIndexes[13] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[13], false, 30, 4, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 10, 11);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 62);
		break;

	case 62:
	{
		int seqIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, 2);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], seqIdx);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
	}
	break;

	case 63:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(160, 33), 0xFBFA, 0, 0, 130, _game.getQuote(0x201));
		_scene->_kernelMessages.add(Common::Point(165, 47), 0xFBFA, 0, 0, 130, _game.getQuote(0x202));
		_scene->_sequences.addTimer(150, 64);
		break;

	case 64:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(210, 27), 0xFDFC, 0, 0, 130, _game.getQuote(0x1E0));
		_scene->_kernelMessages.add(Common::Point(198, 41), 0xFDFC, 0, 0, 130, _game.getQuote(0x1E1));
		_scene->_sequences.addTimer(150, 65);
		_scene->_sequences.addTimer(1, 100);
		local._talkTimer = 1130;
		break;

	case 65:
		_vm->_sound->command(30);
		_globals._sequenceIndexes[16] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[16], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[16], 1, 6);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[16], 9);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[16], SEQUENCE_TRIGGER_EXPIRE, 0, 66);
		break;

	case 66:
	{
		int seqIdx = _globals._sequenceIndexes[16];
		_globals._sequenceIndexes[16] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[16], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[16], 7, 37);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[16], 9);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[16], seqIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[16], SEQUENCE_TRIGGER_EXPIRE, 0, 68);

		_scene->_sequences.remove(_globals._sequenceIndexes[13]);
		_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 12, 13);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 67);
	}
	break;

	case 67:
	{
		int seqIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, 13);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], seqIdx);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
	}
	break;

	case 68:
		_scene->_sequences.remove(_globals._sequenceIndexes[13]);
		_globals._sequenceIndexes[13] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[13], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 12, 13);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 69);
		break;

	case 69:
	{
		int seqIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[13], false, 25, 4, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 10, 11);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], seqIdx);

		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(179, 33), 0xFBFA, 0, 0, 130, _game.getQuote(0x203));
		_scene->_kernelMessages.add(Common::Point(167, 47), 0xFBFA, 0, 0, 130, _game.getQuote(0x204));
		_scene->_sequences.addTimer(150, 71);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
	}
	break;

	case 70:
		_globals._sequenceIndexes[13] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[13], false, 25, 4, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 10, 12);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		break;

	case 71:
		_scene->_sequences.addTimer(210, 73);
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(168, 33), 0xFBFA, 0, 0, 180, _game.getQuote(0x205));
		_scene->_kernelMessages.add(Common::Point(151, 47), 0xFBFA, 0, 0, 180, _game.getQuote(0x206));
		if (!_game._objects.isInInventory(OBJ_REPAIR_LIST))
			local._activeArrows = true;
		break;

	case 72:
	{
		int seqIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, 2);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], seqIdx);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
	}
	break;

	case 73:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(177, 33), 0xFBFA, 0, 0, 150, _game.getQuote(0x207));
		_scene->_kernelMessages.add(Common::Point(172, 47), 0xFBFA, 0, 0, 150, _game.getQuote(0x208));
		local._bartenderSteady = true;
		_game._player._stepEnabled = true;
		local._helgaReady = true;
		local._bartenderReady = true;
		_globals[kBeenThruHelgaScene] = true;
		local._activeArrows = false;
		break;

	default:
		break;
	}

	if (local._helgaReady) {
		int rndVal = _vm->getRandomNumber(1, 1000);
		if (rndVal < 6)
			switch (rndVal) {
			case 1:
				local._cutSceneReady = false;
				local._helgaReady = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[13]);
				_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 10, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 2, 5);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 82);
				break;

			case 2:
				local._cutSceneReady = false;
				local._helgaReady = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[13]);
				_globals._sequenceIndexes[13] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[13], false, 15, 2, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 11, 13);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 26);
				break;

			case 3:
				local._cutSceneReady = false;
				local._helgaReady = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[13]);
				_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 10, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 10, 11);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 26);
				break;

			case 4:
				local._cutSceneReady = false;
				local._helgaReady = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[13]);
				_globals._sequenceIndexes[13] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[13], false, 15, 2, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 14, 15);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 26);
				break;

			case 5:
				local._cutSceneReady = false;
				local._helgaReady = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[13]);
				_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 10, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 16, 19);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
				break;

			default:
				break;
			}
	}

	if (_game._trigger == 80) {
		int seqIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, 19);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], seqIdx);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.addTimer(_vm->getRandomNumber(60, 120), 81);
	}

	if (_game._trigger == 81) {
		_scene->_sequences.remove(_globals._sequenceIndexes[13]);
		_globals._sequenceIndexes[13] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[13], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 16, 19);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 26);
	}

	if (_game._trigger == 26) {
		int seqIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, 2);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], seqIdx);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		local._cutSceneReady = true;

		if (!local._cutSceneNeeded)
			local._helgaReady = true;
	}

	if (_game._trigger == 82) {
		_scene->_sequences.remove(_globals._sequenceIndexes[20]);
		int seqIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 6, 9);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], seqIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 83);
	}

	if (_game._trigger == 83) {
		int seqIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[13], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 5, 9);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], seqIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 84);
	}

	if (_game._trigger == 84) {
		int seqIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[13], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], 1, 4);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], seqIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 85);

		_globals._sequenceIndexes[20] = _scene->_sequences.startCycle(_globals._spriteIndexes[20], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[20], 8);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[20], Common::Point(234, 72));
	}

	if (_game._trigger == 85) {
		int seqIdx = _globals._sequenceIndexes[13];
		_globals._sequenceIndexes[13] = _scene->_sequences.startCycle(_globals._spriteIndexes[13], false, 2);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[13], seqIdx);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 8);
		local._cutSceneReady = true;
		if (!local._cutSceneNeeded)
			local._helgaReady = true;
	}

	if (_game._trigger == 102) {
		_scene->_sequences.remove(_globals._sequenceIndexes[6]);
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], 14, 18);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 5);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 103);
	} else if (_game._trigger == 103) {
		_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
		local._roxOnStool = false;
		_game._player._facing = FACING_SOUTH;
		_game._player.selectSeries();
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_game._player._readyToWalk = true;
	}

	if (local._activeArrows) {
		if (!local._activeArrow1) {
			_globals._sequenceIndexes[17] = _scene->_sequences.startCycle(_globals._spriteIndexes[17], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[17], 1);
			_scene->_sequences.addTimer(_vm->getRandomNumber(10, 15), 49);
			local._activeArrow1 = true;
		}

		if (!local._activeArrow2) {
			_globals._sequenceIndexes[18] = _scene->_sequences.startCycle(_globals._spriteIndexes[18], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[18], 1);
			_scene->_sequences.addTimer(_vm->getRandomNumber(10, 15), 50);
			local._activeArrow2 = true;
		}

		if (!local._activeArrow3) {
			_globals._sequenceIndexes[19] = _scene->_sequences.startCycle(_globals._spriteIndexes[19], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[19], 1);
			_scene->_sequences.addTimer(_vm->getRandomNumber(10, 15), 51);
			local._activeArrow3 = true;
		}
	}

	if (_game._trigger == 49) {
		_scene->_sequences.remove(_globals._sequenceIndexes[17]);
		_scene->_sequences.addTimer(_vm->getRandomNumber(10, 15), 45);
	}

	if (_game._trigger == 45)
		local._activeArrow1 = false;

	if (_game._trigger == 50) {
		_scene->_sequences.remove(_globals._sequenceIndexes[18]);
		_scene->_sequences.addTimer(_vm->getRandomNumber(10, 15), 46);
	}

	if (_game._trigger == 46)
		local._activeArrow2 = false;

	if (_game._trigger == 51) {
		_scene->_sequences.remove(_globals._sequenceIndexes[19]);
		_scene->_sequences.addTimer(_vm->getRandomNumber(10, 15), 47);
	}

	if (_game._trigger == 47)
		local._activeArrow3 = false;
}

static void room_402_pre_parser() {
	if (player_said_2(sit_on, bar_stool) && (_game._player._prepareWalkPos.x != 248))
		_game._player.walk(Common::Point(232, 112), FACING_EAST);

	if (player_said_2(walkto, woman_on_balcony))
		_game._player._needToWalk = _game._player._readyToWalk;

	if (!local._roxOnStool && player_said_2(take, credit_chip) && !_game._objects.isInInventory(OBJ_CREDIT_CHIP))
		_game._player.walk(Common::Point(246, 108), FACING_NORTH);

	if (player_said_1(take))
		_game._player._needToWalk = false;

	if (player_said_2(take, credit_chip) && !local._roxOnStool)
		_game._player._needToWalk = true;

	if (local._roxOnStool) {
		if (player_said_1(look) || player_said_1(bar_stool) || player_said_1(talkto))
			_game._player._needToWalk = false;

		if (player_said_2(take, repair_list) || player_said_2(take, credit_chip))
			_game._player._needToWalk = false;

		if (player_said_2(talkto, woman_in_chair) || player_said_2(talkto, woman_in_alcove))
			_game._player._needToWalk = _game._player._readyToWalk;

		if (_game._player._needToWalk) {
			_game._player._facing = FACING_SOUTH;
			_game._player._readyToWalk = false;
			_game._player._stepEnabled = false;
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addTimer(1, 102);
		}
	}

	if (player_said_2(take, repair_list) && !local._roxOnStool && !_game._objects.isInInventory(OBJ_REPAIR_LIST))
		_game._player.walk(Common::Point(191, 99), FACING_NORTHEAST);

	if (player_said_2(talkto, bartender) && !local._roxOnStool)
		_game._player.walk(Common::Point(228, 83), FACING_SOUTH);

	if (player_said_2(talkto, repair_woman) && !local._roxOnStool)
		_game._player.walk(Common::Point(208, 102), FACING_NORTHEAST);
}

static void room_402_parser() {
	if (player_said_2(take, repair_list) && _game._objects.isInRoom(OBJ_REPAIR_LIST) && local._roxOnStool) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(1, 77);
		_game._player._needToWalk = false;
	} else if (player_said_2(take, repair_list) && _game._objects.isInRoom(OBJ_REPAIR_LIST) && !local._roxOnStool) {
		if (_game._trigger == 0) {
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[21] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[21], false, 7, 2, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[21], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[21]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[21], SEQUENCE_TRIGGER_SPRITE, 2, 165);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[21], SEQUENCE_TRIGGER_EXPIRE, 0, 166);
		} else if (_game._trigger == 165) {
			_scene->_sequences.remove(_globals._sequenceIndexes[14]);
			_game._objects.addToInventory(OBJ_REPAIR_LIST);
		}
	} else if (_game._trigger == 166) {
		_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
		_game._player._visible = true;
		_scene->_sequences.addTimer(20, 167);
	} else if (_game._trigger == 167) {
		_vm->_dialogs->showItem(OBJ_REPAIR_LIST, 40240);
		_game._player._stepEnabled = true;
	} else if (_game._screenObjects._inputMode == kInputConversation)
		handleDialogs();
	else if (player_said_2(walk_into, corridor_to_south))
		_scene->_nextSceneId = 401;
	else if (player_said_2(walk_onto, dance_floor))
		; // just... nothing
	else if (player_said_2(talkto, repair_woman)) {
		switch (_game._trigger) {
		case 0:
		{
			_game._player._stepEnabled = false;
			int random = _vm->getRandomNumber(1, 3);
			if (local._helgaTalkMode == 0)
				random = 1;

			int centerFlag;
			Common::Point centerPos;
			if (local._roxOnStool) {
				centerFlag = 0;
				centerPos = Common::Point(230, 56);
			} else {
				centerFlag = 2;
				centerPos = Common::Point(0, 0);
			}

			switch (random) {
			case 1:
				_scene->_kernelMessages.add(centerPos, 0x1110, 32 | centerFlag, 0, 90, _game.getQuote(0x211));
				_scene->_sequences.addTimer(110, 25);
				break;

			case 2:
				_scene->_kernelMessages.add(centerPos, 0x1110, 32 | centerFlag, 0, 90, _game.getQuote(0x212));
				_scene->_sequences.addTimer(110, 25);
				break;

			case 3:
				_scene->_kernelMessages.add(centerPos, 0x1110, 32 | centerFlag, 0, 90, _game.getQuote(0x213));
				_scene->_sequences.addTimer(110, 25);
				break;

			default:
				break;
			}
		}
		break;

		case 25:
			switch (local._helgaTalkMode) {
			case 0:
				_game._player._stepEnabled = false;
				local._helgaTalkMode = 1;
				_scene->_kernelMessages.add(Common::Point(177, 33), 0xFBFA, 0, 0, 130, _game.getQuote(0x209));
				_scene->_kernelMessages.add(Common::Point(182, 47), 0xFBFA, 0, 0, 130, _game.getQuote(0x20A));
				_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
				_scene->_sequences.addTimer(130, 28);
				break;

			case 1:
				_game._player._stepEnabled = false;
				local._helgaTalkMode = 2;
				_scene->_kernelMessages.add(Common::Point(157, 47), 0xFBFA, 0, 0, 100, _game.getQuote(0x20B));
				_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
				_scene->_sequences.addTimer(100, 28);
				break;

			case 2:
				_game._player._stepEnabled = false;
				local._helgaTalkMode = 3;
				_scene->_kernelMessages.add(Common::Point(172, 47), 0xFBFA, 0, 0, 100, _game.getQuote(0x20C));
				_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
				_scene->_sequences.addTimer(100, 28);
				break;

			case 3:
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
	} else if (player_said_2(talkto, woman_in_chair) && !local._firstTalkToGirlInChair) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x1D7));
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(150, 39);
		_game._player._stepEnabled = false;
		local._firstTalkToGirlInChair = true;
	} else if (player_said_2(talkto, woman_in_chair) && local._firstTalkToGirlInChair) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x1DB));
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(150, 42);
		_game._player._stepEnabled = false;
	} else if (player_said_2(talkto, woman_in_alcove) || player_said_2(walkto, woman_in_alcove)) {
		_scene->_kernelMessages.add(Common::Point(102, 48), 0xFBFA, 0, 0, 120, _game.getQuote(0x1DE));
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(120, 44);
		_game._player._stepEnabled = false;
	} else if (player_said_2(sit_on, bar_stool) && (_game._player._targetPos.x == 248)) {
		_scene->_kernelMessages.add(Common::Point(0, -14), 0x1110, 34, 0, 120, _game.getQuote(0x20D));
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x20E));
	} else if (player_said_2(sit_on, bar_stool) && !local._roxOnStool && (_game._player._targetPos.x != 248)) {
		_game._player._visible = false;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 11);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 5);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 76);
		_game._player._stepEnabled = false;
	} else if (player_said_2(take, credit_chip) && !_game._objects.isInInventory(OBJ_CREDIT_CHIP) && local._roxOnStool) {
		local._roxMode = 20;
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addTimer(1, 92);
	} else if (player_said_2(take, credit_chip) && !_game._objects.isInInventory(OBJ_CREDIT_CHIP) && !local._roxOnStool) {
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[22] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[22], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[22], 1, 2);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[22], Common::Point(_game._player._playerPos.x, _game._player._playerPos.y + 1));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[22], 5);
		_scene->_sequences.setScale(_globals._sequenceIndexes[22], 88);
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[22], SEQUENCE_TRIGGER_EXPIRE, 0, 104);
	} else if (player_said_2(talkto, bartender)) {
		switch (_game._trigger) {
		case 0:
		{
			int centerFlag;
			Common::Point centerPos;
			if (local._roxOnStool) {
				centerFlag = 0;
				centerPos = Common::Point(230, 56);
			} else {
				centerFlag = 2;
				centerPos = Common::Point(0, 0);
			}

			_game._player._stepEnabled = false;
			int quoteId;
			if (local._bartenderCalled) {
				quoteId = 0x210;
			} else {
				quoteId = 0x20F;
				local._bartenderCalled = true;
			}
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(centerPos, 0x1110, 32 | centerFlag, 0, 90, _game.getQuote(quoteId));
			_scene->_sequences.addTimer(110, 29);
			_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addTimer(90, 28);
		}
		break;

		case 29:
			_scene->_kernelMessages.reset();
			if (!local._roxOnStool) {
				if (_game._objects.isInRoom(OBJ_ALIEN_LIQUOR)) {
					_scene->_kernelMessages.add(Common::Point(177, 41), 0xFDFC, 0, 0, 120, _game.getQuote(0x1DF));
					_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
					_scene->_sequences.addTimer(1, 100);
					local._talkTimer = 120;
				} else if (local._rexMode == 0) {
					_scene->_kernelMessages.add(Common::Point(175, 13), 0xFDFC, 0, 0, 180, _game.getQuote(0x1F9));
					_scene->_kernelMessages.add(Common::Point(184, 27), 0xFDFC, 0, 0, 180, _game.getQuote(0x1FA));
					_scene->_kernelMessages.add(Common::Point(200, 41), 0xFDFC, 0, 0, 180, _game.getQuote(0x1FB));
					_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
					_scene->_sequences.addTimer(1, 100);
					local._talkTimer = 180;
					local._rexMode = 1;
				} else if (local._rexMode == 1) {
					_scene->_kernelMessages.add(Common::Point(205, 41), 0xFDFC, 0, 0, 120, _game.getQuote(0x1FC));
					_game._player._stepEnabled = true;
					_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
					_scene->_sequences.addTimer(1, 100);
					local._talkTimer = 120;
					local._rexMode = 3;
				} else {
					_game._player._stepEnabled = true;
				}
			} else {
				if (_game._objects.isInRoom(OBJ_ALIEN_LIQUOR)) {
					if (!local._refuseAlienLiquor) {
						_scene->_kernelMessages.reset();
						_scene->_kernelMessages.add(Common::Point(198, 27), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1E2));
						_scene->_kernelMessages.add(Common::Point(199, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1E3));
						local._bartenderCurrentQuestion = 10;
						_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
						_scene->_sequences.addTimer(1, 100);
						local._talkTimer = 120;
						local._conversationFl = true;
						local._bartenderDialogNode = 1;
						if (local._dialog2.read(0) <= 1)
							local._dialog1.write(0x214, false);

						local._dialog1.start();
					} else {
						_scene->_kernelMessages.add(Common::Point(177, 41), 0xFDFC, 0, 0, INDEFINITE_TIMEOUT, _game.getQuote(0x1EF));
						_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
						_scene->_sequences.addTimer(1, 100);
						local._talkTimer = 120;
						local._bartenderCurrentQuestion = 3;
						local._conversationFl = true;
						local._bartenderDialogNode = 1;
						if (local._dialog2.read(0) <= 1)
							local._dialog1.write(0x214, false);

						local._dialog1.start();
						_game._player._stepEnabled = true;
					}
				} else {
					if (local._rexMode == 0) {
						_scene->_kernelMessages.add(Common::Point(175, 13), 0xFDFC, 0, 0, 180, _game.getQuote(0x1F9));
						_scene->_kernelMessages.add(Common::Point(184, 27), 0xFDFC, 0, 0, 180, _game.getQuote(0x1FA));
						_scene->_kernelMessages.add(Common::Point(200, 41), 0xFDFC, 0, 0, 180, _game.getQuote(0x1FB));
						_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
						_scene->_sequences.addTimer(1, 100);
						local._talkTimer = 180;
						local._rexMode = 1;
					} else if (local._rexMode == 1) {
						_scene->_kernelMessages.add(Common::Point(205, 41), 0xFDFC, 0, 0, 120, _game.getQuote(0x1FC));
						_game._player._stepEnabled = true;
						_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
						_scene->_sequences.addTimer(1, 100);
						local._talkTimer = 120;
						local._rexMode = 3;
					} else {
						_game._player._stepEnabled = true;
					}
				}
			}
			break;

		default:
			break;
		}
	} else if (player_said_2(look, dance_floor))
		_vm->_dialogs->show(40210);
	else if (player_said_2(look, teleporter)) {
		if (_globals[kSomeoneHasExploded])
			_vm->_dialogs->show(40212);
		else
			_vm->_dialogs->show(40211);
	} else if (player_said_2(look, bar))
		_vm->_dialogs->show(40213);
	else if (player_said_2(look, bartender))
		_vm->_dialogs->show(40214);
	else if (player_said_2(look, woman_in_alcove))
		_vm->_dialogs->show(40215);
	else if (player_said_2(look, woman_on_balcony))
		_vm->_dialogs->show(40216);
	else if (player_said_2(look, woman_in_chair))
		_vm->_dialogs->show(40217);
	else if (player_said_2(look, repair_woman))
		_vm->_dialogs->show(40218);
	else if (player_said_2(look, corridor_to_south))
		_vm->_dialogs->show(40219);
	else if (_action._lookFlag)
		_vm->_dialogs->show(40220);
	else if (player_said_2(look, women))
		_vm->_dialogs->show(40221);
	else if (player_said_2(push, repair_woman) || player_said_2(pull, repair_woman))
		_vm->_dialogs->show(40222);
	else if (player_said_2(talkto, women))
		_vm->_dialogs->show(40223);
	else if (player_said_2(talkto, woman_on_balcony))
		_vm->_dialogs->show(40224);
	else if (player_said_2(look, railing))
		_vm->_dialogs->show(40225);
	else if (player_said_2(look, table))
		_vm->_dialogs->show(40226);
	else if (player_said_2(take, table))
		_vm->_dialogs->show(40227);
	else if (player_said_2(look, sign))
		_vm->_dialogs->show(40228);
	else if (player_said_2(take, sign))
		_vm->_dialogs->show(40229);
	else if (player_said_2(look, bar_stool))
		_vm->_dialogs->show(40230);
	else if (player_said_2(take, bar_stool))
		_vm->_dialogs->show(40231);
	else if (player_said_2(look, cactus))
		_vm->_dialogs->show(40232);
	else if (player_said_2(take, cactus))
		_vm->_dialogs->show(40233);
	else if (player_said_2(look, disco_ball))
		_vm->_dialogs->show(40234);
	else if (player_said_2(look, upper_dance_floor))
		_vm->_dialogs->show(40235);
	else if (player_said_2(look, tree))
		_vm->_dialogs->show(40236);
	else if (player_said_2(look, plant))
		_vm->_dialogs->show(40237);
	else if (player_said_2(take, plant))
		_vm->_dialogs->show(40238);
	else if (player_said_2(look, repair_list) && _game._objects.isInRoom(OBJ_REPAIR_LIST))
		_vm->_dialogs->show(40239);
	else
		return;

	_action._inProgress = false;
}

void room_402_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._lightOn);
	s.syncAsByte(local._blowingSmoke);
	s.syncAsByte(local._leftWomanMoving);
	s.syncAsByte(local._rightWomanMoving);
	s.syncAsByte(local._firstTalkToGirlInChair);
	s.syncAsByte(local._waitingGinnyMove);
	s.syncAsByte(local._ginnyLooking);
	s.syncAsByte(local._bigBeatFl);
	s.syncAsByte(local._roxOnStool);
	s.syncAsByte(local._bartenderSteady);
	s.syncAsByte(local._bartenderHandsHips);
	s.syncAsByte(local._bartenderLooksLeft);
	s.syncAsByte(local._bartenderReady);
	s.syncAsByte(local._bartenderTalking);
	s.syncAsByte(local._bartenderCalled);
	s.syncAsByte(local._conversationFl);
	s.syncAsByte(local._activeTeleporter);
	s.syncAsByte(local._activeArrows);
	s.syncAsByte(local._activeArrow1);
	s.syncAsByte(local._activeArrow2);
	s.syncAsByte(local._activeArrow3);
	s.syncAsByte(local._cutSceneReady);
	s.syncAsByte(local._cutSceneNeeded);
	s.syncAsByte(local._helgaReady);
	s.syncAsByte(local._refuseAlienLiquor);

	s.syncAsSint16LE(local._drinkTimer);
	s.syncAsSint16LE(local._beatCounter);
	s.syncAsSint16LE(local._bartenderMode);
	s.syncAsSint16LE(local._bartenderDialogNode);
	s.syncAsSint16LE(local._bartenderCurrentQuestion);
	s.syncAsSint16LE(local._helgaTalkMode);
	s.syncAsSint16LE(local._roxMode);
	s.syncAsSint16LE(local._rexMode);
	s.syncAsSint16LE(local._talkTimer);
}

void room_402_preload() {
	room_init_code_pointer = room_402_init;
	room_pre_parser_code_pointer = room_402_pre_parser;
	room_parser_code_pointer = room_402_parser;
	room_daemon_code_pointer = room_402_daemon;

	section_4_walker();
	section_4_interface();

	_scene->addActiveVocab(words_bartender);
	_scene->addActiveVocab(words_alien_liquor);
	_scene->addActiveVocab(words_drink);
	_scene->addActiveVocab(words_binoculars);
	_scene->addActiveVocab(words_walkto);
	_scene->addActiveVocab(words_credit_chip);
	_scene->addActiveVocab(words_take);
	_scene->addActiveVocab(words_repair_list);
	_scene->addActiveVocab(words_look_at);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
