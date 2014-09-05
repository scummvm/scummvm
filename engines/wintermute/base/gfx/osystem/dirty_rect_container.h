/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_DIRTY_RECT_CONTAINER_H
#define WINTERMUTE_DIRTY_RECT_CONTAINER_H

#include "common/list.h"
#include "common/rect.h"
#include <limits.h>

#define CONSISTENCY_CHECK 0
#define ENABLE_BAILOUT 1
#define DEBUG_COUNT_RECTS 0
#define MAX_INPUT_RECTS 47
#define DISABLE_OPTIMIZATION 0

#if ENABLE_BAILOUT
#include "common/textconsole.h"
#endif

namespace Wintermute {

template <typename T>
class SmartList : public Common::List<T> {
	int _counter;
public:
	SmartList() : Common::List<T>() {
		_counter = 0;
	}
	/** Inserts element at the start of the list. */
	void push_front(const T &element) {
		assert(false);
		// insert(_anchor._next, element);
	}

	/** Appends element to the end of the list. */
	void push_back(const T &element) {
		Common::List<T>::push_back(element);
		_counter++;
	}

	/** Removes the first element of the list. */
	void pop_front() {
		assert(_counter > 0);
		Common::List<T>::pop_front();
		_counter--;
	}

	T &front() {
		return Common::List<T>::front();
	}

	uint size() const {
		return _counter;
	}
};

class DirtyRectContainer {
public:
	DirtyRectContainer();
	~DirtyRectContainer();
	/**
	 * @brief Add a dirty rect. To be called by the outside world.
	 * For historical reasons the dirty rect is to be accompained by
	 * the clipping rect (aka "the viewport").
	 * If getOptimized() is called /before/ a single rect (and
	 * the clipping rect) is added, you get an empty list.
	 *
	 * The clipping rect is supposed to stay the same until the frame is
	 * rendered (or, for our purpouses: until reset() is called)
	 */
	void addDirtyRect(const Common::Rect &rect, const Common::Rect &clipRect);
	/**
	 * @brief Resets the DirtyRectContainer, frees memory and gets ready
	 * for a new frame.
	 * Call this once you've done everything and have rendered your frame
	 * (or at least you've got your list of dirty rects for this frame)
	 * and are ready to start fresh.
	 *
	 * PAY ATTENTION to the fact that addDirtyRect takes aliases.
	 * When you call this, it's gonna delete your rects, so not
	 * a good move if you still need them.
	 */
	void reset();
	/**
	 * @brief Returns an optimized list of rects where duplicates and
	 * intersections are eschewed, thus providing a minimal cover of
	 * the dirtied area.
	 * The returned rect list is supposed to contain only and all
	 * of the pixels contained in the input rects.
	 *
	 * Profiling shows the actual blitting to be, generally speaking,
	 * very costly, so spending some time computing a minimal cover
	 * makes sense except for very big inputs.
	 */
	SmartList<Common::Rect *> getOptimized();
	bool gotDRectOverflow();

private:
#if ENABLE_BAILOUT
	/*
	 * Generally speaking, blitting is very costly and we want to minimize that,
	 * even if at the cost of some overhead in computing a minimal cover for the
	 * dirty area.
	 * At some point we are  however bound to have diminishing returns, with
	 * the naive full screen blitting eventually surpassing the cost of computing
	 * an optimized rect list.
	 * We thus may want to bail out when we hit a certain threshold.
	 */
	/* Max queued rects before we fall back to a single giant rect. */
	static const uint kMaxInputRects = MAX_INPUT_RECTS;
	/* Max input rects before we fall back to a single giant rect. */
#endif

	SmartList<Common::Rect *> _rectList;
	/**
	 * List of temporary rects created by the class to be delete()d
	 * when the renderer is done with them (not before!).
	 * This is performed by reset()
	 */
	SmartList<Common::Rect *> _cleanMe;
	/**
	 * Safe enqueue utility.
	 * Good for leaks, good for readability.
	 */
	void safeEnqueue(Common::Rect *slice, SmartList<Common::Rect *> *queue);
	Common::Rect *_clipRect;
	/**
	 * True if DR are temporarily disabled.
	 * Only the DirtyRectContainer, single point of handling of all matters dirtyrect,
	 * may decide to do so.
	 */
	bool _dRectOverflow;

#if CONSISTENCY_CHECK
	/**
	 * Double-checks consistency of output pixel by pixel.
	 * Returns the number of pixels that would have been drawn
	 * if overlaps were not treated, including duplicates.
	 */
	int consistencyCheck(SmartList<Common::Rect *> &optimized);
#endif

};
} // End of namespace Wintermute

#endif
