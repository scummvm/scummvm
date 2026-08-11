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

#ifndef GRAPHICS_DIRTYRECTS_H
#define GRAPHICS_DIRTYRECTS_H

#include "common/list.h"
#include "common/rect.h"

namespace Graphics {

/**
 * @defgroup graphics_dirtyrects DirtyRects
 * @ingroup graphics
 *
 * @brief DirtyRectList class for tracking dirty rectangles.
 *
 * @{
 */

/**
 * This class keeps track of any areas of a surface that are updated
 * by drawing calls.
 */
class DirtyRectList {
public:
	typedef Common::List<Common::Rect>::const_iterator	const_iterator; /*!< Const-qualified list iterator. */

protected:
	/**
	 * List of affected areas of the screen
	 */
	Common::List<Common::Rect> _dirtyRects;

protected:
	/**
	 * Returns the union of two dirty area rectangles
	 */
	bool unionRectangle(Common::Rect &destRect, const Common::Rect &src1, const Common::Rect &src2);

public:
	/**
	 * Merges together overlapping dirty areas of the screen
	 */
	void merge();

	/**
	 * Returns true if there are no pending screen updates (dirty areas)
	 */
	bool empty() const { return _dirtyRects.empty(); }

	/**
	 * Clear the current dirty rects list
	 */
	void clear() { _dirtyRects.clear(); }

	/**
	 * Adds a rectangle to the list of modified areas of the screen during the
	 * current frame
	 */
	template<class... TArgs>
	void emplace_back(TArgs &&...args) { _dirtyRects.emplace_back(Common::forward<TArgs>(args)...); }

	/**
	 * Adds a rectangle to the list of modified areas of the screen during the
	 * current frame
	 */
	void push_back(const Common::Rect &r) { _dirtyRects.push_back(r); }

	/**
	 * Adds a rectangle to the list of modified areas of the screen during the
	 * current frame
	 */
	void push_back(Common::Rect &&r) { _dirtyRects.push_back(Common::move(r)); }

	/** Return a const iterator to the start of the list.
	 *  This can be used, for example, to iterate from the first element
	 *  of the list to the last element of the list.
	 */
	const_iterator	begin() const {
		return _dirtyRects.begin();
	}

	/** Return a const iterator to the end of the list. */
	const_iterator	end() const {
		return _dirtyRects.end();
	}
};
 /** @} */
} // End of namespace Graphics

#endif
