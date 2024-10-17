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
	T emptyData_;
	T &data_;
	bool empty_;
public:
	HoldData() : data_(emptyData_), empty_(true) {}
	HoldData(T* data, bool empty)
		: data_(data ? * data : emptyData_) {
		empty_ = data ? empty : true;
	}

	void process(T& current) {
		if (empty_) {
			data_ = current;
			empty_ = false;
		} else
			current = data_;
	}

	const T &get() const {
		assert(!empty_);
		return data_;
	}
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_HOLD_DATA_H
