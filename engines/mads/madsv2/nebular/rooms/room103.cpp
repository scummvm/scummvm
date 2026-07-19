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

#include "math/utils.h"
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
	long _updateClock;
};

static Scratch local;


static void room_103_init() {
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('x', 2));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('x', 3));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('x', 4));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('x', 5));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', -1));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('h', -1));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('m', -1));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('t', -1));
	_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('r', -1));
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('c', -1));
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites("*RXMBD_2");
	_globals._spriteIndexes[13] = _scene->_sprites.addSprites("*RXMRD_3");
	_globals._spriteIndexes[15] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 7, 0, 1, 0);

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 0, 2, 0);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 0);

	_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 0, 0, 25);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 72);

	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 0, 1, 37);
	_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 2, 73);

	_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8);
	_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 6);
	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6);
	_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6);

	if (_game._objects.isInRoom(OBJ_TIMER_MODULE))
		_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 6);
	else
		_vm->_game->_scene._hotspots.activate(371, false);

	if (_game._objects.isInRoom(OBJ_REBREATHER))
		_globals._sequenceIndexes[10] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[10], false, 6);
	else
		_vm->_game->_scene._hotspots.activate(289, false);

	if (_globals[kTurkeyExploded]) {
		_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], -2, -2);
		_scene->_hotspots.activate(362, false);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(237, 74);

	if (_scene->_priorSceneId == 102) {
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[6] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[6], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
	}

	section_1_music();

	_vm->_game->loadQuoteSet(70, 51, 71, 7, 73, 0);

	if (!_game._visitedScenes._sceneRevisited) {
		int msgIndex = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(70));
		_scene->_kernelMessages.setQuoted(msgIndex, 4, true);
	}

	if (_scene->_priorSceneId == 102)
		_vm->_sound->command(20);

	_vm->_palette->setEntry(252, 63, 63, 10);
	_vm->_palette->setEntry(253, 45, 45, 10);
	local._updateClock = _scene->_frameStartTime;
}

static void room_103_daemon() {
	switch (_vm->_game->_trigger) {
	case 70:
		_vm->_game->_player._stepEnabled = true;
		break;

	case 72:
	{
		Common::Point pt = _vm->_game->_player._playerPos;
		int dist = Math::hypotenuse(pt.x - 58, pt.y - 93);
		_vm->_sound->command(27, (dist * -128 / 378) + 127);
	}
	break;

	case 73:
	{
		Common::Point pt = _vm->_game->_player._playerPos;
		int dist = Math::hypotenuse(pt.x - 266, pt.y - 81);
		_vm->_sound->command(27, (dist * -127 / 378) + 127);
	}
	break;

	default:
		break;
	}

	if (_scene->_frameStartTime >= local._updateClock) {
		Common::Point pt = _vm->_game->_player._playerPos;
		int dist = Math::hypotenuse(pt.x - 79, pt.y - 137);
		_vm->_sound->command(29, (dist * -127 / 378) + 127);

		pt = _vm->_game->_player._playerPos;
		dist = Math::hypotenuse(pt.x - 69, pt.y - 80);
		_vm->_sound->command(30, (dist * -127 / 378) + 127);

		pt = _vm->_game->_player._playerPos;
		dist = Math::hypotenuse(pt.x - 266, pt.y - 138);
		_vm->_sound->command(32, (dist * -127 / 378) + 127);

		local._updateClock = _scene->_frameStartTime + _vm->_game->_player._ticksAmount;
	}
}

static void room_103_pre_parser() {
	// No implementation
}

static void room_103_parser() {
	if (_action._savedFields._lookFlag)
		_vm->_dialogs->show(10322);
	else if (player_said_2(walk_through, door)) {
		switch (_vm->_game->_trigger) {
		case 0:
			_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 6, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			_game._player._stepEnabled = false;
			_vm->_sound->command(20);
			break;

		case 1:
			_vm->_sound->command(1);
			_scene->_nextSceneId = 102;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(take, timer_module) && _game._objects.isInRoom(OBJ_TIMER_MODULE)) {
		switch (_vm->_game->_trigger) {
		case 0:
			_scene->changeVariant(1);
			_globals._sequenceIndexes[13] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[13], false, 3, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[13]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_SPRITE, 7, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[13], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			_vm->_game->_player._visible = false;
			_vm->_game->_player._stepEnabled = false;
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[11]);
			break;

		case 2:
			_vm->_sound->command(22);
			_game._objects.addToInventory(OBJ_TIMER_MODULE);
			_scene->changeVariant(0);
			_scene->drawElements(kTransitionNone, false);
			_scene->_hotspots.activate(371, false);
			_vm->_game->_player._visible = true;
			_vm->_game->_player._stepEnabled = true;
			_vm->_dialogs->showItem(OBJ_TIMER_MODULE, 805);
			break;

		default:
			break;
		}
	} else if (player_said_2(take, rebreather) && _game._objects.isInRoom(OBJ_REBREATHER)) {
		switch (_vm->_game->_trigger) {
		case 0:
			_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 3, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[12]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_SPRITE, 6, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			_vm->_game->_player._visible = false;
			_vm->_game->_player._stepEnabled = false;
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[10]);
			break;

		case 2:
			_vm->_sound->command(22);
			_game._objects.addToInventory(OBJ_REBREATHER);
			_scene->_hotspots.activate(289, false);
			_vm->_game->_player._visible = true;
			_vm->_game->_player._stepEnabled = true;
			_vm->_dialogs->showItem(OBJ_REBREATHER, 804);
			break;

		default:
			break;
		}
	} else if (player_said_2(look, tasty_turkey))
		_vm->_dialogs->show(10301);
	else if (player_said_2(take, tasty_turkey)) {
		// Take Turkey
		if (!_vm->_game->_trigger)
			_vm->_sound->command(31);

		if (_vm->_game->_trigger < 2) {
			_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 6, _vm->_game->_trigger < 1 ? 1 : 0);
			if (_vm->_game->_trigger) {
				// Lock the turkey into a permanent "exploded" frame
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], -2, -2);

				// Rex says "Gads.."
				const char *msg = _game.getQuote(51);
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 18, 0, 60, msg);
				_scene->_sequences.addTimer(120, _vm->_game->_trigger + 1);
			} else {
				// Initial turky explosion
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			}
		}

		// Re-enable player if sequence is ended, and set global flag
		_game._player._stepEnabled = _game._trigger == 2;
		_globals[kTurkeyExploded] = -1;

		if (_game._trigger == 2) {
			// Show exposition dialog at end of sequence
			_vm->_dialogs->show(10302);
			_scene->_hotspots.activate(362, false);
		}
	} else if (player_said_2(look, oven))
		_vm->_dialogs->show(!_globals[kTurkeyExploded] ? 10323 : 10303);
	else if (player_said_2(talkto, auxiliary_power)) {
		switch (_vm->_game->_trigger) {
		case 0:
		{
			_game._player._stepEnabled = false;
			const char *msg = _game.getQuote(71);
			_scene->_kernelMessages.add(Common::Point(), 0x1110, 18, 1, 120, msg);
			break;
		}

		case 1:
		{
			const char *msg = _game.getQuote(72);
			_scene->_kernelMessages.add(Common::Point(310, 132), 0xFDFC, 16, 2, 120, msg);
			break;
		}

		case 2:
			_scene->_kernelMessages.reset();
			_scene->_sequences.addTimer(1, 3);
			break;

		case 3:
			_game._player._stepEnabled = true;
			_vm->_dialogs->show(10306);
			break;

		default:
			break;
		}
	} else if (player_said_2(look, auxiliary_power))
		_vm->_dialogs->show(10304);
	else if (player_said_2(look, big_pipes))
		_vm->_dialogs->show(10307);
	else if (player_said_2(look, burnt_out_warp_coil))
		_vm->_dialogs->show(10308);
	else if (player_said_2(take, shovel))
		_vm->_dialogs->show(10309);
	else if (player_said_2(take, coal))
		_vm->_dialogs->show(10310);
	else if (player_said_2(look, furnace))
		_vm->_dialogs->show(10312);
	else if (player_said_2(open, furnace))
		_vm->_dialogs->show(10313);
	else if (player_said_2(close, auxiliary_power))
		_vm->_dialogs->show(10314);
	else if (player_said_2(look, shield_generator))
		_vm->_dialogs->show(10315);
	else if (player_said_2(look, hyperdrive_jump_unit))
		_vm->_dialogs->show(10316);
	else if (player_said_2(look, pressure_gauge))
		_vm->_dialogs->show(10317);
	else if (player_said_2(look, engineering_controls))
		_vm->_dialogs->show(10318);
	else if (player_said_2(look, rebreather) && _game._objects.isInInventory(OBJ_REBREATHER))
		_vm->_dialogs->show(10319);
	else if (player_said_2(look, timer_module) && _game._objects.isInInventory(OBJ_TIMER_MODULE))
		_vm->_dialogs->show(10320);
	else if (player_said_2(look, floor))
		_vm->_dialogs->show(10321);
	else if (player_said_2(look, workbench))
		_vm->_dialogs->show(_game._objects.isInInventory(OBJ_TIMER_MODULE) ? 10324 : 10325);
	else
		return;

	_action._inProgress = false;
}

void room_103_error() {
	if (player_said_1(auxiliary_power) && !player_said_1(walkto)) {
		_vm->_dialogs->show(10305);
		_action._inProgress = false;
	} else if (player_said_3(put, coal, furnace)) {
		const char *msg = _game.getQuote(73);
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, msg);
		_action._inProgress = false;
	}
}

void room_103_synchronize(Common::Serializer &s) {
	byte dummy = 0;
	s.syncAsByte(dummy); // In order to avoid to break savegame compatibility
	s.syncAsUint32LE(local._updateClock);
}

void room_103_preload() {
	room_init_code_pointer = room_103_init;
	room_pre_parser_code_pointer = room_103_pre_parser;
	room_parser_code_pointer = room_103_parser;
	room_daemon_code_pointer = room_103_daemon;
	room_error_code_pointer = room_103_error;

	anim_himem_preload(formAnimName('A', -1), 3);

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
