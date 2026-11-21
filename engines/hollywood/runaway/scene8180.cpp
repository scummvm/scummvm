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
#include "hollywood/runaway/scene8180.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{202, 526, 7, {301, 302}, {0, 3}, {189, 517}, 4}, // 1: Street.
	{780, 527, 1, {303, 304}, {0, -1}, {1023, 519}, 5}, // 2: Inside the saloon.
	{405, 427, 7, {305, 306}, {0, 0}, {375, 402}}, // 3: Trough.
	{780, 527, 1, {307, 308}, {0, -1}, {1023, 519}}, // 4: Floor board.
	{643, 457, 1, {309, 310}, {0, 0}, {729, 434}} // 5: Window.
};

const RunawayResponse kResponses[] = {
	{8180, 301, 1, 0},  // Look out into the deserted street.
	{8180, 303, 2, 0},  // Look inside the saloon.
	{8180, 305, 4, 2},  // Examine the mud splashed around the trough.
	{8180, 306, 5, 2},  // Leave the trough alone.
	{8180, 307, 6, 0},  // Examine the doors' pressure plate.
	{8180, 309, 8, 0},  // Read the broken window.
	{8180, 310, 9, 0},  // Refuse to break more glass.
	{8180, 311, 10, 0} // Refuse to collect mud with an inventory item.
};

const RunawayItemAction kItemActions[] = {
	{8180, 8, 3, 311, false}, {8180, 12, 3, 311, false},
	{8180, 13, 3, 311, false}, {8180, 16, 3, 311, false}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8180 = {
	8180, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {80, 75, 65}, {60, 55, 45}, {50, 45, 35}}, Common::Point(319, 549),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene8180::Scene8180(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game), _oscar(false, 6) {}

bool Scene8180::loadCustomResources() {
	return _doors.load(_archive) && loadForeground(0, 4) && loadForeground(1, 5) &&
		_oscarAnimation.load(_archive, 12, 107);
}

bool Scene8180::applyCustomBackgroundPatches() {
	return applyPatch(11);
}

void Scene8180::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[i] == 3) {
			_maps[i] = 0;
		}
	}
}

void Scene8180::prepareCustomGameplayLoop() {
	_doors.resume(_now, _position.x > 726);
	_oscar.resume(_now);
}

void Scene8180::runCustomEntrySequence() {
	switch (_entryScene) {
	case 8181: // Step back onto the porch from the saloon.
		_position = Common::Point(1023, 519);
		_walkPath.reset(_position);
		_doors.resume(_now, true);
		walkTo(Common::Point(586, 534), 5);
		break;
	case 8182: // Enough time has passed for Rutger to finish the brew.
		_position = Common::Point(637, 535);
		_walkPath.reset(_position);
		_facing = 4;
		say(20);
		deferAction(400);
		break;
	default:
		_facing = 3;
		break;
	}
}

void Scene8180::resumeAmbientAudio(bool entering) {
	resumeFinaleOutdoorAudio(entering, 36, 26);
}

void Scene8180::advanceCustomGameplayLoop(uint32 delta) {
	updateFinaleOutdoorAudio();
	_doors.update(_now, _position.x > 726, _audio, 16);
	if (_finale.oscarAway) {
		_oscar.update(_now, _random, _audio);
	}
}

void Scene8180::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (_finale.oscarAway) {
		_oscarAnimation.draw(viewport, _oscar.frame(false, 0), _camera.x, -_camera.y);
	}
	_doors.draw(viewport, _position.x > 726);
}

void Scene8180::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_position.x < 273) {
		drawForeground(viewport, 0);
	}
	if (_position.x > 714) {
		drawForeground(viewport, 1);
	}
}

float Scene8180::actorHeight() const {
	return _position.y < 519 ? _position.y * 1.3565892f - 249.069763f : 455;
}

void Scene8180::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

bool Scene8180::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	for (int y = MAX<int>(0, target.y - 2); y <= MIN<int>(_background.h - 1, target.y + 2); ++y) {
		if (_originalMaps[_mask[y * _background.w + target.x]] == 3) {
			return true;
		}
	}
	target.x = CLIP<int>(target.x, 202, 679);
	return false;
}

void Scene8180::enterSaloon(uint16 scene) {
	if (findRunawayRoom(scene)) {
		walkTo(Common::Point(1023, 519), 1);
	}
	leaveRoom(scene);
}

RunawayRoomScene::ActionResult Scene8180::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Leave the porch and return to Douglasville.
		leaveRoom(8221);
		break;
	case 304: // Walk through the automatic doors into the saloon.
		enterSaloon(8240);
		break;
	case 308: // Step on the board, or point out that Brian is already on it.
		if (_position == Common::Point(780, 527)) {
			say(7);
		} else {
			walkTo(Common::Point(780, 527), 1);
		}
		break;
	case 400: // Return to Rutger for the completed brew.
		enterSaloon(8243);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
