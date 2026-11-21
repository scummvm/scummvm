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
#include "hollywood/runaway/scene6220.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{188, 599, 5, {301, 302}, {0, 3}, {188, 599}, 7}, // 1: Path.
	{778, 512, 3, {303, 304}, {0, 1}, {778, 512}, 6}, // 2: Saloon.
	{125, 441, 7, {305, 306}, {0, 1}, {79, 399}, 4}, // 3: Hotel.
	{388, 284, 7, {307, 308}, {0, 1}, {388, 284}, 4}, // 4: Sheriff's office.
	{574, 242, 7, {309, 310}, {0, 1}, {526, 216}, 4}, // 5: Liquor store.
	{647, 198, 0, {311, 312}, {0, 3}, {646, 195}, 4}, // 6: Bank.
	{722, 192, 0, {313, 314}, {0, 1}, {722, 192}, 5}, // 7: Locomotive.
	{293, 350, 7, {315, 316}, {1, 0}, {285, 340}}, // 8: Wagon.
	{722, 192, 0, {318, 319}, {0, 3}, {722, 192}, 5} // 9: North side of town.
};

const RunawayResponse kResponses[] = {
	{6220, 301, 1, 0}, // Look at the railroad tracks.
	{6220, 303, 2, 0}, // Look at the saloon.
	{6220, 305, 3, 0}, // Look at the hotel.
	{6220, 307, 4, 0}, // Look at the sheriff's office.
	{6220, 309, 5, 0}, // Look at the liquor store.
	{6220, 310, 6, 0}, // Try the boarded-up liquor store.
	{6220, 311, 7, 0}, // Look at the bank.
	{6220, 313, 8, 0}, // Look at the locomotive.
	{6220, 318, 1, 1}  // Look toward the cemetery.
};

const RunawayItemAction kItemActions[] = {
	{6220, 22, 8, 317} // Cut the wagon strap with the pruning shears.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6220 = {
	6220, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {95, 90, 70}, {75, 70, 50}, {55, 50, 30}},
	Common::Point(188, 599),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene6220::Scene6220(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

const char *Scene6220::actionCaptionVariant(byte id, bool look, byte item) const {
	if (id == 8 && _town.strapSeen == 1 && (!item || item == 22)) {
		return "strap";
	}
	return nullptr;
}

bool Scene6220::loadCustomResources() {
	return _sushiAnimation.load(_archive, 7, 170) && _brianAnimation.load(_archive, 8, 45) && _shotAnimation.load(_archive, 9, 60);
}

void Scene6220::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		// The balcony shares this mask but is reached through Kevin's room.
		if (_maps[i] < 2 || _maps[i] > 4) {
			_maps[i] = 0;
		}
	}
}

void Scene6220::resumeAmbientAudio(bool entering) {
	if (entering && _entryScene == 6223) {
		_audio.playMusic(12 + _random.getRandomNumber(4));
	} else {
		RunawayRoomScene::resumeAmbientAudio(entering);
	}
}

void Scene6220::runCustomEntrySequence() {
	struct Entry {
		int16 x, y, targetX, targetY;
		byte facing;
	};
	static const Entry entries[] = {
		{188, 599, 275, 530, 1}, // Arrive from the map.
		{778, 512, 686, 474, 7}, // Leave the saloon.
		{147, 460, 209, 527, 3}, // Leave Sushi's hotel.
		{420, 297, 470, 330, 3}, // Leave the sheriff's office.
		{647, 197, 652, 209, 3}, // Leave the bank.
		{718, 194, 718, 194, 5}  // Leave the locomotive.
	};
	const Entry &entry = entries[_entryScene - 6220];
	_position = Common::Point(entry.x, entry.y);
	_walkPath.reset(_position);
	walkTo(Common::Point(entry.targetX, entry.targetY), entry.facing, true);
	if (!_town.streetVisited) {
		deferTownAction(400);
	}
}

bool Scene6220::updateCustomSequence() {
	if (_townEncounter.phase == TownEncounter::kInactive) {
		return false;
	}
	updateEncounter();
	return true;
}

void Scene6220::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (_townEncounter.phase == TownEncounter::kInactive) {
		return;
	}
	const uint frame = _townEncounter.frame;
	uint sushi, brian, shot;
	if (_townEncounter.phase == TownEncounter::kRifle) {
		sushi = frame;
		shot = frame < 45 ? 59 : MIN<uint>(59, frame - 45);
		brian = frame <= 45 ? 0 : MIN<uint>(33, frame - 45);
	} else if (_townEncounter.phase == TownEncounter::kConversation) {
		sushi = 113 + (speaker() ? speechFrame() : 0);
		shot = 59;
		brian = 33;
	} else {
		sushi = 125 + frame;
		shot = 59;
		brian = MIN<uint>(44, 33 + frame);
	}
	_sushiAnimation.draw(viewport, sushi, _camera.x, -_camera.y);
	_shotAnimation.draw(viewport, shot, _camera.x, -_camera.y);
	_brianAnimation.draw(viewport, brian, _camera.x, -_camera.y);
}

bool Scene6220::shouldDrawActor() const {
	return _townEncounter.phase == TownEncounter::kInactive;
}

float Scene6220::actorHeight() const {
	return _position.y * 0.194835678f - 11.706573f;
}

void Scene6220::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 263;
	params.depthY[2] = 195;
	params.depthY[3] = 490;
	params.depthY[4] = 396;
	params.depthY[5] = 227;
	params.depthY[6] = 247;
	params.depthY[7] = 599;
	params.depthY[8] = 322;
	params.depthY[9] = 599;
}

int8 Scene6220::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (id == 8) {
		if (item && _town.strapSeen != 1) {
			return false;
		}
		if (!item && use && _town.strapSeen == 1) {
			return true;
		}
	}
	return approach;
}

RunawayRoomScene::ActionResult Scene6220::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Leave town by the railroad tracks.
	case 319: // Leave town toward the cemetery.
		leaveTownRoom(6350);
		break;
	case 304: // Enter the saloon.
		leaveTownRoom(6180);
		break;
	case 306: // Visit Sushi in the hotel.
		leaveTownRoom(6370);
		break;
	case 308: // Enter the sheriff's office.
		leaveTownRoom(6200);
		break;
	case 312: // Enter the ruined bank.
		leaveTownRoom(6190);
		break;
	case 314: // Approach the locomotive.
		leaveTownRoom(6260);
		break;
	case 315: // Examine the wagon and identify its strap.
		say(9, _town.strapSeen);
		if (!_town.strapSeen) {
			_town.strapSeen = 1;
		}
		break;
	case 316: // Try to take the wagon or its attached strap.
		say(10, _town.strapSeen == 1 ? 0 : 1);
		break;
	case 317: // Cut the strap only after identifying it on the wagon.
		if (_town.strapSeen != 1) {
			say(25, 0, true);
			break;
		}
		say(11);
		deferTownAction(417);
		break;
	case 417: { // Cut the wagon strap with the pruning shears.
		static const byte frames[] = {
			0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
			20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
		};
		beginRoomAnimation(417, 10, 22, frames, ARRAYSIZE(frames));
		break;
	}
	case 1417: // Collect the cut strap.
		_town.strapSeen = 2;
		_inventory.select(0);
		_inventory.add(11);
		say(11, 1);
		break;
	case 400: // First arrival in Douglasville.
		say(0);
		deferTownAction(401);
		break;
	case 401: // Sushi fires from the hotel and Brian raises his hands.
		_townEncounter.phase = TownEncounter::kRifle;
		_townEncounter.tick = _now;
		_townEncounter.frame = 0;
		_roomAction = 401;
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene6220::handleAnimationFrameHook() {
	if (_roomAction == 417 && _actionStep == 22) {
		_audio.playEffect(17);
	}
}

void Scene6220::updateEncounter() {
	if (_townEncounter.phase == TownEncounter::kInactive) {
		return;
	}
	const uint previous = _townEncounter.frame;
	const uint frame = (_now - _townEncounter.tick) / 80;
	if (_townEncounter.phase == TownEncounter::kRifle) {
		_townEncounter.frame = MIN<uint>(112, frame);
		if (previous < 45 && frame >= 45) {
			_audio.playEffect(11);
		}
		if (previous < 79 && frame >= 79) {
			say(0, 1);
		}
		if (frame >= 112 && !speaking()) {
			_townEncounter.phase = TownEncounter::kConversation;
			say(0, 2, false, 1);
			say(0, 3);
			say(0, 4, false, 1);
		}
	} else if (_townEncounter.phase == TownEncounter::kConversation) {
		if (!speaking()) {
			_townEncounter.phase = TownEncounter::kDeparture;
			_townEncounter.tick = _now;
			_townEncounter.frame = 0;
		}
	} else {
		_townEncounter.frame = MIN<uint>(44, frame);
		if (frame >= 45) {
			_townEncounter.phase = TownEncounter::kInactive;
			_roomAction = 0;
			_town.streetVisited = 1;
			walkTo(Common::Point(125, 441), 7, true);
			leaveTownRoom(6370);
		}
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
