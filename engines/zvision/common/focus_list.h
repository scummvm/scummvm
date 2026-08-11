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

namespace ZVision {

/**
 * FILO array of unique members
 * 
 * Tracks redraw order of layered graphical elements.
 * When an element has current focus, it is reshuffled to the top of the pile.
 * When redrawing, start with last (bottom) element of list and finish with first (top)
 * Used to: 
 *	- ensure scrolling menus are drawn in the order in which they last had mouse focus.
 * 	- ensure most recently updated subtitle is drawn atop all others.
 */

template<class T>
class FocusList : public Common::Array<T> {
private:
	typedef uint size_type;
public:
	/**
	 * Move unique entry to front of list; add to list if not already present.
	 * Sequence of all remaining members remains unchanged.
	 */
	void set(const T currentFocus) {
		if (!this->size())
			this->push_back(currentFocus);
		else {
			if (this->front() != currentFocus) {
				Common::Array<T> buffer;
				while (this->size() > 0) {
					if (this->back() != currentFocus)
						buffer.push_back(this->back());
					this->pop_back();
				}
				this->push_back(currentFocus);
				while (buffer.size() > 0) {
					this->push_back(buffer.back());
					buffer.pop_back();
				}
			}
		}
	}

	/**
	 * Remove unique entry, if present.
	 * Sequence of all remaining members remains unchanged.
	 */
	void remove(const T value) {
		if (this->size()) {
			Common::Array<T> buffer;
			while (this->size() > 0) {
				if (this->back() != value)
					buffer.push_back(this->back());
				this->pop_back();
			}
			while (buffer.size() > 0) {
				this->push_back(buffer.back());
				buffer.pop_back();
			}
		}
	}

};

}  // End of namespace ZVision

#endif
