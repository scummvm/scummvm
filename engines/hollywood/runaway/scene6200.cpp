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
#include "hollywood/runaway/scene6200.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1379, 491, 0, {301, 302}, {0, 3}, {1379, 491}, 5}, // 1: Town.
	{285, 501, 0, {303, 304}, {1, 0}, {258, 427}}, // 2: Cell.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{536, 504, 0, {306, 307}, {0, 1}, {536, 425}}, // 4: Firewood.
	{1020, 483, 0, {308, 309}, {0, 0}, {1020, 458}}, // 5: Desk.
	{1020, 483, 0, {310, 311}, {0, 0}, {1020, 458}}, // 6: Gun cabinet.
	{857, 490, 0, {312, 313}, {0, 0}, {857, 435}} // 7: Bulletin board.
};

const RunawayResponse kResponses[] = {
	{6200, 301, 1, 0},  // Look out toward the saloon.
	{6200, 306, 4, 0},  // Look at the firewood.
	{6200, 308, 6, 0},  // Examine the desk.
	{6200, 309, 7, 0},  // Search the desk.
	{6200, 310, 8, 0},  // Look at the gun cabinet.
	{6200, 311, 9, 0},  // Try to take its chain.
	{6200, 312, 10, 0}, // Read the wanted posters.
	{6200, 313, 11, 0}  // Try to take the wanted posters.
};

const RunawayItemAction kItemActions[] = {
	{6200, 9, 2, 314} // Unlock the cell with the sheriff's key.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6200 = {
	6200, 1380, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {85, 80, 80}, {65, 60, 65}, {55, 50, 55}, {45, 40, 45}, {55, 50, 55}},
	Common::Point(1227, 510),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene6200::Scene6200(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

const char *Scene6200::actionCaptionVariant(byte id, bool look, byte item) const {
	return !look && !item && id == 2 && _town.cellOpen ? "open" : nullptr;
}

bool Scene6200::usesAlternateActor() const {
	return false;
}

bool Scene6200::loadCustomResources() {
	return _cellDoorAnimation.load(_archive, 6, 26);
}

bool Scene6200::applyCustomBackgroundPatches() {
	return !_town.firewoodTaken || applyPatch(4);
}

void Scene6200::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		// The approach to the cell is scripted; free walking stays in the office.
		if (_maps[i] > 1) {
			_maps[i] = 0;
		}
		if (_town.firewoodTaken && _maps[256 + i] == 4) {
			_maps[256 + i] = 0;
		}
	}
}

void Scene6200::resumeAmbientAudio(bool entering) {
	const bool restart = entering && _entryScene == 6200;
	if (restart || !_audio.musicPlaying()) {
		_audio.playMusic(18);
		_audio.setMusicVolume(191);
	} else if (entering) {
		_audio.setMusicVolume(191);
	}
}

void Scene6200::runCustomEntrySequence() {
	if (_entryScene == 6201) {
		_position = Common::Point(258, 445);
		_walkPath.reset(_position);
		updateCamera();
		walkTo(Common::Point(211, 506), 5, true);
	} else if (!_town.bodySeen) {
		_facing = 5;
		say(0);
		deferTownAction(400);
	}
}

void Scene6200::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (!drawingRoomAnimation() || _roomAction != 314) {
		_cellDoorAnimation.draw(viewport, _town.cellOpen ? 25 : 0, _camera.x, -_camera.y);
	}
}

float Scene6200::actorHeight() const {
	return _position.y > 501 ? _position.y * 0.618556678f + 84.4845352f :
		_position.y * 1.38297868f - 299.255310f;
}

void Scene6200::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
}

RunawayRoomScene::ActionResult Scene6200::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Return to the street.
		leaveTownRoom(6223);
		break;
	case 303: // Examine the prisoner before or after learning his identity.
		say(2, !_town.doctorKnown ? 2 : !_town.cellOpen ? 1 : 0);
		break;
	case 304: // Try the cell door.
		if (!_town.cellOpen) {
			say(3);
			break;
		}
		// fall through
	case 305: // Enter the unlocked cell.
		walkTo(Common::Point(258, 445), 0, true);
		leaveTownRoom(6210);
		break;
	case 307: // Take the firewood.
		say(5);
		deferTownAction(407);
		break;
	case 407: // Collect the firewood.
		_town.firewoodTaken = 1;
		_inventory.add(14);
		say(5, 1);
		return kBackgroundChanged;
	case 314: // Unlock the cell.
		if (_town.cellOpen) {
			dispatchCustomSceneAction(305);
		} else {
			byte brian[47], door[47];
			for (uint i = 0; i < ARRAYSIZE(door); ++i) {
				brian[i] = i ? i - 1 : 0;
				door[i] = i > 21 ? i - 21 : 0;
			}
			say(12);
			beginRoomAnimation(314, 7, 47, brian, ARRAYSIZE(brian));
			setRoomAnimationOverlay(6, 26, door, ARRAYSIZE(door));
			_inventory.select(0);
		}
		break;
	case 1314: // Finish unlocking the cell and remove the key from inventory.
		_town.cellOpen = 1;
		_inventory.remove(9);
		say(12, 1);
		break;
	case 400: // Walk up to the cell on the first visit.
		walkTo(Common::Point(285, 501), 0);
		deferTownAction(401);
		break;
	case 401: // Discover the dead prisoner.
		say(0, 1);
		_town.bodySeen = 1;
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene6200::handleAnimationFrameHook() {
	if (_roomAction == 314) {
		if (_actionStep == 13) {
			_audio.playEffect(58);
		} else if (_actionStep == 22) {
			_audio.playEffect(59);
		}
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
