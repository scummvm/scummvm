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

#include "common/util.h"
#include "hollywood/runaway/scene.h"

namespace Hollywood {

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
	if (_roomAction == 54) {
		updateHospitalAudio();
		updateHospitalInventory();
		return;
	}
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
	deferAction(54);
	_actionStage = 0;
	return true;
}

void RunawayRoomScene::updateHospitalInventory() {
	if (speaking()) {
		return;
	}
	const uint elapsed = _now - _actionStarted;
	switch (_actionStage) {
	case 0:
		_actionStage = 1;
		break;
	case 1: // Lower Brian's head before searching the bag.
		if (elapsed < 9 * 60) {
			return;
		}
		_audio.playEffect(32);
		_actionStage = 2;
		break;
	case 2:
		if (_audio.effectPlaying() && elapsed < 1500) {
			return;
		}
		_audio.playEffect(33);
		_actionStage = 3;
		break;
	case 3:
		if (_audio.effectPlaying() && elapsed < 1500) {
			return;
		}
		if (!_inventory.contains(9)) {
			_inventory.add(9, false);
			_actionStep = 2;
		} else if (_inventory.contains(17) && !_inventory.headWig() && !_inventory.contains(10)) {
			_inventory.add(10, false);
			_actionStep = 4;
		} else {
			_actionStep = _random.getRandomBit() ? 3 : 5;
		}
		if (_actionStep == 2 || _actionStep == 4) {
			// The item appears directly in the open inventory.
			_audio.playEffect(1, 255, _audio.effectPlaying() ? 1 : 0);
		}
		_audio.playEffect(32, 255, 1);
		_actionStage = 4;
		break;
	case 4:
		if (_audio.effectPlaying(1) && elapsed < 1500) {
			return;
		}
		_actionStage = 5;
		break;
	case 5: // Return to the speaking pose before describing the result.
		if (elapsed < 11 * 60) {
			return;
		}
		_roomAction = 0;
		say(54, _actionStep, true);
		return;
	}
	_actionStarted = _now;
}

byte RunawayRoomScene::inventoryFrame() const {
	if (!hospitalChapter() || _roomAction != 54) {
		return 0xff;
	}
	const uint frame = (_now - _actionStarted) / 60;
	switch (_actionStage) {
	case 1:
		return 84 + MIN<uint>(8, frame);
	case 2:
	case 3:
	case 4:
		return 92;
	case 5:
		return frame < 10 ? 93 + frame : 0;
	default:
		return 0xff;
	}
}

void RunawayRoomScene::leaveHospitalRoom(uint16 scene, Common::Point position, byte facing) {
	_hospitalExit.scene = scene;
	_hospitalExit.position = position;
	_hospitalExit.facing = facing;
	_audio.playEffect(47);
}

void RunawayRoomScene::updateHospitalAudio() {
	_audio.updateBackgroundMusic(_now, _random, 12, 4);
}

} // End of namespace Hollywood
