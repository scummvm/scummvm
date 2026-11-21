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
#include "hollywood/runaway/scene6280.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1023, 500, 3, {301, 302}, {0, 3}, {1023, 0}, 5}, // 1: Outside the crater.
	{662, 546, 7, {303, 304}, {0, 1}, {597, 512}}, // 2: Joshua's helmet.
	{662, 546, 7, {305, 306}, {0, 0}, {597, 512}} // 3: Black mark.
};

const RunawayResponse kResponses[] = {
	{6280, 301, 1, 0}, // Look toward Joshua's camp.
	{6280, 303, 2, 0}, // Examine the abandoned helmet.
	{6280, 305, 4, 0}, // Examine the scorch mark.
	{6280, 306, 5, 0}, // Refuse to touch the scorch mark.
	{6280, 307, 6, 0}  // Keep the helmet instead of leaving it in the crater.
};

const RunawayItemAction kItemActions[] = {
	{6280, 13, 3, 307, false}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6280 = {
	6280, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {90, 80, 80}}, Common::Point(1023, 500),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene6280::Scene6280(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene6280::applyCustomBackgroundPatches() {
	return !_town.joshuaGone || applyPatch(_town.helmetTaken ? 5 : 4);
}

void Scene6280::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (id >= 2 && id <= 4) {
			if (!_town.joshuaGone) {
				id = 0;
			} else if (id == 4) {
				id = _town.helmetTaken ? 3 : 2;
			} else if (id == 2 && _town.helmetTaken) {
				id = 0;
			}
		}
	}
}

void Scene6280::resumeAmbientAudio(bool entering) {
	_audio.playMusic(30);
	_audio.playMusicEffect(38, 128, 1, true);
}

void Scene6280::runCustomEntrySequence() {
	walkTo(Common::Point(850, 504), 5, true);
	if (!_town.craterFloorVisited) {
		deferTownAction(400);
	}
}

float Scene6280::actorHeight() const {
	return _position.y * 0.477386922f + 169.045227f;
}

void Scene6280::findWalkPath(Common::Point target) {
	walkTown(target);
}

bool Scene6280::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = MAX<int>(662, target.x);
	return false;
}

RunawayRoomScene::ActionResult Scene6280::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Climb back to Joshua's camp.
		leaveTownRoom(6290);
		break;
	case 304: // Pick up Joshua's helmet.
		say(3);
		deferTownAction(404);
		break;
	case 404: // Collect the helmet after Brian's reaction.
		_audio.playMusicEffect(39);
		_facing = 4;
		_town.helmetTaken = 1;
		_inventory.add(13);
		say(3, 1);
		return kBackgroundChanged;
	case 400: // First descent to the crater floor.
		say(0);
		deferTownAction(401);
		break;
	case 401: // Finish examining the crater.
		_facing = 4;
		say(0, 1);
		_town.craterFloorVisited = 1;
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
