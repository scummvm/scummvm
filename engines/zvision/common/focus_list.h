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

#ifndef focus_list_H
#define focus_list_H

#include "common/array.h"

template<typename T>
class FocusList {
private:
  Common::Array<T> focus;
  typedef uint _size_type;  //TODO - find a way to make this typedef inherit from the definition in Common::Array
public:
  void set(T currentFocus) {
    if(!focus.size())
      focus.push_back(currentFocus);     
    else {
      if(focus.front() != currentFocus) {
        Common::Array<T> buffer;
        while(focus.size() > 0) {
          if(focus.back() != currentFocus)
            buffer.push_back(focus.back());
          focus.pop_back();
        }
        focus.push_back(currentFocus);
        while(buffer.size() > 0) {
          focus.push_back(buffer.back());
          buffer.pop_back();
        }
      }
    }
  }

  T get(_size_type idx=0) {
    return focus[idx];
  }

  T front() {
    return focus.front();
  }
  
	T &operator[](_size_type idx) {
		assert(idx < focus.size());
		return focus[idx];
	}

  _size_type size() {
    return focus.size();
  }
  
  void clear() {
    focus.clear();
  }
  
  void remove(T value) {
    if(focus.size()) {
      Common::Array<T> buffer;
      while(focus.size() > 0) {
        if(focus.back() != value)
          buffer.push_back(focus.back());
        focus.pop_back();
      }
      while(buffer.size() > 0) {
        focus.push_back(buffer.back());
        buffer.pop_back();
      }
    }
  }
  
};

#endif
