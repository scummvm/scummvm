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
#include "hollywood/runaway/scene_data.h"

namespace Hollywood {
namespace {

const byte kMariolaTurnFrames[] = {
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
	45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
	61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76,
	77, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
	116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
	132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147,
	148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 77, 78, 79,
	80, 81, 82, 83
};

const byte kMariolaRestFrames[] = {
	95, 96, 97, 98, 99, 100, 101, 161, 162, 163, 164, 165, 166, 167, 168, 169,
	170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185,
	186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200
};

const RunawayHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Path", 1479, 599, 0, {301, 302}, {0, 1}},
	{"Lula", 349, 442, 7, {303, 304}, {0, 1}},
	{"Mariola", 878, 477, 0, {305, 306}, {0, 0}},
	{"Inside Of The Bus", 679, 448, 1, {307, 308}, {0, 1}},
	{"Trunk", 376, 445, 1, {309, 310}, {1, 1}},
	{"Bottle", 878, 477, 0, {311, 312}, {1, 1}},
	{"Chair", 383, 446, 5, {313, 314}, {0, 0}},
	{"Radio And Cassette Player", 349, 442, 5, {315, 316}, {0, 0}},
	{"Sunglasses", 878, 477, 0, {317, 318}, {0, 0}},
	{"Tanning Oil", 0, 0, 0, {0, 0}, {0, 0}},
	{"Lawn Chair", 0, 0, 0, {0, 0}, {0, 0}},
};

const RunawayResponse kResponses[] = {
	{5060, 301, 1, 0},
	{5060, 309, 5, 0},
	{5060, 314, 11, 0},
	{5060, 315, 12, 0},
	{5060, 316, 13, 0},
};

const RunawayItemAction kItemActions[] = {
	{5060, 15, 2, 319, false},
	{5060, 20, 9, 320, true},
};

const byte kSearchTrunkFrames[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
	14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
	14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
	14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5060 = {
	5060, 1380, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {100, 95, 90}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(954, 506),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

RunawayRoomScene::ActionResult RunawayRoomScene::execute5060(uint16 action) {
	switch (action) {
	case 302:
		leaveDesertRoom(5010);
		break;
	case 303:
		say(2, _random.getRandomBit() * (_desert.lulaHelped ? 2 : 1));
		break;
	case 304:
		if (_desert.guard == 2) {
			say(20, 12);
		} else if (_desert.lulaHelped) {
			return kUnhandled;
		} else {
			say(20);
			deferDesertAction(404);
		}
		break;
	case 404: {
		Common::Array<byte> frames;
		for (uint i = 120 + (_now / 80) % 75; i < 195; ++i)
			frames.push_back(i);
		for (uint i = 86; i < 120; ++i) {
			frames.push_back(i);
			if (i == 94)
				frames.push_back(i);
		}
		beginRoomAnimation(action, 12, 195, frames.data(), frames.size(), false);
		break;
	}
	case 1404:
		if (!_desert.guardWeaknessKnown) {
			if (!_desert.visited[6]) {
				say(20, 1);
				say(21, 0, false, 2);
				say(20, 2);
				say(21, 1, false, 2);
			} else {
				say(20, 3);
				say(21, 2, false, 2);
				say(20, 4);
				say(21, 3, false, 2);
				say(20, 5);
				_desert.lulaCalled = 1;
			}
		} else {
			say(20, 6);
			say(21, 4, false, 2);
			say(20, 7);
			if (_desert.lulaCalled || _desert.lulaRefused) {
				say(21, 5, false, 2);
			} else {
				say(21, 8, false, 2);
				say(20, 13);
				say(21, 9, false, 2);
			}
			say(20, 8);
			_desert.lulaRefused = 1;
		}
		deferDesertAction(414);
		break;
	case 414: {
		Common::Array<byte> frames;
		for (uint i = 95; i < 120; ++i)
			frames.push_back(i);
		for (uint i = 12; i < 85; ++i)
			frames.push_back(i);
		beginRoomAnimation(action, 12, 195, frames.data(), frames.size(), false);
		break;
	}
	case 1414:
		break;
	case 305:
		say(3, _desert.mariolaHelped != 0);
		break;
	case 306:
		_facing = 4;
		say(19, _random.getRandomNumber(2));
		break;
	case 307:
		say(4, _desert.visited[9] != 0);
		break;
	case 308:
		beginRoomAnimation(action, 5, 22);
		break;
	case 1308:
		leaveDesertRoom(5170);
		break;
	case 310:
		say(6, _desert.ballTaken ? 2 : 0);
		beginRoomAnimation(action, 10, 28, kSearchTrunkFrames, _desert.ballTaken ? ARRAYSIZE(kSearchTrunkFrames) : 57);
		break;
	case 1310:
		if (_desert.ballTaken) {
			say(6, 3);
		} else {
			_inventory.add(15);
			_desert.ballTaken = 1;
			say(6, 1);
			holdRoomAnimation(410, 10, 28, 14);
		}
		break;
	case 1410:
		beginRoomAnimation(420, 10, 28, kSearchTrunkFrames + 57, ARRAYSIZE(kSearchTrunkFrames) - 57);
		break;
	case 1420:
		break;
	case 311:
		if (!_desert.tanningOil) {
			say(7);
			_desert.tanningOil = 1;
		} else {
			say(163, 0, true);
		}
		break;
	case 312:
		if (!_desert.tanningOil)
			execute5060(311);
		say(8);
		deferDesertAction(412);
		break;
	case 412:
		beginRoomAnimation(action, 7, 201, kMariolaTurnFrames, ARRAYSIZE(kMariolaTurnFrames), false);
		break;
	case 1412:
		say(9, _desert.mariolaHelped != 0, false, 1);
		if (_desert.mariolaHelped)
			say(8, 1);
		deferDesertAction(422);
		break;
	case 422:
		beginRoomAnimation(432, 7, 201, kMariolaRestFrames, ARRAYSIZE(kMariolaRestFrames), false);
		if (_desert.mariolaHelped) {
			_desert.tanningOil = 2;
			_inventory.add(12);
			say(8, 2);
		}
		return kBackgroundChanged;
	case 1432:
		break;
	case 313:
		say(10, _desert.busPosterSeen != 0);
		_desert.busPosterSeen = 1;
		break;
	case 317:
		say(14, _desert.mariolaHelped != 0);
		break;
	case 318:
		say(15, !_inventory.contains(20) ? 0 : _desert.mariolaHelped ? 2 : 1);
		break;
	case 319:
		if (!_desert.guardWeaknessKnown)
			say(17, 7);
		else if (_inventory.desert().ball < 2)
			say(17, 8);
		else
			return kUnhandled;
		break;
	case 320:
		if (!_inventory.desert().glasses) {
			say(16);
		} else if (_inventory.desert().glasses == 1) {
			say(16, 1);
			beginRoomAnimation(action, 11, 21);
		} else {
			say(15, 2);
		}
		break;
	case 1320:
		_desert.mariolaHelped = 1;
		_inventory.desert().glasses = 2;
		say(16, 2);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void RunawayRoomScene::drawScene5060(Graphics::ManagedSurface &viewport) {
	if (!drawingRoomAnimation() || (_roomAction != 412 && _roomAction != 432)) {
		uint frame = (_now / 80) % 13;
		if (_roomAction == 422 || _roomAction == 432)
			frame = 83 + (speaker() == 1 ? speechFrame() : 0);
		_npc.draw(viewport, frame, _camera.x, -_camera.y);
	}
	static const byte mouth[] = {5, 0, 1, 2, 3, 4, 11, 6, 7, 8, 9, 10};
	if (!drawingRoomAnimation() || (_roomAction != 404 && _roomAction != 414)) {
		uint frame = 120 + (_now / 80) % 75;
		if (_roomAction == 414)
			frame = mouth[speaker() == 2 ? speechFrame() : 0];
		_machine.draw(viewport, frame, _camera.x, -_camera.y);
	}
}

} // End of namespace Hollywood
