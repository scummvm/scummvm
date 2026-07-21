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
#include "mads/nebular/rooms/section1.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _sittingFl;
	int16 _panelOpened;
	int16 _messageNum;
	int16 _posY;
	int16 _shieldSpriteIdx;
	int16 _chairHotspotId;
	int16 _oldSpecial;
};

static Scratch local;


static void room_101_say_dang() {
	_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
	_game._player._stepEnabled = false;

	switch (_game._trigger) {
	case 0:
		_scene->_sequences.remove(_globals._sequenceIndexes[11]);
		_globals._sequenceIndexes[11] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[11], false, 3, 6, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 21);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		_vm->_sound->command(17);
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 3, 2, 0, 0);
		break;

	case 72:
		_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 17);
		_scene->_kernelMessages.add(Common::Point(143, 61), 0x1110, 0, 0, 60, _game.getQuote(57));
		_scene->_sequences.addTimer(120, 73);
		break;

	case 73:
		_vm->_dialogs->show(10117);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

static void room_101_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 5));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('x', 6));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('x', 7));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('m', -1));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('b', 1));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('b', 2));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites(formAnimName('x', 8));
	_globals._spriteIndexes[14] = _scene->_sprites.addSprites(formAnimName('x', 0));

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 0, 0, 25);
	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 4, 0, 1, 0);
	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 2, 0);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 7, 70);
	_globals._sequenceIndexes[4] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[4], false, 10, 0, 0, 60);
	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 5, 0, 1, 0);
	_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 10, 0, 2, 0);
	_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6, 0, 0, 0);
	_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6, 0, 10, 4);
	_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 6, 0, 32, 47);

	_scene->_hotspots.activate(words_shield_modulator, false);
	local._panelOpened = false;

	if (_scene->_priorSceneId != RETURNING_FROM_LOADING)
		_globals[kNeedToStandUp] = false;

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(100, 152);

	if ((_scene->_priorSceneId == 112) || ((_scene->_priorSceneId == RETURNING_FROM_DIALOG) && local._sittingFl)) {
		_game._player._visible = false;
		local._sittingFl = true;
		_game._player._playerPos = Common::Point(161, 123);
		_game._player._facing = FACING_NORTHEAST;
		_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 17);
		_scene->_hotspots.activate(words_chair, false);
		local._chairHotspotId = _scene->_dynamicHotspots.add(words_chair, words_sit_in, -1, Common::Rect(159, 84, 159 + 33, 84 + 36));
		if (_scene->_priorSceneId == 112)
			room_101_say_dang();
	} else {
		_globals._sequenceIndexes[12] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[12], false, 6, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 4);
	}

	_game.loadQuoteSet(0x31, 0x32, 0x39, 0x36, 0x37, 0x38, 0);

	if (_globals[kNeedToStandUp]) {
		_scene->loadAnimation(Resources::formatName(101, 'S', -1, EXT_AA, ""), 71);
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_game._player._playerPos = Common::Point(68, 140);
		_game._player._facing = FACING_WEST;

		local._messageNum = 0;
		local._posY = 30;
	}

	local._oldSpecial = false;

	section_1_music();
}

static void room_101_daemon() {
	if (local._oldSpecial != _game._player._special) {
		local._oldSpecial = _game._player._special;
		if (local._oldSpecial)
			_vm->_sound->command(39);
		else
			_vm->_sound->command(11);
	}

	switch (_game._trigger) {
	case 70:
		_vm->_sound->command(9);
		break;

	case 71:
		_globals[kNeedToStandUp] = false;
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		break;

	case 72:
	case 73:
		room_101_say_dang();
		break;

	default:
		break;
	}

	if (_scene->_animation[0] != nullptr) {
		if ((_scene->_animation[0]->getCurrentFrame() >= 6) && (local._messageNum == 0)) {
			local._messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, local._posY), 0x1110, 0, 0, 240, _game.getQuote(49));
			local._posY += 14;
		}

		if ((_scene->_animation[0]->getCurrentFrame() >= 7) && (local._messageNum == 1)) {
			local._messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, local._posY), 0x1110, 0, 0, 240, _game.getQuote(54));
			local._posY += 14;
		}

		if ((_scene->_animation[0]->getCurrentFrame() >= 10) && (local._messageNum == 2)) {
			local._messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, local._posY), 0x1110, 0, 0, 240, _game.getQuote(55));
			local._posY += 14;
		}

		if ((_scene->_animation[0]->getCurrentFrame() >= 17) && (local._messageNum == 3)) {
			local._messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, local._posY), 0x1110, 0, 0, 240, _game.getQuote(56));
			local._posY += 14;
		}

		if ((_scene->_animation[0]->getCurrentFrame() >= 20) && (local._messageNum == 4)) {
			local._messageNum++;
			_scene->_kernelMessages.add(Common::Point(63, local._posY), 0x1110, 0, 0, 240, _game.getQuote(50));
			local._posY += 14;
		}
	}
}

static void room_101_pre_parser() {
	if (player_said_2(look, view_screen))
		_game._player._needToWalk = true;

	if (local._sittingFl) {
		if (player_said_1(look) || player_said_1(chair) || player_said_1(talkto) || player_said_1(peer_through) || player_said_1(examine))
			_game._player._needToWalk = false;

		if (_game._player._needToWalk) {
			switch (_game._trigger) {
			case 0:
				_game._player._readyToWalk = false;
				_game._player._stepEnabled = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[11]);
				_globals._sequenceIndexes[11] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 1, 17);
				_vm->_sound->command(16);
				break;

			case 1:
				local._sittingFl = false;
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				_game._player._readyToWalk = true;
				_scene->_hotspots.activate(71, true);
				_scene->_dynamicHotspots.remove(local._chairHotspotId);
				_globals._sequenceIndexes[12] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[12], false, 6, 0, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 4);
				break;

			default:
				break;
			}
		}
	}

	if (local._panelOpened && !(player_said_1(shield_access_panel) || player_said_1(shield_modulator))) {
		switch (_game._trigger) {
		case 0:
			if (_game._player._needToWalk) {
				_scene->_sequences.remove(_globals._sequenceIndexes[13]);
				local._shieldSpriteIdx = _game._objects.isInRoom(OBJ_SHIELD_MODULATOR) ? 13 : 14;
				_globals._sequenceIndexes[13] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[local._shieldSpriteIdx], false, 6, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_game._player._stepEnabled = false;
				_vm->_sound->command(20);
			}
			break;

		case 1:
			_game._player._stepEnabled = true;
			local._panelOpened = false;
			_scene->_hotspots.activate(words_shield_modulator, false);
			break;

		default:
			break;
		}
	}
}

static void room_101_parser() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(10125);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(walkto, life_support_section)) {
		_scene->_nextSceneId = 102;
		_action._inProgress = false;
		return;
	}

	if (player_said_2(sit_in, chair) || (player_said_2(look, view_screen) && !local._sittingFl)) {
		if (!local._sittingFl) {
			switch (_game._trigger) {
			case 0:
				_scene->_sequences.remove(_globals._sequenceIndexes[12]);
				_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 1);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 1, 17);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_SPRITE, 10, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_action._inProgress = false;
				return;

			case 1:
				_vm->_sound->command(16);
				break;

			case 2:
				_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 0, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 17);
				_game._player._stepEnabled = true;
				local._sittingFl = true;
				_scene->_hotspots.activate(71, false);
				local._chairHotspotId = _scene->_dynamicHotspots.add(words_chair, words_sit_in, -1, Common::Rect(159, 84, 159 + 33, 84 + 36));
				if (!player_said_2(look, view_screen)) {
					_action._inProgress = false;
					return;
				}
				_game._trigger = 0;
				break;

			default:
				break;
			}
		} else {
			_vm->_dialogs->show(10131);
			_action._inProgress = false;
			return;
		}
	}

	if ((player_said_2(walkto, shield_access_panel) || player_said_2(open, shield_access_panel)) && !local._panelOpened) {
		switch (_game._trigger) {
		case 0:
			local._shieldSpriteIdx = _game._objects.isInRoom(OBJ_SHIELD_MODULATOR) ? 13 : 14;
			_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[local._shieldSpriteIdx], false, 6, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(20);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[13]);
			_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[local._shieldSpriteIdx], false, 6, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], -2, -2);
			_game._player._stepEnabled = true;
			local._panelOpened = true;
			if (_game._objects.isInRoom(OBJ_SHIELD_MODULATOR))
				_scene->_hotspots.activate(words_shield_modulator, true);
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if ((player_said_2(take, shield_modulator) || player_said_2(pull, shield_modulator)) && _game._objects.isInRoom(OBJ_SHIELD_MODULATOR)) {
		_game._objects.addToInventory(OBJ_SHIELD_MODULATOR);
		_scene->_sequences.remove(_globals._sequenceIndexes[13]);
		_globals._sequenceIndexes[13] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[14], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[13], -2, -2);
		_scene->_hotspots.activate(words_shield_modulator, false);
		_vm->_dialogs->showItem(OBJ_SHIELD_MODULATOR, 10120);
		_vm->_sound->command(22);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, shield_access_panel) || (player_said_2(look, shield_modulator) && !_game._objects.isInInventory(OBJ_SHIELD_MODULATOR))) {
		if (local._panelOpened) {
			if (_game._objects.isInRoom(OBJ_SHIELD_MODULATOR))
				_vm->_dialogs->show(10128);
			else
				_vm->_dialogs->show(10129);
		} else
			_vm->_dialogs->show(10127);

		_action._inProgress = false;
		return;
	}

	if (player_said_2(open, shield_access_panel) && local._panelOpened) {
		_vm->_dialogs->show(10130);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, view_screen) && local._sittingFl) {
		if (_globals[kWatchedViewScreen])
			room_101_say_dang();
		else {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[11]);
				_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 21);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_vm->_sound->command(17);
				break;

			case 1:
				_globals._sequenceIndexes[11] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[11], false, 3, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 21);
				break;

			case 2:
				_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 3, 0, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[11], 17, 17);
				_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 3, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				_game._player._stepEnabled = true;
				_globals[kWatchedViewScreen] = true;
				local._sittingFl = true;
				_scene->_nextSceneId = 112;
				break;

			default:
				break;
			}
		}
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, chair)) {
		_vm->_dialogs->show(10101);
		_action._inProgress = false;
		return;
	}

	if ((player_said_1(look) || player_said_1(peer_through)) && (player_said_1(front_window) || player_said_1(outside))) {
		_vm->_dialogs->show(10102);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, hull) || player_said_2(look, outer_hull) || player_said_2(examine, hull) || player_said_2(examine, outer_hull)) {
		_vm->_dialogs->show(10103);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, fuzzy_dice)) {
		_vm->_dialogs->show(10104);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, mirror) || player_said_2(look_in, mirror)) {
		_vm->_dialogs->show(10105);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, curtains)) {
		_vm->_dialogs->show(10106);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, plastic_jesus)) {
		_vm->_dialogs->show(10107);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, escape_hatch) || (player_said_2(open, escape_hatch) && !_game._objects.isInInventory(OBJ_REBREATHER))) {
		_vm->_dialogs->show(10109);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(open, escape_hatch)) {
		_vm->_dialogs->show(10110);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, target_computer)) {
		_vm->_dialogs->show(10111);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, library_computer)) {
		_vm->_dialogs->show(10126);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, damage_control_panel)) {
		_vm->_dialogs->show(10112);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, navigation_controls)) {
		_vm->_dialogs->show(10113);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, engineering_controls)) {
		_vm->_dialogs->show(10114);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, weapons_display)) {
		_vm->_dialogs->show(10115);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, shield_status_panel)) {
		_vm->_dialogs->show(10116);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(take, plastic_jesus)) {
		_vm->_dialogs->show(10118);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(take, fuzzy_dice)) {
		_vm->_dialogs->show(10119);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(open, damage_control_panel)) {
		_vm->_dialogs->show(10121);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(open, curtains)) {
		_vm->_dialogs->show(10122);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(close, curtains)) {
		_vm->_dialogs->show(10123);
		_action._inProgress = false;
		return;
	}

	if ((player_said_1(look) || player_said_1(play)) && player_said_1(video_game)) {
		_vm->_dialogs->show(10124);
		_action._inProgress = false;
		return;
	}
}

void room_101_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._sittingFl);
	s.syncAsByte(local._panelOpened);
	s.syncAsSint16LE(local._messageNum);
	s.syncAsSint16LE(local._posY);
	s.syncAsSint16LE(local._shieldSpriteIdx);
	s.syncAsSint16LE(local._chairHotspotId);
	s.syncAsSint16LE(local._oldSpecial);
}

void room_101_preload() {
	room_init_code_pointer = room_101_init;
	room_pre_parser_code_pointer = room_101_pre_parser;
	room_parser_code_pointer = room_101_parser;
	room_daemon_code_pointer = room_101_daemon;

	anim_himem_preload(formAnimName('A', -1), 3);

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
