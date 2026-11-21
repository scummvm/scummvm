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
#include "hollywood/runaway/scene5070.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1472, 495, 0, {302, 301}, {0, 1}, {1472, 495}, 5}, // 1: Path.
	{0, 0, 7, {1, 1}, {0, 0}, {0, 0}}, // 2: Mariola.
	{1122, 509, 7, {305, 306}, {0, 0}, {1076, 456}}, // 3: Detonator.
	{301, 264, 7, {307, 308}, {0, 0}, {230, 253}}, // 4: Oil Well.
	{0, 0, 0, {309, 310}, {0, 0}, {620, 268}}, // 5: Tree.
	{0, 0, 7, {313, 314}, {0, 0}, {957, 459}}, // 6: Rock.
	{287, 312, 7, {315, 316}, {1, 0}, {270, 302}}, // 7: Puddle Of Oil.
};

const RunawayResponse kResponses[] = {
	{5070, 302, 1, 0}, // Look at the path.
	{5070, 307, 4, 0}, // Look at the oil well.
	{5070, 308, 5, 0}, // Use the oil well.
	{5070, 309, 6, 0}, // Look at the tree.
};

const RunawayItemAction kItemActions[] = {
	{5070, 5, 4, 311, true},
	{5070, 20, 7, 317, false},
	{5070, 25, 4, 312, false},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5070 = {
	5070, 1372, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {100, 95, 90}, {80, 75, 70}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(1245, 495),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

namespace Runaway {

Scene5070::Scene5070(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene5070::loadCustomResources() {
	return _ambientAnimations[0].load(_archive, 5, 896) &&
		_ambientAnimations[1].load(_archive, 6, 945) &&
		_ambientAnimations[2].load(_archive, 7, 953) &&
		_ambientAnimations[3].load(_archive, 8, 551) &&
		_ambientAnimations[4].load(_archive, 4, 48);
}

void Scene5070::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (id == 3 || id == 8) {
			id = _desert.dynamitePlaced ? 3 : 0;
		}
	}
}

float Scene5070::actorHeight() const {
	return _position.y * 1.171428561f - 226.085709f;
}

int8 Scene5070::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (id == 7 && !use && !item) {
		return !_desert.oilSeen;
	}
	return approach;
}

RunawayRoomScene::ActionResult Scene5070::dispatchCustomSceneAction(uint16 action) {
	RunawaySaveState::DesertItems &items = _inventory.desert();
	switch (action) {
	case 301: // Go to the desert overview.
		leaveDesertRoom(5010);
		break;
	case 305: // Look at the detonator.
		say(2, _random.getRandomBit());
		break;
	case 306: { // Use the detonator.
		const byte variation = _random.getRandomBit();
		say(3, variation);
		if (variation) {
			say(3, 2);
		}
		break;
	}
	case 310: // Use the tree.
		say(5, 0, true);
		break;
	case 311: // Place the dynamite at the oil well.
		say(7);
		beginRoomAnimation(action, 11, 40);
		break;
	case 1311: // Finish setting the explosives.
		_desert.dynamitePlaced = 1;
		items.bombReady = 1;
		_inventory.remove(5);
		say(7, 1);
		if (items.gunReady && items.helicopterReady) {
			say(174, 4, true);
			leaveDesertRoom(5062);
		}
		return kBackgroundChanged;
	case 312: // Use the bucket with the oil well.
		if (items.bucket != 1) {
			say(17, 0, true);
		} else {
			say(8, _desert.dynamitePlaced != 0);
		}
		break;
	case 313: // Look at the rock.
		say(9, _desert.dynamitePlaced != 0);
		break;
	case 314: // Use the rock.
		say(10);
		break;
	case 315: // Look at the puddle of oil.
		say(11, _desert.oilSeen != 0);
		_desert.oilSeen = 1;
		break;
	case 316: // Take oil from the puddle.
		if (!_desert.oilSeen) {
			dispatchCustomSceneAction(315);
		}
		say(12);
		break;
	case 317: // Dip the sunglasses in the oil.
		if (items.glasses) {
			say(13, items.glasses + 1);
		} else {
			say(13);
			beginRoomAnimation(action, 9, 29);
		}
		break;
	case 1317: // Finish darkening the sunglasses.
		items.glasses = 1;
		_inventory.select(0);
		say(13, 1);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene5070::runCustomEntrySequence() {
	visitDesertRoom(4, 1);
}

void Scene5070::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	drawAmbientAnimations(viewport, _ambientAnimations, ARRAYSIZE(_ambientAnimations));
}

} // End of namespace Runaway
} // End of namespace Hollywood
