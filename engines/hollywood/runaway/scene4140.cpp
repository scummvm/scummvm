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
#include "hollywood/runaway/scene4140.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {

const RunawayRoom kRunawayScene4140 = {
	kMuseumKeypad, 1024, 600, nullptr, 0, {},
	Common::Point(382, 529),
	nullptr, 0,
	nullptr, 0,
	nullptr, 0
};

namespace Runaway {

Scene4140::Scene4140(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

void Scene4140::click(Common::Point mouse, RunawayVerb verb, byte item) {
	if (busy()) {
		return;
	}
	const byte id = hotspotAt(mouse);
	if (id == 16) { // The keypad casing is not an exit or a button.
		return;
	}
	_pressedKey = id;
	_keyUntil = _now + 180;
	_keyResult = 0;
	if (!id) { // Leave the keypad close-up.
		leaveMuseumRoom(4063, Common::Point(1153, 297), 7);
	} else if (id == 10) { // Clear the entered digits.
		_audio.playEffect(museumChapter() ? 30 : 10);
		_keyCount = 0;
		_museum.keypadDigits = 0;
	} else if (id == 12) { // Confirm the security code.
		if (_keyCount == 4 && _museum.keypad >= 3 && _museum.keypadDigits == 8137) {
			_keyResult = 2;
			_museum.keypad = 4;
			_audio.playEffect(31);
			say(3, 3);
			deferMuseumAction(414);
		} else {
			_keyResult = 1;
			if (museumChapter()) {
				_audio.playEffect(28);
			}
		}
		_keyUntil = _now + 500;
		_keyCount = 0;
		_museum.keypadDigits = 0;
	} else { // Enter a digit; button 11 is zero.
		const byte digit = id == 11 ? 0 : id;
		_audio.playEffect(museumChapter() ? 18 + digit : id == 11 ? 11 : 11 + id);
		if (_keyCount < 4) {
			++_keyCount;
			_museum.keypadDigits = _museum.keypadDigits * 10 + digit;
		} else {
			say(4);
		}
	}
}

Common::U32String Scene4140::actionCaption(byte id, RunawayVerb verb, byte item) const {
	return _strings.sceneActionCaption(_room, id, 0, false);
}

Common::U32String Scene4140::hotspotName(byte id) const {
	return actionCaption(id, kVerbUse, 0);
}

void Scene4140::update(uint32 now, int scrollDirection, bool inventoryOpen) {
	if (_pressedKey && (int32)(now - _keyUntil) >= 0) {
		_pressedKey = 0;
	}
	RunawayRoomScene::update(now, scrollDirection, inventoryOpen);
}

bool Scene4140::canSave() const {
	return RunawayRoomScene::canSave() && !_pressedKey;
}

bool Scene4140::loadCustomResources() {
	for (uint i = 3; i < ARRAYSIZE(_keyPatches); ++i) {
		if (!_archive.readVariableChunk(i, _keyPatches[i])) {
			return false;
		}
	}
	return true;
}

void Scene4140::runCustomEntrySequence() {
	_museum.keypadDigits = 0;
	_keyCount = 0;
	if (_entryScene == 4141) {
		say(2);
		deferMuseumAction(414);
	} else if (_entryScene == 4142) {
		say(3);
		say(3, 1);
	} else if (_museum.keypad == 1 || _museum.keypad == 2) {
		say(1);
		deferMuseumAction(414);
	} else if (_museum.keypad >= 3) {
		say(3, _museum.keypad == 4 ? 4 : 2);
	}
}

bool Scene4140::drawCustomComposite(Graphics::ManagedSurface &viewport) {
	bool valid = applyRunawayPatch(_keyPatches[_museum.keypad >= 3 ? 17 : _museum.keypad ? 16 : 15], viewport);
	for (uint i = 0; valid && i < _keyCount; ++i) {
		valid = applyRunawayPatch(_keyPatches[21 + i], viewport);
	}
	if (valid && _pressedKey && (int32)(_now - _keyUntil) < 0) {
		valid = applyRunawayPatch(_keyPatches[2 + _pressedKey], viewport);
	}
	if (valid && _keyResult && (int32)(_now - _keyUntil) < 0) {
		valid = applyRunawayPatch(_keyPatches[18 + _keyResult], viewport);
	}
	if (!valid) {
		error("Invalid museum keypad image");
	}
	return true;
}

void Scene4140::finishCustomAnimation(uint16 action) {
	if (action == 414) {
		leaveMuseumRoom(4063, Common::Point(1153, 297), 7);
	}
}

bool Scene4140::shouldUpdateActor() const {
	return false;
}

bool Scene4140::isValidHotspot(byte id) const {
	return id <= 12 || id == 16;
}

} // End of namespace Runaway
} // End of namespace Hollywood
