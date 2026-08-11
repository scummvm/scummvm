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

#ifndef QDENGINE_MINIGAMES_ADV_HOLD_DATA_H
#define QDENGINE_MINIGAMES_ADV_HOLD_DATA_H

namespace QDEngine {

template <class T>
class HoldData {
	T _emptyData;
	T &_data;
	bool _empty;
public:
	HoldData() : _data(_emptyData), _empty(true) {}
	HoldData(T* data, bool empty)
		: _data(data ? * data : _emptyData) {
		_empty = data ? empty : true;
	}

	void process(T& current) {
		if (_empty) {
			_data = current;
			_empty = false;
		} else
			current = _data;
	}

	const T &get() const {
		assert(!_empty);
		return _data;
	}
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_HOLD_DATA_H
