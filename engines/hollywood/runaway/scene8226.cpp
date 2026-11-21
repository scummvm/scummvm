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
#include "hollywood/runaway/scene8226.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1023, 331, 3, {301, 302}, {0, 1}, {1023, 331}, 5}, // 1: Door.
	{1001, 245, 3, {301, 303}, {0, 1}, {1001, 245}, 5}, // 2: Door.
	{983, 216, 3, {301, 304}, {0, 1}, {983, 216}, 5}, // 3: Door.
	{834, 370, 7, {305, 306}, {0, 0}, {0, 0}} // 4: Street.
};

const RunawayResponse kResponses[] = {
	{8226, 301, 20, 0}, // Examine a door to Saturn's workshop.
	{8226, 306, 22, 0}  // Refuse to jump into the street.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8226 = {
	8226, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {95, 90, 70}, {65, 60, 40}, {55, 50, 30}}, Common::Point(844, 230),
	kResponses, ARRAYSIZE(kResponses), nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene8226::Scene8226(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game), _oscar(false, 6) {}

bool Scene8226::loadWalkMap() {
	return loadRunawayMask(_archive, 1024, 600, _mask, _originalMaps, 4);
}

bool Scene8226::loadCustomResources() {
	return _oscarAnimation.load(_archive, 7, 107) && _carAnimation.load(_archive, 9, 161);
}

void Scene8226::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[256 + i] == 5) {
			_maps[256 + i] = 0;
		}
	}
}

void Scene8226::prepareCustomGameplayLoop() {
	_oscar.resume(_now);
}

void Scene8226::runCustomEntrySequence() {
	static const Common::Point entries[] = {
		Common::Point(844, 230), Common::Point(836, 263), Common::Point(864, 348)
	};
	_position = entries[_entryScene - 8226];
	_walkPath.reset(_position);
	_facing = 5;
}

void Scene8226::resumeAmbientAudio(bool entering) {
	resumeFinaleOutdoorAudio(entering, 36, 51);
}

void Scene8226::advanceCustomGameplayLoop(uint32 delta) {
	updateFinaleOutdoorAudio();
	if (_finale.oscarAway) {
		_oscar.update(_now, _random, _audio);
	}
}

void Scene8226::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (_finale.oscarAway) {
		_oscarAnimation.draw(viewport, _oscar.frame(false, 0), _camera.x, -_camera.y);
		_carAnimation.draw(viewport, 58, _camera.x, -_camera.y);
	}
}

float Scene8226::actorHeight() const {
	return _position.y * 1.14457834f - 138.253006f;
}

void Scene8226::setupActorDepth(RunawayActor::RenderParams &params) const {
	static const uint16 depth[] = {0, 263, 195, 490, 396, 227, 247, 349, 322, 0};
	for (uint i = 1; i < ARRAYSIZE(depth); ++i) {
		params.depthY[i] = depth[i];
	}
}

void Scene8226::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

bool Scene8226::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = CLIP<int>(target.x, 834, 908);
	return false;
}

RunawayRoomScene::ActionResult Scene8226::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Enter Saturn's workshop through the nearest door.
		leaveRoom(8301);
		break;
	case 303: // Enter Saturn's workshop through the middle door.
		leaveRoom(8302);
		break;
	case 304: // Enter Saturn's workshop through the far door.
		leaveRoom(8303);
		break;
	case 305: // Look down at the street and its new sheriff.
		say(21, _finale.oscarAway);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
