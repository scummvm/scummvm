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
#include "common/util.h"
#include "hollywood/runaway/scene.h"
#include "hollywood/runaway/scene_data.h"

namespace Hollywood {
namespace {

const byte kStudFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 55, 55, 55, 55, 55, 55, 55, 55,
	55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 56, 57, 58, 59,
	60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75,
	76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
	92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
	92, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78,
	77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62,
	61, 60, 59, 58, 57, 56, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
	55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 54, 53, 52, 99, 100,
	101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
	117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132,
	133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148,
	149, 150, 150, 150, 150, 150, 150, 150, 150, 150
};

const RunawayHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Path", 1279, 267, 0, {301, 302}, {0, 1}},
	{"Barn Doors", 1259, 354, 7, {303, 304}, {1, 0}},
	{"Bottle", 0, 0, 0, {305, 306}, {1, 1}},
	{"Window", 1279, 267, 4, {1, 302}, {0, 1}},
	{"Motorcycle", 164, 581, 0, {307, 308}, {1, 0}},
	{"Stud", 0, 0, 7, {309, 310}, {1, 1}},
	{"?", 0, 0, 4, {311, 312}, {0, 0}},
	{"Piece Of Metal", 0, 0, 7, {313, 314}, {0, 1}},
	{"Barrel", 0, 0, 1, {315, 316}, {1, 0}},
	{"Barrel", 0, 0, 4, {317, 318}, {0, 0}},
	{"Helicopter", 0, 0, 7, {319, 320}, {0, 0}},
	{"?", 253, 418, 4, {321, 322}, {0, 0}},
	{"Bucket", 0, 0, 1, {323, 324}, {1, 0}},
	{"Outside", 253, 418, 4, {1, 325}, {0, 0}},
	{"Stud", 1053, 403, 3, {1, 329}, {0, 1}},
	{"Beer Can", 0, 0, 0, {0, 0}, {0, 0}},
	{"Peanut Butter", 0, 0, 0, {0, 0}, {0, 0}},
	{"Foot Rest", 0, 0, 0, {0, 0}, {0, 0}},
};

const RunawayResponse kResponses[] = {
	{5100, 301, 2, 0},
	{5100, 308, 16, 0},
	{5100, 311, 18, 0},
	{5100, 312, 19, 0},
	{5100, 316, 22, 0},
	{5100, 318, 24, 0},
	{5100, 319, 25, 0},
	{5100, 320, 26, 0},
	{5100, 322, 8, 0},
	{5100, 324, 28, 0},
};

const RunawayItemAction kItemActions[] = {
	{5100, 2, 5, 327, true},
	{5100, 9, 6, 16, false},
	{5100, 9, 12, 328, true},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5100 = {
	5100, 1880, 700, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {100, 95, 90}, {60, 55, 50}, {90, 85, 80}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(1279, 267),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

bool RunawayRoomScene::applyBackground5100() {
	const uint mask = _desert.insideBarn ? 23 : _desert.guard == 2 ? 26 : 1;
	return loadRunawayMask(_archive, _background.w, _background.h, _mask, _originalMaps, mask) &&
		(_desert.barnDoor != 2 || applyPatch(5)) && (_desert.peanutButter != 2 || applyPatch(6));
}

void RunawayRoomScene::updateMaps5100() {
	_desertHotspots.assign(kHotspots, kHotspots + ARRAYSIZE(kHotspots));
	if (_desert.insideBarn) {
		_desertHotspots[6].x = 253;
		_desertHotspots[6].y = 418;
		_desertHotspots[6].facing = 4;
	} else if (_desert.guard == 2) {
		static const Common::Point positions[] = {
			{0, 0}, {1279, 267}, {1259, 354}, {271, 598}, {1279, 267},
			{164, 581}, {656, 600}, {1326, 459}, {164, 581}, {261, 595},
			{992, 460}, {1259, 354}, {253, 418}, {736, 444}
		};
		for (uint i = 1; i < ARRAYSIZE(positions); ++i) {
			_desertHotspots[i].x = positions[i].x;
			_desertHotspots[i].y = positions[i].y;
		}
	}
	for (uint i = 0; i < 256; ++i) {
		if (_desert.guard != 2 && _maps[i] == 3)
			_maps[i] = 0;
		byte &id = _maps[256 + i];
		if ((id == 4 && _desert.guard == 2) || (id == 8 && _desert.footrest != 1))
			id = 0;
		if (id == 3 && _desert.peanutButter == 2)
			id = 0;
		if (id == 7 && _desert.peanutButter != 2)
			id = 0;
	}
}

void RunawayRoomScene::walkHangar(Common::Point target, byte facing) {
	// Scripted entrances cross the floor that is blocked while the guard watches.
	_walkPath.findPath(RunawayWalkMap(_background.w, _background.h, _mask, _originalMaps), target);
	_arrivalFacing = facing;
	_walkElapsed = _lastFootstep = 0;
	if (!_walkPath.active())
		arrive();
}

RunawayRoomScene::ActionResult RunawayRoomScene::execute5100(uint16 action) {
	switch (action) {
	case 302:
		leaveDesertRoom(5010);
		break;
	case 303:
		if (!_desert.barnDoor) {
			Common::Array<byte> frames;
			frames.push_back(1);
			for (uint i = 1; i <= 18; ++i)
				frames.push_back(i);
			for (uint i = 0; i < 12; ++i)
				frames.push_back(18);
			for (int i = 17; i >= 1; --i)
				frames.push_back(i);
			say(3);
			beginRoomAnimation(action, 11, 19, frames.data(), frames.size());
		} else {
			say(3, _desert.barnDoor == 1 ? 3 : 4);
		}
		break;
	case 1303:
		_desert.barnDoor = 1;
		say(3, 1);
		say(3, 2);
		break;
	case 304:
		say(4, _desert.barnDoor);
		if (_desert.barnDoor != 2)
			deferDesertAction(404);
		break;
	case 404:
		_desert.barnDoor = 1;
		_desert.insideBarn = 1;
		if (!applyBackground5100())
			error("Cannot load Runaway hangar interior");
		updateMaps();
		walkHangar(Common::Point(680, 355), 0);
		deferDesertAction(414);
		break;
	case 414:
		walkHangar(Common::Point(253, 418), 4);
		break;
	case 305:
		say(10, _desert.peanutButter != 0);
		if (!_desert.peanutButter)
			_desert.peanutButter = 1;
		break;
	case 306:
		if (!_desert.peanutButter)
			execute5100(305);
		say(11);
		deferDesertAction(406);
		break;
	case 406:
		_inventory.add(19);
		_desert.peanutButter = 2;
		say(11, 1);
		say(12, 0, false, 1);
		deferDesertAction(416);
		return kBackgroundChanged;
	case 416:
		walkTo(Common::Point(_desertHotspots[6].x, _desertHotspots[6].y), 7);
		deferDesertAction(426);
		break;
	case 426:
		say(11, 2);
		say(12, 1, false, 1);
		say(11, 3);
		say(12, 2, false, 1);
		say(11, 4);
		say(12, 3, false, 1);
		say(12, 4, false, 1);
		say(11, 5);
		_inventory.desert().antTrickKnown = 1;
		break;
	case 307: {
		const byte variation = _random.getRandomBit();
		say(15, variation);
		if (variation)
			_desert.motorcycleSeen = 1;
		break;
	}
	case 309:
		say(5, _desert.insideBarn ? _random.getRandomNumber(3) : 4 + _random.getRandomBit());
		break;
	case 310:
		say(6, !_desert.insideBarn);
		break;
	case 313:
		if (!_desert.footrestSeen) {
			say(20);
			_desert.footrestSeen = 1;
		} else {
			say(172, 0, true);
		}
		break;
	case 314:
		if (!_desert.footrestSeen)
			execute5100(313);
		_inventory.add(21);
		_desert.footrest = 2;
		say(18, 0, true);
		return kBackgroundChanged;
	case 315:
		say(21, _random.getRandomBit());
		break;
	case 317:
		say(23, _random.getRandomNumber(2));
		break;
	case 321:
		say(7, _desert.beerSeen != 0);
		_desert.beerSeen = 1;
		break;
	case 323:
		say(27, _random.getRandomBit());
		break;
	case 325:
		walkHangar(Common::Point(680, 355), 0);
		deferDesertAction(425);
		break;
	case 425:
		walkHangar(Common::Point(1348, 332), 5);
		deferDesertAction(435);
		break;
	case 435:
		_desert.insideBarn = 0;
		return kBackgroundChanged;
	case 327:
		if (_desert.footrest) {
			say(17, 2);
		} else {
			if (!_desert.motorcycleSeen) {
				say(15, 1);
				_desert.motorcycleSeen = 1;
			}
			say(17);
			byte brian[99] = {}, footrest[99] = {};
			for (uint i = 1; i < 78; ++i)
				brian[i] = i - 1;
			for (uint i = 78; i < 88; ++i)
				brian[i] = 87 - i;
			static const byte wobble[] = {1, 2, 3, 2, 1};
			for (uint i = 0; i < ARRAYSIZE(wobble); ++i)
				footrest[59 + i] = wobble[i];
			for (uint i = 73; i < ARRAYSIZE(footrest); ++i)
				footrest[i] = i - 72;
			beginRoomAnimation(action, 14, 77, brian, ARRAYSIZE(brian));
			setRoomAnimationOverlay(7, 27, footrest, ARRAYSIZE(footrest));
		}
		break;
	case 1327:
		_desert.footrest = 1;
		say(17, 1);
		return kBackgroundChanged;
	case 328:
		if (_desert.guard)
			return kUnhandled;
		say(9);
		_desert.guardWeaknessKnown = 1;
		break;
	case 329:
		say(1, _random.getRandomBit());
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void RunawayRoomScene::drawScene5100(Graphics::ManagedSurface &viewport, bool foreground) {
	if (foreground != (walkZone() == 7))
		return;
	const uint frame = kStudFrames[(_now / 80) % ARRAYSIZE(kStudFrames)];
	if (_desert.guard == 2)
		_npcMouth.draw(viewport, speaker() == 1 ? 118 + speechFrame() % 6 : 110, _camera.x, -_camera.y);
	else if (!_desert.guard)
		_npc.draw(viewport, speaker() == 1 ? 92 + speechFrame() % 6 : frame, _camera.x, -_camera.y);
	if (_desert.guard == 2 || (frame >= 82 && frame <= 92))
		_desertAnimations[0].draw(viewport, 0, _camera.x, -_camera.y);
	if (_desert.footrest < 2 && (!drawingRoomAnimation() || _roomAction != 327))
		_desertAnimations[1].draw(viewport, _desert.footrest ? 26 : 0, _camera.x, -_camera.y);
}

void RunawayRoomScene::updateScene5100() {
	const uint frame = kStudFrames[(_now / 80) % ARRAYSIZE(kStudFrames)];
	byte canHotspot = 12;
	if (_desert.guard != 2 && (frame < 82 || frame > 92))
		canHotspot = _desert.guard ? 0 : 14;
	for (uint i = 0; i < 256; ++i) {
		if (_originalMaps[256 + i] == 12)
			_maps[256 + i] = canHotspot;
	}
}

} // End of namespace Hollywood
