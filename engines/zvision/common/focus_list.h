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

#ifndef ZVISION_FOCUS_LIST_H
#define ZVISION_FOCUS_LIST_H

#include "common/array.h"

template<typename T>
class FocusList {
private:
	Common::Array<T> _focus;
	typedef uint SizeType;  //TODO - find a way to make this typedef inherit from the definition in Common::Array
public:
	void set(T currentFocus) {
		if (!_focus.size())
			_focus.push_back(currentFocus);
		else {
			if (_focus.front() != currentFocus) {
				Common::Array<T> buffer;
				while (_focus.size() > 0) {
					if (_focus.back() != currentFocus)
						buffer.push_back(_focus.back());
					_focus.pop_back();
				}
				_focus.push_back(currentFocus);
				while (buffer.size() > 0) {
					_focus.push_back(buffer.back());
					buffer.pop_back();
				}
			}
		}
	}

	T get(SizeType idx = 0) {
		return _focus[idx];
	}

	T front() {
		return _focus.front();
	}

	T &operator[](SizeType idx) {
		assert(idx < _focus.size());
		return _focus[idx];
	}

	SizeType size() {
		return _focus.size();
	}

	void clear() {
		_focus.clear();
	}

	void remove(T value) {
		if (_focus.size()) {
			Common::Array<T> buffer;
			while (_focus.size() > 0) {
				if (_focus.back() != value)
					buffer.push_back(_focus.back());
				_focus.pop_back();
			}
			while (buffer.size() > 0) {
				_focus.push_back(buffer.back());
				buffer.pop_back();
			}
		}
	}

};

#endif
