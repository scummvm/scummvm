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
#include "hollywood/runaway/scene5060.h"

namespace Hollywood {
namespace {

const byte kMariolaTurnFrames[] = {
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
	45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
	61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76,
	77, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
	116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
	132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147,
	148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 77, 78, 79,
	80, 81, 82, 83
};

const byte kMariolaRestFrames[] = {
	95, 96, 97, 98, 99, 100, 101, 161, 162, 163, 164, 165, 166, 167, 168, 169,
	170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185,
	186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200
};

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1479, 599, 0, {301, 302}, {0, 1}, {1379, 587}, 6}, // 1: Path.
	{349, 442, 7, {303, 304}, {0, 1}, {238, 424}, 3}, // 2: Lula.
	{878, 477, 0, {305, 306}, {0, 0}, {866, 445}, 3}, // 3: Mariola.
	{679, 448, 1, {307, 308}, {0, 1}, {710, 431}, 5}, // 4: Inside Of The Bus.
	{376, 445, 1, {309, 310}, {1, 1}, {413, 417}}, // 5: Trunk.
	{878, 477, 0, {311, 312}, {1, 1}, {840, 466}}, // 6: Bottle.
	{383, 446, 5, {313, 314}, {0, 0}, {298, 476}}, // 7: Chair.
	{349, 442, 5, {315, 316}, {0, 0}, {149, 466}}, // 8: Radio And Cassette Player.
	{878, 477, 0, {317, 318}, {0, 0}, {866, 445}}, // 9: Sunglasses.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 10: Tanning Oil.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 11: Lawn Chair.
};

const RunawayResponse kResponses[] = {
	{5060, 301, 1, 0}, // Look at the path.
	{5060, 309, 5, 0}, // Look at the trunk.
	{5060, 314, 11, 0}, // Use the lawn chair.
	{5060, 315, 12, 0}, // Look at the radio and cassette player.
	{5060, 316, 13, 0}, // Use the radio and cassette player.
};

const RunawayItemAction kItemActions[] = {
	{5060, 15, 2, 319, false}, // Show the basketball to Lula.
	{5060, 20, 9, 320, true}, // Exchange Mariola's sunglasses.
};

const byte kSearchTrunkFrames[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
	14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
	14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
	14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5060 = {
	5060, 1380, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {100, 95, 90}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(954, 506),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

namespace Runaway {

Scene5060::Scene5060(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene5060::loadCustomResources() {
	return _ambientAnimations[0].load(_archive, 15, 543) &&
		_ambientAnimations[1].load(_archive, 16, 411) &&
		_ambientAnimations[2].load(_archive, 17, 791) &&
		_ambientAnimations[3].load(_archive, 18, 769) &&
		loadForeground(0, 8) &&
		_mariolaAnimation.load(_archive, 7, 201) &&
		_lulaAnimation.load(_archive, 12, 195);
}

bool Scene5060::applyCustomBackgroundPatches() {
	return _desert.tanningOil != 2 || applyPatch(9);
}

void Scene5060::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (_desert.tanningOil == 2 && id == 6) {
			id = 0;
		}
	}
}

float Scene5060::actorHeight() const {
	return _position.y * 0.107991360f + 207.213821f;
}

RunawayRoomScene::ActionResult Scene5060::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 480: { // Gather everyone after all three rescue preparations are complete.
		static const byte speakers[] = {0, 1, 0, 2, 0, 2, 1, 3};
		for (uint i = 0; i < ARRAYSIZE(speakers); ++i) {
			say(0, i + 2, false, speakers[i]);
		}
		deferDesertAction(481);
		break;
	}
	case 481: { // Carla comes out of the bus.
		Common::Array<byte> frames;
		frames.push_back(0);
		for (uint i = 0; i <= 45; ++i) {
			frames.push_back(i);
		}
		beginRoomAnimation(action, 19, 58, frames.data(), frames.size(), false);
		break;
	}
	case 1481: // Carla reports for duty.
		say(0, 10, false, 3);
		say(0, 11);
		deferDesertAction(491);
		break;
	case 491: // Move to the rescue briefing.
		leaveDesertRoom(5130);
		break;
	case 302: // Go to the desert overview.
		leaveDesertRoom(5010);
		break;
	case 303: // Look at Lula.
		say(2, _random.getRandomBit() * (_desert.lulaHelped ? 2 : 1));
		break;
	case 304: // Talk to Lula.
		if (_desert.guard == 2) {
			say(20, 12);
		} else {
			say(20);
			deferDesertAction(404);
		}
		break;
	case 404: // Let Lula finish dancing before talking.
	case 419: { // Let Lula notice Brian bouncing the basketball.
		Common::Array<byte> frames;
		for (uint i = 120 + (_now / 80) % 75; i < 195; ++i) {
			frames.push_back(i);
		}
		for (uint i = 86; i < 120; ++i) {
			frames.push_back(i);
			if (i == 94) {
				frames.push_back(i);
			}
		}
		beginRoomAnimation(action, 12, 195, frames.data(), frames.size(), action == 419);
		if (action == 419) {
			Common::Array<byte> brian;
			for (uint i = 0; i < frames.size(); ++i) {
				if (i < 29) {
					brian.push_back(i ? i - 1 : 0);
				} else {
					brian.push_back(28 + (i - 29) % 14);
				}
			}
			setRoomAnimationOverlay(13, 62, brian.data(), brian.size());
		}
		break;
	}
	case 1404: // Ask Lula about distracting Stud.
		if (_desert.lulaHelped) {
			say(20, 9);
			say(21, 6, false, 2);
			say(20, 10);
			say(21, 7, false, 2);
			say(20, 11);
			deferDesertAction(469);
			break;
		} else if (!_desert.guardWeaknessKnown) {
			if (!_desert.visited[6]) {
				say(20, 1);
				say(21, 0, false, 2);
				say(20, 2);
				say(21, 1, false, 2);
			} else {
				say(20, 3);
				say(21, 2, false, 2);
				say(20, 4);
				say(21, 3, false, 2);
				say(20, 5);
				_desert.lulaCalled = 1;
			}
		} else {
			say(20, 6);
			say(21, 4, false, 2);
			say(20, 7);
			if (_desert.lulaCalled || _desert.lulaRefused) {
				say(21, 5, false, 2);
			} else {
				say(21, 8, false, 2);
				say(20, 13);
				say(21, 9, false, 2);
			}
			say(20, 8);
			_desert.lulaRefused = 1;
		}
		deferDesertAction(414);
		break;
	case 414: { // Let Lula resume dancing.
		Common::Array<byte> frames;
		for (uint i = 95; i < 120; ++i) {
			frames.push_back(i);
		}
		for (uint i = 12; i < 85; ++i) {
			frames.push_back(i);
		}
		beginRoomAnimation(action, 12, 195, frames.data(), frames.size(), false);
		break;
	}
	case 1414: // Finish the conversation with Lula.
		break;
	case 305: // Look at Mariola.
		say(3, _desert.mariolaHelped != 0);
		break;
	case 306: // Talk to Mariola.
		_facing = 4;
		say(19, _random.getRandomNumber(2));
		break;
	case 307: // Look inside the bus.
		say(4, _desert.visited[9] != 0);
		break;
	case 308: // Enter the bus.
		beginRoomAnimation(action, 5, 22);
		break;
	case 1308: // Finish entering the bus.
		leaveDesertRoom(5170);
		break;
	case 1400: // Finish stepping out of the bus.
		visitDesertRoom(3, 2);
		break;
	case 310: // Search the trunk.
		say(6, _desert.ballTaken ? 2 : 0);
		beginRoomAnimation(action, 10, 28, kSearchTrunkFrames, _desert.ballTaken ? ARRAYSIZE(kSearchTrunkFrames) : 57);
		break;
	case 1310: // Collect the basketball from the trunk.
		if (_desert.ballTaken) {
			say(6, 3);
		} else {
			_inventory.add(15);
			_desert.ballTaken = 1;
			say(6, 1);
			holdRoomAnimation(410, 10, 28, 14);
		}
		break;
	case 1410: // Close the trunk.
		beginRoomAnimation(420, 10, 28, kSearchTrunkFrames + 57, ARRAYSIZE(kSearchTrunkFrames) - 57);
		break;
	case 1420: // Finish searching the trunk.
		break;
	case 311: // Look at the tanning oil.
		if (!_desert.tanningOil) {
			say(7);
			_desert.tanningOil = 1;
		} else {
			say(163, 0, true);
		}
		break;
	case 312: // Ask Mariola for the tanning oil.
		if (!_desert.tanningOil) {
			dispatchCustomSceneAction(311);
		}
		say(8);
		deferDesertAction(412);
		break;
	case 412: // Let Mariola turn toward Brian.
		beginRoomAnimation(action, 7, 201, kMariolaTurnFrames, ARRAYSIZE(kMariolaTurnFrames), false);
		break;
	case 1412: // Listen to Mariola's answer about the oil.
		say(9, _desert.mariolaHelped != 0, false, 1);
		if (_desert.mariolaHelped) {
			say(8, 1);
		}
		deferDesertAction(422);
		break;
	case 422: // Take the oil if Mariola agrees.
		beginRoomAnimation(432, 7, 201, kMariolaRestFrames, ARRAYSIZE(kMariolaRestFrames), false);
		if (_desert.mariolaHelped) {
			_desert.tanningOil = 2;
			_inventory.add(12);
			say(8, 2);
		}
		return kBackgroundChanged;
	case 1432: // Finish asking for the tanning oil.
		break;
	case 313: // Look at the lawn chair.
		say(10, _desert.busPosterSeen != 0);
		_desert.busPosterSeen = 1;
		break;
	case 317: // Look at Mariola's sunglasses.
		say(14, _desert.mariolaHelped != 0);
		break;
	case 318: // Take Mariola's sunglasses.
		say(15, !_inventory.contains(20) ? 0 : _desert.mariolaHelped ? 2 : 1);
		break;
	case 319: // Give the repaired basketball to Lula.
		if (!_desert.guardWeaknessKnown) {
			say(17, 7);
		} else if (_inventory.desert().ball < 2) {
			say(17, 8);
		} else {
			_inventory.select(0);
			walkTo(Common::Point(490, 457), 7);
			deferDesertAction(409);
		}
		break;
	case 409: // Call Lula over to see the basketball.
		say(17);
		deferDesertAction(419);
		break;
	case 1419: // Keep bouncing the ball while Lula asks for it.
		say(18, 5, false, 2);
		loopRoomAnimationDuringSpeech(429, 13, 62, 28, 41);
		break;
	case 1429: { // Throw the basketball to Lula.
		_audio.stopEffect();
		byte lula[20];
		for (uint i = 0; i < ARRAYSIZE(lula); ++i) {
			lula[i] = i == ARRAYSIZE(lula) - 1 ? 5 : MIN<uint>(i, 4);
		}
		beginRoomAnimationRange(439, 13, 62, 42, 61);
		setRoomAnimationOverlay(21, 102, lula, ARRAYSIZE(lula));
		break;
	}
	case 1439: // Watch Lula demonstrate her basketball skills.
		beginRoomAnimationRange(449, 21, 102, 6, 96, false);
		break;
	case 1449: // Approach Lula after she catches the ball.
		_inventory.remove(15);
		walkTo(Common::Point(349, 442), 7);
		deferDesertAction(459);
		break;
	case 459: // Reveal Lula's identity and arrange the distraction.
		say(18, 6, false, 2);
		for (uint i = 1; i <= 5; ++i) {
			say(17, i);
			say(18, i - 1, false, 2);
		}
		say(17, 6);
		deferDesertAction(479);
		break;
	case 479: // Finish enlisting Lula's help.
		_desert.lulaHelped = 1;
		// Fall through.
	case 469: // Take Lula to the hangar, including repeat attempts.
		leaveDesertRoom(5101);
		break;
	case 320: // Give the darkened sunglasses to Mariola.
		if (!_inventory.desert().glasses) {
			say(16);
		} else if (_inventory.desert().glasses == 1) {
			say(16, 1);
			beginRoomAnimation(action, 11, 21);
		} else {
			say(15, 2);
		}
		break;
	case 1320: // Finish replacing Mariola's sunglasses.
		_desert.mariolaHelped = 1;
		_inventory.desert().glasses = 2;
		say(16, 2);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene5060::runCustomEntrySequence() {
	if (_entryScene == 5062) {
		_position = Common::Point(870, 525);
		_walkPath.reset(_position);
		_facing = 7;
		setCameraX(132);
		execute(480);
		return;
	}
	if (_entryScene == 5061) {
		_position = Common::Point(679, 448);
		_walkPath.reset(_position);
		_facing = 3;
		updateCamera();
		beginRoomAnimationRange(400, 6, 26, 9, 25);
		return;
	}
	visitDesertRoom(3, 2);
}

Common::U32String Scene5060::hotspotName(byte id) const {
	if (id == 6 && _desert.tanningOil) {
		return RunawayRoomScene::hotspotName(10);
	}
	if (id == 7 && _desert.busPosterSeen) {
		return RunawayRoomScene::hotspotName(11);
	}
	return RunawayRoomScene::hotspotName(id);
}

void Scene5060::resumeAmbientAudio(bool entering) {
	if (!_audio.musicPlaying()) {
		_audio.playMusic(21);
	}
}

bool Scene5060::lulaDancing() const {
	if (_entryScene == 5062 || _roomAction == 414 || _roomAction == 429 ||
			_roomAction == 459 || _roomAction == 469 || _roomAction == 479) {
		return false;
	}
	return !drawingRoomAnimation() || (_roomAction != 404 && _roomAction != 419 &&
		_roomAction != 439 && _roomAction != 449);
}

void Scene5060::playLulaFootstep(uint frame) {
	if (frame == 124 || frame == 128 || frame == 159 || frame == 181 || frame == 191) {
		_audio.playFootstep(2, _random.getRandomBit(), 153, 160);
	}
}

void Scene5060::advanceCustomGameplayLoop(uint32 delta) {
	if (lulaDancing()) {
		for (uint32 tick = (_now - delta) / 80 + 1; tick <= _now / 80; ++tick) {
			playLulaFootstep(120 + tick % 75);
		}
	}
}

void Scene5060::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	drawAmbientAnimations(viewport, _ambientAnimations, ARRAYSIZE(_ambientAnimations));
	static const byte mouth[] = {5, 0, 1, 2, 3, 4, 11, 6, 7, 8, 9, 10};
	if (_entryScene == 5062) {
		if (_roomAction == 491) {
			_actionAnimation.draw(viewport, 45 + (speaker() == 3 ? speechFrame() : 0), _camera.x, -_camera.y);
		}
		_mariolaAnimation.draw(viewport, 83 + (speaker() == 1 ? speechFrame() : 0), _camera.x, -_camera.y);
		_lulaAnimation.draw(viewport, mouth[speaker() == 2 ? speechFrame() : 0], _camera.x, -_camera.y);
		return;
	}
	if (!drawingRoomAnimation() || (_roomAction != 412 && _roomAction != 432)) {
		uint frame = (_now / 80) % 13;
		if (_roomAction == 422 || _roomAction == 432) {
			frame = 83 + (speaker() == 1 ? speechFrame() : 0);
		}
		_mariolaAnimation.draw(viewport, frame, _camera.x, -_camera.y);
	}
	if (_roomAction == 459 || _roomAction == 479) {
		_actionAnimation.draw(viewport, 96 + (speaker() == 2 ? speechFrame() % 6 : 0), _camera.x, -_camera.y);
	} else if (!drawingRoomAnimation() || (_roomAction != 404 && _roomAction != 414 &&
			_roomAction != 419 && _roomAction != 439 && _roomAction != 449)) {
		const uint frame = lulaDancing() ? 120 + (_now / 80) % 75 : mouth[speaker() == 2 ? speechFrame() : 0];
		_lulaAnimation.draw(viewport, frame, _camera.x, -_camera.y);
	}
}

void Scene5060::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_position.x < 670) {
		drawForeground(viewport, 0);
	}
}

void Scene5060::handleAnimationFrameHook() {
	const byte frame = _actionFrames[_actionStep];
	if (_roomAction == 308 && (frame == 8 || frame == 15 || frame == 21)) {
		_audio.playFootstep(4, _random.getRandomBit(), 179, _position.x);
	} else if (_roomAction == 400 && (frame == 14 || frame == 21 || frame == 25)) {
		_audio.playFootstep(frame == 14 ? 4 : 2, _random.getRandomBit(), 179, 679);
	} else if (_roomAction == 404 || _roomAction == 419) {
		playLulaFootstep(frame);
	}
	if (_roomAction == 481) {
		if (_actionStep == 3 || _actionStep == 12 || _actionStep == 17 || _actionStep == 25 || _actionStep == 29) {
			_audio.playFootstep(4, _random.getRandomBit(), 153);
		}
		if (_actionStep == 39) {
			_audio.playFootstep(4, _random.getRandomBit(), 230);
		}
	}
	if (_roomAction == 419 && _actionStep == 28) {
		_audio.playEffect(56, 217, 1, true);
	}
	if (_roomAction == 449 && (_actionStep == 6 || _actionStep == 11 || _actionStep == 51)) {
		_audio.playFootstep(1, _random.getRandomBit(), 153);
	}
	if (_roomAction == 449 && _actionStep == 57) {
		_audio.playFootstep(1, _random.getRandomBit());
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
