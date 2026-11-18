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

const MuseumRoom kRunawayScene4140 = {
	kMuseumKeypad, 1024, 600, nullptr, 0, {},
	Common::Point(382, 529),
	nullptr, 0,
	nullptr, 0,
	nullptr, 0
};

void RunawayMuseumScene::clickScene4140(byte id) {
	_pressedKey = id;
	_keyUntil = _now + 180;
	if (!id) {
		_transition.scene = kMuseumHall;
		_transition.position = Common::Point(1153, 297);
		_transition.facing = 7;
	} else if (id == 10) {
		_keyCount = 0;
	} else if (id == 12) {
		_keyCount = 0;
		warning("Runaway museum: the fingerprint puzzle is not implemented yet");
	} else {
		_audio.playEffect(id == 11 ? 11 : 11 + id);
		if (_keyCount < 4)
			++_keyCount;
		else
			say(4);
	}
}

void RunawayMuseumScene::drawScene4140(Graphics::ManagedSurface &viewport) {
	bool valid = applyRunawayPatch(_keyPatches[15], viewport);
	for (uint i = 0; valid && i < _keyCount; ++i)
		valid = applyRunawayPatch(_keyPatches[21 + i], viewport);
	if (valid && _pressedKey && (int32)(_now - _keyUntil) < 0)
		valid = applyRunawayPatch(_keyPatches[2 + _pressedKey], viewport);
	if (!valid)
		error("Invalid museum keypad image");
}

bool RunawayMuseumScene::loadScene4140() {
	for (uint i = 3; i < ARRAYSIZE(_keyPatches); ++i) {
		if (!_archive.readVariableChunk(i, _keyPatches[i]))
			return false;
	}
	return true;
}

} // End of namespace Hollywood
