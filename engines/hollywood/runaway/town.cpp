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
	case 224: // Decorate the polished sculpture with Brian's amber.
		if (!_town.fuelTradeKnown) {
			say(224, 2, true);
		} else if (_inventory.town().sculpture != 1 || !_town.polishedSculptureRefused) {
			say(224, 1, true);
		} else {
			say(224, 0, true);
			_audio.playEffect(32);
			_inventory.town().sculpture = 2;
			_inventory.remove(4);
			say(219, 4, true);
		}
		return true;
	case 228: // Empty a measuring tube back into the concentrate.
		if (reply.variation) {
			break;
		}
		_audio.playEffect(34);
		say(228, 0, true);
		return true;
	case 233: // Once Joshua is gone, there is no reason to save another liter of water.
		if (reply.variation == 1 && _town.joshuaGone) {
			say(1, 2, true);
			return true;
		}
		return false;
	case 234: // Pour concentrate between the measuring tubes.
	case 236:
		if (reply.variation != 2) {
			return false;
		}
		_audio.playEffect(34);
		say(reply.cue, reply.variation, true);
		return true;
	case 240: // Polish the rough stone after Saturn explains what is wrong with it.
		if (_inventory.town().sculpture) {
			say(240, 3, true);
		} else if (!_town.fuelTradeKnown || !_town.roughSculptureRefused) {
			say(240, 0, true);
		} else {
			say(240, 1, true);
			_audio.playEffect(29);
			_inventory.town().sculpture = 1;
			_inventory.select(0);
			say(240, 2, true);
		}
		return true;
	default:
		break;
	}
	// These inventory operations have a spoken lead-in and completion response.
	uint16 effect = 0;
	byte first = reply.variation, last = reply.variation;
	if (reply.cue == 225 && reply.variation == 1) {
		effect = 65;
		last = 2;
	} else if (reply.cue == 227 && reply.variation == 1) {
		effect = 30;
		last = 2;
	} else if (reply.cue == 228 && (reply.variation == 2 || reply.variation == 3)) {
		effect = 34;
		first = 1;
	} else if (reply.cue == 231 && reply.variation == 2) {
		effect = 19;
		last = 3;
	} else if (reply.cue == 237 && reply.variation == 2) {
		effect = 34;
		last = 3;
	} else if (reply.cue == 241 && reply.variation == 1) {
		last = 2;
	} else {
		return false;
	}
	say(reply.cue, first, true);
	if (effect) {
		_audio.playEffect(effect);
	}
	say(reply.cue, last, true);
	return true;
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
