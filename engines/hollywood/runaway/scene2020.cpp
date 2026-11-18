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

#include "common/textconsole.h"
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
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Hallway door", 40, 599, 5, {351, 352}, {1, 0}},
};

const RunawayResponse kResponses[] = {
	{kHospitalBedroom, 301, 1, 0},
	{kHospitalBedroom, 304, 4, 0},
	{kHospitalBedroom, 306, 5, 0},
	{kHospitalBedroom, 308, 6, 0},
	{kHospitalBedroom, 310, 7, 0},
	{kHospitalBedroom, 311, 8, 0},
	{kHospitalBedroom, 312, 9, 0},
	{kHospitalBedroom, 314, 10, 0},
	{kHospitalBedroom, 315, 11, 0},
	{kHospitalBedroom, 316, 12, 0},
	{kHospitalBedroom, 317, 13, 0},
	{kHospitalBedroom, 318, 14, 0},
	{kHospitalBedroom, 319, 15, 0},
	{kHospitalBedroom, 320, 16, 0},
	{kHospitalBedroom, 321, 17, 0},
	{kHospitalBedroom, 322, 18, 0},
	{kHospitalBedroom, 323, 19, 0},
	{kHospitalBedroom, 324, 20, 0},
	{kHospitalBedroom, 325, 21, 0},
	{kHospitalBedroom, 326, 22, 0},
	{kHospitalBedroom, 327, 23, 0},
	{kHospitalBedroom, 328, 24, 0},
	{kHospitalBedroom, 329, 25, 0},
	{kHospitalBedroom, 331, 27, 0},
	{kHospitalBedroom, 333, 29, 0},
	{kHospitalBedroom, 343, 37, 1},
	{kHospitalBedroom, 344, 39, 0},
	{kHospitalBedroom, 345, 40, 0},
};

const RunawayItemAction kItemActions[] = {
	{kHospitalBedroom, 5, 6, 333},   // Bag in the locker.
	{kHospitalBedroom, 9, 12, 343},  // Matches at the sprinkler.
	{kHospitalBedroom, 13, 2, 349},  // Water for Gina.
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
		_ambient.load(_archive, 22, 19) && _overlay.load(_archive, 17, 1) &&
		_archive.readVariableChunk(20, _monitorPatches[0]) && _archive.readVariableChunk(21, _monitorPatches[1]);
}

void RunawayRoomScene::start2020() {
	for (byte item = 1; item <= 4; ++item)
		_inventory.add(item);
	_inventory.takeNewItem();
	_facing = 7;
	_hospitalEntryStep = 1;
	say(0);
}

bool RunawayRoomScene::applyBackground2020() {
	if ((_hospital.glassTaken && !applyPatch(8)) || (_hospital.pillsTaken && !applyPatch(9)) ||
			(_hospital.sheetTaken && !applyPatch(7)) || (_hospital.bagTaken && !applyPatch(6)))
		return false;
	return applyPatch(11) && applyRunawayPatch(_monitorPatches[1], _background) &&
		applyRunawayPatch(_monitorPatches[0], _background);
}

void RunawayRoomScene::updateMaps2020() {
	for (uint i = 0; i < 256; ++i) {
		// Zones 5/6 are reserved for scripted movement, not normal walking.
		if (_maps[i] > 4)
			_maps[i] = 0;
		byte &id = _maps[256 + i];
		if ((_hospital.glassTaken && id == 3) || (_hospital.pillsTaken && id == 4) ||
				(_hospital.bagTaken && id == 7) || id == 11)
			id = 0;
		else if (id == 5 || id == 21)
			id = _hospital.sheetTaken ? 19 : 5;
		else if (id == 20)
			id = 19;
	}
}

bool RunawayRoomScene::sayInventory2020(const RunawayInventory::Reply &reply) {
	if (reply.cue != 54)
		return false;
	// Searching Gina's bag reveals the matches on the first examination.
	say(54, 1, true);
	if (_inventory.contains(9)) {
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
		say(2, _random.getRandomNumber(2));
		return kHandled;
	case 303:
		say(3, _random.getRandomBit());
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
		say(26, 1);
		_hospital.exitMapRead = 1;
		return kHandled;
	case 332:
		if (!_hospital.exitMapRead) {
			say(28);
			return kHandled;
		}
		// The ledge (2060) and bathroom (2080) have separate scene scripts.
		warning("Runaway: hospital ledge scene 2060 is not implemented yet");
		return kHandled;
	case 346:
		warning("Runaway: hospital bathroom scene 2080 is not implemented yet");
		return kHandled;
	case 349:
		say(4, 0, true);
		return kHandled;
	case 351:
		warning("Runaway: hospital hallway sequence is not implemented yet");
		return kHandled;
	case 352:
		say(42, _random.getRandomBit());
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
			break;
		default:
			_hospitalEntryStep = 0;
			break;
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
	// Gina and the near bed occlude Brian only when he is behind them.
	if (foreground == (_position.y < 503)) {
		_ambient.draw(viewport, _ambientFrame, _camera.x, 0);
		drawForeground(viewport, 2);
		_overlay.draw(viewport, 0, _camera.x, 0);
	}
	if (foreground) {
		if (_position.x < 600 && _position.y < 434)
			drawForeground(viewport, 1);
		if (_position.x > 1000)
			drawForeground(viewport, 0);
	}
}

} // End of namespace Hollywood
