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
#include "hollywood/runaway/scene6270.h"

namespace Hollywood {

const RunawayRoom kRunawayScene6270 = {
	6270, 1024, 700, nullptr, 0, {{100, 100, 100}, {80, 70, 70}},
	Common::Point(133, 699), nullptr, 0, nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene6270::Scene6270(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene6270::loadBackground() {
	Graphics::ManagedSurface image;
	if (!loadRunawaySurface(_archive, 0, 1024, 600, image)) {
		return false;
	}
	// The mask extends below the picture for Brian's entrance from off screen.
	_background.create(1024, 700, image.format);
	_background.clear();
	_background.blitFrom(image);
	return true;
}

void Scene6270::runCustomEntrySequence() {
	walkTo(Common::Point(341, 498), 1);
	deferTownAction(400);
}

float Scene6270::actorHeight() const {
	return _position.y * 0.149253726f + 225.671646f;
}

int Scene6270::sceneViewportYOffset() const {
	return 0;
}

RunawayRoomScene::ActionResult Scene6270::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 400: // Brian looks down into the crater.
		if (!_town.craterVisited) {
			say(0);
			deferTownAction(401);
		} else {
			if (!_random.getRandomNumber(4)) {
				say(0, 2);
			}
			deferTownAction(402);
		}
		break;
	case 401: // Finish Brian's first reaction to the crater.
		_facing = 4;
		say(0, 1);
		_town.craterVisited = 1;
		deferTownAction(402);
		break;
	case 402: // Descend to Joshua's camp.
		walkTo(Common::Point(336, 503));
		leaveTownRoom(6291);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
