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
#include "hollywood/runaway/scene8190.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1250, 347, 3, {301, 302}, {0, 3}, {1459, 435}, 6}, // 1: Town.
	{560, 282, 0, {303, 305}, {1, 1}, {550, 255}}, // 2: Entrance to the basement.
	{757, 446, 5, {306, 307}, {1, 0}, {656, 528}}, // 3: Hole.
	{778, 314, 7, {308, 309}, {1, 1}, {747, 290}}, // 4: Shelves.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{479, 232, 5, {312, 313}, {1, 1}, {450, 251}}, // 6: Desk.
	{759, 334, 4, {314, 315}, {0, 1}, {763, 334}}, // 7: Cash register.
	{601, 391, 5, {316, 317}, {0, 0}, {508, 480}} // 8: Couch.
};

const RunawayResponse kResponses[] = {
	{8190, 301, 1, 0},  // Look toward town.
	{8190, 303, 2, 1},  // Examine the entrance made by Saturn's statue.
	{8190, 305, 4, 1},  // Refuse to return to the basement.
	{8190, 306, 5, 0},  // Examine the rubble-filled hole.
	{8190, 307, 6, 1},  // Refuse to climb through the rubble.
	{8190, 308, 7, 2},  // Examine the searched shelves.
	{8190, 309, 8, 1},  // Leave the shelves alone.
	{8190, 312, 11, 0}, // Examine Douglas's desk.
	{8190, 313, 12, 2}, // Find nothing else in the desk.
	{8190, 314, 13, 0}, // Examine the cash register.
	{8190, 315, 14, 0}, // Open the empty cash register.
	{8190, 316, 15, 0}, // Examine the couch.
	{8190, 317, 16, 0}  // Try to sit on the couch.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8190 = {
	8190, 1460, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {95, 90, 70}, {65, 60, 40}, {65, 60, 40}, {75, 70, 50}, {90, 85, 65}, {95, 90, 70}},
	Common::Point(1250, 347),
	kResponses, ARRAYSIZE(kResponses), nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene8190::Scene8190(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene8190::usesAlternateActor() const {
	return false;
}

bool Scene8190::applyCustomBackgroundPatches() {
	return applyPatch(5) && applyPatch(8) && applyPatch(9);
}

void Scene8190::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[256 + i] == 5) {
			_maps[256 + i] = 4;
		}
	}
}

void Scene8190::runCustomEntrySequence() {
	if (_entryScene == 8191) {
		_position = Common::Point(561, 288);
		_walkPath.reset(_position);
		_facing = 4;
		updateCamera();
		beginRoomAnimation(400, 7, 24);
	} else {
		_facing = 7;
		walkTo(Common::Point(669, 413), 7);
	}
}

void Scene8190::resumeAmbientAudio(bool entering) {
	resumeFinaleOutdoorAudio(entering, 36, 51);
}

void Scene8190::advanceCustomGameplayLoop(uint32 delta) {
	updateFinaleOutdoorAudio();
}

float Scene8190::actorHeight() const {
	return (_position.y * 0.426829278f + 16.4634151f + _position.x * 0.0980392173f + 41.7647057f) / 2;
}

void Scene8190::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
	params.depthY[2] = 388;
}

void Scene8190::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

RunawayRoomScene::ActionResult Scene8190::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Return to the street.
		leaveRoom(8224);
		break;
	case 1400: // Finish climbing back into the bank.
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
