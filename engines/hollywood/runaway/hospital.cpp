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

#include "hollywood/runaway/scene.h"

namespace Hollywood {

void RunawayRoomScene::executeItemAction(uint16 action) {
	if (hospitalChapter()) {
		// Native 0x410000 replaces generic item failures with weighted refusals.
		if (action == 1 || action == 5) {
			static const byte actions[] = {15, 17, 3, 24, 2, 31, 32, 5};
			static const byte limits[] = {10, 20, 25, 28, 30, 32, 34, 35};
			const uint roll = _random.getRandomNumber(34);
			for (uint i = 0; i < ARRAYSIZE(actions); ++i) {
				if (roll < limits[i]) {
					action = actions[i];
					break;
				}
			}
		}
		switch (action) {
		case 2: // A variation of "That won't work."
			say(1, _random.getRandomNumber(2), true);
			return;
		case 24: // Think before rejecting the combination.
			say(20, 0, true);
			say(20, 1 + _random.getRandomNumber(2), true);
			return;
		case 29: // Refuse to burn the object.
			say(_random.getRandomNumber(4) ? 23 : 14, 0, true);
			return;
		case 32: // "I don't get it."
			say(26, 0, true);
			return;
		default:
			break;
		}
	}
	execute(action);
}

void RunawayRoomScene::resumeHospital() {
	if (_sequence.active()) {
		return;
	}
	resumeAmbientAudio(_entering);
	if (_entering) {
		runCustomEntrySequence();
		_entering = false;
	}
}

void RunawayRoomScene::updateHospital(uint elapsed) {
	advanceCustomGameplayLoop(elapsed);
	if (!updateCustomSequence()) {
		updateRoomAnimation();
	}
	if (_sequence.active() || _transition.scene) {
		return;
	}
	updateHospitalAudio();
	if (_hospitalExit.scene && !_audio.effectPlaying()) {
		_transition = _hospitalExit;
	}
}

bool RunawayRoomScene::sayHospitalInventory(const RunawayInventory::Reply &reply) {
	if (reply.cue == 68) {
		if (!_hospital.attacked) {
			say(68, 0, true);
		} else if (!_inventory.contains(8)) {
			say(68, 1, true);
		} else {
			say(68, 2, true);
			say(68, 3, true);
			_hospital.medicineRead = 1;
		}
		_audio.playEffect(35, 179);
		return true;
	}
	if ((reply.cue == 69 && reply.variation == 0) || (reply.cue == 73 && reply.variation == 1) ||
			(reply.cue == 72 && reply.variation == 2)) {
		_audio.playEffect(reply.cue == 69 ? 36 : reply.cue == 73 ? 37 : 34, 179);
	}
	if (reply.cue != 54) {
		return false;
	}
	// Searching Gina's bag reveals the matches on the first examination.
	say(54, 1, true);
	if (_inventory.contains(9) && _inventory.contains(17) && !_inventory.headWig() && !_inventory.contains(10)) {
		_inventory.add(10);
		say(54, 4, true);
	} else if (_inventory.contains(9)) {
		say(54, _random.getRandomBit() ? 3 : 5, true);
	} else {
		_inventory.add(9);
		say(54, 2, true);
	}
	return true;
}

void RunawayRoomScene::leaveHospitalRoom(uint16 scene, Common::Point position, byte facing) {
	_hospitalExit.scene = scene;
	_hospitalExit.position = position;
	_hospitalExit.facing = facing;
	_audio.playEffect(47);
}

void RunawayRoomScene::updateHospitalAudio() {
	if (!_audio.musicPlaying()) {
		if (!_nextMusic) {
			static const uint16 delays[] = {0, 240, 120, 60};
			_nextMusic = _now + delays[_random.getRandomNumber(3)] * 250;
		}
		if ((int32)(_now - _nextMusic) >= 0) {
			byte cue;
			do {
				cue = 12 + _random.getRandomNumber(3);
			} while (cue == _musicCue);
			_musicCue = cue;
			_audio.playMusic(cue, false);
			_nextMusic = 0;
		}
	}
}

} // End of namespace Hollywood
