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
#include "hollywood/runaway/scene8350.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{0, 0, 0, {1, 301}, {0, 0}, {0, 0}, 7}, // 1: Entrance to the mine.
	{0, 0, 0, {1, 302}, {0, 0}, {0, 0}, 5}, // 2: Douglasville.
	{0, 0, 0, {1, 303}, {0, 0}, {0, 0}, 6}, // 3: Mama Dorita's house.
	{0, 0, 0, {1, 304}, {0, 0}, {0, 0}, 5}, // 4: Crater.
	{0, 0, 0, {1, 305}, {0, 0}, {0, 0}, 4}, // 5: Path to the trailer.
	{0, 0, 0, {1, 306}, {0, 0}, {0, 0}, 7} // 6: Locomotive.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8350 = {
	8350, 1024, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}},
	Common::Point(512, 300), nullptr, 0, nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene8350::Scene8350(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene8350::applyCustomBackgroundPatches() {
	return (_finale.craterVisited || applyPatch(3)) && (!_finale.trailerFound || applyPatch(4));
}

void Scene8350::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[256 + i] == 5 && !_finale.trailerFound) {
			_maps[256 + i] = 0;
		}
	}
}

void Scene8350::resumeAmbientAudio(bool entering) {
	resumeFinaleOutdoorAudio(entering, 36, 51);
}

void Scene8350::advanceCustomGameplayLoop(uint32 delta) {
	updateFinaleOutdoorAudio();
}

RunawayRoomScene::ActionResult Scene8350::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 1: // Looking at the overview does not start a speech.
		break;
	case 301: // Refuse to return through the mine.
		say(3);
		break;
	case 302: // Return to Douglasville's main street.
		leaveRoom(8220);
		break;
	case 303: // Go to Mama Dorita's house by the oil well.
		leaveRoom(8130);
		break;
	case 304: // Visit the abandoned crater once, then leave it alone.
		if (_finale.craterVisited) {
			say(4);
		} else {
			_finale.craterVisited = 1;
			leaveRoom(8290);
		}
		break;
	case 305: { // Follow Saturn's directions, or check whether the gangsters are still there.
		const uint16 destination = _finale.nunsHabit == 2 && !_finale.oscarAway ? 8403 : 8400;
		if (_finale.trailerFound == 1) {
			say(5);
			_finale.trailerFound = 2;
		}
		if (destination == 8403) {
			say(5, 1);
		}
		leaveRoom(destination);
		break;
	}
	case 306: // Approach the derailed locomotive.
		leaveRoom(8265);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
