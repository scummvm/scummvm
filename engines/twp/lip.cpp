
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

#include "twp/lip.h"

namespace Twp {

void Lip::load(Common::SeekableReadStream *stream) {
	_items.clear();
	while (!stream->eos()) {
		LipItem item;
		Common::String line = stream->readLine();
		sscanf(line.c_str(), "%f\t%c", &item.time, &item.letter);
		_items.push_back(item);
	}
}

char Lip::letter(float time) {
	if (_items.size() == 0)
		return 'A';
	for (int i = 0; i < _items.size() - 1; i++) {
		if (time < _items[i + 1].time) {
			return _items[i].letter;
		}
	}
	return _items[_items.size() - 1].letter;
}

} // namespace Twp
