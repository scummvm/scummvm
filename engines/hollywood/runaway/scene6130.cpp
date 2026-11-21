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
#include "hollywood/runaway/scene6130.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{0, 599, 5, {301, 302}, {0, 3}, {-100, 599}, 7}, // 1: Path.
	{402, 374, 1, {303, 304}, {0, 3}, {637, 237}, 5}, // 2: House.
	{491, 495, 3, {305, 306}, {0, 0}, {715, 540}}, // 3: Well.
	{486, 519, 3, {307, 308}, {1, 1}, {527, 550}}, // 4: Stone.
	{0, 0, 0, {309, 310}, {0, 0}, {323, 544}} // 5: Skull.
};

const RunawayResponse kResponses[] = {
	{6130, 303, 2, 0},  // Look at Oscar in front of the house.
	{6130, 305, 3, 0},  // Examine the abandoned well.
	{6130, 309, 7, 0},  // Examine the skull.
	{6130, 310, 8, 0}   // Leave Mama Dorita's decoration alone.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6130 = {
	6130, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {100, 95, 90}, {80, 75, 70}}, Common::Point(268, 450),
	kResponses, ARRAYSIZE(kResponses), nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene6130::Scene6130(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game), _oscar(false) {}

bool Scene6130::loadCustomResources() {
	return _oscarAnimation.load(_archive, 6, 106);
}

bool Scene6130::applyCustomBackgroundPatches() {
	return (_town.stone != 2 || applyPatch(4)) && (_town.vessel != 2 || applyPatch(5));
}

void Scene6130::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[256 + i] == 4 && _town.stone == 2) {
			_maps[256 + i] = 0;
		}
	}
}

void Scene6130::prepareCustomGameplayLoop() {
	_oscar.resume(_now);
}

void Scene6130::runCustomEntrySequence() {
	if (_entryScene == 6131) {
		_position = Common::Point(401, 375);
		_walkPath.reset(_position);
		_facing = 5;
	} else {
		_facing = 1;
		if (!_town.houseVisited) {
			say(0);
			deferTownAction(400);
		}
	}
}

void Scene6130::advanceCustomGameplayLoop(uint32 delta) {
	_oscar.update(_now, _random, _audio);
}

void Scene6130::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	_oscarAnimation.draw(viewport, _oscar.frame(speaker(), speechFrame()), _camera.x, -_camera.y);
}

float Scene6130::actorHeight() const {
	return _position.y * 1.027668f - 160.57312f;
}

void Scene6130::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 480;
	params.depthY[2] = 599;
}

int8 Scene6130::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (!item && id == 4 && !use && _town.stone) {
		return false;
	}
	return approach;
}

RunawayRoomScene::ActionResult Scene6130::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 301: // Look toward Douglasville.
		say(1, _town.streetVisited);
		break;
	case 302: // Return to the overview.
		leaveTownRoom(6350);
		break;
	case 304: // Approach Mama Dorita's doorway.
		leaveTownRoom(6090);
		break;
	case 306: // Refuse to jump into the well.
		say(4, _random.getRandomBit());
		break;
	case 307: // Notice the hole through the stone.
		say(5, _town.stone != 0);
		if (!_town.stone) {
			_town.stone = 1;
		}
		break;
	case 308: // Take the unusually shaped stone.
		if (!_town.stone) {
			dispatchCustomSceneAction(307);
		}
		say(6);
		deferTownAction(408);
		break;
	case 408: // Collect the stone with the hole.
		_town.stone = 2;
		_inventory.add(29);
		say(6, 1);
		return kBackgroundChanged;
	case 400: // First visit: go and introduce Brian to Oscar.
		walkTo(Common::Point(402, 374), 1);
		deferTownAction(401);
		break;
	case 401: // Arrive at Mama Dorita's doorway for the first meeting.
		_town.houseVisited = 1;
		leaveTownRoom(6090);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
