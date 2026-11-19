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
#include "hollywood/runaway/scene_data.h"

namespace Hollywood {

const RunawayRoom kRunawayScene4140 = {
	kMuseumKeypad, 1024, 600, nullptr, 0, {},
	Common::Point(382, 529),
	nullptr, 0,
	nullptr, 0,
	nullptr, 0
};

void RunawayRoomScene::start4140() {
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

void RunawayRoomScene::clickScene4140(byte id) {
	_pressedKey = id;
	_keyUntil = _now + 180;
	_keyResult = 0;
	if (!id) {
		leaveMuseumRoom(4063, Common::Point(1153, 297), 7);
	} else if (id == 10) {
		_audio.playEffect(museumChapter() ? 30 : 10);
		_keyCount = 0;
		_museum.keypadDigits = 0;
	} else if (id == 12) {
		if (_keyCount == 4 && _museum.keypad >= 3 && _museum.keypadDigits == 8137) {
			_keyResult = 2;
			_museum.keypad = 4;
			_audio.playEffect(31);
			say(3, 3);
			deferMuseumAction(414);
		} else {
			_keyResult = 1;
			if (museumChapter())
				_audio.playEffect(28);
		}
		_keyUntil = _now + 500;
		_keyCount = 0;
		_museum.keypadDigits = 0;
	} else {
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

void RunawayRoomScene::drawScene4140(Graphics::ManagedSurface &viewport) {
	bool valid = applyRunawayPatch(_keyPatches[_museum.keypad >= 3 ? 17 : _museum.keypad ? 16 : 15], viewport);
	for (uint i = 0; valid && i < _keyCount; ++i)
		valid = applyRunawayPatch(_keyPatches[21 + i], viewport);
	if (valid && _pressedKey && (int32)(_now - _keyUntil) < 0)
		valid = applyRunawayPatch(_keyPatches[2 + _pressedKey], viewport);
	if (valid && _keyResult && (int32)(_now - _keyUntil) < 0)
		valid = applyRunawayPatch(_keyPatches[18 + _keyResult], viewport);
	if (!valid)
		error("Invalid museum keypad image");
}

bool RunawayRoomScene::loadScene4140() {
	for (uint i = 3; i < ARRAYSIZE(_keyPatches); ++i) {
		if (!_archive.readVariableChunk(i, _keyPatches[i]))
			return false;
	}
	return true;
}

} // End of namespace Hollywood
