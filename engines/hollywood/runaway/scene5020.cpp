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

#include "common/file.h"
#include "common/util.h"
#include "hollywood/runaway/scene5020.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1123, 416, 3, {302, 301}, {0, 3}, {1123, 416}, 6}, // 1: Path.
	{456, 405, 7, {0, 1}, {0, 0}, {0, 0}},
	{573, 400, 7, {304, 305}, {0, 0}, {533, 379}}, // 3: Rocking Chair.
	{644, 397, 1, {310, 311}, {0, 0}, {677, 372}}, // 4: Tires.
	{456, 405, 7, {306, 307}, {0, 0}, {428, 336}}, // 5: Door.
	{618, 386, 0, {308, 309}, {1, 1}, {623, 383}, 5}, // 6: Trap Door.
	{605, 399, 7, {312, 313}, {1, 1}, {576, 384}}, // 7: Bucket.
};

const RunawayResponse kResponses[] = {
	{5020, 302, 2, 0}, // Look at the path.
	{5020, 303, 1, 0}, // Use an item with the cabin.
	{5020, 304, 3, 0}, // Look at the rocking chair.
	{5020, 305, 4, 0}, // Use the rocking chair.
	{5020, 306, 5, 0}, // Look at the door.
	{5020, 307, 6, 0}, // Open the door.
	{5020, 308, 7, 0}, // Look at the trap door.
	{5020, 310, 9, 0}, // Look at the tires.
	{5020, 311, 10, 0}, // Use the tires.
	{5020, 312, 11, 0}, // Look at the bucket.
};

const RunawayItemAction kItemActions[] = {
	{5020, 1, 2, 303, true},
	{5020, 2, 2, 303, true},
	{5020, 3, 2, 303, true},
	{5020, 4, 2, 303, true},
	{5020, 5, 2, 303, true},
	{5020, 6, 2, 303, true},
	{5020, 7, 2, 303, true},
	{5020, 8, 2, 303, true},
	{5020, 9, 2, 303, true},
	{5020, 10, 2, 303, true},
	{5020, 11, 2, 303, true},
	{5020, 12, 2, 303, true},
	{5020, 13, 2, 303, true},
	{5020, 14, 2, 303, true},
	{5020, 15, 2, 303, true},
	{5020, 16, 2, 303, true},
	{5020, 17, 2, 303, true},
	{5020, 18, 2, 303, true},
	{5020, 19, 2, 303, true},
	{5020, 20, 2, 303, true},
	{5020, 21, 2, 303, true},
	{5020, 22, 2, 303, true},
	{5020, 23, 2, 303, true},
	{5020, 24, 2, 303, true},
	{5020, 25, 2, 303, true},
	{5020, 26, 2, 303, true},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5020 = {
	5020, 1024, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {90, 85, 80}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(953, 406),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

namespace Runaway {

Scene5020::Scene5020(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene5020::loadCustomResources() {
	return _ambientAnimations[0].load(_archive, 7, 947) &&
		_ambientAnimations[1].load(_archive, 4, 452) &&
		_ambientAnimations[2].load(_archive, 5, 667) &&
		_ambientAnimations[3].load(_archive, 6, 47) &&
		_ambientAnimations[4].load(_archive, 8, 662);
}

bool Scene5020::applyCustomBackgroundPatches() {
	return !_desert.bucketTaken || applyPatch(9);
}

void Scene5020::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (_desert.bucketTaken && id == 7) {
			id = 0;
		}
	}
}

float Scene5020::actorHeight() const {
	return _position.y * -0.140845075f + 264.225342f;
}

RunawayRoomScene::ActionResult Scene5020::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 301: // Go to the desert overview.
		leaveDesertRoom(5010);
		break;
	case 309: // Enter the cabin through the trap door.
		if (!_desert.hatchSeen) {
			_desert.hatchSeen = 1;
			say(8);
		}
		leaveDesertRoom(5031);
		break;
	case 313: // Take the bucket.
		_desert.bucketTaken = 1;
		_inventory.add(25);
		say(12);
		return kBackgroundChanged;
	case 400: // Check that everyone is in position for the rescue.
		say(50);
		deferDesertAction(410);
		break;
	case 410: // Rescue Gina and escape in the helicopter.
		startRescueFilm();
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene5020::runCustomEntrySequence() {
	if (_entryScene == 5021 && !_desert.escapedCabin) {
		_desert.escapedCabin = 1;
		say(0);
		leaveDesertRoom(5140);
		return;
	}
	if (_entryScene == 5022) {
		_audio.playMusic(22);
		_audio.setMusicVolume(255);
		walkTo(Common::Point(618, 386), 0);
		deferDesertAction(400);
		return;
	}
	visitDesertRoom(1, 0);
}

void Scene5020::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	drawAmbientAnimations(viewport, _ambientAnimations, ARRAYSIZE(_ambientAnimations));
}

void Scene5020::startRescueFilm() {
	typedef RunawaySequence S;
	_filmCommands = {
		{S::kOpen, 2}, {S::kPlay, 0, 40},
		// Check Mariola's position at the oil well.
		{S::kPlay, 91, 92}, {S::kSpeech, 50, 1}, {S::kPlay, 93, 128}, {S::kWaitSpeech},
		{S::kTalk, 50, 3, 1}, {S::kTalk, 50, 4, 1, 1},
		// Check Lula's position at the machine gun.
		{S::kEffect, 77, 255}, {S::kPlay, 162, 163}, {S::kSpeech, 50, 5},
		{S::kPlay, 164, 253}, {S::kWaitSpeech}, {S::kTalk, 50, 6, 3}, {S::kTalk, 50, 7, 3, 1},
		// Check Carla's helicopter, then give the signal.
		{S::kStill, 254}, {S::kTalk, 50, 8, 5, 1}, {S::kTalk, 50, 9, 5}, {S::kTalk, 50, 10, 5, 1},
		{S::kStill, 40}, {S::kTalk, 50, 11}, {S::kMusic, 23, 255, 1},
		// The explosion draws the guards outside.
		{S::kEffect, 78, 255}, {S::kPlay, 346, 374}, {S::kSpeech, 50, 12},
		{S::kPlay, 375, 430}, {S::kWaitSpeech}, {S::kStill, 430},
		{S::kTalk, 50, 13, 7}, {S::kTalk, 50, 14, 9},
		// Gina escapes while Lula covers the retreat.
		{S::kEffect, 79, 255}, {S::kPlay, 454, 499}, {S::kSpeech, 50, 17, 19},
		{S::kPlay, 500, 699}, {S::kSpeech, 50, 20}, {S::kPlay, 700, 1173},
		{S::kSpeech, 50, 21, 22}, {S::kPlay, 1174, 1379},
		// Carla narrowly avoids the wrecked airplane during takeoff.
		{S::kSpeech, 50, 23, 24}, {S::kSpeech, 50, 31}, {S::kPlay, 1380, 1524}, {S::kWaitSpeech},
		{S::kTalk, 50, 32}, {S::kTalk, 50, 33}, {S::kTalk, 50, 34}, {S::kWaitEffect, 5000}, {S::kEnd}
	};
	if (Common::File::exists("DATAVC00.000")) {
		// GOG removes the CD portrait frames and keeps three separate stills
		// inside the movie. The remaining action retains its original timing.
		for (uint i = 0; i < _filmCommands.size(); ++i) {
			S::Command &command = _filmCommands[i];
			if (command.operation == S::kStill) {
				command.a = command.a == 254 ? 172 : command.a == 40 ? 173 : 259;
			} else if (command.operation == S::kPlay) {
				const uint offset = command.a >= 454 ? 195 : command.a >= 346 ? 172 : command.a >= 162 ? 82 : command.a >= 91 ? 49 : 0;
				command.a -= offset;
				command.b -= offset;
			}
		}
	}
	_sequence.start(_filmCommands.data(), _text, _now, 'E');
}

void Scene5020::finishCustomFilm() {
	if (_entryScene == 5022) {
		_chapterComplete = true;
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
