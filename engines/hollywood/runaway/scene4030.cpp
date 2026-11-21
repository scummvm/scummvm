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
#include "hollywood/runaway/scene4030.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1165, 594, 2, {322, 301}, {0, 0}, {1165, 594}, 6}, // 1: Door.
	{355, 595, 1, {302, 303}, {0, 1}, {430, 546}}, // 2: Pair of books.
	{355, 595, 1, {304, 305}, {1, 0}, {430, 546}}, // 3: Crack.
	{550, 647, 7, {306, 307}, {1, 0}, {492, 600}}, // 4: Money.
	{550, 647, 7, {308, 319}, {1, 0}, {492, 600}}, // 5: Papers.
	{550, 647, 7, {309, 310}, {1, 1}, {492, 600}}, // 6: Device.
	{815, 508, 1, {311, 312}, {0, 0}, {867, 464}}, // 7: Shelves.
	{550, 647, 7, {313, 314}, {0, 0}, {492, 600}}, // 8: Secret safe.
	{104, 536, 7, {315, 316}, {0, 0}, {70, 507}}, // 9: Shelves.
	{369, 499, 7, {317, 318}, {1, 0}, {342, 470}}, // 10: Window.
};

const RunawayResponse kResponses[] = {
	{kMuseumOffice, 305, 4, 0}, // Use the crack.
	{kMuseumOffice, 306, 5, 0}, // Look at the money.
	{kMuseumOffice, 307, 6, 0}, // Take the money.
	{kMuseumOffice, 311, 11, 0}, // Look at the right shelves.
	{kMuseumOffice, 312, 12, 0}, // Take books from the right shelves.
	{kMuseumOffice, 313, 13, 0}, // Look at the safe.
	{kMuseumOffice, 314, 14, 0}, // Use the safe.
	{kMuseumOffice, 315, 15, 0}, // Look at the left shelves.
	{kMuseumOffice, 316, 16, 0}, // Take books from the left shelves.
	{kMuseumOffice, 317, 17, 0}, // Look at the window.
	{kMuseumOffice, 318, 18, 0}, // Open the window.
	{kMuseumOffice, 319, 8, 0}, // Take the papers.
	{kMuseumOffice, 320, 19, 0}, // Use an item with the crack.
	{kMuseumOffice, 322, 22, 0}, // Look at the office door.
};

const RunawayItemAction kItemActions[] = {
	{kMuseumOffice, 17, 3, 321}, // Open or close the safe with the strange key.
	{kMuseumOffice, 0, 3, 320, false}, // Use an item with the crack.
};

const RunawayExit kExits[] = {
	{kMuseumOffice, 301, kMuseumHall, 607, 323, true, true}, // Leave the office.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene4030 = {
	kMuseumOffice, 1200, 670, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {50, 50, 50}, {80, 80, 60}},
	Common::Point(934, 563),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	kExits, ARRAYSIZE(kExits)
};

namespace Runaway {

Scene4030::Scene4030(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

void Scene4030::setCameraX(int x) {
	// The original fixes the camera at (0, 0): the 1200x670 walk area extends
	// beyond the 1024x600 artwork so Brian can walk off screen.
	_camera.x = 0;
}

int Scene4030::sceneViewportYOffset() const {
	return 0;
}

bool Scene4030::loadCustomResources() {
	return loadForeground(0, 4) && loadForeground(1, 5) && loadForeground(2, 6) &&
		loadForeground(3, 7) && loadForeground(4, 12);
}

bool Scene4030::applyCustomBackgroundPatches() {
	if (_books == kTaken && !applyPatch(8)) {
		return false;
	}
	if (!_safeAnimation.load(_archive, _recorder == kTaken ? 15 : 14, 13)) {
		return false;
	}
	_safeAnimation.draw(_background, _safeOpen ? 12 : 0, 0, 0);
	return true;
}

void Scene4030::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (_books != kTaken && (id == 2 || id == 3)) {
			id = 2;
		} else if (_books == kTaken && id == 2) {
			id = 0;
		}
		if (!_safeOpen && (id == 4 || id == 5 || id == 6 || id == 8)) {
			id = museumChapter() && _museum.safeUsed ? 8 : 0;
		} else if (_safeOpen && id == 8) {
			id = 0;
		}
		if (_recorder == kTaken && id == 6) {
			id = 4;
		}
	}
}

void Scene4030::runCustomEntrySequence() {
	if (!_museum.officeVisited) {
		_museum.officeVisited = 1;
		say(0);
	}
}

void Scene4030::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	const byte zone = walkZone();
	if (zone == 3) {
		drawForeground(viewport, 0);
	}
	if (zone == 4 && _books == kUnexamined) {
		drawForeground(viewport, 1);
	}
	if (zone == 5) {
		drawForeground(viewport, 2);
	}
	drawForeground(viewport, 3);
	if (zone == 4 && _books != kUnexamined) {
		drawForeground(viewport, 4);
	}
}

int8 Scene4030::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (museumChapter() && !item) {
		if (id == 1 && use) {
			return _safeOpen ? 0 : 3;
		}
		if (id == 2 && !use) {
			return _books == kUnexamined;
		}
	}
	return approach;
}

RunawayRoomScene::ActionResult Scene4030::dispatchCustomSceneAction(uint16 action) {
	if (action == 301 && _safeOpen) {
		say(21);
		return kHandled;
	}
	switch (action) {
	case 302: // Look at the books.
		say(_books != kUnexamined ? 111 : 1, 0, _books != kUnexamined);
		_books = MAX(_books, kIdentified);
		return kHandled;
	case 303: // Take the books, revealing the crack.
		if (_books == kUnexamined) {
			say(1);
		}
		_books = kTaken;
		_inventory.add(16);
		say(2);
		say(2, 1);
		say(2, 2);
		return kBackgroundChanged;
	case 304: // Look at the crack.
		say(3, museumChapter() ? _museum.safeUsed : _safeOpen);
		return kHandled;
	case 308: // Look at the papers.
		say(7, _moneyIdentified);
		_moneyIdentified = true;
		return kHandled;
	case 309: // Look at the tape recorder.
		say(9, _recorder != kUnexamined);
		_recorder = MAX(_recorder, kIdentified);
		return kHandled;
	case 310: // Take the tape recorder.
		if (_recorder == kUnexamined) {
			say(9);
		}
		_recorder = kTaken;
		_inventory.add(5);
		say(10);
		say(10, 1);
		return kBackgroundChanged;
	case 311: // Look at the right shelves.
		say(11, _museum.shelfBooksSeen ? _random.getRandomBit() : 0);
		_museum.shelfBooksSeen = 1;
		return kHandled;
	case 315: // Look at the left shelves.
		say(_museum.shelfBooksSeen ? 15 : 11);
		_museum.shelfBooksSeen = 1;
		return kHandled;
	case 321: // Open or close the safe with the strange key.
		if (museumChapter()) {
			byte brian[52], safe[52];
			for (uint i = 0; i < ARRAYSIZE(brian); ++i) {
				brian[i] = i <= 26 ? (i ? i - 1 : 0) : 51 - i;
				const uint frame = MIN<uint>(12, i > 27 ? i - 27 : 0);
				safe[i] = _safeOpen ? 12 - frame : frame;
			}
			if (!_safeOpen && !_museum.safeUsed) {
				say(20);
			}
			beginRoomAnimation(321, 16, 26, brian, ARRAYSIZE(brian));
			setRoomAnimationOverlay(_recorder == kTaken ? 15 : 14, 13, safe, ARRAYSIZE(safe));
			_inventory.select(0);
			return kHandled;
		}
		_safeOpen = !_safeOpen;
		_inventory.select(0);
		if (_safeOpen) {
			say(20);
			say(20, 1);
		}
		return kBackgroundChanged;
	default:
		break;
	}
	return kUnhandled;
}

void Scene4030::finishCustomAnimation(uint16 action) {
	if (action == 321) {
		_safeOpen = !_safeOpen;
		if (_safeOpen && !_museum.safeUsed) {
			_museum.safeUsed = 1;
			say(20, 1);
		}
		refreshMuseum();
	}
}

void Scene4030::handleAnimationFrameHook() {
	if (_roomAction == 321) {
		if (_actionStep == 21) {
			_audio.playEffect(36);
		}
		if (_actionStep == 27) {
			_audio.playEffect(_safeOpen ? 39 : 38, 77);
		}
	}
}

float Scene4030::actorHeight() const {
	const int x = _position.x, y = _position.y;
	return x < 459 ? y * 0.369863f + 208.301376f : y * 0.516304f + 110.625f;
}

Common::U32String Scene4030::hotspotName(byte id) const {
	if (id == 2 && _books != kUnexamined) {
		return _strings.text(RunawayStrings::kAnthropologyBooks);
	}
	if (id == 5 && _moneyIdentified) {
		return _strings.text(RunawayStrings::kDocuments);
	}
	if (id == 6 && _recorder != kUnexamined) {
		return _inventory.name(5);
	}
	return RunawayRoomScene::hotspotName(id);
}

} // End of namespace Runaway
} // End of namespace Hollywood
