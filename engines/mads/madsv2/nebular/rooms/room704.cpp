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
#include "mads/madsv2/nebular/rooms/section7.h"
#include "mads/madsv2/nebular/rooms/dialog.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _bottleHotspotId;
	int16 _boatCurrentFrame;
	int16 _animationMode;
	int16 _boatDirection;
	bool _takeBottleFl;
	Dialog _dialog1;
};

static Scratch local;


static void handleBottleInterface() {
	switch (_globals[kBottleStatus]) {
	case 0:
		local._dialog1.write(0x311, true);
		local._dialog1.write(0x312, true);
		local._dialog1.write(0x313, true);
		local._dialog1.write(0x314, true);
		local._dialog1.write(0x315, true);
		break;

	case 1:
		local._dialog1.write(0x311, false);
		local._dialog1.write(0x312, true);
		local._dialog1.write(0x313, true);
		local._dialog1.write(0x314, true);
		local._dialog1.write(0x315, true);
		break;

	case 2:
		local._dialog1.write(0x311, false);
		local._dialog1.write(0x312, false);
		local._dialog1.write(0x313, true);
		local._dialog1.write(0x314, true);
		local._dialog1.write(0x315, true);
		break;

	case 3:
		local._dialog1.write(0x311, false);
		local._dialog1.write(0x312, false);
		local._dialog1.write(0x313, false);
		local._dialog1.write(0x314, true);
		local._dialog1.write(0x315, true);
		break;

	default:
		break;
	}
}

static void setBottleSequence() {
	_scene->_userInterface.setup(kInputBuildingSentences);
	_game._player._stepEnabled = false;
	if (local._boatDirection == 2)
		local._animationMode = 6;
	else
		local._animationMode = 7;
}

static void handleFillBottle(int quote) {
	switch (quote) {
	case 0x311:
		_globals[kBottleStatus] = 1;
		setBottleSequence();
		break;

	case 0x312:
		_globals[kBottleStatus] = 2;
		setBottleSequence();
		break;

	case 0x313:
		_globals[kBottleStatus] = 3;
		setBottleSequence();
		break;

	case 0x314:
		_globals[kBottleStatus] = 4;
		setBottleSequence();
		break;

	case 0x315:
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	default:
		break;
	}
}

static void room_704_init() {
	if (_game._objects[OBJ_BOTTLE]._roomNumber == _scene->_currentSceneId) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('b', 0));
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 6, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		if (_scene->_priorSceneId == 705) {
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(123, 125));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		} else {
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(190, 122));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
		}
		int idx = _scene->_dynamicHotspots.add(words_bottle, words_look_at, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		local._bottleHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(-2, 0), FACING_NONE);
	}

	_game._player._visible = false;
	local._takeBottleFl = false;
	local._boatCurrentFrame = -1;

	if (_scene->_priorSceneId == 705) {
		_game._player._stepEnabled = false;
		local._animationMode = 2;
		local._boatDirection = 2;
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(36);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._stepEnabled = false;
		local._boatDirection = 1;
		_scene->loadAnimation(formAnimName('A', -1));
	} else if (local._boatDirection == 1) {
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(8);
	} else if (local._boatDirection == 2) {
		if (_game._objects[OBJ_BOTTLE]._roomNumber == _scene->_currentSceneId) {
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(123, 125));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		}
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(57);
	}

	if (_scene->_roomChanged)
		_globals[kMonsterAlive] = false;

	_game.loadQuoteSet(0x311, 0x312, 0x313, 0x314, 0x315, 0);
	local._dialog1.setup(0x98, 0x311, 0x312, 0x313, 0x314, 0x315, 0);

	section_7_music();
	_vm->_sound->command(28);
}

static void room_704_daemon() {
	if (_scene->_animation[0] != nullptr) {
		if (_scene->_animation[0]->getCurrentFrame() != local._boatCurrentFrame) {
			local._boatCurrentFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			switch (local._boatCurrentFrame) {
			case 10:
				switch (local._animationMode) {
				case 1:
					nextFrame = 10;
					break;
				case 5:
					nextFrame = 74;
					break;
				case 7:
					local._animationMode = 0;
					nextFrame = 92;
					break;
				default:
					if (!_game._player._stepEnabled)
						_game._player._stepEnabled = true;

					nextFrame = 8;
					break;
				}
				break;

			case 36:
				if (local._animationMode != 2)
					_scene->_nextSceneId = 705;
				break;

			case 59:
				switch (local._animationMode) {
				case 3:
					nextFrame = 59;
					break;

				case 4:
					nextFrame = 65;
					break;

				case 6:
					local._animationMode = 0;
					nextFrame = 83;
					break;

				default:
					if (!_game._player._stepEnabled) {
						_game._player._stepEnabled = true;
					}
					nextFrame = 57;
					break;
				}
				break;

			case 65:
				_scene->_nextSceneId = 703;
				break;

			case 74:
				nextFrame = 10;
				break;

			case 83:
				nextFrame = 59;
				break;

			case 90:
				if (local._takeBottleFl) {
					_scene->_sequences.remove(_globals._sequenceIndexes[1]);
					_scene->_dynamicHotspots.remove(local._bottleHotspotId);
					_game._objects.addToInventory(OBJ_BOTTLE);
					_vm->_sound->command(15);
					_vm->_dialogs->showItem(OBJ_BOTTLE, 70415);
				}
				break;

			case 92:
				nextFrame = 57;
				if (!_game._player._stepEnabled && !local._takeBottleFl) {
					_scene->_sequences.addTimer(30, 70);
					_game._player._stepEnabled = true;
				}
				break;

			case 98:
				if (local._takeBottleFl) {
					_scene->_sequences.remove(_globals._sequenceIndexes[1]);
					_scene->_dynamicHotspots.remove(local._bottleHotspotId);
					_game._objects.addToInventory(OBJ_BOTTLE);
					_vm->_sound->command(15);
					_vm->_dialogs->showItem(OBJ_BOTTLE, 70415);
				}
				break;

			case 101:
				nextFrame = 8;
				if (!_game._player._stepEnabled && !local._takeBottleFl) {
					_scene->_sequences.addTimer(30, 70);
					_game._player._stepEnabled = true;
				}
				break;

			default:
				break;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				local._boatCurrentFrame = nextFrame;
			}
		}
	}

	if (_game._trigger == 70) {
		switch (_globals[kBottleStatus]) {
		case 0:
			_vm->_dialogs->show(432);
			break;

		case 1:
			_vm->_dialogs->show(70324);
			break;

		case 2:
			_vm->_dialogs->show(70325);
			break;

		case 3:
			_vm->_dialogs->show(70326);
			break;

		case 4:
			_vm->_dialogs->show(70327);
			break;

		default:
			break;
		}
	}
}

static void room_704_parser() {
	if (_game._screenObjects._inputMode == kInputConversation)
		handleFillBottle(_action._activeAction._verbId);
	else if (_action.isAction(words_steer_towards, words_open_water_to_south)) {
		_game._player._stepEnabled = false;
		if (local._boatDirection == 1)
			local._animationMode = 5;
		else
			local._animationMode = 3;
	} else if (_action.isAction(words_steer_towards, words_building_to_north)) {
		_game._player._stepEnabled = false;
		if (local._boatDirection == 2)
			local._animationMode = 4;
		else
			local._animationMode = 1;
	} else if (_action.isAction(words_take, words_bottle)) {
		if (!_game._objects.isInInventory(OBJ_BOTTLE)) {
			_game._player._stepEnabled = false;
			local._takeBottleFl = true;
			if (local._boatDirection == 2) {
				local._animationMode = 6;
			} else {
				local._animationMode = 7;
			}
		}
	} else if (_action.isAction(words_put, words_bottle, words_water) || _action.isAction(words_fill, words_bottle, words_water)) {
		if (_game._objects.isInInventory(OBJ_BOTTLE)) {
			if (_globals[kBottleStatus] != 4) {
				local._takeBottleFl = false;
				handleBottleInterface();
				local._dialog1.start();
			} else
				_vm->_dialogs->show(70323);
		}
	} else if (_action._lookFlag || _action.isAction(words_look, words_water))
		_vm->_dialogs->show(70410);
	else if (_action.isAction(words_look, words_building_to_north)) {
		if (_game._visitedScenes.exists(710))
			_vm->_dialogs->show(70411);
		else
			_vm->_dialogs->show(70412);
	} else if (_action.isAction(words_look, words_volcano_rim))
		_vm->_dialogs->show(70413);
	else if (_action.isAction(words_look, words_bottle) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(70414);
	else if (_action.isAction(words_look, words_open_water_to_south))
		_vm->_dialogs->show(70416);
	else if (_action.isAction(words_look, words_sky))
		_vm->_dialogs->show(70417);
	else
		return;

	_action._inProgress = false;
}

void room_704_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._bottleHotspotId);
	s.syncAsSint16LE(local._boatCurrentFrame);
	s.syncAsSint16LE(local._animationMode);
	s.syncAsSint16LE(local._boatDirection);

	s.syncAsByte(local._takeBottleFl);
}

void room_704_preload() {
	room_init_code_pointer = room_704_init;
	room_daemon_code_pointer = room_704_daemon;
	room_parser_code_pointer = room_704_parser;

	*player.series_name = '\0';
	section_7_interface();
	_scene->addActiveVocab(words_bottle);
	_scene->addActiveVocab(words_look_at);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
