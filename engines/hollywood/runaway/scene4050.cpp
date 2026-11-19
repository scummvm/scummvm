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
#include "hollywood/runaway/scene4050.h"

namespace Hollywood {
namespace {

const byte kRecordSusan[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 29, 28, 27, 26, 25, 26,
	27, 28, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
};

const byte kGrindCoffee[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 26, 27, 28, 29, 30, 31, 32, 33, 34, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 26, 27, 28, 29, 30, 31, 32, 33, 34, 26, 27, 28, 29, 30, 31, 32, 33,
	34, 26, 27, 28, 29, 30, 31, 32, 33, 34, 26, 27, 28, 29, 30, 31, 32, 33, 34, 26,
	27, 28, 29, 30, 31, 32, 33, 34, 26, 27, 28, 29, 30, 31, 32, 33, 34, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13,
	12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
};

const RunawayHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Door", 227, 503, 7, {301, 339}, {0, 1}},
	{"Shelves", 311, 493, 7, {302, 303}, {0, 0}},
	{"Susan", 382, 529, 1, {304, 305}, {0, 1}},
	{"Electric lathe", 395, 529, 1, {306, 307}, {0, 0}},
	{"Microscope", 460, 543, 1, {308, 309}, {0, 0}},
	{"Water dispenser", 637, 570, 1, {310, 311}, {0, 0}},
	{"Wastepaper basket", 703, 575, 1, {312, 313}, {1, 0}},
	{"Tank", 844, 505, 1, {314, 315}, {0, 0}},
	{"Machine", 884, 580, 1, {316, 317}, {0, 0}},
	{"Laser", 992, 556, 1, {318, 319}, {1, 0}},
	{"Shelves", 1327, 521, 1, {320, 321}, {0, 0}},
	{"Briefcase", 1277, 514, 7, {322, 323}, {1, 1}},
	{"Skeleton", 1211, 510, 7, {324, 325}, {0, 0}},
	{"Strange brush", 824, 531, 7, {337, 338}, {1, 1}},
};

const RunawayResponse kResponses[] = {
	{kMuseumLaboratory, 301, 1, 0}, // Look at the laboratory door.
	{kMuseumLaboratory, 306, 6, 0}, // Look at the electric lathe.
	{kMuseumLaboratory, 307, 7, 0}, // Use the electric lathe.
	{kMuseumLaboratory, 308, 8, 0}, // Look at the microscope.
	{kMuseumLaboratory, 309, 9, 0}, // Use the microscope.
	{kMuseumLaboratory, 310, 10, 0}, // Look at the water dispenser.
	{kMuseumLaboratory, 311, 11, 0}, // Use the water dispenser.
	{kMuseumLaboratory, 314, 12, 0}, // Look at the tank.
	{kMuseumLaboratory, 316, 13, 0}, // Look at the machine.
	{kMuseumLaboratory, 319, 15, 0}, // Use the laser.
	{kMuseumLaboratory, 320, 16, 0}, // Look at the right shelves.
	{kMuseumLaboratory, 321, 17, 0}, // Take objects from the right shelves.
	{kMuseumLaboratory, 322, 18, 0}, // Look inside the briefcase.
	{kMuseumLaboratory, 324, 20, 0}, // Look at the skeleton.
	{kMuseumLaboratory, 325, 21, 0}, // Use the skeleton.
	{kMuseumLaboratory, 326, 22, 2}, // Leave the crucifix on the restoration shelves.
	{kMuseumLaboratory, 332, 27, 0}, // Use an item with the electric lathe.
	{kMuseumLaboratory, 334, 29, 0}, // Use an item with the microscope.
	{kMuseumLaboratory, 337, 31, 0}, // Look at the talcum powder brush.
};

const RunawayItemAction kItemActions[] = {
	{kMuseumLaboratory, 7, 2, 326}, // Leave the crucifix on the restoration shelves.
	{kMuseumLaboratory, 0, 2, 15}, // Use an item with shelves.
	{kMuseumLaboratory, 5, 3, 328}, // Record Susan's voice.
	{kMuseumLaboratory, 7, 3, 327}, // Give the crucifix to Susan.
	{kMuseumLaboratory, 8, 3, 329, false}, // Show the Mayan object to Susan.
	{kMuseumLaboratory, 11, 3, 330, false}, // Show the Mayan mask to Susan.
	{kMuseumLaboratory, 12, 3, 330, false}, // Show the ruby to Susan.
	{kMuseumLaboratory, 15, 3, 331}, // Give the coffee to Susan.
	{kMuseumLaboratory, 0, 3, 30}, // Use an item with susan.
	{kMuseumLaboratory, 14, 4, 333}, // Grind the coffee beans with the lathe.
	{kMuseumLaboratory, 0, 4, 332}, // Use an item with the electric lathe.
	{kMuseumLaboratory, 0, 5, 334}, // Use an item with the microscope.
	{kMuseumLaboratory, 0, 7, 19}, // Use an item with wastepaper basket.
	{kMuseumLaboratory, 12, 10, 336}, // Fit the ruby to the laser.
	{kMuseumLaboratory, 0, 10, 335}, // Use an item with the laser.
	{kMuseumLaboratory, 0, 11, 15}, // Use an item with shelves.
};

const RunawayExit kExits[] = {
	{kMuseumLaboratory, 339, kMuseumHall, 873, 311, true, true}, // Leave the laboratory.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene4050 = {
	kMuseumLaboratory, 1552, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {60, 60, 70}, {70, 70, 80}, {60, 60, 70}},
	Common::Point(382, 529),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	kExits, ARRAYSIZE(kExits)
};

namespace Runaway {

Scene4050::Scene4050(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene4050::loadCustomResources() {
	return _susanAnimation.load(_archive, 8, 75) && _laserAnimation.load(_archive, 22, 1) &&
		loadForeground(0, 4) && loadForeground(1, 5) && loadForeground(2, 6) && loadForeground(3, 7);
}

bool Scene4050::applyCustomBackgroundPatches() {
	if (_talcTaken && !applyPatch(11)) {
		return false;
	}
	if (!museumChapter()) {
		return true;
	}
	if (_museum.restoration && !applyPatch(_museum.restoration == 1 ? 9 : 10)) {
		return false;
	}
	if (!_susanAnimation.load(_archive, _museum.susan == 1 ? 15 : _museum.susan == 3 ? 16 : 8,
			_museum.susan == 1 ? 170 : _museum.susan == 3 ? 2 : 75)) {
		return false;
	}
	_susanFrame = _museum.susan == 1 ? 79 : _museum.susan == 3 ? 1 : 0;
	return true;
}

void Scene4050::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (_talcTaken && id == 14) {
			id = 0;
		}
		if (museumChapter() && _museum.susan == 1) {
			if (id == 15 || id == 16 || id == 17) {
				id = 3;
			} else if (id == 3 || id == 18 || id == 19) {
				id = 0;
			}
		} else {
			if (id == 15 || (id == 4 && (!museumChapter() || !_museum.susan)) || (id == 19 && (!museumChapter() || !_museum.susan))) {
				id = 0;
			} else if (id == 16) {
				id = 13;
			} else if (id == 17) {
				id = 10;
			} else if (id == 18 || id == 19) {
				id = 3;
			} else if (id == 3 && museumChapter() && _museum.susan >= 2) {
				id = 0;
			}
		}
	}
}

void Scene4050::prepareCustomGameplayLoop() {
	_nextLaser = _now + 25000 + _random.getRandomNumber(5000);
}

void Scene4050::runCustomEntrySequence() {
	if (_museum.susan == 2) {
		_museum.susan = 3;
		_inventory.museum().restoredCrucifix = 1;
		say(32);
		say(32, 1);
		deferMuseumAction(405);
		refreshMuseum();
	} else if (!_museum.susanMet) {
		_museum.susanMet = 1;
		for (uint i = 0; i < 3; ++i) {
			say(80, i);
			say(90, i, false, 1);
		}
	}
}

void Scene4050::resumeAmbientAudio(bool entering) {
	if (entering || !_audio.musicPlaying()) {
		_audio.playMusic(13, false);
	}
}

void Scene4050::advanceCustomGameplayLoop(uint32 delta) {
	if (speaker()) {
		_susanFrame = (_museum.susan == 1 ? 52 : 45) + speechFrame();
	} else if (_museum.susan == 3) {
		_susanFrame = 1;
	} else if (_museum.susan == 1) {
		_susanFrame = 79;
	} else {
		_susanFrame = (_now / 80) % 26;
	}
	if (!_museum.susan && !busy() && !_inventoryOpen && _position.x < 530 && (int32)(_now - _nextLaser) >= 0) {
		beginRoomAnimation(450, 23, 95, nullptr, 0, false);
		_nextLaser = _now + 25000 + _random.getRandomNumber(5000);
	}
}

void Scene4050::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	const bool movingSusan = museumChapter() && drawingRoomAnimation() &&
		(_roomAction == 450 || _roomAction == 451 || _roomAction == 452 || _roomAction == 331 || _roomAction == 454);
	if (!movingSusan) {
		_susanAnimation.draw(viewport, museumChapter() ? _susanFrame : 0, _camera.x, -_camera.y);
	}
	_laserAnimation.draw(viewport, 0, _camera.x, -_camera.y);
}

void Scene4050::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_position.x > 1200) {
		drawForeground(viewport, 0);
	}
	if (_position.x > 789 && _position.x < 846 && _position.y < 541) {
		drawForeground(viewport, 1);
	}
	if (_position.x < 430) {
		drawForeground(viewport, 2);
	}
	if (_position.y < 501) {
		drawForeground(viewport, 3);
	}
}

RunawayRoomScene::ActionResult Scene4050::dispatchCustomSceneAction(uint16 action) {
	if (museumChapter()) {
		switch (action) {
		case 302: // Look at the restoration shelves.
			say(2, !_museum.restoration ? 0 : _museum.restoration == 2 ? 3 : _museum.susan ? 2 : 1);
			_museum.shelvesSeen = 1;
			return kHandled;
		case 303: // Take objects from the restoration shelves.
			say(3, _museum.restoration == 1);
			return kHandled;
		case 304: // Look at Susan.
			say(4, _museum.susan == 3 ? 2 : _museum.susan == 1 ? 1 : _museum.restoration && _museum.leftLabOnce ? 3 : 0);
			return kHandled;
		case 305: // Talk to Susan.
			if (_museum.susan == 3) {
				say(5);
			} else if (_museum.susan == 1) {
				say(5, 1 + _random.getRandomNumber(3));
				say(5, 5 + _random.getRandomNumber(2), false, 1);
			} else {
				startSusanConversation();
			}
			return kHandled;
		case 312: // Look in the wastepaper basket.
			say(19, 1, true);
			return kHandled;
		case 313: // Use the wastepaper basket.
			say(4, 0, true);
			return kHandled;
		case 315: // Use the tank.
			say(7, 0, true);
			return kHandled;
		case 317: // Use the machine.
		case 335: // Use an item with the laser.
			say(14, _random.getRandomNumber(2), true);
			return kHandled;
		case 318: // Look at the laser.
			say(14, _museum.laserUpgraded ? 2 : _museum.laserSeen ? 1 : 0);
			return kHandled;
		case 322: // Look inside the briefcase.
			say(18, _museum.varnishSeen);
			_museum.varnishSeen = 1;
			return kHandled;
		case 326: // Leave the crucifix on the restoration shelves.
			if (_inventory.museum().restoredCrucifix) {
				say(22, 2);
			} else {
				if (!_museum.shelvesSeen) {
					say(2);
				}
				_museum.shelvesSeen = 1;
				say(22);
				beginRoomAnimation(326, 17, 43);
			}
			return kHandled;
		case 327: // Give the crucifix to Susan.
			if (_museum.susan == 3) {
				say(5);
			} else {
				say(_inventory.museum().restoredCrucifix ? 22 : 23, _inventory.museum().restoredCrucifix ? 2 : 0);
			}
			return kHandled;
		case 328: // Record Susan's voice.
			if (!_museum.voiceLockSeen) {
				say(1, 0, true);
			} else if (_museum.susan == 3) {
				say(5);
			} else if (_inventory.contains(6) || _inventory.museum().batteryInLadle) {
				say(24, 3);
			} else if (_inventory.museum().voiceRecorded) {
				say(24, 2);
			} else {
				beginRoomAnimation(328, 21, 56, kRecordSusan, 27);
			}
			return kHandled;
		case 329: // Show the Mayan object to Susan.
		case 330: // Show the mask or ruby to Susan.
			say(_museum.susan == 3 ? 5 : 25);
			return kHandled;
		case 331: // Give the coffee to Susan.
			if (_museum.susan != 1) {
				say(1, 0, true);
			} else {
				say(26);
				byte brian[25], susan[25];
				for (uint i = 0; i < ARRAYSIZE(brian); ++i) {
					brian[i] = i ? i - 1 : 0;
					susan[i] = i < 11 ? 102 + (i ? i - 1 : 0) : i < 20 ? 112 : 93 + i;
				}
				beginRoomAnimation(331, 20, 24, brian, ARRAYSIZE(brian));
				setRoomAnimationOverlay(15, 170, susan, ARRAYSIZE(susan));
			}
			return kHandled;
		case 333: // Grind the coffee beans with the lathe.
			if (_inventory.museum().coffee == 2) {
				say(1, 2, true);
			} else if (_inventory.museum().coffee || !_museum.needsGroundCoffee) {
				say(1, 0, true);
			} else {
				say(28);
				beginRoomAnimation(333, 18, 35, kGrindCoffee, ARRAYSIZE(kGrindCoffee));
			}
			return kHandled;
		case 336: // Fit the ruby to the laser.
			if (!_museum.laserSeen) {
				say(1, 0, true);
			} else {
				say(30);
				beginRoomAnimation(336, 19, 13);
			}
			return kHandled;
		case 337: // Look at the talcum powder brush.
			say(31, _museum.talcSeen);
			_museum.talcSeen = 1;
			return kHandled;
		case 339: // Leave the laboratory.
			_museum.leftLabOnce = 1;
			leaveMuseumRoom(4061, Common::Point(873, 311), 3, true);
			return kHandled;
		default:
			break;
		}
	}

	switch (action) {
	case 323: // Take the varnish from the briefcase.
		if (_varnishTaken) {
			say(19, 2);
			return kHandled;
		}
		if (museumChapter() && !_museum.varnishSeen) {
			say(18);
		}
		_museum.varnishSeen = 1;
		_varnishTaken = true;
		_inventory.add(9);
		say(19);
		say(19, 1);
		return kHandled;
	case 338: // Take the talcum powder.
		if (!museumChapter() || !_museum.talcSeen) {
			say(31);
		}
		_museum.talcSeen = 1;
		_talcTaken = true;
		_inventory.add(10);
		if (museumChapter()) {
			say(18, _random.getRandomNumber(2), true);
		}
		return kBackgroundChanged;
	default:
		break;
	}
	return kUnhandled;
}

void Scene4050::finishCustomAnimation(uint16 action) {
	switch (action) {
	case 326: // Leave the crucifix and collect the Mayan object.
		_inventory.remove(7);
		_inventory.add(8);
		_museum.restoration = 1;
		say(22, 1);
		break;
	case 328: // Ask Susan to say her name.
		say(24);
		holdRoomAnimation(428, 21, 56, 29);
		break;
	case 428: // Press the recorder button.
		beginRoomAnimation(438, 21, 56, kRecordSusan + 27, 8);
		break;
	case 438: // Record Susan's reply.
		say(24, 1, false, 1);
		holdRoomAnimation(448, 21, 56, 29);
		break;
	case 448: // Put the recorder away.
		beginRoomAnimation(458, 21, 56, kRecordSusan + 35, 29);
		break;
	case 458: // Remember that Susan's voice was recorded.
		_inventory.museum().voiceRecorded = 1;
		break;
	case 331: // Susan drinks the coffee.
		_inventory.remove(15);
		_audio.playEffect(48, 204);
		beginRoomAnimationRange(454, 15, 170, 117, 143, false);
		return;
	case 454: // Finish the exchange with Susan.
		say(26, 1);
		say(26, 2, false, 1);
		deferMuseumAction(453);
		break;
	case 333: // Collect the ground coffee.
		_inventory.museum().coffee = 1;
		say(28, 1);
		break;
	case 336: // Show the ruby installation film.
		startMuseumFilm(kRubyFilm);
		return;
	case 405: // Collect the restored crucifix.
		_inventory.add(7);
		say(32, 2);
		say(32, 3);
		break;
	case 456: // Finish installing the ruby and approach Susan.
		_inventory.remove(12);
		say(30, 1);
		walkTo(Common::Point(464, 544), 3, true);
		beginRoomAnimation(452, 23, 95, nullptr, 0, false);
		return;
	case 450: // Show the drilling film.
		startMuseumFilm(kDrillingFilm);
		return;
	case 451: // Resume the laboratory after drilling.
		_nextLaser = _now + 25000 + _random.getRandomNumber(5000);
		break;
	case 452: // Show the upgraded laser film.
		startMuseumFilm(kLaserFilm);
		return;
	case 453: // Show the mobsters film.
		startMuseumFilm(kMobstersFilm);
		return;
	default:
		return;
	}
	refreshMuseum();
}

void Scene4050::handleAnimationFrameHook() {
	if ((_roomAction == 328 || _roomAction == 428) && (_actionStep == 4 || _actionStep == 2)) {
		_audio.playEffect(37, 204);
	}
}

void Scene4050::finishCustomFilm() {
	switch (_museumFilm) {
	case kDrillingFilm:
		_museum.laserSeen = 1;
		beginRoomAnimation(451, 24, 101, nullptr, 0, false);
		break;
	case kRubyFilm:
		beginRoomAnimationRange(456, 19, 13, 4, 0);
		break;
	case kLaserFilm:
		_museum.susan = 1;
		_museum.laserUpgraded = 1;
		say(30, 2);
		say(33, 1, false, 1);
		refreshMuseum();
		break;
	case kMobstersFilm:
		_museum.susan = 2;
		_museum.restoration = 2;
		leaveMuseumRoom(4061, Common::Point(873, 311), 3, true);
		break;
	default:
		break;
	}
}

float Scene4050::actorHeight() const {
	const int x = _position.x, y = _position.y;
	const byte zone = walkZone();
	if (zone == 4) {
		return y * 1.3125f - 341.0625f;
	}
	if (zone == 3) {
		return x * 0.089147285f + 291.697662f + y - 578;
	}
	if (zone == 5) {
		return 291.697662f + 992 * 0.089147285f - (583 - y) * 0.5882353f - 1;
	}
	if (x < 396) {
		return 327;
	}
	if (x < 913) {
		return x * 0.089147285f + 291.697662f;
	}
	if (x < 1174) {
		return x * 0.09578544f + 285.643677f;
	}
	return MIN(y, 587) * 0.8181818f - 82.272728f;
}

Common::Point Scene4050::customizeSceneActionPosition(byte id, byte item, Common::Point position) const {
	if (museumChapter() && id == 3 && _museum.susan == 1) {
		return Common::Point(912, 584);
	}
	return position;
}

byte Scene4050::customizeSceneActionFacing(byte id, byte facing) const {
	return museumChapter() && id == 3 && _museum.susan == 1 ? 1 : facing;
}

void Scene4050::startSusanConversation() {
	startConversation(1);
	static const DialogueChoice susan[][8] = {
		{{1,0,3,3,3,1,255}, {1,0,1,4,4,1,255}, {1,1,1,5,5,1,255}, {1,0,3,6,6,1,255}, {1,0,0,7,7,1,0}},
		{{1,0,3,8,8,2,0}, {1,0,3,9,9,2,0}, {1,0,3,10,10,2,0}},
		{{1,1,3,11,11,2,0}, {1,1,3,12,12,2,0}, {1,1,3,13,13,2,0}},
		{{1,0,2,14,14,1,255}}
	};
	memcpy(_dialogue[0], susan[0], sizeof(susan[0]));
	memcpy(_dialogue[10], susan[1], sizeof(susan[1]));
	memcpy(_dialogue[11], susan[2], sizeof(susan[2]));
	memcpy(_dialogue[20], susan[3], sizeof(susan[3]));
	say(80, 15);
	say(90, 15, false, 1);
}

} // End of namespace Runaway
} // End of namespace Hollywood
