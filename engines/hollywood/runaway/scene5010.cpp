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
#include "hollywood/runaway/scene5010.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{0, 0, 0, {1, 301}, {0, 0}, {0, 0}, 4}, // 1: Unvisited Area.
	{0, 0, 0, {1, 302}, {0, 0}, {0, 0}, 4}, // 2: Thugs' Cabin.
	{0, 0, 0, {1, 303}, {0, 0}, {0, 0}, 5}, // 3: Unvisited Area.
	{0, 0, 0, {1, 304}, {0, 0}, {0, 0}, 7}, // 4: Unvisited Area.
	{0, 0, 0, {1, 305}, {0, 0}, {0, 0}, 5}, // 5: Unvisited Area.
	{0, 0, 0, {1, 306}, {0, 0}, {0, 0}, 6}, // 6: Drag Queen Bus.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 7: Oil Well.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 8: Hangar.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 9: Airplane Graveyard.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 10: Railway Yard.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5010 = {
	5010, 1024, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {90, 85, 80}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(512, 500),
	nullptr, 0,
	nullptr, 0,
	nullptr, 0
};

namespace Runaway {

Scene5010::Scene5010(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene5010::loadCustomResources() {
	return _ambientAnimations[0].load(_archive, 9, 48) &&
		_ambientAnimations[1].load(_archive, 11, 186) &&
		_ambientAnimations[2].load(_archive, 14, 307) &&
		_ambientAnimations[3].load(_archive, 10, 700) &&
		_ambientAnimations[4].load(_archive, 12, 31) &&
		_ambientAnimations[5].load(_archive, 15, 1052);
}

bool Scene5010::applyCustomBackgroundPatches() {
	return (!_desert.visited[4] || applyPatch(3)) && (!_desert.visited[6] || applyPatch(4)) &&
		(!_desert.visited[2] || applyPatch(5)) && (!_desert.visited[5] || applyPatch(6)) && applyPatch(7);
}

void Scene5010::runCustomEntrySequence() {
	visitDesertRoom(0, 0);
}

Common::U32String Scene5010::hotspotName(byte id) const {
	if (id == 1 && _desert.visited[4]) {
		return RunawayRoomScene::hotspotName(7);
	}
	if (id == 3 && _desert.visited[6]) {
		return RunawayRoomScene::hotspotName(8);
	}
	if (id == 4 && _desert.visited[2]) {
		return RunawayRoomScene::hotspotName(9);
	}
	if (id == 5 && _desert.visited[5]) {
		return RunawayRoomScene::hotspotName(10);
	}
	return RunawayRoomScene::hotspotName(id);
}

RunawayRoomScene::ActionResult Scene5010::dispatchCustomSceneAction(uint16 action) {
	const uint16 destinations[] = {
		5070, // 301: Go to the oil well.
		5020, // 302: Go to the thugs' cabin.
		5100, // 303: Go to the hangar.
		5050, // 304: Go to the airplane graveyard.
		5080, // 305: Go to the railway yard.
		5060  // 306: Go to the bus.
	};
	if (action >= 301 && action <= 306) {
		leaveDesertRoom(destinations[action - 301]);
		return kHandled;
	}
	return kUnhandled;
}

bool Scene5010::shouldUpdateActor() const {
	return false;
}

bool Scene5010::shouldPlayGameplayClickPath() const {
	return false;
}

bool Scene5010::drawCustomComposite(Graphics::ManagedSurface &viewport) {
	drawAmbientAnimations(viewport, _ambientAnimations, ARRAYSIZE(_ambientAnimations));
	return true;
}

} // End of namespace Runaway
} // End of namespace Hollywood
