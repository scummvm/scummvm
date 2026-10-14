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

#include "hollywood/runaway/saveload.h"

namespace Hollywood {

void RunawaySaveState::sync(Common::Serializer &s) {
	s.syncAsUint16LE(scene);
	s.syncAsUint16LE(x);
	s.syncAsUint16LE(y);
	s.syncAsUint16LE(camera);
	s.syncAsByte(facing);
	s.syncAsByte(lidOpen);
	s.syncAsByte(unplugged);
	s.syncAsByte(windowClean);
	s.syncAsByte(coolerMoved);
	s.syncAsByte(coolerInspected);
	s.syncAsByte(peeked);
	s.syncAsByte(ice);
	s.syncAsByte(hatch);
	s.syncBytes(pickedUp, sizeof(pickedUp));
	s.syncBytes(examined, sizeof(examined));
	s.syncAsByte(itemCount);
	s.syncBytes(items, sizeof(items));
	s.syncAsByte(selected);
	s.syncAsByte(cloth);
	s.syncAsByte(inventoryOpen);
	s.syncAsByte(verb);
	s.syncAsUint32LE(randomSeed);
	s.syncAsUint32LE(playTime);
}

bool RunawaySaveState::valid() const {
	if (scene != 5030 || x >= 1376 || y >= 600 || camera > 352 || facing >= 8 ||
			lidOpen > 1 || unplugged > 1 || windowClean > 1 || coolerMoved > 1 ||
			coolerInspected > 1 || peeked > 1 || ice > 2 || hatch > 3 ||
			itemCount > sizeof(items) || selected > sizeof(items) || cloth > 2 || inventoryOpen > 1 || verb > 2)
		return false;
	if ((coolerMoved && ice != 2) || ((hatch != 0) != (coolerMoved != 0)))
		return false;
	for (uint i = 0; i < sizeof(pickedUp); ++i) {
		if (pickedUp[i] > 1 || examined[i] > 1)
			return false;
	}
	bool owned[27] = {};
	for (uint i = 0; i < itemCount; ++i) {
		if (!items[i] || items[i] >= sizeof(owned) || owned[items[i]])
			return false;
		owned[items[i]] = true;
	}
	return !selected || owned[selected];
}

bool RunawaySaveState::read(Common::SeekableReadStream &stream) {
	if (stream.readUint32BE() != MKTAG('R', 'U', 'N', 'A') || stream.readUint16LE() != 1)
		return false;
	Common::Serializer s(&stream, nullptr);
	sync(s);
	return !s.err() && !stream.eos() && valid();
}

bool RunawaySaveState::write(Common::WriteStream &stream) {
	if (!valid())
		return false;
	stream.writeUint32BE(MKTAG('R', 'U', 'N', 'A'));
	stream.writeUint16LE(1);
	Common::Serializer s(nullptr, &stream);
	sync(s);
	return !s.err();
}

} // End of namespace Hollywood
