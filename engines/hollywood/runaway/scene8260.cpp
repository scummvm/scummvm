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
#include "hollywood/runaway/scene8260.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{0, 510, 5, {301, 302}, {0, 3}, {0, 510}, 7}, // 1: Town.
	{363, 395, 7, {303, 304}, {0, 1}, {273, 286}, 7}, // 2: Driver's cab.
	{495, 433, 1, {305, 306}, {0, 1}, {582, 344}}, // 3: Smokestack.
	{432, 411, 0, {307, 308}, {0, 0}, {435, 351}}, // 4: Water tank opening.
	{413, 485, 7, {309, 310}, {1, 1}, {392, 465}}, // 5: Object.
	{257, 430, 1, {311, 312}, {0, 0}, {257, 397}}, // 6: Whistle.
	{1023, 437, 0, {315, 316}, {0, 3}, {1100, 400}, 5} // 7: Outskirts of town.
};

const RunawayResponse kResponses[] = {
	{8260, 301, 1, 1},  // Look toward the familiar town.
	{8260, 303, 2, 0},  // Examine the damaged cab.
	{8260, 305, 4, 0},  // Examine the smokestack.
	{8260, 306, 5, 0},  // Look inside the sooty smokestack.
	{8260, 307, 6, 0},  // Identify the water inlet.
	{8260, 308, 7, 0},  // The inlet has no cap to open.
	{8260, 311, 10, 0}, // Examine the whistle.
	{8260, 312, 11, 0}, // The whistle is operated from the cab.
	{8260, 315, 16, 1} // Look toward the crater.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8260 = {
	8260, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {100, 90, 80}}, Common::Point(123, 469),
	kResponses, ARRAYSIZE(kResponses), nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene8260::Scene8260(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	Scene6260(audio, inventory, strings, game) {}

bool Scene8260::applyCustomBackgroundPatches() {
	return applyPatch(_finale.locomotiveBadge == 1 || _finale.locomotiveBadge == 2 ? 8 : 7);
}

void Scene8260::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		// Reserve the upper zone for approaches to the cab and water inlet.
		if (_maps[i] == 2) {
			_maps[i] = 0;
		}
		if (_maps[256 + i] == 5 && (_finale.locomotiveBadge == 0 || _finale.locomotiveBadge == 3)) {
			_maps[256 + i] = 0;
		}
	}
}

void Scene8260::runCustomEntrySequence() {
	if (_entryScene == 8261) {
		beginRoomAnimation(401, 5, 88, nullptr, 0, true, 60);
	} else if (_entryScene == 8262) {
		beginRoomAnimation(402, 9, 45, nullptr, 0, true, 60);
		_actionFrames.insert_at(0, 0);
	} else {
		const bool outskirts = _entryScene == 8265;
		_position = outskirts ? Common::Point(825, 458) : Common::Point(123, 469);
		_walkPath.reset(_position);
		_facing = outskirts ? 7 : 1;
	}
}

void Scene8260::resumeAmbientAudio(bool entering) {
	resumeFinaleOutdoorAudio(entering, 36, 51);
}

void Scene8260::advanceCustomGameplayLoop(uint32 delta) {
	updateFinaleOutdoorAudio();
}

Common::U32String Scene8260::hotspotName(byte id) const {
	if (id == 5 && _finale.locomotiveBadge >= 2) {
		return _strings.hotspotName(_room, 8);
	}
	return RunawayRoomScene::hotspotName(id);
}

int8 Scene8260::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (id == 5 && !item && !use && _finale.locomotiveBadge == 2) {
		return false;
	}
	return approach;
}

RunawayRoomScene::ActionResult Scene8260::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Return to the main street.
		leaveRoom(8225);
		break;
	case 304: // Climb through the cab's broken window.
		beginRoomAnimation(304, 4, 131, nullptr, 0, true, 60);
		break;
	case 1304: // Enter the driver's cab after climbing through the window.
		leaveRoom(8263);
		break;
	case 309: // Identify the sheriff's badge ejected from the smokestack.
		_facing = 4;
		say(14, _finale.locomotiveBadge != 1);
		if (_finale.locomotiveBadge == 1) {
			_finale.locomotiveBadge = 2;
		}
		break;
	case 310: // Examine the badge before picking it up.
		if (_finale.locomotiveBadge == 1) {
			dispatchCustomSceneAction(309);
		}
		say(15);
		deferAction(410);
		break;
	case 410: // Collect the sheriff's badge.
		_facing = 4;
		_finale.locomotiveBadge = 3;
		_inventory.add(12);
		say(15, 1);
		return kBackgroundChanged;
	case 316: // Leave through the outskirts to the town overview.
		leaveRoom(8350);
		break;
	case 1401: // Finish climbing out of the cab.
		_position = Common::Point(363, 395);
		_walkPath.reset(_position);
		_facing = 3;
		break;
	case 1402: // The steam blast has dislodged the sheriff's badge.
		_finale.locomotiveBadge = 1;
		leaveRoom(8264);
		return kBackgroundChanged;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene8260::handleAnimationFrameHook() {
	if (_roomAction == 304 && _actionStep == 20) {
		_audio.playEffect(19);
	} else if (_roomAction == 401 && _actionStep == 32) {
		_audio.playEffect(20);
	} else if (_roomAction == 402) {
		if (_actionStep == 3) {
			_audio.playEffect(31, 230);
		} else if (_actionStep == 17) {
			_audio.playEffect(32, 255, 1);
		}
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
