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

#include "common/textconsole.h"
#include "common/file.h"
#include "common/util.h"
#include "hollywood/runaway/scene.h"
#include "hollywood/runaway/scene_data.h"

namespace Hollywood {
namespace {

// RESOURCE.B02; approach points and action IDs from the retail room at 0x444aa0.
const RunawayHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Bathroom door", 1265, 546, 1, {301, 346}, {0, 1}},
	{"Gina", 1075, 534, 7, {302, 303}, {0, 0}},
	{"Glass", 846, 453, 1, {304, 305}, {1, 1}},
	{"Package of pills", 858, 453, 1, {306, 307}, {1, 1}},
	{"Sheet", 622, 457, 7, {308, 309}, {0, 1}},
	{"Locker", 417, 444, 7, {310, 311}, {0, 0}},
	{"Gina's bag", 417, 444, 7, {312, 313}, {0, 1}},
	{"Window", 265, 456, 7, {314, 332}, {1, 1}},
	{"Gina's medical chart", 690, 540, 1, {315, 316}, {1, 1}},
	{"Chart", 453, 455, 1, {317, 318}, {1, 1}},
	{"Fake patient", 687, 448, 7, {319, 320}, {0, 1}},
	{"Fire sprinkler", 459, 473, 7, {321, 322}, {0, 0}},
	{"Buzzer", 846, 453, 7, {323, 324}, {0, 0}},
	{"Buzzer", 712, 445, 3, {323, 324}, {0, 0}},
	{"Radiator", 265, 456, 7, {325, 326}, {0, 0}},
	{"Poster", 713, 533, 1, {327, 328}, {0, 0}},
	{"Poster", 454, 455, 1, {329, 328}, {0, 0}},
	{"Emergency exit map", 1265, 546, 1, {330, 331}, {1, 0}},
	{"Bed", 404, 456, 1, {344, 345}, {0, 0}},
};

const RunawayResponse kResponses[] = {
	{kHospitalBedroom, 301, 1, 0},
	{kHospitalBedroom, 304, 4, 0},
	{kHospitalBedroom, 306, 5, 0},
	{kHospitalBedroom, 308, 6, 0},
	{kHospitalBedroom, 310, 7, 0},
	{kHospitalBedroom, 311, 8, 0},
	{kHospitalBedroom, 312, 9, 0},
	{kHospitalBedroom, 320, 16, 0},
	{kHospitalBedroom, 321, 17, 0},
	{kHospitalBedroom, 322, 18, 0},
	{kHospitalBedroom, 323, 19, 0},
	{kHospitalBedroom, 325, 21, 0},
	{kHospitalBedroom, 326, 22, 0},
	{kHospitalBedroom, 327, 23, 0},
	{kHospitalBedroom, 328, 24, 0},
	{kHospitalBedroom, 329, 25, 0},
	{kHospitalBedroom, 331, 27, 0},
	{kHospitalBedroom, 333, 29, 0},
	{kHospitalBedroom, 344, 39, 0},
	{kHospitalBedroom, 345, 40, 0},
};

const RunawayItemAction kItemActions[] = {
	{kHospitalBedroom, 5, 6, 333, false},
	{kHospitalBedroom, 6, 9, 334, false},
	{kHospitalBedroom, 6, 10, 335},
	{kHospitalBedroom, 9, 12, 343},  // Matches at the sprinkler.
	{kHospitalBedroom, 10, 2, 336, false},
	{kHospitalBedroom, 13, 2, 349},  // Water for Gina.
	{kHospitalBedroom, 14, 19, 341, false},
	{kHospitalBedroom, 15, 2, 337, false},
	{kHospitalBedroom, 16, 9, 339},
	{kHospitalBedroom, 16, 10, 338, false},
	{kHospitalBedroom, 17, 19, 340},
	{kHospitalBedroom, 18, 12, 342},
};

// Retail chapter-one film scripts, from 0x43c450..0x4403cb.
const RunawaySequence::Command kGinaStory[] = {
	{RunawaySequence::kOpen, 9},
	{RunawaySequence::kMusic, 11, 255, 0},
	{RunawaySequence::kPlay, 0, 28},
	{RunawaySequence::kTalk, 83, 0, 1, 0},
	{RunawaySequence::kTalk, 83, 1, 3, 0},
	{RunawaySequence::kTalk, 83, 2, 1, 0},
	{RunawaySequence::kTalk, 83, 3, 3, 0},
	{RunawaySequence::kTalk, 83, 4, 1, 0},
	{RunawaySequence::kTalk, 83, 5, 5, 0},
	{RunawaySequence::kSpeech, 83, 6, 0},
	{RunawaySequence::kPlay, 65, 105, RunawaySequence::kIdleDuringSpeech},
	{RunawaySequence::kMusic, 29, 255, 0},
	{RunawaySequence::kPlay, 106, 214},
	{RunawaySequence::kEffect, 25, 102, 1, 1},
	{RunawaySequence::kTalk, 83, 7, 7, 0},
	{RunawaySequence::kPlay, 222, 254},
	{RunawaySequence::kMusic, 25, 255, 1},
	{RunawaySequence::kTalk, 83, 8, 9, 0},
	{RunawaySequence::kTalk, 83, 9, 11, 0},
	{RunawaySequence::kTalk, 83, 10, 9, 0},
	{RunawaySequence::kTalk, 83, 11, 11, 0},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kPlay, 1608, 1632},
	{RunawaySequence::kSpeech, 83, 12, 0},
	{RunawaySequence::kPlay, 1633, 1687, RunawaySequence::kIdleDuringSpeech},
	{RunawaySequence::kTalk, 83, 13, 5, 0},
	{RunawaySequence::kMusic, 26, 255, 0},
	{RunawaySequence::kTalk, 83, 14, 5, 1},
	{RunawaySequence::kTalk, 83, 15, 5, 0},
	{RunawaySequence::kSpeech, 83, 16, 0},
	{RunawaySequence::kPlay, 1632, 1687, RunawaySequence::kIdleDuringSpeech},
	{RunawaySequence::kMusic, 27, 255, 1},
	{RunawaySequence::kPlay, 1688, 1713},
	{RunawaySequence::kTalk, 83, 17, 15, 0},
	{RunawaySequence::kPlay, 302, 324},
	{RunawaySequence::kTalk, 83, 18, 13, 0},
	{RunawaySequence::kEffect, 22, 255, 0, 0},
	{RunawaySequence::kPlay, 336, 350},
	{RunawaySequence::kSpeech, 83, 19, 22},
	{RunawaySequence::kPlay, 351, 990},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kTalk, 83, 23, 15, 0},
	{RunawaySequence::kPlay, 1003, 1025},
	{RunawaySequence::kTalk, 83, 24, 17, 0},
	{RunawaySequence::kPlay, 1038, 1117},
	{RunawaySequence::kTalk, 83, 25, 19, 0},
	{RunawaySequence::kEffect, 23, 255, 0, 0},
	{RunawaySequence::kPlay, 1129, 1146},
	{RunawaySequence::kSpeech, 83, 26, 0},
	{RunawaySequence::kPlay, 1147, 1208},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kTalk, 83, 27, 21, 0},
	{RunawaySequence::kTalk, 83, 28, 23, 0},
	{RunawaySequence::kEffect, 24, 255, 0, 0},
	{RunawaySequence::kPlay, 1232, 1270},
	{RunawaySequence::kSpeech, 83, 29, 0},
	{RunawaySequence::kPlay, 1271, 1300},
	{RunawaySequence::kSpeech, 83, 30, 0},
	{RunawaySequence::kMusic, 24, 255, 1},
	{RunawaySequence::kPlay, 1301, 1360},
	{RunawaySequence::kSpeech, 83, 31, 0},
	{RunawaySequence::kPlay, 1361, 1560},
	{RunawaySequence::kMusic, 11, 255, 0},
	{RunawaySequence::kSpeech, 83, 32, 0},
	{RunawaySequence::kPlay, 1561, 1590},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kTalk, 83, 33, 25, 0},
	{RunawaySequence::kTalk, 83, 34, 27, 0},
	{RunawaySequence::kEnd},
};

const RunawaySequence::Command kLedgeCrossing[] = {
	{RunawaySequence::kOpen, 1},
	{RunawaySequence::kMusic, 28, 255, 1},
	{RunawaySequence::kEffect, 19, 255, 0, 0},
	{RunawaySequence::kPlay, 0, 151},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kEffect, 21, 255, 1, 1},
	{RunawaySequence::kTalk, 70, 0, 1, 0},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kEffect, 20, 255, 0, 0},
	{RunawaySequence::kPlay, 163, 315},
	{RunawaySequence::kMusic, 11, 255, 0},
	{RunawaySequence::kEnd},
};

const RunawaySequence::Command kBuildDecoy[] = {
	{RunawaySequence::kOpen, 3},
	{RunawaySequence::kEffect, 26, 255, 0, 0},
	{RunawaySequence::kPlay, 0, 75},
	{RunawaySequence::kEnd},
};

const RunawaySequence::Command kHospitalAmbush[] = {
	{RunawaySequence::kOpen, 5},
	{RunawaySequence::kMusic, 21, 255, 0},
	{RunawaySequence::kEffect, 11, 255, 0, 0},
	{RunawaySequence::kPlay, 0, 204},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kTalk, 81, 0, 0, 0},
	{RunawaySequence::kPlay, 204, 310},
	{RunawaySequence::kEffect, 12, 255, 0, 0},
	{RunawaySequence::kPlay, 311, 318},
	{RunawaySequence::kDelay, 2000},
	{RunawaySequence::kPlay, 318, 388},
	{RunawaySequence::kTalk, 81, 1, 0, 0},
	{RunawaySequence::kMusic, 22, 255, 0},
	{RunawaySequence::kPlay, 388, 412},
	{RunawaySequence::kEffect, 13, 255, 0, 0},
	{RunawaySequence::kPlay, 413, 496},
	{RunawaySequence::kSpeech, 81, 2, 0},
	{RunawaySequence::kPlay, 497, 531},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kEffect, 15, 255, 0, 0},
	{RunawaySequence::kWaitEffect, 3000},
	{RunawaySequence::kEffect, 14, 255, 0, 0},
	{RunawaySequence::kDelay, 1000},
	{RunawaySequence::kSpeech, 81, 3, 0},
	{RunawaySequence::kPlay, 531, 590},
	{RunawaySequence::kSpeech, 81, 4, 0},
	{RunawaySequence::kPlay, 591, 679},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kMusic, 11, 255, 0},
	{RunawaySequence::kEnd},
};

const RunawaySequence::Command kHospitalEscape[] = {
	{RunawaySequence::kOpen, 6},
	{RunawaySequence::kMusic, 23, 255, 0},
	{RunawaySequence::kEffect, 39, 204, 0, 0},
	{RunawaySequence::kPlay, 0, 118},
	{RunawaySequence::kSpeech, 82, 0, 0},
	{RunawaySequence::kPlay, 119, 160},
	{RunawaySequence::kSpeech, 82, 1, 0},
	{RunawaySequence::kPlay, 161, 210},
	{RunawaySequence::kSpeech, 82, 2, 0},
	{RunawaySequence::kPlay, 211, 248},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kMusic, 24, 255, 1},
	{RunawaySequence::kSpeech, 82, 3, 5},
	{RunawaySequence::kPlay, 249, 649, RunawaySequence::kLoopDuringSpeech},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kEffect, 40, 255, 0, 0},
	{RunawaySequence::kPlay, 650, 950},
	{RunawaySequence::kOpen, 2},
	{RunawaySequence::kMusic, 18, 255, 1},
	{RunawaySequence::kEffect, 30, 3, 1, 1},
	{RunawaySequence::kEffect, 16, 255, 0, 0},
	{RunawaySequence::kPlay, 0, 100},
	{RunawaySequence::kSpeech, 80, 0, 0},
	{RunawaySequence::kSpeech, 80, 2, 0},
	{RunawaySequence::kPlay, 101, 400},
	{RunawaySequence::kEffect, 18, 51, 0, 0},
	{RunawaySequence::kPlay, 401, 719, RunawaySequence::kUntilSpeechEnds},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kPlay, 875, 932},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kTalk, 80, 3, 1, 0},
	{RunawaySequence::kPlay, 820, 863},
	{RunawaySequence::kTalk, 80, 4, 3, 0},
	{RunawaySequence::kTalk, 80, 5, 1, 0},
	{RunawaySequence::kTalk, 80, 6, 3, 0},
	{RunawaySequence::kTalk, 80, 7, 1, 0},
	{RunawaySequence::kEffect, 30, 3, 1, 1},
	{RunawaySequence::kSpeech, 80, 8, 11},
	{RunawaySequence::kPlay, 200, 719, RunawaySequence::kLoopDuringSpeech},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kTalk, 80, 12, 3, 0},
	{RunawaySequence::kTalk, 80, 13, 3, 1},
	{RunawaySequence::kEffect, 30, 3, 1, 1},
	{RunawaySequence::kEffect, 17, 255, 0, 0},
	{RunawaySequence::kSpeech, 80, 14, 0},
	{RunawaySequence::kPlay, 720, 819},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kStopEffects},
	{RunawaySequence::kOpen, 7},
	{RunawaySequence::kMusic, 31, 255, 1},
	{RunawaySequence::kPlay, 0, 36},
	{RunawaySequence::kTalk, 84, 1, 0, 0},
	{RunawaySequence::kTalk, 84, 2, 0, 0},
	{RunawaySequence::kTalk, 84, 3, 0, 0},
	{RunawaySequence::kTalk, 84, 4, 0, 0},
	{RunawaySequence::kTalk, 84, 5, 0, 0},
	{RunawaySequence::kTalk, 84, 6, 0, 0},
	{RunawaySequence::kTalk, 84, 7, 0, 0},
	{RunawaySequence::kEffect, 44, 153, 0, 0},
	{RunawaySequence::kPlay, 37, 40},
	{RunawaySequence::kSpeech, 84, 8, 0},
	{RunawaySequence::kPlay, 41, 236},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kTalk, 84, 9, 1, 0},
	{RunawaySequence::kTalk, 84, 10, 3, 0},
	{RunawaySequence::kTalk, 84, 11, 5, 0},
	{RunawaySequence::kTalk, 84, 12, 3, 0},
	{RunawaySequence::kTalk, 84, 13, 5, 0},
	{RunawaySequence::kTalk, 84, 14, 3, 0},
	{RunawaySequence::kTalk, 84, 15, 3, 1},
	{RunawaySequence::kTalk, 84, 16, 5, 0},
	{RunawaySequence::kPlay, 272, 284},
	{RunawaySequence::kSpeech, 84, 17, 0},
	{RunawaySequence::kPlay, 285, 371},
	{RunawaySequence::kWaitSpeech},
	{RunawaySequence::kDelay, 1000},
	{RunawaySequence::kEnd},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene2020 = {
	kHospitalBedroom, 1520, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {60, 60, 80}, {80, 80, 100}},
	Common::Point(1075, 534),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

bool RunawayRoomScene::loadScene2020() {
	return loadForeground(0, 12) && loadForeground(1, 14) && loadForeground(2, 11, true) &&
		loadForeground(3, 15) && _safe.load(_archive, 5, 1) &&
		_ambient.load(_archive, 22, 19) && _overlay.load(_archive, 17, 1) &&
		_archive.readVariableChunk(20, _monitorPatches[0]) && _archive.readVariableChunk(21, _monitorPatches[1]);
}

void RunawayRoomScene::start2020() {
	for (byte item = 1; item <= 4; ++item)
		_inventory.add(item);
	_inventory.takeNewItem();
	_facing = 7;
	startHospitalFilm(kGinaStoryFilm);
}

bool RunawayRoomScene::applyBackground2020() {
	if ((_hospital.glassTaken && !applyPatch(8)) || (_hospital.pillsTaken && !applyPatch(9)) ||
			(_hospital.sheetTaken && !applyPatch(7)) || (_hospital.bagTaken && !applyPatch(6)))
		return false;
	return (!_hospital.decoy || applyPatch(13)) && applyPatch(11) && applyRunawayPatch(_monitorPatches[1], _background) &&
		applyRunawayPatch(_monitorPatches[0], _background);
}

void RunawayRoomScene::updateMaps2020() {
	for (uint i = 0; i < 256; ++i) {
		// Zones 5/6 are reserved for scripted movement, not normal walking.
		if (_maps[i] > 4)
			_maps[i] = 0;
		byte &id = _maps[256 + i];
		if ((_hospital.glassTaken && id == 3) || (_hospital.pillsTaken && id == 4) ||
				(_hospital.bagTaken && id == 7) || (id == 11 && !_hospital.decoy))
			id = 0;
		else if (_hospital.decoy && (id == 20 || id == 21))
			id = 11;
		else if (id == 5 || id == 21)
			id = _hospital.sheetTaken ? 19 : 5;
		else if (id == 20)
			id = 19;
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
	if (reply.cue != 54)
		return false;
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

RunawayRoomScene::ActionResult RunawayRoomScene::execute2020(uint16 action) {
	switch (action) {
	case 302:
		say(2, _random.getRandomNumber(2) + (_hospital.attacked ? 3 : 0));
		return kHandled;
	case 303:
		say(3, _random.getRandomBit() + (_hospital.attacked ? 2 : 0));
		return kHandled;
	case 305:
		_hospital.glassTaken = 1;
		_inventory.add(13);
		_facing = 4;
		say(62, 0, true);
		return kBackgroundChanged;
	case 307:
		_hospital.pillsTaken = 1;
		_inventory.add(8);
		_facing = 4;
		say(57, 0, true);
		return kBackgroundChanged;
	case 309:
		_hospital.sheetTaken = 1;
		_inventory.add(7);
		_facing = 4;
		say(18, _random.getRandomNumber(2), true);
		return kBackgroundChanged;
	case 313:
		_hospital.bagTaken = 1;
		_inventory.add(5);
		_facing = 6;
		say(9, 1);
		return kBackgroundChanged;
	case 330:
		if (!_hospital.exitMapRead)
			say(26);
		say(26, _hospital.window ? 2 : 1);
		_hospital.exitMapRead = 1;
		return kHandled;
	case 332:
		if (!_hospital.exitMapRead) {
			say(28);
			return kHandled;
		}
		if (!_hospital.window)
			say(28, 1);
		_hospital.window = MIN<int>(2, _hospital.window + 1);
		beginRoomAnimation(action, 4, 42);
		return kHandled;
	case 314:
		say(10, _hospital.window >= 2);
		return kHandled;
	case 315:
	case 316:
		say(action == 315 ? 11 : 12, _hospital.chartReplaced);
		return kHandled;
	case 317:
	case 318:
		say(action == 317 ? 13 : 14, _hospital.chartMoved);
		return kHandled;
	case 319:
		say(15, _hospital.attacked);
		return kHandled;
	case 324:
		say(20, _hospital.attacked);
		return kHandled;
	case 334:
		say(30);
		return kHandled;
	case 335:
		if (!_hospital.decoy) {
			say(31);
			return kHandled;
		}
		beginRoomAnimation(action, 16, 29);
		return kHandled;
	case 336:
		say(32);
		return kHandled;
	case 337:
		if (_hospital.attacked)
			say(33);
		else
			say(4, 0, true);
		return kHandled;
	case 338:
		if (_inventory.chartWritten())
			say(4, 0, true);
		else
			say(34);
		return kHandled;
	case 339: {
		if (!_inventory.chartWritten()) {
			say(34);
			return kHandled;
		}
		beginRoomAnimation(action, 19, 35);
		_actionFrames.push_back(33);
		for (int frame = 32; frame >= 0; --frame)
			_actionFrames.push_back(frame);
		return kHandled;
	}
	case 340:
	case 341:
		if (!_inventory.contains(17))
			say(36, 5);
		else if (!_inventory.headWig())
			say(36, action == 340 ? 2 : 6);
		else if (!_inventory.contains(14))
			say(36);
		else if (!_inventory.contains(7))
			say(36, 1);
		else {
			say(36, 3);
			_roomAction = 340;
			_actionStage = 0;
		}
		return kHandled;
	case 342:
		if (!_hospital.attacked || !_hospital.medicineRead) {
			if (_inventory.contains(9))
				say(37, 1);
			else
				say(1, 2, true);
		} else if (!_inventory.contains(9)) {
			say(37);
		} else {
			say(37, 2);
			_roomAction = action;
		}
		return kHandled;
	case 343:
		say(_hospital.attacked && _hospital.medicineRead ? 38 : 37, _hospital.attacked && _hospital.medicineRead ? 0 : 1);
		return kHandled;
	case 346:
		leaveHospitalRoom(kHospitalBathroom, Common::Point(0, 596), 1);
		return kHandled;
	case 349:
		if (_hospital.attacked)
			say(41, _inventory.water() != 0);
		else
			say(4, 0, true);
		return kHandled;
	default:
		return kUnhandled;
	}
}

float RunawayRoomScene::actorHeight2020() const {
	// The original switches perspective planes at x = 864 (0x443be0).
	return _position.x < 864 ? _position.y * 1.26388884f - 302.069458f :
		_position.y * 0.44444445f + 188.777771f;
}

void RunawayRoomScene::update2020() {
	if (_hospitalEntryStep && !speaking() && _pathIndex == _path.size()) {
		switch (_hospitalEntryStep++) {
		case 1:
			_facing = 4;
			say(0, 1);
			break;
		case 2:
			walkTo(Common::Point(1075, 599));
			break;
		case 3:
			say(0, 2);
			break;
		case 4:
			_facing = 3;
			say(0, 3);
			_hospitalEntryStep = 0;
			break;
		case 5:
			_facing = 4;
			say(81, 6);
			_hospitalEntryStep = 0;
			break;
		case 6:
			say(36, 4);
			_hospitalEntryStep = 0;
			break;
		default:
			_hospitalEntryStep = 0;
			break;
		}
	}
	if (!speaking() && _pathIndex == _path.size()) {
		if (_roomAction == 340) {
			if (!_actionStage) {
				walkTo(Common::Point(531, 416), 3, true);
				_actionStage = 1;
			} else {
				startHospitalFilm(kDecoyFilm);
			}
		} else if (_roomAction == 342) {
			startHospitalFilm(kEscapeFilm);
		}
	}
	if (_now - _ambientTick >= 80) {
		_ambientTick = _now;
		if ((_ambientFrame > 0 && _ambientFrame < 18) || !_random.getRandomNumber(49)) {
			_ambientFrame += _ambientDirection;
			if (_ambientFrame == 18 || _ambientFrame == 0)
				_ambientDirection = -_ambientDirection;
		}
	}
	// The bedside indicator alternates after 17 and 9 animation ticks.
	applyRunawayPatch(_monitorPatches[(_now / 80) % 26 >= 17], _background);
}

void RunawayRoomScene::adaptGogFilm(const RunawaySequence::Command *commands) {
	typedef RunawaySequence S;
	_filmCommands.clear();
	uint movie = 0, part = 1;
	for (uint i = 0; commands[i].operation != S::kEnd; ++i) {
		S::Command command = commands[i];
		if (command.operation == S::kOpen) {
			movie = command.a;
			part = 1;
		}
		if (movie == 2 && command.operation == S::kPlay) {
			if (command.a >= 820)
				continue; // These CD shots are separate stills in the GOG edition.
			uint targetPart = command.a >= 720 ? 4 : command.a >= 200 ? 3 : 1;
			if (command.a == 101) {
				_filmCommands.push_back(S::Command(S::kPlay, 101, 199));
				targetPart = 3;
				command.a = 200;
			}
			if (part != targetPart) {
				_filmCommands.push_back(S::Command(S::kOpen, movie, targetPart));
				part = targetPart;
			}
			const uint offset = part == 4 ? 720 : part == 3 ? 200 : 0;
			command.a -= offset;
			command.b -= offset;
		} else if (movie == 2 && command.operation == S::kTalk && command.c) {
			_filmCommands.push_back(S::Command(S::kStill, command.c == 1 ? 0 : 1, 1));
			command.c = command.c == 1 ? 3 : 1;
		} else if (movie == 7 && command.operation == S::kTalk && command.b >= 9) {
			static const uint16 stills[] = {236, 237, 238, 239, 240, 241, 241, 242};
			_filmCommands.push_back(S::Command(S::kStill, stills[command.b - 9]));
		} else if (movie == 9 && command.operation == S::kPlay) {
			uint targetPart = 1;
			if (command.a == 1608) {
				targetPart = 2; command.a = 0; command.b = 23;
			} else if (command.a == 1632 || command.a == 1633) {
				targetPart = 3; command.a = 0; command.b = 54;
			} else if (command.a == 1688) {
				targetPart = 4; command.a = 0; command.b = 24;
			} else if (command.a == 65) {
				command.a = 66;
			} else if (command.a == 106) {
				command.b = 215;
			} else if (command.a >= 222 && command.a <= 1147) {
				++command.a;
				++command.b;
			}
			if (part != targetPart) {
				_filmCommands.push_back(S::Command(S::kOpen, movie, targetPart));
				part = targetPart;
			}
		} else if (movie == 9 && command.operation == S::kTalk) {
			static const uint16 stills[] = {
				29, 42, 43, 44, 45, 53, 0, 216, 255, 257, 258, 259, 0, 260, 260, 260,
				0, 302, 325, 0, 0, 0, 0, 991, 1026, 1118, 0, 1209, 1222, 0, 0, 0, 0, 1591, 1598
			};
			if (part != 1) {
				_filmCommands.push_back(S::Command(S::kOpen, movie));
				part = 1;
			}
			_filmCommands.push_back(S::Command(S::kStill, stills[command.b]));
		}
		_filmCommands.push_back(command);
	}
	_filmCommands.push_back(S::Command(S::kEnd));
}

void RunawayRoomScene::startHospitalFilm(HospitalFilm film) {
	const RunawaySequence::Command *commands = nullptr;
	switch (film) {
	case kGinaStoryFilm: commands = kGinaStory; break;
	case kLedgeFilm: commands = kLedgeCrossing; break;
	case kDecoyFilm: commands = kBuildDecoy; break;
	case kAmbushFilm: commands = kHospitalAmbush; break;
	case kEscapeFilm: commands = kHospitalEscape; break;
	}
	_hospitalFilm = film;
	_roomAction = 0;
	_actionFrames.clear();
	_caption.clear();
	if (Common::File::exists("DATAVC00.000")) {
		adaptGogFilm(commands);
		commands = _filmCommands.data();
	}
	_sequence.start(commands, _text, _now);
}

void RunawayRoomScene::finishHospitalFilm() {
	switch (_hospitalFilm) {
	case kGinaStoryFilm:
		_hospitalEntryStep = 1;
		say(0);
		break;
	case kLedgeFilm:
		_transition.scene = kHospitalStoreroom;
		_transition.position = Common::Point(419, 546);
		_transition.facing = 3;
		break;
	case kDecoyFilm:
		_hospital.decoy = 1;
		_inventory.remove(7);
		_inventory.remove(14);
		_inventory.remove(17);
		if (!rebuildBackground())
			error("Cannot build Runaway hospital decoy");
		updateMaps();
		walkTo(Common::Point(404, 456), 1, true);
		_hospitalEntryStep = 6;
		break;
	case kAmbushFilm:
		_hospitalEntryStep = 5;
		say(81, 5);
		break;
	case kEscapeFilm:
		_chapterComplete = true;
		_audio.stopMusic();
		return;
	}
	_audio.playMusic(11, false);
	if (_room == kHospitalBedroom)
		_audio.playEffect(30, 3, 1, true);
}

void RunawayRoomScene::finishHospitalAnimation(uint16 action) {
	switch (action) {
	case 332:
		startHospitalFilm(kLedgeFilm);
		return;
	case 302:
		_transition.scene = 2022;
		_transition.position = Common::Point(265, 456);
		_transition.facing = 3;
		return;
	case kHospitalStoreroom:
		if (!_hospital.storeroomVisited)
			say(0);
		_hospital.storeroomVisited = 1;
		_facing = 3;
		return;
	case 2022:
		_facing = 3;
		return;
	case 335:
		_hospital.chartMoved = 1;
		_inventory.remove(6);
		say(31, 1);
		break;
	case 339:
		_hospital.chartReplaced = 1;
		_inventory.remove(16);
		_inventory.add(6);
		say(35);
		break;
	default:
		return;
	}
	if (!rebuildBackground())
		error("Cannot update Runaway hospital chart");
	updateMaps();
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

void RunawayRoomScene::drawScene2020(Graphics::ManagedSurface &viewport, bool foreground) {
	if (!foreground && _hospital.chartMoved)
		_safe.draw(viewport, 0, _camera.x, 0);
	// Gina and the near bed occlude Brian only when he is behind them.
	if (foreground == (_position.y < 503)) {
		_ambient.draw(viewport, _ambientFrame, _camera.x, 0);
		drawForeground(viewport, 2);
		if (_roomAction != 339 || !_actionStep)
			_overlay.draw(viewport, 0, _camera.x, 0);
	}
	if (foreground) {
		if (_position.x < 600 && _position.y < 434)
			drawForeground(viewport, _hospital.decoy ? 3 : 1);
		if (_position.x > 1000)
			drawForeground(viewport, 0);
	}
}

} // End of namespace Hollywood
