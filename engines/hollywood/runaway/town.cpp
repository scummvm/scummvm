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

#include "common/textconsole.h"
#include "common/util.h"
#include "hollywood/runaway/scene.h"

namespace Hollywood {

bool RunawayRoomScene::sayTownInventory(const RunawayInventory::Reply &reply) {
	switch (reply.cue) {
	case 200: // Inspect the crucifix.
		return sayHopiInventory(reply);
	case 209: { // Read the mine map from the inventory.
		typedef RunawaySequence S;
		static const S::Command commands[] = {
			{S::kImage, 146}, {S::kSpeech, 209, 0, 0, 1}, {S::kWaitInput}, {S::kEnd}
		};
		_inventorySequence = true;
		_sequence.start(commands, _text, _now, 'F');
		return true;
	}
	case 219: // Inspect the sculpture after Saturn has rejected its current finish.
		say(219, _inventory.town().sculpture == 2 ? 4 : _inventory.town().sculpture == 1 ?
			(_town.polishedSculptureRefused ? 3 : 2) : _town.roughSculptureRefused, true);
		return true;
	case 230: // Refuse attaching the strap, or pouring concentrate in German.
	case 242: // The prepared tobacco needs no more grinding.
		if (!_restoredContent) {
			executeItemAction(1);
			return true;
		}
		return false;
	case 233: // Once Joshua is gone, there is no reason to save another liter of water.
		if (reply.variation == 1 && _town.joshuaGone) {
			say(1, 2, true);
			return true;
		}
		return false;
	default:
		break;
	}
	return false;
}

void RunawayRoomScene::resumeTown() {
	prepareCustomGameplayLoop();
	if (!_entering) {
		resumeAmbientAudio(false);
		return;
	}
	_entering = false;
	_town.arrived = 1;
	resumeAmbientAudio(true);
	runCustomEntrySequence();
}

void RunawayRoomScene::walkTown(Common::Point target) {
	findGameplayWalkPath(target);
}

void RunawayRoomScene::deferTownAction(uint16 action) {
	deferAction(action);
}

void RunawayRoomScene::leaveTownRoom(uint16 scene) {
	leaveRoom(scene);
}

} // End of namespace Hollywood
