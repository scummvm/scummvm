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
#include "hollywood/runaway/scene8130.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{0, 599, 5, {301, 302}, {0, 3}, {-100, 599}, 7}, // 1: Path.
	{402, 374, 1, {303, 304}, {0, 3}, {637, 237}, 5}, // 2: House.
	{491, 495, 3, {305, 306}, {0, 0}, {715, 540}}, // 3: Well.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{0, 0, 0, {309, 310}, {0, 0}, {323, 544}} // 5: Skull.
};

const RunawayResponse kResponses[] = {
	{8130, 301, 1, 1}, // Look toward Douglasville.
	{8130, 305, 3, 0}, // Examine the abandoned well.
	{8130, 309, 7, 0}  // Examine the skull.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8130 = {
	8130, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {100, 95, 90}, {80, 75, 70}}, Common::Point(268, 450),
	kResponses, ARRAYSIZE(kResponses), nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene8130::Scene8130(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game), _oscar(false, 6) {}

bool Scene8130::loadCustomResources() {
	return _oscarAnimation.load(_archive, 6, 106) && _departureAnimation.load(_archive, 7, 40);
}

bool Scene8130::applyCustomBackgroundPatches() {
	return applyPatch(4) && applyPatch(5);
}

void Scene8130::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[256 + i] == 4) {
			_maps[256 + i] = 0;
		}
	}
}

void Scene8130::prepareCustomGameplayLoop() {
	_oscar.resume(_now);
	_departureTick = _now;
}

void Scene8130::runCustomEntrySequence() {
	switch (_entryScene) {
	case 8131: // Return from Mama Dorita's doorway.
		_position = Common::Point(402, 374);
		_walkPath.reset(_position);
		walkTo(Common::Point(401, 375), 5);
		break;
	case 8132: // See Oscar off on his mission to confront the gangsters.
		_position = Common::Point(456, 358);
		_walkPath.reset(_position);
		_facing = 5;
		_departing = true;
		say(20);
		deferAction(400);
		break;
	default:
		_position = Common::Point(0, 599);
		_walkPath.reset(_position);
		walkTo(Common::Point(268, 450), 1, true);
		break;
	}
}

void Scene8130::resumeAmbientAudio(bool entering) {
	resumeFinaleOutdoorAudio(entering, 40, 77);
}

void Scene8130::advanceCustomGameplayLoop(uint32 delta) {
	updateFinaleOutdoorAudio(51);
	if (!_finale.oscarAway && !_departing) {
		_oscar.update(_now, _random, _audio);
	}
}

bool Scene8130::updateCustomSequence() {
	if (!_departing) {
		return false;
	}
	// Oscar keeps walking while Brian says goodbye.
	static const byte footsteps[] = {3, 8, 15, 21, 27, 32, 39};
	while (_departureFrame < 39 && _now - _departureTick >= 80) {
		_departureTick += 80;
		++_departureFrame;
		for (uint i = 0; i < ARRAYSIZE(footsteps); ++i) {
			if (_departureFrame == footsteps[i]) {
				_audio.playFootstep(2, _random.getRandomBit(), (65 + i * 5) * 255 / 100, 400 - i * 50);
				break;
			}
		}
	}
	if (_departureFrame < 39 || speaking()) {
		return true;
	}
	_departing = false;
	return false;
}

void Scene8130::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (!_finale.oscarAway && !_departing) {
		_oscarAnimation.draw(viewport, _oscar.frame(false, 0), _camera.x, -_camera.y);
	}
}

void Scene8130::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_departing) {
		_departureAnimation.draw(viewport, _departureFrame, _camera.x, -_camera.y);
	}
}

float Scene8130::actorHeight() const {
	return _position.y * 1.027668f - 160.57312f;
}

void Scene8130::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 480;
	params.depthY[2] = 599;
}

void Scene8130::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

bool Scene8130::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = MIN<int>(target.x, 946);
	return false;
}

RunawayRoomScene::ActionResult Scene8130::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Follow the path back to the town overview.
		leaveRoom(8350);
		break;
	case 303: // Look at the house before or after Oscar leaves.
		say(2, _finale.oscarAway ? 2 : 1);
		break;
	case 304: // Approach Mama Dorita's doorway.
		leaveRoom(8090);
		break;
	case 306: // Refuse to jump into the well now that Gina is safe.
		say(4, 2 + _random.getRandomBit());
		break;
	case 310: // Leave Mama Dorita's skull decoration alone.
		say(8, _finale.oscarAway);
		break;
	case 400: // Oscar has left; return to the house to discuss the plan.
		_finale.oscarAway = 1;
		leaveRoom(8094);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
