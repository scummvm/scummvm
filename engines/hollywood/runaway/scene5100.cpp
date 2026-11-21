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

#include "common/scummsys.h"
#include "common/str-enc.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "hollywood/runaway/scene5100.h"

namespace Hollywood {
namespace {

const byte kStudFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 55, 55, 55, 55, 55, 55, 55, 55,
	55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 56, 57, 58, 59,
	60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75,
	76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
	92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
	92, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78,
	77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62,
	61, 60, 59, 58, 57, 56, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
	55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 54, 53, 52, 99, 100,
	101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
	117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132,
	133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148,
	149, 150, 150, 150, 150, 150, 150, 150, 150, 150,
	150, 150, 150, 150, 150, 150, 150, 150, 150, 150
};

// The four throws reuse different ranges of the can animation.
const byte kCanThrows[][2] = {{0, 19}, {21, 40}, {42, 71}, {72, 88}};

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1279, 267, 0, {301, 302}, {0, 1}, {1449, 255}, 4}, // 1: Path.
	{1259, 354, 7, {303, 304}, {1, 0}, {1101, 298}}, // 2: Barn Doors.
	{0, 0, 0, {305, 306}, {1, 1}, {274, 484}}, // 3: Bottle.
	{1279, 267, 4, {1, 302}, {0, 1}, {0, 0}}, // 4: Window.
	{164, 581, 0, {307, 308}, {1, 0}, {164, 570}}, // 5: Motorcycle.
	{0, 0, 7, {309, 310}, {1, 1}, {490, 493}, 3, true}, // 6: Stud.
	{0, 0, 4, {311, 312}, {0, 0}, {1329, 514}}, // 7: Remains Of Rocking Chair.
	{0, 0, 7, {313, 314}, {0, 1}, {29, 503}}, // 8: Piece Of Metal.
	{0, 0, 1, {315, 316}, {1, 0}, {335, 525}}, // 9: Barrel.
	{0, 0, 4, {317, 318}, {0, 0}, {993, 558}}, // 10: Barrel.
	{0, 0, 7, {319, 320}, {0, 0}, {1101, 298}}, // 11: Helicopter.
	{253, 418, 4, {321, 322}, {0, 0}, {245, 491}}, // 12: Can.
	{0, 0, 1, {323, 324}, {1, 0}, {786, 403}}, // 13: Bucket.
	{253, 418, 4, {1, 325}, {0, 0}, {253, 600}}, // 14: Outside.
	{1053, 403, 3, {1, 329}, {0, 1}, {490, 493}}, // 15: Stud.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 16: Beer Can.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 17: Peanut Butter.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 18: Foot Rest.
};

const RunawayResponse kResponses[] = {
	{5100, 301, 2, 0}, // Look at the path.
	{5100, 308, 16, 0}, // Use the motorcycle.
	{5100, 311, 18, 0}, // Look at the ants.
	{5100, 312, 19, 0}, // Use the ants.
	{5100, 316, 22, 0}, // Use the left barrel.
	{5100, 318, 24, 0}, // Use the right barrel.
	{5100, 319, 25, 0}, // Look at the helicopter.
	{5100, 320, 26, 0}, // Use the helicopter.
	{5100, 322, 8, 0}, // Take the beer can.
	{5100, 324, 28, 0}, // Use the bucket.
};

const RunawayItemAction kItemActions[] = {
	{5100, 2, 5, 327, true}, // Pry the footrest off the motorcycle.
	{5100, 9, 6, 16, false}, // Offer the sleeping pill directly to Stud.
	{5100, 9, 12, 328, true}, // Put the sleeping pill in Stud's beer.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5100 = {
	5100, 1880, 700, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {100, 95, 90}, {60, 55, 50}, {90, 85, 80}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(1279, 267),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

namespace Runaway {

Scene5100::Scene5100(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game),
	_hangarSpeech(audio, _random, RunawaySpeech::kNoLipSync) {}

const char *Scene5100::actionCaptionVariant(byte id, bool look, byte item) const {
	if (!item && id == 2) {
		if (look && _desert.barnDoor) {
			return "inside";
		}
		if (!look && _desert.barnDoor) {
			return _desert.barnDoor == 2 ? "open" : "enter";
		}
	}
	return nullptr;
}

bool Scene5100::loadCustomResources() {
	_hangarSpeech.reset();
	return _beerAnimation.load(_archive, 4, 1) &&
		_footrestAnimation.load(_archive, 7, 27) &&
		_canAnimation.load(_archive, 8, 89) &&
		_studAnimation.load(_archive, 9, 151) &&
		_drowsyStudAnimation.load(_archive, 12, 126) &&
		_lulaAnimation.load(_archive, 16, 79);
}

float Scene5100::actorHeight() const {
	return _position.y * 0.668016195f + 87.591095f;
}

int Scene5100::sceneViewportYOffset() const {
	// The walk area extends 100 pixels below the painted hangar exterior.
	return 0;
}

void Scene5100::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
	params.depthY[2] = 303;
	params.depthY[3] = 446;
	params.depthY[4] = 699;
}

RunawayRoomScene::ActionResult Scene5100::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Go to the desert overview.
		leaveDesertRoom(5010);
		break;
	case 303: // Look through the barn doors.
		if (!_desert.barnDoor) {
			Common::Array<byte> frames;
			frames.push_back(1);
			for (uint i = 1; i <= 18; ++i) {
				frames.push_back(i);
			}
			for (uint i = 0; i < 12; ++i) {
				frames.push_back(18);
			}
			for (int i = 17; i >= 1; --i) {
				frames.push_back(i);
			}
			say(3);
			beginRoomAnimation(action, 11, 19, frames.data(), frames.size());
		} else {
			say(3, _desert.barnDoor == 1 ? 3 : 4);
		}
		break;
	case 1303: // Describe the hangar after peeking inside.
		_desert.barnDoor = 1;
		say(3, 1);
		say(3, 2);
		break;
	case 304: // Enter the hangar.
		say(4, _desert.barnDoor);
		if (_desert.barnDoor != 2) {
			deferDesertAction(404);
		}
		break;
	case 404: // Walk behind the barn doors.
		_desert.barnDoor = 1;
		_desert.insideBarn = 1;
		if (!applyCustomBackgroundPatches()) {
			error("Cannot load Runaway hangar interior");
		}
		updateMaps();
		walkHangar(Common::Point(680, 355), 0);
		deferDesertAction(414);
		break;
	case 414: // Approach Stud from inside the hangar.
		walkHangar(Common::Point(253, 418), 4);
		break;
	case 305: // Look at the peanut butter jar.
		say(10, _desert.peanutButter != 0);
		if (!_desert.peanutButter) {
			_desert.peanutButter = 1;
		}
		break;
	case 306: // Take the peanut butter jar.
		if (!_desert.peanutButter) {
			dispatchCustomSceneAction(305);
		}
		say(11);
		deferDesertAction(406);
		break;
	case 406: // Collect the jar and attract Stud's attention.
		_inventory.add(19);
		_desert.peanutButter = 2;
		say(11, 1);
		say(12, 0, false, 1);
		deferDesertAction(416);
		return kBackgroundChanged;
	case 416: // Approach Stud to ask about the jar.
		walkTo(Common::Point(_hotspots[6].x, _hotspots[6].y), 7);
		deferDesertAction(426);
		break;
	case 426: // Learn about the ants from Stud.
		say(11, 2);
		say(12, 1, false, 1);
		say(11, 3);
		say(12, 2, false, 1);
		say(11, 4);
		say(12, 3, false, 1);
		say(12, 4, false, 1);
		say(11, 5);
		_inventory.desert().antTrickKnown = 1;
		break;
	case 307: { // Look at the motorcycle.
		const byte variation = _random.getRandomBit();
		say(15, variation);
		if (variation) {
			_desert.motorcycleSeen = 1;
		}
		break;
	}
	case 309: // Look at Stud.
		say(5, _desert.insideBarn ? _random.getRandomNumber(3) : 4 + _random.getRandomBit());
		break;
	case 310: // Talk to Stud.
		say(6, !_desert.insideBarn);
		break;
	case 313: // Look at the loose footrest.
		if (!_desert.footrestSeen) {
			say(20);
			_desert.footrestSeen = 1;
		} else {
			say(172, 0, true);
		}
		break;
	case 314: // Take the footrest.
		if (!_desert.footrestSeen) {
			dispatchCustomSceneAction(313);
		}
		_inventory.add(21);
		_desert.footrest = 2;
		say(18, 0, true);
		return kBackgroundChanged;
	case 315: // Look at the left barrel.
		say(21, _random.getRandomBit());
		break;
	case 317: // Look at the right barrel.
		say(23, _random.getRandomNumber(2));
		break;
	case 321: // Look at the beer can.
		say(7, _desert.beerSeen != 0);
		_desert.beerSeen = 1;
		break;
	case 323: // Look at the bucket.
		say(27, _random.getRandomBit());
		break;
	case 325: // Leave the hangar through the side entrance.
		walkHangar(Common::Point(680, 355), 0);
		deferDesertAction(425);
		break;
	case 425: // Walk outside the barn doors.
		walkHangar(Common::Point(1348, 332), 5);
		deferDesertAction(435);
		break;
	case 435: // Restore the exterior hangar map.
		_desert.insideBarn = 0;
		return kBackgroundChanged;
	case 327: // Pry the footrest off the motorcycle.
		if (_desert.footrest) {
			say(17, 2);
		} else {
			if (!_desert.motorcycleSeen) {
				say(15, 1);
				_desert.motorcycleSeen = 1;
			}
			say(17);
			byte brian[99] = {}, footrest[99] = {};
			for (uint i = 1; i < 78; ++i) {
				brian[i] = i - 1;
			}
			for (uint i = 78; i < 88; ++i) {
				brian[i] = 87 - i;
			}
			static const byte wobble[] = {1, 2, 3, 2, 1};
			for (uint i = 0; i < ARRAYSIZE(wobble); ++i) {
				footrest[59 + i] = wobble[i];
			}
			for (uint i = 73; i < ARRAYSIZE(footrest); ++i) {
				footrest[i] = i - 72;
			}
			beginRoomAnimation(action, 14, 77, brian, ARRAYSIZE(brian));
			setRoomAnimationOverlay(7, 27, footrest, ARRAYSIZE(footrest));
		}
		break;
	case 1327: // Finish loosening the footrest.
		_desert.footrest = 1;
		say(17, 1);
		return kBackgroundChanged;
	case 328: // Put the sleeping pill in Stud's beer.
		if (!_desert.guard) {
			say(9);
			_desert.guardWeaknessKnown = 1;
		} else if (_desert.guard == 1) {
			say(9, 1);
			deferDesertAction(548);
		}
		break;
	case 548: // Wait for the current can to land before reaching for the beer.
		if (_hangarThrowing) {
			_hangarThrowing = false;
			beginRoomAnimationRange(558, 8, 89, _hangarShotFrame, kCanThrows[_hangarShot][1], false);
		} else {
			deferDesertAction(568);
		}
		break;
	case 1558: // The current throw has finished; Stud is still distracted.
	case 568: // Reach out and put the sleeping pill in the beer.
		beginRoomAnimation(328, 13, 38);
		break;
	case 1328: // Finish spiking Stud's beer.
		_inventory.remove(9);
		say(9, 2);
		say(9, 3);
		deferDesertAction(428);
		break;
	case 428: // Retreat through the hangar after spiking the beer.
		walkHangar(Common::Point(680, 355), 0);
		deferDesertAction(438);
		break;
	case 438: // Return to the path while Stud comes back.
		walkHangar(Common::Point(1279, 267), 3);
		deferDesertAction(448);
		break;
	case 448: // Restore Stud at his post and approach the hiding place.
		_desert.guard = 0;
		_desert.insideBarn = 0;
		walkHangar(Common::Point(1053, 403), 3);
		deferDesertAction(458);
		return kBackgroundChanged;
	case 458: // Hide while waiting for the sleeping pill to work.
		say(9, 4);
		beginRoomAnimationRange(action, 10, 29, 0, 18);
		break;
	case 1458: { // Watch Stud finish the spiked beer.
		const uint count = 249 - 129 + 1;
		byte brian[count];
		memset(brian, 18, sizeof(brian));
		setCameraX(0);
		beginRoomAnimation(468, 9, 151, kStudFrames + 129, count);
		setRoomAnimationOverlay(10, 29, brian, ARRAYSIZE(brian));
		break;
	}
	case 1468: // Stud notices that the beer has made him dizzy.
		say(12, 6, false, 1);
		holdRoomAnimation(478, 10, 29, 18);
		break;
	case 1478: { // Let the sleeping pill take effect.
		byte brian[111];
		memset(brian, 18, sizeof(brian));
		_desert.guard = 2;
		beginRoomAnimationRange(488, 12, 126, 0, 110);
		setRoomAnimationOverlay(10, 29, brian, ARRAYSIZE(brian));
		return kBackgroundChanged;
	}
	case 1488: // Emerge from the hiding place.
		beginRoomAnimationRange(498, 10, 29, 19, 28);
		break;
	case 1498: // Approach the now harmless guard.
		walkTo(Common::Point(656, 600), 7);
		deferDesertAction(508);
		break;
	case 508: // Hear Stud's threats and decide to open the hangar.
		say(9, 5);
		say(12, 5, false, 1);
		say(9, 6);
		say(9, 7);
		deferDesertAction(518);
		break;
	case 518: // Approach the barn doors.
		walkTo(Common::Point(1259, 354), 7);
		deferDesertAction(528);
		break;
	case 528: // Open the hangar and mark the helicopter ready.
		_audio.playEffect(49);
		_desert.barnDoor = 2;
		_inventory.desert().helicopterReady = 1;
		if (_inventory.desert().gunReady && _inventory.desert().bombReady) {
			say(174, 4, true);
			leaveDesertRoom(5062);
		}
		return kBackgroundChanged;
	case 340: // Abandon the distraction and return to cover outside.
		say(13);
		// Fall through.
	case 500: // Retreat when Lula and Stud finish their five throws.
		if (action == 500) {
			say(13, 2);
		}
		_pendingHotspot = 0;
		walkHangar(Common::Point(1279, 267), 3);
		deferDesertAction(510);
		break;
	case 510: // Let Stud resume guarding the hangar after a failed attempt.
		_desert.guard = 0;
		_desert.insideBarn = 0;
		walkHangar(Common::Point(1322, 339), 5);
		return kBackgroundChanged;
	case 400: { // Arrive with Lula and the concealed walkie-talkie.
		_desert.visited[6] = 1;
		_desert.insideBarn = 1;
		_desert.guard = 0;
		_hangarIntroduction = true;
		_hangarLulaPose = 12;
		_position = Common::Point(253, 418);
		_walkPath.reset(_position);
		_facing = 4;
		_camera = Common::Point(0, 0);
		byte brian[13];
		for (uint i = 0; i < ARRAYSIZE(brian); ++i) {
			brian[i] = MIN<uint>(i, 11);
		}
		beginRoomAnimationRange(action, 16, 79, 0, 12);
		setRoomAnimationOverlay(15, 16, brian, ARRAYSIZE(brian));
		return kBackgroundChanged;
	}
	case 1400: // Coach Lula through her first encounter, or repeat the challenge.
		if (!_desert.lulaDistractedGuard) {
			say(94, 0, false, 1);
			say(95, 0, false, 2);
			say(81);
		} else {
			say(95, 5, false, 2);
			say(94, 5, false, 1);
			say(95, 6, false, 2);
			say(94, 6, false, 1);
		}
		holdRoomAnimation(410, 15, 16, 11);
		break;
	case 1410: { // Lula lowers her hand from the earpiece.
		byte brian[13];
		memset(brian, 11, sizeof(brian));
		_hangarLulaPose = 0;
		beginRoomAnimationRange(420, 16, 79, 12, 0);
		setRoomAnimationOverlay(15, 16, brian, ARRAYSIZE(brian));
		break;
	}
	case 1420: // Compliment Stud's shooting and suggest a challenge.
		if (_desert.lulaDistractedGuard) {
			deferDesertAction(450);
		} else {
			say(95, 1, false, 2);
			say(94, 1, false, 1);
			say(81, 1);
			holdRoomAnimation(430, 15, 16, 11);
		}
		break;
	case 1430: // Convince Stud to leave his post for the competition.
		say(95, 2, false, 2);
		say(94, 2, false, 1);
		say(95, 3, false, 2);
		say(94, 3, false, 1);
		holdRoomAnimation(440, 15, 16, 11);
		break;
	case 1440: // Put away the walkie-talkie as Lula leads Stud away.
		deferDesertAction(450);
		break;
	case 450: { // Put away the radio as Lula walks out of the hangar view.
		byte brian[60];
		for (uint i = 0; i < ARRAYSIZE(brian); ++i) {
			brian[i] = MIN<uint>(12 + i, 15);
		}
		_hangarLulaPose = 79;
		beginRoomAnimationRange(action, 16, 79, 19, 78);
		setRoomAnimationOverlay(15, 16, brian, ARRAYSIZE(brian));
		break;
	}
	case 1450: // Stud follows Lula away from his beer.
		_audio.playFootstep(2, _random.getRandomBit(), 255, 100);
		_desert.guard = 1;
		beginRoomAnimation(460, 17, 52, nullptr, 0, false);
		return kBackgroundChanged;
	case 1460: // Lula demonstrates her shooting on the first visit.
		_audio.playFootstep(2, _random.getRandomBit(), 255, 100);
		if (_desert.lulaDistractedGuard) {
			deferDesertAction(490);
		} else {
			say(95, 4, false, 2);
			beginRoomAnimationRange(470, 8, 89, 0, 19, false);
		}
		break;
	case 1470: // Stud responds with a throw of his own.
		say(94, 4, false, 1);
		beginRoomAnimationRange(480, 8, 89, 21, 40, false);
		break;
	case 1480: // Finish the demonstration before returning control to Brian.
		deferDesertAction(490);
		break;
	case 490: // Give Brian five throws to spike the unattended beer.
		_hangarIntroduction = false;
		_desert.lulaDistractedGuard = 1;
		_hangarShot = _hangarShotCount = _hangarShotFrame = 0;
		_hangarShotElapsed = 0;
		_hangarThrowing = true;
		say(14);
		break;
	case 329: { // Step out and back when trying to walk past Stud.
		byte frames[30];
		for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
			frames[i] = i <= 18 ? i : i - 1;
		}
		beginRoomAnimation(action, 10, 29, frames, ARRAYSIZE(frames));
		break;
	}
	case 1329: // Decide against trying to pass Stud.
		say(1, _random.getRandomBit());
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene5100::runCustomEntrySequence() {
	if (_entryScene == 5101) {
		execute(400);
		return;
	}
	if (_entryScene == 5100) {
		walkTo(Common::Point(1053, 403), 3);
	}
	visitDesertRoom(6, 2);
}

Common::U32String Scene5100::hotspotName(byte id) const {
	if (id == 3 && _desert.peanutButter) {
		return RunawayRoomScene::hotspotName(17);
	}
	if (id == 8 && _desert.footrestSeen) {
		return RunawayRoomScene::hotspotName(18);
	}
	if (id == 12 && _desert.beerSeen) {
		return RunawayRoomScene::hotspotName(16);
	}
	return RunawayRoomScene::hotspotName(id);
}

bool Scene5100::applyCustomBackgroundPatches() {
	const uint mask = _desert.insideBarn ? 23 : _desert.guard == 2 ? 26 : 1;
	return loadRunawayMask(_archive, _background.w, _background.h, _mask, _originalMaps, mask) &&
		(_desert.barnDoor != 2 || applyPatch(5)) && (_desert.peanutButter != 2 || applyPatch(6));
}

void Scene5100::applyCustomHotspotState() {
	_hotspots.assign(kHotspots, kHotspots + ARRAYSIZE(kHotspots));
	if (_desert.insideBarn) {
		_hotspots[6].x = 253;
		_hotspots[6].y = 418;
		_hotspots[6].facing = 4;
		_hotspots[14].action[1] = _desert.guard == 1 ? 340 : 325;
	} else if (_desert.guard == 2) {
		static const Common::Point positions[] = {
			{0, 0}, {1279, 267}, {1259, 354}, {271, 598}, {1279, 267},
			{164, 581}, {656, 600}, {1326, 459}, {164, 581}, {261, 595},
			{992, 460}, {1259, 354}, {253, 418}, {736, 444}
		};
		for (uint i = 1; i < ARRAYSIZE(positions); ++i) {
			_hotspots[i].x = positions[i].x;
			_hotspots[i].y = positions[i].y;
		}
	}
	for (uint i = 0; i < 256; ++i) {
		if (_desert.guard != 2 && _maps[i] == 3) {
			_maps[i] = 0;
		}
		byte &id = _maps[256 + i];
		if ((id == 4 && _desert.guard == 2) || (id == 8 && _desert.footrest != 1)) {
			id = 0;
		}
		if (id == 3 && _desert.peanutButter == 2) {
			id = 0;
		}
		if (id == 7 && _desert.peanutButter != 2) {
			id = 0;
		}
		if (id == 6 && _desert.guard == 1) {
			id = 14;
		}
	}
}

void Scene5100::walkHangar(Common::Point target, byte facing) {
	// Scripted entrances cross the floor that is blocked while the guard watches.
	_walkPath.findPath(RunawayWalkMap(_background.w, _background.h, _mask, _originalMaps), target);
	_arrivalFacing = facing;
	_walkElapsed = _lastFootstep = 0;
	if (!_walkPath.active()) {
		arrive();
	}
}

uint Scene5100::studFrame() const {
	if (_roomAction == 468 && drawingRoomAnimation()) {
		return _actionFrames[_actionStep];
	}
	if (_roomAction == 478) {
		return 150;
	}
	return kStudFrames[(_now / 80) % ARRAYSIZE(kStudFrames)];
}

void Scene5100::startHangarTaunt() {
	// These one-line taunts play in the background while Brian remains controllable.
	const SceneSpeechCue cue = _text.stageCue(_hangarShotCount & 1 ? 94 : 95, 7 + _random.getRandomNumber(4));
	if (cue.valid()) {
		_hangarSpeech.start(Common::U32String(_text.largeTextRecord(cue.textRecordId), Common::kDos850), cue.voiceSampleId, false, _now);
	}
}

void Scene5100::handleAnimationFrameHook() {
	static const AnimationFootstep lulaSteps[] = {
		{26, 800}, {35, 700}, {44, 600}, {53, 500}, {62, 400}, {71, 300}
	};
	static const AnimationFootstep studSteps[] = {
		{19, 512}, {30, 500}, {37, 400}, {44, 300}, {51, 200}
	};
	if (_roomAction == 328 && _actionStep == 21) {
		_audio.playEffect(39, 204);
	}
	if (_roomAction == 450) {
		playAnimationFootsteps(_actionFrames[_actionStep], 2, lulaSteps, ARRAYSIZE(lulaSteps));
	} else if (_roomAction == 460) {
		playAnimationFootsteps(_actionStep, 2, studSteps, ARRAYSIZE(studSteps));
	}
	if ((_roomAction == 329 || _roomAction == 458 || _roomAction == 498) &&
			(_actionFrames[_actionStep] == 10 || _actionFrames[_actionStep] == 28)) {
		const byte volume = CLIP<int>((int)(actorHeight() * 255 / 455), 0, 255);
		_audio.playFootstep(2, _random.getRandomBit(), volume, _position.x);
	}
	if (_roomAction == 470 || _roomAction == 480 || _roomAction == 558) {
		const byte frame = _actionFrames[_actionStep];
		if (frame == 9 || frame == 32 || frame == 70 || frame == 84) {
			_audio.playEffect(43 + _random.getRandomNumber(2));
		}
	}
	if (_roomAction == 468) {
		if (_actionStep == 37) {
			_audio.playEffect(47, 230);
		}
		if (_actionStep == 79) {
			_audio.playEffect(42, 217);
		}
		if (_actionStep == 100) {
			_audio.playEffect(45);
		}
	}
	if (_roomAction == 488 && _actionStep == 27) {
		_audio.playEffect(48, 191);
	}
}

void Scene5100::drawRoomComposite(Graphics::ManagedSurface &viewport, bool foreground) {
	if (foreground != (walkZone() == 7)) {
		return;
	}
	const uint frame = studFrame();
	const bool guardAnimation = drawingRoomAnimation() && (_roomAction == 460 || _roomAction == 468 || _roomAction == 488);
	if (!guardAnimation) {
		if (_desert.guard == 2) {
			_drowsyStudAnimation.draw(viewport, speaker() == 1 ? 118 + speechFrame() % 6 : 110, _camera.x, -_camera.y);
		} else if (!_desert.guard) {
			uint pose = _hangarIntroduction ? 92 : frame;
			if (speaker() == 1) {
				pose = 92 + speechFrame() % 6;
			}
			_studAnimation.draw(viewport, pose, _camera.x, -_camera.y);
		}
	}
	if (_desert.guard || _hangarIntroduction || (frame >= 82 && frame <= 92)) {
		_beerAnimation.draw(viewport, 0, _camera.x, -_camera.y);
	}
	if (_desert.footrest < 2 && (!drawingRoomAnimation() || _roomAction != 327)) {
		_footrestAnimation.draw(viewport, _desert.footrest ? 26 : 0, _camera.x, -_camera.y);
	}
	if (_hangarIntroduction && _hangarLulaPose < 79 &&
			(!drawingRoomAnimation() || (_roomAction != 400 && _roomAction != 420))) {
		static const byte mouth[] = {18, 13, 14, 15, 16, 17};
		_lulaAnimation.draw(viewport, speaker() == 2 ? mouth[speechFrame() % ARRAYSIZE(mouth)] : _hangarLulaPose, _camera.x, -_camera.y);
	}
	if (_desert.guard == 1 && !_hangarIntroduction && _hangarThrowing) {
		_canAnimation.draw(viewport, _hangarShotFrame, _camera.x, -_camera.y);
	}
}

void Scene5100::advanceCustomGameplayLoop(uint32 delta) {
	const uint frame = studFrame();
	byte canHotspot = 12;
	if (!_desert.guard && (frame < 82 || frame > 92)) {
		canHotspot = 14;
	}
	for (uint i = 0; i < 256; ++i) {
		if (_originalMaps[256 + i] == 12) {
			_maps[256 + i] = canHotspot;
		}
	}
	if (!_hangarSpeech.subtitle().empty() && _hangarSpeech.finished(_now)) {
		_hangarSpeech.stop();
	}
	if (_desert.guard != 1 || _hangarIntroduction || _roomAction || _hospitalExit.scene || speaking() || _inventoryOpen) {
		return;
	}
	if (!_hangarSpeech.subtitle().empty()) {
		return;
	}
	if (!_hangarThrowing) {
		_hangarShot = (_hangarShot + 1 + _random.getRandomNumber(2)) % ARRAYSIZE(kCanThrows);
		_hangarShotFrame = kCanThrows[_hangarShot][0];
		_hangarThrowing = true;
	}
	_hangarShotElapsed += delta;
	while (_hangarShotElapsed >= 80) {
		_hangarShotElapsed -= 80;
		if (++_hangarShotFrame > kCanThrows[_hangarShot][1]) {
			_hangarThrowing = false;
			if (++_hangarShotCount == 5) {
				execute(500);
				return;
			}
			startHangarTaunt();
			return;
		}
		if (_hangarShotFrame == 9 || _hangarShotFrame == 32 || _hangarShotFrame == 70 || _hangarShotFrame == 84) {
			_audio.playEffect(43 + _random.getRandomNumber(2));
		}
	}
}

const RunawayHotspot *Scene5100::hotspot(byte id) const {
	return id && id < _hotspots.size() ? &_hotspots[id] : nullptr;
}

void Scene5100::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	drawRoomComposite(viewport, false);
}

void Scene5100::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	drawRoomComposite(viewport, true);
}

void Scene5100::stopAmbientSpeech() {
	if (!_hangarSpeech.subtitle().empty()) {
		_hangarSpeech.stop();
	}
}

const Common::U32String &Scene5100::subtitle() const {
	if (!_sequence.active() && !speaking()) {
		return _hangarSpeech.subtitle();
	}
	return RunawayRoomScene::subtitle();
}

} // End of namespace Runaway
} // End of namespace Hollywood
