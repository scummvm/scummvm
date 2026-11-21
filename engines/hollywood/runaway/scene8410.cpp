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
#include "hollywood/runaway/scene8410.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{678, 442, 1, {301, 302}, {0, 3}, {706, 433}, 6}, // 1: Outside the trailer.
	{655, 445, 3, {303, 304}, {0, 1}, {761, 486}}, // 2: Garment to the right.
	{417, 433, 1, {305, 306}, {1, 0}, {496, 406}}, // 3: Trunk.
	{352, 403, 0, {307, 308}, {0, 0}, {346, 349}}, // 4: Bed.
	{167, 419, 7, {309, 310}, {0, 0}, {100, 377}} // 5: Refrigerator.
};

const RunawayResponse kResponses[] = {
	{8410, 301, 1, 0}, // Look outside the trailer.
	{8410, 305, 4, 0}, // Inspect the contents of the trunk.
	{8410, 306, 5, 0}, // Leave the clothes in the trunk.
	{8410, 307, 6, 0}, // Examine the bed.
	{8410, 308, 7, 0}, // Decline to sleep in the trailer.
	{8410, 309, 8, 0}, // Examine the refrigerator.
	{8410, 310, 9, 0}  // Leave the rotten food alone.
};

struct InsectRoute {
	uint16 first, last;
	byte sound;
};

const InsectRoute kInsectRoutes[] = {
	{4, 152, 0}, {152, 4, 0}, {250, 153, 45}, {153, 250, 45}, {251, 297, 46}, {297, 251, 46}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8410 = {
	8410, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {70, 70, 70}, {55, 55, 55}}, Common::Point(667, 443),
	kResponses, ARRAYSIZE(kResponses), nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene8410::Scene8410(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene8410::loadCustomResources() {
	// RESOURCE.001 bank 179 is Brian reaching upward in the alternate outfit.
	return _insect.load(_archive, 5, 298) && _actor.loadAction(179, 11);
}

bool Scene8410::applyCustomBackgroundPatches() {
	return _finale.nunsHabit != 2 || applyPatch(4);
}

void Scene8410::applyCustomHotspotState() {
	if (_finale.nunsHabit == 2) {
		for (uint i = 0; i < 256; ++i) {
			if (_maps[256 + i] == 2) {
				_maps[256 + i] = 0;
			}
		}
	}
}

void Scene8410::prepareCustomGameplayLoop() {
	_insectTick = _now;
	_insectRoute = 2 * _random.getRandomNumber(2);
	_insectFrame = kInsectRoutes[_insectRoute].first;
}

void Scene8410::runCustomEntrySequence() {
	_facing = 5;
	if (!_finale.trailerInteriorVisited) {
		_finale.trailerInteriorVisited = 1;
		say(0);
	}
}

void Scene8410::resumeAmbientAudio(bool entering) {
	_audio.stopEffect(2);
	if (entering || !_audio.musicPlaying()) {
		_audio.playMusic(12 + _random.getRandomNumber(4), false);
	}
}

void Scene8410::startInsectRoute(uint route) {
	_insectRoute = route;
	_insectFrame = kInsectRoutes[route].first;
	_insectMoving = true;
	if (kInsectRoutes[route].sound) {
		_audio.playEffect(kInsectRoutes[route].sound, 102, 1, true);
	}
}

void Scene8410::advanceInsect() {
	if (_insectMoving) {
		const InsectRoute &route = kInsectRoutes[_insectRoute];
		if (_insectFrame != route.last) {
			_insectFrame += route.first < route.last ? 1 : -1;
		} else {
			_insectMoving = false;
			_insectRight = !(_insectRoute % 2);
			_insectTwitching = _random.getRandomBit();
			_insectFrame = _insectRight ? 2 : 0;
			if (route.sound) {
				_audio.stopEffect(1);
			}
		}
		return;
	}
	const uint restingFrame = _insectRight ? 2 : 0;
	if (_insectTwitching && _insectFrame == restingFrame) {
		++_insectFrame;
		return;
	}
	_insectFrame = restingFrame;
	if (!_random.getRandomNumber(29)) {
		const uint choice = _random.getRandomNumber(3);
		if (choice < 3) {
			startInsectRoute(choice * 2 + _insectRight);
		} else {
			_insectTwitching = !_insectTwitching;
		}
	}
}

void Scene8410::advanceCustomGameplayLoop(uint32 delta) {
	updateOutdoorMusic(5);
	while (_now - _insectTick >= 80) {
		_insectTick += 80;
		advanceInsect();
	}
}

void Scene8410::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	_insect.draw(viewport, _insectFrame, 0, 0);
}

float Scene8410::actorHeight() const {
	return _position.y * 1.857142806f - 417.857147f;
}

void Scene8410::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
}

void Scene8410::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

bool Scene8410::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = CLIP<int>(target.x, 144, 678);
	return false;
}

Common::U32String Scene8410::hotspotName(byte id) const {
	if (id == 2 && _finale.nunsHabit) {
		return _inventory.name(15);
	}
	return RunawayRoomScene::hotspotName(id);
}

bool Scene8410::updateCustomSequence() {
	if (!_takingHabit) {
		return false;
	}
	if (_now - _pickupStarted < 11 * 60) {
		return true;
	}
	_takingHabit = false;
	return false;
}

void Scene8410::updateActor(uint elapsed, int scrollDirection, bool inventoryOpen) {
	if (_takingHabit) {
		_actor.setActionFrame(MIN<uint>(10, (_now - _pickupStarted) / 60));
		return;
	}
	RunawayRoomScene::updateActor(elapsed, scrollDirection, inventoryOpen);
}

void Scene8410::examineHabit() {
	if (_finale.bankResearch >= 3) {
		say(2, 2);
	} else if (!_finale.nunsHabit) {
		_finale.nunsHabit = 1;
		say(2);
	} else {
		say(2, 1);
	}
}

RunawayRoomScene::ActionResult Scene8410::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Leave the trailer; taking the habit triggers the gangsters' arrival.
		_audio.stopEffect(1);
		leaveRoom(_finale.nunsHabit == 2 && !_finale.oscarAway ? 8402 : 8401);
		break;
	case 303: // Identify the garment hanging next to the bed.
		examineHabit();
		break;
	case 304: // Examine the garment before deciding whether to take it.
		if (!_finale.nunsHabit) {
			examineHabit();
		}
		deferAction(400);
		break;
	case 400: // Take the habit only after Sushi has explained the bank's security.
		if (_finale.bankResearch < 3) {
			say(3);
		} else {
			say(3, 1);
			deferAction(401);
		}
		break;
	case 401: // Reach up to remove the habit from the wall.
		_takingHabit = true;
		_pickupStarted = _now;
		deferAction(402);
		break;
	case 402: // Remove the garment from the room and add it to the inventory.
		_finale.nunsHabit = 2;
		_inventory.add(15);
		say(3, 2);
		return kBackgroundChanged;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
