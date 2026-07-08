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
#include "mads/madsv2/nebular/rooms/section1.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	byte _fridgeOpenedFl;
	byte _fridgeOpenedDescr;
	byte _fridgeFirstOpenFl;
	byte _chairDescrFl;
	byte _drawerDescrFl;
	byte _activeMsgFl;

	int16 _fridgeCommentCount;
};

static Scratch local;

static void addRandomMessage() {
	_scene->_kernelMessages.reset();
	_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
	int quoteId = _vm->getRandomNumber(65, 69);
	_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 73, 120, _game.getQuote(quoteId));
	local._activeMsgFl = true;
}

static void room_102_init() {
	section_1_music();

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 5));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', -1));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('c', -1));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('e', -1));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('n', -1));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('g', -1));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites("*RXMRC_8");
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('x', 0));

	_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 8, 0, 0, 0);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 170, 0, 1, 6);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 11, 0, 2, 3);
	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 4, 0, 1, 0);
	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 3, 0, 0, 5);

	if (_game._objects.isInRoom(OBJ_BINOCULARS))
		_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 24, 0, 0, 24);
	else
		_scene->_hotspots.activate(NOUN_BINOCULARS, false);

	_scene->_hotspots.activate(NOUN_BURGER, false);

	if (_globals[kMedicineCabinetOpen]) {
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], -2, -2);
	}

	if (_scene->_priorSceneId == 101) {
		_game._player._playerPos = Common::Point(229, 109);
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[6] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
	} else if (_scene->_priorSceneId == 103)
		_game._player._playerPos = Common::Point(47, 152);
	else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._facing = FACING_NORTHWEST;
		_game._player._playerPos = Common::Point(32, 129);
	}

	if (_scene->_priorSceneId != 106) {
		if (_globals[kWaterInAPuddle]) {
			_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 6, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], -2, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 5);
		}
	} else {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 5);
		_vm->_sound->command(24);
		_vm->_sound->command(28);
	}

	local._fridgeOpenedFl = false;
	local._fridgeOpenedDescr = false;
	local._fridgeCommentCount = 0;
	local._fridgeFirstOpenFl = true;
	local._chairDescrFl = false;
	local._activeMsgFl = false;

	_game.loadQuoteSet(0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x45, 0x43, 0);

	if (_scene->_priorSceneId == 101)
		_vm->_sound->command(20);
}

static void room_102_daemon() {
	if (_game._trigger == 70)
		_game._player._stepEnabled = true;

	if (_game._trigger == 72) {
		_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[13], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], -2, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[13], 5);
		_scene->_sequences.addTimer(48, 90);
	}

	if (_game._trigger >= 90) {
		if (_game._trigger >= 94) {
			_scene->loadAnimation(formAnimName('B', -1), 71);
			_game._player._stepEnabled = false;
			_game._player._visible = false;

			_globals[kWaterInAPuddle] = true;
			_vm->_sound->command(24);
		} else {
			_vm->_sound->command(23);
			_scene->_sequences.addTimer(48, _game._trigger + 1);
		}
	}

	if (_game._trigger == 71) {
		_game._player._stepEnabled = true;
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
	}

	if (local._fridgeOpenedFl && !local._fridgeOpenedDescr) {
		local._fridgeCommentCount++;
		if (local._fridgeCommentCount > 16384) {
			local._fridgeOpenedDescr = true;
			_vm->_dialogs->show(10213);
		}
	}

	if (!local._activeMsgFl && (_game._player._playerPos == Common::Point(177, 114)) && (_game._player._facing == FACING_NORTH)
		&& (_vm->getRandomNumber(1, 5000) == 1)) {
		_scene->_kernelMessages.reset();
		local._activeMsgFl = false;
		addRandomMessage();
	}

	if (_game._trigger == 73)
		local._activeMsgFl = false;
}

static void room_102_pre_parser() {
	if (_action.isObject(NOUN_REFRIGERATOR) || _action.isObject(NOUN_POSTER))
		_game._player._needToWalk = _game._player._readyToWalk;

	if (local._fridgeOpenedFl && !_action.isObject(NOUN_REFRIGERATOR)) {
		switch (_game._trigger) {
		case 0:
			if (_game._player._needToWalk) {
				_scene->_sequences.remove(_globals._sequenceIndexes[7]);
				_globals._sequenceIndexes[7] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[7], false, 6, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 15);
				_game._player._stepEnabled = false;
				_vm->_sound->command(20);
			}
			break;

		case 1:
			if (_game._objects.isInRoom(OBJ_BURGER)) {
				_scene->_sequences.remove(_globals._sequenceIndexes[10]);
				_scene->_hotspots.activate(NOUN_BURGER, false);
			}
			local._fridgeOpenedFl = false;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	}

	if (_game._player._needToWalk)
		_scene->_kernelMessages.reset();
}

static void room_102_parser() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(10234);
		_action._inProgress = false;
		return;
	}

	bool justOpenedFl = false;
	if (_action.isObject(NOUN_REFRIGERATOR) && !local._fridgeOpenedFl) {
		switch (_game._trigger) {
		case 0:
			_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 15);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			if (_game._objects.isInRoom(OBJ_BURGER)) {
				_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 7, 0, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 14);
			}
			_game._player._stepEnabled = false;
			_vm->_sound->command(20);
			_action._inProgress = false;
			return;

		case 1:
			_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], -2, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 15);
			int delay;
			if (_action.isAction(VERB_WALKTO) && !local._fridgeFirstOpenFl)
				delay = 0;
			else
				delay = 48;
			_scene->_sequences.addTimer(delay, 2);
			_action._inProgress = false;
			return;

		case 2:
			local._fridgeOpenedFl = true;
			local._fridgeOpenedDescr = false;
			local._fridgeCommentCount = 0;
			_game._player._stepEnabled = true;
			justOpenedFl = true;
			if (_game._objects.isInRoom(OBJ_BURGER))
				_scene->_hotspots.activate(NOUN_BURGER, true);
			break;

		default:
			break;
		}
	}

	if (_action.isAction(VERB_LOOK, NOUN_REFRIGERATOR) || _action.isAction(VERB_OPEN, NOUN_REFRIGERATOR)) {
		if (_game._objects.isInRoom(OBJ_BURGER))
			_vm->_dialogs->show(10230);
		else
			_vm->_dialogs->show(10229);

		local._fridgeFirstOpenFl = false;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALKTO, NOUN_REFRIGERATOR) && justOpenedFl) {
		local._fridgeFirstOpenFl = false;
		int quoteId = _vm->getRandomNumber(59, 63);
		const char *curQuote = _game.getQuote(quoteId);
		int width = _scene->_kernelMessages._talkFont->getWidth(curQuote, -1);
		_scene->_kernelMessages.reset();
		_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_kernelMessages.add(Common::Point(210, 60), 0x1110, 0, 73, 120, curQuote);
		_scene->_kernelMessages.add(Common::Point(214 + width, 60), 0x1110, 0, 73, 120, _game.getQuote(64));
		local._activeMsgFl = true;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_REFRIGERATOR)) {
		_vm->_dialogs->show(10213);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_REFRIGERATOR)) {
		_vm->_dialogs->show(8);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOOR)) {
		switch (_game._trigger) {
		case 0:
			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(20);
			break;

		case 1:
			_scene->_nextSceneId = 101;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALKTO, NOUN_ENGINEERING_SECTION)) {
		_scene->_nextSceneId = 103;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_WALKTO, NOUN_POSTER) || _action.isAction(VERB_LOOK, NOUN_POSTER) || _action.isAction(VERB_WALKTO, NOUN_BINOCULARS)) {
		addRandomMessage();
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_WEIGHT_MACHINE)) {
		_vm->_dialogs->show(10212);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_ENGINEERING_SECTION)) {
		_vm->_dialogs->show(10205);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_DOOR)) {
		_vm->_dialogs->show(10204);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_STARE_AT, NOUN_CEILING) || _action.isAction(VERB_LOOK, NOUN_CEILING)) {
		_vm->_dialogs->show(10203);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_STARE_AT, NOUN_OVERHEAD_LAMP) || _action.isAction(VERB_LOOK, NOUN_OVERHEAD_LAMP)) {
		_vm->_dialogs->show(10202);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_ROBO_KITCHEN)) {
		_vm->_dialogs->show(10215);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PUT, NOUN_BURGER, NOUN_ROBO_KITCHEN) && _game._objects.isInInventory(OBJ_BURGER)) {
		_vm->_dialogs->show(10216);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PUT, NOUN_REFRIGERATOR) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId))) {
		_vm->_dialogs->show(10217);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_PUT, NOUN_DEAD_FISH, NOUN_ROBO_KITCHEN) || _action.isAction(VERB_PUT, NOUN_STUFFED_FISH, NOUN_ROBO_KITCHEN)) {
		_vm->_dialogs->show(10230);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_ROBO_KITCHEN)) {
		_vm->_dialogs->show(10218);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_CLOSET)) {
		_vm->_dialogs->show(10219);
		_action._inProgress = false;
		return;
	}

	if ((_action.isObject(NOUN_LADDER) || _action.isObject(NOUN_HATCHWAY)) && (_action.isAction(VERB_LOOK) || _action.isAction(VERB_CLIMB_UP) || _action.isAction(VERB_CLIMB_THROUGH))) {
		if (_game._objects.isInInventory(OBJ_REBREATHER)) {
			if (!_action.isAction(VERB_CLIMB_UP) && !_action.isAction(VERB_CLIMB_THROUGH)) {
				_vm->_dialogs->show(10231);
				_action._inProgress = false;
				return;
			}
		} else if (_action.isAction(VERB_LOOK) || (_game._difficulty != DIFFICULTY_HARD)) {
			_vm->_dialogs->show(10222);
			_action._inProgress = false;
			return;
		}
	}

	if ((_action.isObject(NOUN_LADDER) || _action.isObject(NOUN_HATCHWAY)) && (_action.isAction(VERB_CLIMB_UP) || _action.isAction(VERB_CLIMB_THROUGH))) {
		switch (_game._trigger) {
		case 0:
			_scene->loadAnimation(formAnimName('A', -1), 1);
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			break;

		case 1:
			_vm->_sound->command(24);
			_scene->_sequences.addTimer(48, 2);
			break;

		case 2:
		case 3:
		case 4:
			_vm->_sound->command(23);
			_scene->_sequences.addTimer(48, _game._trigger + 1);
			break;

		case 5:
			_vm->_sound->command(24);
			_scene->_sequences.addTimer(48, _game._trigger + 1);
			break;

		case 6:
			if (_game._objects.isInInventory(OBJ_REBREATHER) && !_game._visitedScenes.exists(106))
				_vm->_dialogs->show(10237);
			_scene->_nextSceneId = 106;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_POWER_STATUS_PANEL)) {
		_vm->_dialogs->show(10226);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_WINDOW) || _action.isAction(VERB_LOOK_THROUGH, NOUN_WINDOW)) {
		_vm->_dialogs->show(10227);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_DOORWAY) || _action.isAction(VERB_WALKTO, NOUN_DOORWAY)) {
		_vm->_dialogs->show(10228);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_DRAWER) || ((_action.isAction(VERB_CLOSE, NOUN_DRAWER) || _action.isAction(VERB_PUSH, NOUN_DRAWER)) && !local._drawerDescrFl)) {
		_vm->_dialogs->show(10220);
		local._drawerDescrFl = true;
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_DRAWER) || _action.isAction(VERB_PUSH, NOUN_DRAWER)) {
		_vm->_dialogs->show(10221);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_DRAWER)) {
		_vm->_dialogs->show(10236);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_CHAIR) || (_action.isAction(VERB_SIT_IN, NOUN_CHAIR) && !local._chairDescrFl)) {
		local._chairDescrFl = true;
		_vm->_dialogs->show(10210);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_SIT_IN, NOUN_CHAIR)) {
		_vm->_dialogs->show(10211);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_MEDICINE_CABINET)) {
		if (_globals[kMedicineCabinetOpen])
			_vm->_dialogs->show(10207);
		else
			_vm->_dialogs->show(10206);

		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_CLOSE, NOUN_MEDICINE_CABINET) && _globals[kMedicineCabinetOpen]) {
		switch (_game._trigger) {
		case 0:
			_scene->_sequences.remove(_globals._sequenceIndexes[8]);
			_globals._sequenceIndexes[8] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[8], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(21);
			break;

		case 1:
			_scene->_sequences.addTimer(48, 2);
			break;

		case 2:
			_game._player._stepEnabled = true;
			_globals[kMedicineCabinetOpen] = false;
			_vm->_dialogs->show(10209);
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_OPEN, NOUN_MEDICINE_CABINET) && !_globals[kMedicineCabinetOpen]) {
		switch (_game._trigger) {
		case 0:
			_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(21);
			break;

		case 1:
			_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 6, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], -2, -2);
			_scene->_sequences.addTimer(48, 2);
			break;

		case 2:
			_game._player._stepEnabled = true;
			_globals[kMedicineCabinetOpen] = true;
			if (_globals[kMedicineCabinetVirgin]) {
				_vm->_dialogs->show(10208);
			} else {
				_vm->_dialogs->show(10207);
			}
			_globals[kMedicineCabinetVirgin] = false;
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_BINOCULARS) && _game._objects.isInRoom(OBJ_BINOCULARS)) {
		switch (_game._trigger) {
		case 0:
			_globals._sequenceIndexes[11] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[11]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			break;

		case 1:
			_game._objects.addToInventory(OBJ_BINOCULARS);
			_scene->_sequences.remove(_globals._sequenceIndexes[9]);
			_scene->_hotspots.activate(NOUN_BINOCULARS, false);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_vm->_sound->command(22);
			_vm->_dialogs->showItem(OBJ_BINOCULARS, 10201);
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_BURGER) && _game._objects.isInRoom(OBJ_BURGER)) {
		if (_game._trigger == 0) {
			_vm->_dialogs->showItem(OBJ_BURGER, 10235);
			_scene->_sequences.remove(_globals._sequenceIndexes[10]);
			_game._objects.addToInventory(OBJ_BURGER);
			_scene->_hotspots.activate(NOUN_BURGER, false);
			_vm->_sound->command(22);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
		}
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_TAKE, NOUN_POSTER)) {
		_vm->_dialogs->show(10224);
		_action._inProgress = false;
		return;
	}

	if ((_action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)) && _action.isObject(NOUN_WEIGHT_MACHINE)) {
		_vm->_dialogs->show(10225);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_FLOOR)) {
		_vm->_dialogs->show(10232);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_BINOCULARS) && !_game._objects.isInInventory(OBJ_BINOCULARS)) {
		_vm->_dialogs->show(10233);
		_action._inProgress = false;
		return;
	}

	if (_action.isAction(VERB_LOOK, NOUN_BURGER) && (_action._mainObjectSource == CAT_HOTSPOT)) {
		_vm->_dialogs->show(801);
		_action._inProgress = false;
	}
}

static void room_102_error() {
	if (_action.isAction(VERB_PUT, NOUN_ROBO_KITCHEN) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId))) {
		_vm->_dialogs->show(10217);
		_action._inProgress = false;
	}
}

void room_102_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._fridgeOpenedFl);
	s.syncAsByte(local._fridgeOpenedDescr);
	s.syncAsByte(local._fridgeFirstOpenFl);
	s.syncAsByte(local._chairDescrFl);
	s.syncAsByte(local._drawerDescrFl);
	s.syncAsByte(local._activeMsgFl);

	s.syncAsSint16LE(local._fridgeCommentCount);
}

void room_102_preload() {
	room_init_code_pointer = room_102_init;
	room_pre_parser_code_pointer = room_102_pre_parser;
	room_parser_code_pointer = room_102_parser;
	room_daemon_code_pointer = room_102_daemon;
	room_error_code_pointer = room_102_error;

	anim_himem_preload(formAnimName('A', -1), 3);

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
