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
#include "hollywood/runaway/scene5050.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{-100, 510, 0, {302, 301}, {0, 1}, {0, 495}, 7}, // 1: Path.
	{303, 534, 1, {303, 304}, {0, 1}, {341, 510}}, // 2: Helmet.
	{251, 420, 1, {305, 306}, {0, 1}, {276, 406}}, // 3: Hanging Bullet Belt.
	{251, 420, 1, {307, 308}, {0, 1}, {336, 337}}, // 4: Machine Gun.
	{200, 450, 1, {309, 310}, {0, 0}, {559, 486}}, // 5: Bullet Hole.
	{200, 450, 7, {311, 312}, {0, 0}, {497, 428}}, // 6: Pilot'S Seat.
	{200, 450, 1, {313, 314}, {0, 0}, {283, 291}}, // 7: Gunner'S Seat.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 8: String Of Bullets.
};

const RunawayResponse kResponses[] = {
	{5050, 302, 1, 0}, // Look at the path.
	{5050, 303, 2, 0}, // Look at the helmet.
	{5050, 309, 8, 0}, // Look at the bullet hole.
	{5050, 310, 9, 0}, // Use the bullet hole.
	{5050, 311, 10, 0}, // Look at the pilot's seat.
	{5050, 312, 11, 0}, // Use the pilot's seat.
	{5050, 313, 12, 0}, // Look at the gunner's seat.
	{5050, 314, 13, 0}, // Use the gunner's seat.
};

const RunawayItemAction kItemActions[] = {
	{5050, 3, 4, 319, false},
	{5050, 4, 4, 319, false},
	{5050, 10, 4, 316, false},
	{5050, 11, 4, 317, false},
	{5050, 12, 4, 315, true},
	{5050, 14, 4, 318, true},
	{5050, 19, 4, 319, false},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5050 = {
	5050, 1024, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {90, 90, 80}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(130, 461),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

namespace Runaway {

Scene5050::Scene5050(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

const char *Scene5050::actionCaptionVariant(byte id, bool look, byte item) const {
	if (id == 4 && _desert.gunSeen && (item == 3 || item == 4 || item == 12 || item == 19)) {
		return "grease";
	}
	return nullptr;
}

bool Scene5050::loadCustomResources() {
	return loadForeground(0, 4) &&
		loadForeground(1, 5);
}

bool Scene5050::applyCustomBackgroundPatches() {
	return (!_desert.helmetTaken || applyPatch(6)) && (!_desert.beltTaken || applyPatch(7)) &&
		(!_desert.gun || applyPatch(8));
}

void Scene5050::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if ((_desert.helmetTaken && id == 2) || (_desert.beltTaken && id == 3)) {
			id = 0;
		}
	}
}

float Scene5050::actorHeight() const {
	return _position.y * 0.128865972f + 197.809280f;
}

int8 Scene5050::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (id == 3 && !use && !item) {
		return !_desert.beltSeen;
	}
	return approach;
}

RunawayRoomScene::ActionResult Scene5050::dispatchCustomSceneAction(uint16 action) {
	RunawaySaveState::DesertItems &items = _inventory.desert();
	switch (action) {
	case 301: // Go to the desert overview.
		leaveDesertRoom(5010);
		break;
	case 304: // Take the helmet.
		_desert.helmetTaken = 1;
		_inventory.add(6);
		say(3);
		return kBackgroundChanged;
	case 305: // Look at the ammunition belt.
		say(4, _desert.beltSeen != 0);
		_desert.beltSeen = 1;
		break;
	case 306: // Take the ammunition belt.
		_desert.beltTaken = 1;
		_inventory.add(14);
		say(5);
		return kBackgroundChanged;
	case 307: // Look at the machine gun.
		say(6, _desert.gun);
		_desert.gunSeen = 1;
		break;
	case 308: // Use the machine gun.
		say(7, _desert.gun);
		_desert.gunSeen = 1;
		break;
	case 315: // Use tanning oil with the machine gun.
		if (_desert.gun) {
			say(14, 2);
		} else if (!_desert.gunSeen) {
			say(1, 0, true);
		} else {
			say(14);
			beginRoomAnimation(action, 11, 46);
		}
		break;
	case 1315: // Finish lubricating the machine gun.
		_desert.gun = 1;
		say(14, 1);
		say(14, 3);
		return kBackgroundChanged;
	case 316: // Use the lipstick with the machine gun.
		say(15, items.lipstick != 0);
		break;
	case 317: // Use the pile of lipstick with the machine gun.
		say(16, items.lipsticks != 0);
		break;
	case 318: // Load the ammunition belt into the machine gun.
		if (items.belt < 2) {
			say(17, items.belt);
		} else if (!_desert.gun) {
			say(17, 2);
		} else {
			say(17, 3);
			beginRoomAnimation(action, 12, 37);
		}
		break;
	case 1318: // Finish loading the machine gun.
		_desert.gun = 2;
		items.gunReady = 1;
		_inventory.remove(14);
		say(17, 4);
		if (items.bombReady && items.helicopterReady) {
			say(174, 4, true);
			leaveDesertRoom(5062);
		}
		return kBackgroundChanged;
	case 319: // Use unsuitable lubricant with the machine gun.
		say(18, _desert.gun != 0);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene5050::runCustomEntrySequence() {
	visitDesertRoom(2, 2);
}

Common::U32String Scene5050::hotspotName(byte id) const {
	if (id == 3 && _desert.beltSeen) {
		return RunawayRoomScene::hotspotName(8);
	}
	return RunawayRoomScene::hotspotName(id);
}

void Scene5050::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_position.x < 314) {
		drawForeground(viewport, 0);
	}
	if (_position.x > 195 && _position.y < 465) {
		drawForeground(viewport, 1);
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
