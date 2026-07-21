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
#include "mads/nebular/rooms/section7.h"
#include "mads/nebular/rooms/dialog.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _monsterMode;
	int16 _boatFrame;
	int16 _curSequence;
	int16 _boatDir;

	bool _useBomb;
	bool _startMonsterTimer;
	bool _rexDeathFl;
	bool _restartTrigger70Fl;

	int32 _lastFrameTime;
	int32 _monsterTime;

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
	if (local._boatDir == 2)
		local._curSequence = 6;
	else
		local._curSequence = 7;
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

static void room_703_init() {
	_game._player._visible = false;

	if (!_game._visitedScenes._sceneRevisited) {
		if (_scene->_priorSceneId == 704)
			_globals[kMonsterAlive] = false;
		else
			_globals[kMonsterAlive] = true;
	}

	local._startMonsterTimer = true;
	local._rexDeathFl = true;
	local._monsterTime = 0;
	local._restartTrigger70Fl = true;
	local._useBomb = false;
	local._boatFrame = -1;

	if (!_globals[kMonsterAlive])
		_scene->_hotspots.activate(words_sea_monster, false);

	if (_scene->_priorSceneId == 704) {
		_game._player._stepEnabled = false;
		local._curSequence = 2;
		local._boatDir = 2;
		local._monsterMode = 0;
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(34);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._stepEnabled = false;
		local._boatDir = 1;
		if (_globals[kMonsterAlive]) {
			local._monsterMode = 1;
			local._curSequence = 0;
			_scene->loadAnimation(formAnimName('B', -1));
		} else {
			local._curSequence = 0;
			local._monsterMode = 0;
			_scene->loadAnimation(formAnimName('A', -1));
		}
	} else if (_globals[kMonsterAlive]) {
		local._curSequence = 0;
		local._boatDir = 1;
		local._monsterMode = 1;
		_scene->loadAnimation(formAnimName('B', -1));
		_scene->_animation[0]->setCurrentFrame(39);
	} else if (local._boatDir == 1) {
		local._curSequence = 0;
		local._monsterMode = 0;
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(9);
	} else if (local._boatDir == 2) {
		local._curSequence = 0;
		local._monsterMode = 0;
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(56);
	}

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_TWINKIFRUIT);
		_game._objects.addToInventory(OBJ_BOMB);
		_game._objects.addToInventory(OBJ_CHICKEN);
		_game._objects.addToInventory(OBJ_BONES);
	}

	_game.loadQuoteSet(0x311, 0x312, 0x313, 0x314, 0x315, 0);
	local._dialog1.setup(0x98, 0x311, 0x312, 0x313, 0x314, 0x315, 0);
	section_7_music();
	_vm->_sound->command(28);
}

static void room_703_daemon() {
	if (local._startMonsterTimer) {
		long diff = _scene->_frameStartTime - local._lastFrameTime;
		if ((diff >= 0) && (diff <= 12))
			local._monsterTime += diff;
		else
			local._monsterTime++;

		local._lastFrameTime = _scene->_frameStartTime;
	}

	if ((local._monsterTime >= 2400) && !local._rexDeathFl && !local._useBomb) {
		local._startMonsterTimer = false;
		local._rexDeathFl = true;
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		local._monsterMode = 3;
		_scene->loadAnimation(formAnimName('D', -1));
		local._rexDeathFl = false;
		local._monsterTime = 0;
	}


	if (_game._trigger == 70)
		_scene->_reloadSceneFlag = true;

	if ((local._monsterMode == 3) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != local._boatFrame) {
			local._boatFrame = _scene->_animation[0]->getCurrentFrame();
			int nextBoatFrame = -1;

			if (local._boatFrame == 62) {
				nextBoatFrame = 61;
				if (local._restartTrigger70Fl) {
					local._restartTrigger70Fl = false;
					_scene->_sequences.addTimer(15, 70);
				}
			}

			if ((nextBoatFrame >= 0) && (nextBoatFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextBoatFrame);
				local._boatFrame = nextBoatFrame;
			}
		}
	}

	if (_game._trigger == 70)
		_scene->_reloadSceneFlag = true;

	if ((local._monsterMode == 0) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != local._boatFrame) {
			local._boatFrame = _scene->_animation[0]->getCurrentFrame();
			int nextBoatFrame = -1;

			switch (local._boatFrame) {
			case 11:
				if (local._curSequence == 7) {
					local._curSequence = 0;
					nextBoatFrame = 100;
				} else if (local._curSequence == 5)
					nextBoatFrame = 82;
				else if (local._curSequence == 1)
					nextBoatFrame = 11;
				else {
					nextBoatFrame = 9;
					if (!_game._player._stepEnabled)
						_game._player._stepEnabled = true;
				}
				break;

			case 34:
				if (local._curSequence != 2)
					_scene->_nextSceneId = 704;
				break;

			case 57:
				if (local._curSequence == 6) {
					local._curSequence = 0;
					nextBoatFrame = 91;
				} else if (local._curSequence == 4)
					nextBoatFrame = 73;
				else if (local._curSequence == 3)
					nextBoatFrame = 57;
				else {
					nextBoatFrame = 56;
					if (!_game._player._stepEnabled)
						_game._player._stepEnabled = true;
				}
				break;

			case 73:
				_scene->_nextSceneId = 701;
				break;

			case 82:
				nextBoatFrame = 11;
				break;

			case 91:
				nextBoatFrame = 57;
				break;

			case 100:
				nextBoatFrame = 56;
				if (!_game._player._stepEnabled) {
					_scene->_sequences.addTimer(30, 80);
					_game._player._stepEnabled = true;
				}
				break;

			case 110:
				nextBoatFrame = 9;
				if (!_game._player._stepEnabled) {
					_scene->_sequences.addTimer(30, 80);
					_game._player._stepEnabled = true;
				}
				break;

			default:
				break;
			}

			if ((nextBoatFrame >= 0) && (nextBoatFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextBoatFrame);
				local._boatFrame = nextBoatFrame;
			}
		}
	}

	if (_game._trigger == 80) {
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


	if ((local._monsterMode == 1) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != local._boatFrame) {
			local._boatFrame = _scene->_animation[0]->getCurrentFrame();
			int nextBoatFrame = -1;

			switch (local._boatFrame) {
			case 39:
				_game._player._stepEnabled = true;
				local._startMonsterTimer = true;
				local._rexDeathFl = false;
				break;

			case 40:
			case 49:
			case 54:
			case 67:
			case 78:
			case 87:
			case 96:
			case 105:
			case 114:
			case 123:
				if (local._curSequence == 8)
					nextBoatFrame = 129;

				break;

			case 129:
				nextBoatFrame = 39;
				break;

			case 151:
				_scene->_nextSceneId = 701;
				break;

			default:
				break;
			}

			if ((nextBoatFrame >= 0) && (nextBoatFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextBoatFrame);
				local._boatFrame = nextBoatFrame;
			}
		}
	}

	if ((local._monsterMode == 2) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != local._boatFrame) {
			local._boatFrame = _scene->_animation[0]->getCurrentFrame();
			int nextBoatFrame = -1;

			switch (local._boatFrame) {
			case 14:
				if (!local._useBomb) {
					if (_game._difficulty == DIFFICULTY_HARD)
						_game._objects.setRoom(OBJ_CHICKEN, 1);
					else
						_vm->_dialogs->show(70319);
				}
				nextBoatFrame = 80;
				break;

			case 33:
				if (_game._objects.isInInventory(OBJ_BONES)) {
					_game._objects.setRoom(OBJ_BONES, 1);
					_game._objects.addToInventory(OBJ_BONE);
				} else
					_game._objects.setRoom(OBJ_BONE, 1);

				nextBoatFrame = 80;
				break;

			case 53:
				_game._objects.setRoom(OBJ_TWINKIFRUIT, 1);
				nextBoatFrame = 80;
				local._curSequence = 9;
				break;

			case 80:
				if (_game._difficulty == DIFFICULTY_HARD) {
					_game._objects.setRoom(OBJ_BOMB, 1);
					_vm->_dialogs->show(70318);
				} else
					_vm->_dialogs->show(70317);

				_scene->freeAnimation();
				local._monsterMode = 1;
				_scene->loadAnimation(formAnimName('B', -1));
				_scene->_animation[0]->setCurrentFrame(39);
				_game._player._stepEnabled = true;
				break;

			case 91:
				if (!local._useBomb) {
					_scene->freeAnimation();
					local._monsterMode = 1;
					_scene->loadAnimation(formAnimName('B', -1));
					_scene->_animation[0]->setCurrentFrame(39);
					_game._player._stepEnabled = true;
				} else
					_game._objects.setRoom(OBJ_CHICKEN_BOMB, 1);

				break;

			case 126:
				_scene->_hotspots.activate(words_sea_monster, false);
				_globals[kMonsterAlive] = false;
				_scene->freeAnimation();
				local._monsterMode = 0;
				_scene->loadAnimation(formAnimName('A', -1));
				_scene->_animation[0]->setCurrentFrame(9);
				_game._player._stepEnabled = true;
				if (_game._storyMode == STORYMODE_NAUGHTY)
					_vm->_dialogs->show(70321);
				else
					_vm->_dialogs->show(70322);

				break;

			default:
				break;
			}

			if ((nextBoatFrame >= 0) && (nextBoatFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextBoatFrame);
				local._boatFrame = nextBoatFrame;
			}
		}
	}
}

static void room_703_parser() {
	if (_game._screenObjects._inputMode == kInputConversation)
		handleFillBottle(_action._activeAction._verbId);
	else if (player_said_2(steer_towards, dock_to_south)) {
		_game._player._stepEnabled = false;
		if (_globals[kMonsterAlive])
			local._curSequence = 8;
		else if (local._boatDir == 1)
			local._curSequence = 5;
		else
			local._curSequence = 3;
	} else if (player_said_2(steer_towards, building_to_north)) {
		_game._player._stepEnabled = false;
		if (_globals[kMonsterAlive]) {
			local._startMonsterTimer = false;
			local._rexDeathFl = true;
			local._monsterTime = 0;
			_scene->freeAnimation();
			local._monsterMode = 3;
			_scene->loadAnimation(formAnimName('D', -1));
		} else if (local._boatDir == 2)
			local._curSequence = 4;
		else
			local._curSequence = 1;
	} else if (player_said_3(throw, bone, sea_monster) || player_said_3(throw, bones, sea_monster)) {
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		local._monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
		_scene->_animation[0]->setCurrentFrame(19);
	} else if (player_said_3(throw, chicken, sea_monster)) {
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		local._monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
	} else if (player_said_3(throw, twinkifruit, sea_monster)) {
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		local._monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
		_scene->_animation[0]->setCurrentFrame(39);
	} else if (player_said_3(throw, bomb, sea_monster)) {
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		local._monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
		_scene->_animation[0]->setCurrentFrame(59);
	} else if (player_said_3(throw, chicken_bomb, sea_monster)) {
		local._useBomb = true;
		_game._player._stepEnabled = false;
		_scene->freeAnimation();
		local._monsterMode = 2;
		_scene->loadAnimation(formAnimName('C', -1));
	} else if (player_said_3(put, bottle, water) || player_said_3(fill, bottle, water)) {
		if (_globals[kBottleStatus] != 4) {
			handleBottleInterface();
			local._dialog1.start();
		} else
			_vm->_dialogs->show(70323);
	} else if (_action._lookFlag || player_said_2(look, sea_monster)) {
		if (_globals[kMonsterAlive])
			_vm->_dialogs->show(70310);
	} else if (player_said_2(look, water)) {
		if (!_globals[kMonsterAlive])
			_vm->_dialogs->show(70311);
		else
			_vm->_dialogs->show(70312);
	} else if (player_said_2(look, building_to_north)) {
		if (_globals[kMonsterAlive])
			_vm->_dialogs->show(70313);
		else if (_game._visitedScenes.exists(710))
			_vm->_dialogs->show(70314);
		else
			_vm->_dialogs->show(70315);
	} else if (player_said_2(look, volcano_rim))
		_vm->_dialogs->show(70316);
	else
		return;

	_action._inProgress = false;
}

void room_703_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._monsterMode);
	s.syncAsSint16LE(local._boatFrame);
	s.syncAsSint16LE(local._curSequence);
	s.syncAsSint16LE(local._boatDir);

	s.syncAsByte(local._useBomb);
	s.syncAsByte(local._startMonsterTimer);
	s.syncAsByte(local._rexDeathFl);
	s.syncAsByte(local._restartTrigger70Fl);

	s.syncAsUint32LE(local._lastFrameTime);
	s.syncAsUint32LE(local._monsterTime);
}

void room_703_preload() {
	room_init_code_pointer = room_703_init;
	room_daemon_code_pointer = room_703_daemon;
	room_parser_code_pointer = room_703_parser;

	*player.series_name = '\0';
	section_7_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
