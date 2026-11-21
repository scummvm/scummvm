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
#include "hollywood/runaway/scene7030.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{0, 0, 0, {301, 302}, {0, 0}, {0, 0}, 4}, // 1: Hopi village.
	{0, 0, 0, {303, 304}, {0, 0}, {0, 0}, 6} // 2: Hopi sanctuary.
};

const Common::Point kVillageTrail[] = {
	Common::Point(544, 472), Common::Point(520, 482), Common::Point(495, 496),
	Common::Point(456, 491), Common::Point(415, 487), Common::Point(377, 488),
	Common::Point(346, 497)
};

const Common::Point kSanctuaryTrail[] = {
	Common::Point(761, 775), Common::Point(797, 760), Common::Point(810, 748),
	Common::Point(835, 744), Common::Point(870, 751), Common::Point(904, 748),
	Common::Point(962, 732), Common::Point(986, 754), Common::Point(995, 774),
	Common::Point(1019, 779), Common::Point(1041, 777), Common::Point(1047, 787),
	Common::Point(1108, 782), Common::Point(1115, 770), Common::Point(1117, 751),
	Common::Point(1169, 728)
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene7030 = {
	7030, 1520, 900, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {60, 60, 60}, {80, 80, 80}, {70, 70, 70}, {90, 90, 90}}, Common::Point(562, 467),
	nullptr, 0, nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene7030::Scene7030(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene7030::loadCustomResources() {
	Graphics::ManagedSurface scratch(1, 1, _screen.format);
	for (uint i = 0; i < ARRAYSIZE(_layers); ++i) {
		if (!_archive.readVariableChunk(4 + i, _layers[i]) || !applyRunawayPatch(_layers[i], scratch)) {
			return false;
		}
	}
	return _water[0].load(_archive, 10, 96) && _water[1].load(_archive, 11, 96) &&
		_water[2].load(_archive, 8, 48) && _water[3].load(_archive, 9, 48);
}

void Scene7030::resumeAmbientAudio(bool entering) {
	_audio.playEffect(12, 179, 1, true);
}

void Scene7030::runCustomEntrySequence() {
	_facing = 5;
	if (_entryScene == 7031) {
		_position = Common::Point(1169, 728);
		_camera = Common::Point(496, 300);
	} else {
		_position = Common::Point(562, 467);
		_camera = Common::Point(350, _hopi.canyonVisited ? 161 : 100);
		if (!_hopi.canyonVisited) {
			say(0);
			deferAction(400);
		}
	}
	_walkPath.reset(_position);
}

void Scene7030::advanceCustomGameplayLoop(uint32 delta) {
	updateOutdoorMusic();
}

void Scene7030::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	drawAmbientAnimations(viewport, _water, ARRAYSIZE(_water));
}

void Scene7030::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	// The three rock layers move independently of the distant canyon.
	static const int divisors[] = {6, 4, 2};
	static const int offsets[] = {-124, -166, 0};
	for (uint i = 0; i < ARRAYSIZE(_layers); ++i) {
		const int x = (496 - _camera.x) / divisors[i] + offsets[i] - _camera.x;
		const int y = (300 - _camera.y) / divisors[i] - _camera.y;
		applyRunawayPatch(_layers[i], viewport, y, x);
	}
}

void Scene7030::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 900;
}

void Scene7030::updateActor(uint elapsed, int scrollDirection, bool inventoryOpen) {
	const Common::Point camera = _camera;
	RunawayRoomScene::updateActor(elapsed, _crossing ? 0 : scrollDirection, inventoryOpen);
	if (!_crossing || inventoryOpen) {
		return;
	}
	_panElapsed += elapsed;
	const int steps = _panElapsed / 16;
	_panElapsed %= 16;
	setCameraX(camera.x + (_crossing == 1 ? -steps : steps));
	_camera.y = MIN<int>(300, camera.y + (_crossing == 1 ? steps : 0));
}

void Scene7030::advanceRoute() {
	if (_routeStep < _routeCount) {
		walkTo(_route[_routeStep++], 0xff, true);
		deferAction(404);
	} else {
		execute(_crossing == 1 ? 402 : 403);
	}
}

RunawayRoomScene::ActionResult Scene7030::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 301: // Look back at Gina's village.
		say(1, _position.x < 900 ? 0 : 1);
		break;
	case 302: // Return to Gina through the unexplored end of the village.
		leaveRoom(7022);
		break;
	case 303: // Locate the sanctuary across the canyon.
		say(2, _position.x < 900 ? 0 : 1);
		break;
	case 304: // Return to the sanctuary after the first crossing.
		leaveRoom(7040);
		break;
	case 400: // Notice the suspension bridges on the first visit.
		say(0, 1);
		deferAction(401);
		break;
	case 401: // Follow the ledge out of the village.
		_crossing = 1;
		_route = kVillageTrail;
		_routeCount = ARRAYSIZE(kVillageTrail);
		_routeStep = 0;
		advanceRoute();
		break;
	case 402: // Continue across the bridges in the foreground.
		_position = Common::Point(718, 899);
		_walkPath.reset(_position);
		_crossing = 2;
		_route = kSanctuaryTrail;
		_routeCount = ARRAYSIZE(kSanctuaryTrail);
		_routeStep = 0;
		advanceRoute();
		break;
	case 403: // Reach the sanctuary and remember the completed crossing.
		_crossing = 0;
		_hopi.canyonVisited = 1;
		leaveRoom(7040);
		break;
	case 404: // Continue to the next bend of the canyon trail.
		advanceRoute();
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
