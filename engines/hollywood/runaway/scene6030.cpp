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
#include "hollywood/runaway/scene6030.h"

namespace Hollywood {

// Both shaft actions use RESOURCE.F03 and the mine entrance's text.
const RunawayRoom kRunawayScene6030 = {
	6030, 1024, 600, nullptr, 0, {{100, 100, 100}},
	Common::Point(512, 500), nullptr, 0, nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene6030::Scene6030(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

uint Scene6030::sceneResourceNumber() const {
	return 3;
}

uint Scene6030::sceneTextNumber() const {
	return 608;
}

bool Scene6030::loadWalkMap() {
	_mask.resize(1024 * 600, 0);
	_originalMaps.resize(1536, 0);
	return true;
}

void Scene6030::runCustomEntrySequence() {
	dispatchCustomSceneAction(400);
}

bool Scene6030::drawCustomComposite(Graphics::ManagedSurface &viewport) {
	if (drawingRoomAnimation()) {
		const uint frame = _roomAction == 401 ? 29 + speechFrame() :
			_actionFrames[MIN<uint>(_actionStep, _actionFrames.size() - 1)];
		_actionAnimation.draw(viewport, frame, 0, 0);
	}
	return true;
}

RunawayRoomScene::ActionResult Scene6030::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 400: // Lean over the shaft.
		beginRoomAnimationRange(400, 1, 41, 0, 29, true, 60);
		break;
	case 1400: { // Look down, or explain why the ladder cannot be used.
		const bool climb = _entryScene == 6031 || _entryScene == 6015;
		say(climb ? 3 : 2, climb ? 0 : _random.getRandomNumber(3));
		holdRoomAnimation(401, 1, 41, 29);
		break;
	}
	case 1401: // Withdraw from the opening.
		beginRoomAnimationRange(402, 1, 41, 29, 0, true, 60);
		break;
	case 1402: // Return to the mine entrance after inspecting the shaft.
		leaveTownRoom(6081);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
