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
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _curAnimationFrame;
	int32 _newIngredient;
	int32 _newQuantity;
	int32 _resetFrame;
	int32 _badThreshold;

	bool _killRox;
	bool _makeMushroomCloud;

	Dialog _dialog1;
	Dialog _dialog2;
	Dialog _dialog3;
	Dialog _dialog4;
};

static Scratch local;


static bool addIngredient() {
	bool retVal = false;

	switch (local._newIngredient) {
	case OBJ_LECITHIN:
		if (_globals[kIngredientList + _globals[kNextIngredient]] == 1)
			retVal = true;

		local._badThreshold = 1;
		break;

	case OBJ_ALIEN_LIQUOR:
		if (_globals[kIngredientList + _globals[kNextIngredient]] == 0)
			retVal = true;

		local._badThreshold = 0;
		break;

	case OBJ_FORMALDEHYDE:
		if (_globals[kIngredientList + _globals[kNextIngredient]] == 3)
			retVal = true;

		local._badThreshold = 3;
		break;

	case OBJ_PETROX:
		if (_globals[kIngredientList + _globals[kNextIngredient]] == 2)
			retVal = true;

		local._badThreshold = 2;
		break;

	default:
		break;
	}

	if (!retVal && (_globals[kNextIngredient] == 0))
		_globals[kBadFirstIngredient] = local._badThreshold;

	if (_globals[kNextIngredient] == 0)
		retVal = true;

	return(retVal);
}

static bool addQuantity() {
	bool retVal = false;

	if (_globals[kIngredientQuantity + _globals[kNextIngredient]] == local._newQuantity)
		retVal = true;

	if (!retVal && (_globals[kNextIngredient] == 0))
		_globals[kBadFirstIngredient] = local._badThreshold;

	if (_globals[kNextIngredient] == 0)
		retVal = true;

	return(retVal);
}

static int computeQuoteAndQuantity() {
	int quoteId;
	int quantity;

	switch (_action._activeAction._verbId) {
	case 0x252:
		quoteId = 0x26F;
		quantity = 0;
		break;

	case 0x253:
		quoteId = 0x271;
		quantity = 0;
		break;

	case 0x254:
		quoteId = 0x270;
		quantity = 0;
		break;

	case 0x255:
		quoteId = 0x272;
		quantity = 0;
		break;

	case 0x256:
		quoteId = 0x267;
		quantity = 2;
		break;

	case 0x257:
		quoteId = 0x269;
		quantity = 2;
		break;

	case 0x258:
		quoteId = 0x268;
		quantity = 2;
		break;

	case 0x259:
		quoteId = 0x26A;
		quantity = 2;
		break;

	case 0x25A:
		quoteId = 0x26B;
		quantity = 3;
		break;

	case 0x25B:
		quoteId = 0x26D;
		quantity = 3;
		break;

	case 0x25C:
		quoteId = 0x26C;
		quantity = 3;
		break;

	case 0x25D:
		quoteId = 0x26E;
		quantity = 3;
		break;

	case 0x25E:
		quoteId = 0x263;
		quantity = 1;
		break;

	case 0x25F:
		quoteId = 0x265;
		quantity = 1;
		break;

	case 0x260:
		quoteId = 0x264;
		quantity = 1;
		break;

	case 0x261:
		quoteId = 0x266;
		quantity = 1;
		break;

	default:
		quoteId = 0;
		quantity = 0;
		break;
	}

	_scene->_kernelMessages.add(Common::Point(202, 82), 0x1110, 32, 0, 120, _game.getQuote(quoteId));
	return quantity;
}

static void handleKettleAction() {
	switch (_globals[kNextIngredient]) {
	case 1:
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4],
			false, 15, 0, 0, 0);
		break;

	case 2:
		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4],
			false, 6, 0, 0, 0);
		break;

	case 3:
		local._makeMushroomCloud = true;
		break;

	default:
		break;
	}
}

static void handleDialog() {
	if ((_action._activeAction._verbId != 0x262) && (_game._trigger == 0)) {
		if (_game._objects.isInInventory(local._newIngredient)) {
			switch (local._newIngredient) {
			case OBJ_FORMALDEHYDE:
				local._resetFrame = 17;
				break;

			case OBJ_PETROX:
				local._resetFrame = 55;
				break;

			case OBJ_LECITHIN:
				local._resetFrame = 36;
				break;

			default:
				local._resetFrame = 112;
				break;
			}

			_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
			_game._player._visible = false;
			_game._player._stepEnabled = false;
			_scene->_animation[0]->setCurrentFrame(local._resetFrame);
		}
		_scene->_kernelMessages.reset();
		local._newQuantity = computeQuoteAndQuantity();

		if ((_globals[kNextIngredient] == 1) && (_globals[kBadFirstIngredient] > -1))
			local._killRox = true;
		else if (addIngredient() && addQuantity()) {
			handleKettleAction();
			_globals[kNextIngredient]++;
		} else
			local._killRox = true;

		_scene->_userInterface.setup(kInputBuildingSentences);
	} else if (_action._activeAction._verbId == 0x262)
		_scene->_userInterface.setup(kInputBuildingSentences);
}

static void giveToRex(int object_id) {
	switch (object_id) {
	case 0:
		_game._objects.addToInventory(OBJ_ALIEN_LIQUOR);
		break;

	case 1:
		_game._objects.addToInventory(OBJ_LECITHIN);
		break;

	case 2:
		_game._objects.addToInventory(OBJ_PETROX);
		break;

	case 3:
		_game._objects.addToInventory(OBJ_FORMALDEHYDE);
		break;

	default:
		break;
	}
}

static void room_411_init() {
	if (_scene->_priorSceneId == 411) {
		if ((_globals[kNextIngredient] == 1) && (_globals[kBadFirstIngredient] > -1))
			giveToRex(_globals[kBadFirstIngredient]);
		else if (_globals[kNextIngredient] > 0) {
			for (int i = 0; i < _globals[kNextIngredient]; i++)
				giveToRex(_globals[kIngredientList + i]);
		}
		_globals[kNextIngredient] = 0;
		_globals[kBadFirstIngredient] = -1;
	}

	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('f', 0));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('f', 1));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('f', 2));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('c', 1));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('a', 6));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites("*ROXRC_9");

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 0, 0, 0);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 50, 0, 0, 0);

	_game.loadQuoteSet(0x252, 0x25E, 0x25A, 0x256, 0x253, 0x25F, 0x25B, 0x257, 0x254, 0x260, 0x25C, 0x258, 0x255,
		0x261, 0x25D, 0x259, 0x262, 0x267, 0x263, 0x26B, 0x26F, 0x268, 0x264, 0x26C, 0x270, 0x26A, 0x266, 0x26E,
		0x272, 0x269, 0x265, 0x26D, 0x271, 0);

	local._dialog1.setup(0x5B, 0x252, 0x25E, 0x25A, 0x256, 0x262, -1);
	local._dialog2.setup(0x5C, 0x253, 0x25F, 0x25B, 0x257, 0x262, -1);
	local._dialog3.setup(0x5D, 0x254, 0x260, 0x25C, 0x258, 0x262, -1);
	local._dialog4.setup(0x5E, 0x255, 0x261, 0x25D, 0x259, 0x262, -1);

	if (_globals[kNextIngredient] >= 4 && !_game._objects[OBJ_CHARGE_CASES].getQuality(3)) {
		_scene->_hotspots.activate(words_kettle, false);
		_scene->_hotspots.activate(words_explosives, true);
	} else {
		_scene->_hotspots.activate(words_explosives, false);
		_scene->_hotspots.activate(words_kettle, true);
	}

	if (_globals[kNextIngredient] >= 4 && _game._objects[OBJ_CHARGE_CASES].getQuality(3)) {
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], true, 6);
	} else if (!_game._objects[OBJ_CHARGE_CASES].getQuality(3)) {
		switch (_globals[kNextIngredient]) {
		case 1:
			_vm->_sound->command(53);
			break;

		case 2:
			_vm->_sound->command(53);
			_vm->_sound->command(54);
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 15, 0, 0, 0);
			break;

		case 3:
			_vm->_sound->command(53);
			_vm->_sound->command(54);
			_vm->_sound->command(55);
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 0, 0, 0);
			break;

		case 4:
			_vm->_sound->command(53);
			_vm->_sound->command(54);
			_vm->_sound->command(55);
			_vm->_sound->command(56);
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 0, 0, 0);
			break;

		default:
			_vm->_sound->command(10);
			break;
		}
	}

	if (_globals[kNextIngredient] >= 4 && _game._objects[OBJ_CHARGE_CASES].getQuality(3)) {
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], true, 6);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
	}

	if (_game._objects.isInRoom(OBJ_FORMALDEHYDE)) {
		_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 1);
		int idx = _scene->_dynamicHotspots.add(words_formaldehyde, words_walkto, _globals._sequenceIndexes[7], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(206, 145), FACING_SOUTHEAST);
	}

	if (_game._objects.isInRoom(OBJ_PETROX)) {
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 8);
		int idx = _scene->_dynamicHotspots.add(words_petrox, words_walkto, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(186, 112), FACING_NORTHEAST);
	}

	if (_game._objects.isInRoom(OBJ_LECITHIN)) {
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 8);
		int idx = _scene->_dynamicHotspots.add(words_lecithin, words_walkto, _globals._sequenceIndexes[6], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(220, 121), FACING_NORTHEAST);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(60, 146);
		_game._player._facing = FACING_NORTHEAST;
	}

	section_4_music();

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_ALIEN_LIQUOR);
		_game._objects.addToInventory(OBJ_CHARGE_CASES);
		_game._objects.addToInventory(OBJ_TAPE_PLAYER);
		_game._objects.addToInventory(OBJ_AUDIO_TAPE);
	}

	_scene->loadAnimation(formAnimName('a', -1));
	_scene->_animation[0]->setCurrentFrame(128);

	local._makeMushroomCloud = false;
	local._killRox = false;
}

static void room_411_daemon() {
	if (_scene->_animation[0] != nullptr) {
		if (local._curAnimationFrame != _scene->_animation[0]->getCurrentFrame()) {
			local._curAnimationFrame = _scene->_animation[0]->getCurrentFrame();
			local._resetFrame = -1;

			switch (local._curAnimationFrame) {
			case 16:
				_game._player._stepEnabled = true;
				_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
				_game._player._visible = true;
				local._resetFrame = 128;
				break;

			case 35:
			case 54:
			case 71:
			case 127:
				if (local._killRox) {
					local._resetFrame = 72;
				} else {
					local._resetFrame = 0;
					_game._objects.removeFromInventory(local._newIngredient, NOWHERE);
					switch (_globals[kNextIngredient]) {
					case 1:
						_vm->_sound->command(53);
						break;

					case 2:
						_vm->_sound->command(54);
						break;

					case 3:
						_vm->_sound->command(55);
						break;

					case 4:
						_vm->_sound->command(56);
						break;

					default:
						break;
					}
				}
				break;

			case 22:
			case 41:
			case 59:
			case 115:
				if (local._makeMushroomCloud) {
					_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 5, 1, 0, 0);
					local._makeMushroomCloud = false;
					_scene->_hotspots.activate(words_kettle, false);
					_scene->_hotspots.activate(words_explosives, true);
				}
				break;

			case 111:
				local._resetFrame = 111;
				_scene->_reloadSceneFlag = true;
				break;

			case 129:
				local._resetFrame = 128;
				break;

			default:
				break;
			}

			if ((local._resetFrame >= 0) && (local._resetFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(local._resetFrame);
				local._curAnimationFrame = local._resetFrame;
			}
		}
	}

	if (_scene->_animation[0]->getCurrentFrame() == 86)
		_vm->_sound->command(59);
}

static void room_411_pre_parser() {
	if (player_said_2(look, petrox) && (_game._objects.isInRoom(OBJ_PETROX)))
		_game._player._needToWalk = true;

	if (player_said_2(look, lecithin) && (_game._objects.isInRoom(OBJ_LECITHIN)))
		_game._player._needToWalk = true;

	if (player_said_2(look, formaldehyde) && (_game._objects.isInRoom(OBJ_FORMALDEHYDE)))
		_game._player._needToWalk = true;

	if (player_said_2(look, explosives) || player_said_2(look, kettle) || player_said_2(look, mishap) ||
		player_said_2(look, alcove) || player_said_2(look, sink) || player_said_2(put, sink) ||
		player_said_2(look, experiment) || player_said_2(look, drawing_board))
		_game._player._needToWalk = true;

	if (player_said_2(pull, knife_switch) || player_said_2(push, knife_switch))
		_game._player._needToWalk = false;
}

static void room_411_parser() {
	if (_game._screenObjects._inputMode == kInputConversation) {
		handleDialog();
		_action._inProgress = false;
		return;
	}

	if (player_said_2(walk_into, corridor_to_south)) {
		_scene->_nextSceneId = 406;
		_vm->_sound->command(10);
		_action._inProgress = false;
		return;
	}

	if ((_globals[kNextIngredient] >= 4) && (player_said_2(take, explosives) || player_said_3(put, charge_cases, explosives))
		&& !_game._objects[OBJ_CHARGE_CASES].getQuality(3)
		&& _game._objects.isInInventory(OBJ_CHARGE_CASES)) {
		switch (_game._trigger) {
		case 0:
			_vm->_sound->command(10);
			_vm->_sound->command(57);
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 1, 6);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 3);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 110);
			break;

		case 110:
		{
			int idx = _globals._sequenceIndexes[10];
			_globals._sequenceIndexes[10] = _scene->_sequences.startCycle(_globals._spriteIndexes[10], false, 6);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 3);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[10], idx);
			_scene->_sequences.addTimer(180, 111);
		}
		break;

		case 111:
			_scene->_hotspots.activate(words_kettle, true);
			_scene->_hotspots.activate(words_explosives, false);
			_scene->_sequences.remove(_globals._sequenceIndexes[4]);
			_scene->_sequences.remove(_globals._sequenceIndexes[10]);

			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], true, 6);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);

			_globals._sequenceIndexes[10] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[10], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 1, 6);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 3);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 112);
			// fall through
		case 112:
			_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_game._objects[OBJ_CHARGE_CASES].setQuality(3, 1);
			_vm->_dialogs->showItem(OBJ_CHARGE_CASES, 41142);
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	} else if (!_game._objects.isInInventory(OBJ_CHARGE_CASES) && player_said_2(take, explosives)) {
		_vm->_dialogs->show(41143);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(take, petrox) && (_game._objects.isInRoom(OBJ_PETROX) || _game._trigger)) {
		switch (_game._trigger) {
		case 0:
			_vm->_sound->command(57);
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], false, 7, 2, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[8]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[5]);
			_game._objects.addToInventory(OBJ_PETROX);
			_vm->_dialogs->showItem(OBJ_PETROX, 41120);
			break;

		case 2:
			_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
			_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
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

	if (player_said_2(take, lecithin) && (_game._objects.isInRoom(OBJ_LECITHIN) || _game._trigger)) {
		switch (_game._trigger) {
		case 0:
			_vm->_sound->command(57);
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[8] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[8], false, 7, 2, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[8]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[6]);
			_game._objects.addToInventory(OBJ_LECITHIN);
			_vm->_dialogs->showItem(OBJ_LECITHIN, 41124);
			break;

		case 2:
			_game._player._priorTimer = _scene->_frameStartTime + _game._player._ticksAmount;
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
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

	if (player_said_2(take, formaldehyde) && _game._objects.isInRoom(OBJ_FORMALDEHYDE) && (_game._trigger == 0)) {
		_vm->_sound->command(57);
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[11] = _scene->_sequences.startCycle(_globals._spriteIndexes[11], false, 2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[11], 1);
		_scene->_sequences.addTimer(20, 100);
		_scene->_sequences.remove(_globals._sequenceIndexes[7]);
		_game._objects.addToInventory(OBJ_FORMALDEHYDE);
		_action._inProgress = false;
		return;
	}

	if (_game._trigger == 100) {
		_scene->_sequences.remove(_globals._sequenceIndexes[11]);
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_scene->_sequences.addTimer(20, 10);
	}

	if (_game._trigger == 10)
		_vm->_dialogs->showItem(OBJ_FORMALDEHYDE, 41124);

	if (player_said_1(put) && player_said_1(kettle)) {
		if (player_said_1(petrox) ||
			player_said_1(formaldehyde) ||
			player_said_1(lecithin) ||
			player_said_1(alien_liquor)) {
			local._newIngredient = _game._objects.getIdFromDesc(_action._activeAction._objectNameId);
			switch (local._newIngredient) {
			case OBJ_ALIEN_LIQUOR:
				local._dialog1.start();
				break;

			case OBJ_FORMALDEHYDE:
				local._dialog3.start();
				break;

			case OBJ_PETROX:
				local._dialog4.start();
				break;

			case OBJ_LECITHIN:
				local._dialog2.start();
				break;

			default:
				break;
			}
		}
	}


	if (player_said_2(look, monitor))
		_vm->_dialogs->show(41110);
	else if (player_said_2(look, air_purifier))
		_vm->_dialogs->show(41111);
	else if (player_said_2(look, lab_equipment))
		_vm->_dialogs->show(41112);
	else if (player_said_2(look, knife_switch))
		_vm->_dialogs->show(41113);
	else if (player_said_2(push, knife_switch) || player_said_2(pull, knife_switch))
		_vm->_dialogs->show(41114);
	else if (player_said_2(look, toxic_waste))
		_vm->_dialogs->show(41115);
	else if (player_said_2(take, toxic_waste))
		_vm->_dialogs->show(41116);
	else if (player_said_2(look, drawing_board))
		_vm->_dialogs->show(41117);
	else if (player_said_2(look, experiment))
		_vm->_dialogs->show(41118);
	else if (player_said_2(look, petrox) && _game._objects.isInRoom(OBJ_PETROX))
		_vm->_dialogs->show(41119);
	else if (player_said_2(look, alcove))
		_vm->_dialogs->show(41121);
	else if ((player_said_2(look, formaldehyde)) && (_game._objects.isInRoom(OBJ_FORMALDEHYDE)))
		_vm->_dialogs->show(41122);
	else if ((player_said_2(look, lecithin)) && (_game._objects.isInRoom(OBJ_LECITHIN)))
		_vm->_dialogs->show(41123);
	else if (player_said_2(look, kettle)) {
		if (_globals[kNextIngredient] > 0 && !_game._objects[OBJ_CHARGE_CASES].getQuality(3)) {
			_vm->_dialogs->show(41126);
		} else if (_globals[kNextIngredient] == 0 || _game._objects[OBJ_CHARGE_CASES].getQuality(3)) {
			_vm->_dialogs->show(41125);
		}
	} else if (player_said_2(look, explosives) && _game._objects[OBJ_CHARGE_CASES].getQuality(3) == 0) {
		_vm->_dialogs->show(41127);
	} else if (player_said_2(take, kettle))
		_vm->_dialogs->show(41128);
	else if (player_said_2(look, control_panel))
		_vm->_dialogs->show(41129);
	else if (player_said_2(look, mishap))
		_vm->_dialogs->show(41130);
	else if (player_said_2(look, corridor_to_south))
		_vm->_dialogs->show(41131);
	else if (_action._lookFlag)
		_vm->_dialogs->show(41132);
	else if (player_said_2(look, air_horn))
		_vm->_dialogs->show(41133);
	else if (player_said_2(look, debris))
		_vm->_dialogs->show(41134);
	else if (player_said_2(look, heater))
		_vm->_dialogs->show(41135);
	else if (player_said_2(look, pipe))
		_vm->_dialogs->show(41136);
	else if (player_said_2(look, sink))
		_vm->_dialogs->show(41137);
	else if (player_said_2(put, sink))
		_vm->_dialogs->show(41138);
	else if (player_said_2(take, experiment))
		_vm->_dialogs->show(41139);
	else if (player_said_2(look, electrodes))
		_vm->_dialogs->show(41140);
	else if (player_said_2(take, electrodes))
		_vm->_dialogs->show(41141);
	else
		return;

	_action._inProgress = false;
}

void room_411_synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(local._curAnimationFrame);
	s.syncAsSint32LE(local._newIngredient);
	s.syncAsSint32LE(local._newQuantity);
	s.syncAsSint32LE(local._resetFrame);
	s.syncAsSint32LE(local._badThreshold);

	s.syncAsByte(local._killRox);
	s.syncAsByte(local._makeMushroomCloud);
}

void room_411_preload() {
	room_init_code_pointer = room_411_init;
	room_pre_parser_code_pointer = room_411_pre_parser;
	room_parser_code_pointer = room_411_parser;
	room_daemon_code_pointer = room_411_daemon;

	section_4_walker();
	section_4_interface();
	_scene->addActiveVocab(words_walkto);
	_scene->addActiveVocab(words_alien_liquor);
	_scene->addActiveVocab(words_formaldehyde);
	_scene->addActiveVocab(words_petrox);
	_scene->addActiveVocab(words_lecithin);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
