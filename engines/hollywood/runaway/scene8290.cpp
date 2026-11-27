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
#include "hollywood/runaway/scene8290.h"

namespace Hollywood {
namespace {

const uint kGestureFrameDuration = 60;
// Clip the original (1839, 442) entry path to the room's right edge.
const Common::Point kTrail(1739, 451);

} // End of anonymous namespace

const RunawayRoom kRunawayScene8290 = {
	8290, 1740, 600, nullptr, 0,
	{{100, 100, 100}, {80, 70, 70}, {70, 60, 60}, {60, 50, 50}}, kTrail,
	nullptr, 0, nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene8290::Scene8290(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

void Scene8290::setCameraX(int x) {
	// The return to the empty campsite keeps the rightmost view throughout.
	_camera.x = _background.w - kRunawayScreenWidth;
}

bool Scene8290::loadCustomResources() {
	// RESOURCE.001 bank 187 is Brian's front-facing shrug in the alternate outfit.
	return loadForeground(0, 4) && _actor.loadAction(187, 13);
}

void Scene8290::runCustomEntrySequence() {
	_facing = 5;
	walkTo(Common::Point(1363, 484), 5, true);
	deferAction(400);
}

void Scene8290::resumeAmbientAudio(bool entering) {
	resumeFinaleOutdoorAudio(entering, 36, 20);
}

void Scene8290::advanceCustomGameplayLoop(uint32 delta) {
	updateFinaleOutdoorAudio();
}

void Scene8290::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_position.x > 1380) {
		drawForeground(viewport, 0);
	}
}

float Scene8290::actorHeight() const {
	return (_position.x * -0.0231607631f + 425.276581f +
		_position.y * 0.440251559f + 191.289307f) / 2;
}

bool Scene8290::updateCustomSequence() {
	if (_gesture == kNoGesture) {
		return false;
	}
	if (_now - _gestureStarted < 13 * kGestureFrameDuration) {
		return true;
	}
	_gesture = kNoGesture;
	return false;
}

void Scene8290::updateActor(uint elapsed, int scrollDirection, bool inventoryOpen) {
	if (_gesture == kShrug) {
		_actor.setActionFrame(MIN<uint>(12, (_now - _gestureStarted) / kGestureFrameDuration));
		return;
	}
	RunawayRoomScene::updateActor(elapsed, 0, false);
}

RunawayRoomScene::ActionResult Scene8290::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 400: // Discover that Joshua and all his equipment have disappeared.
		say(0, 50);
		deferAction(401);
		break;
	case 401: // Turn to face the player and shrug.
		_facing = 4;
		_gesture = kShrug;
		_gestureStarted = _now;
		deferAction(402);
		break;
	case 402: // Give up trying to explain Joshua's disappearance.
		say(0, 51);
		deferAction(403);
		break;
	case 403: // Look to both sides of the empty campsite.
		beginHeadMovement(406, RunawayHeadMovement::kRight, 1);
		break;
	case 406:
		beginHeadMovement(404, RunawayHeadMovement::kLeft, 1);
		break;
	case 404: // Decide to stay away from the crater.
		say(0, 52);
		deferAction(405);
		break;
	case 405: // Walk back up the trail to the town overview.
		leaveRoom(8350);
		if (_hospitalExit.scene) {
			walkTo(kTrail, 3, true);
		}
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
