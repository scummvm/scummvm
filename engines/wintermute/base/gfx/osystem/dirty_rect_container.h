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
#define DEBUG_COUNT_RECTS 0
#define DISABLE_OPTIMIZATION 0
#define MAX_INPUT_RECTS 47
#define ENABLE_BAILOUT 1
/*
 * We usually want to stop computing a cover altogether ("bail out")
 * when we hit the rect threshold.
 */

#if ENABLE_BAILOUT
#include "common/textconsole.h"
#endif

namespace Wintermute {
/**
 * SmartList extends Common::List caching the number of elements
 * (as _number) in order to gain some speed.
 */
/*
 * TODO: It's probably worth exploring replacing this
 * with a priority queue where larger rects and opaque rects
 * end up at the front. This should save a couple of cycles,
 * provided that it's not much costly.
 */
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
	 * The clipping rect is supposed to stay the same or be expanded (never
	 * shrinked) until the frame is rendered (or, for our purpouses: until
	 * reset() is called)
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
	 */
	SmartList<Common::Rect *> getOptimized();
	/**
	 * There comes a point where computing the minimal cover, which is
	 * quadratic, overshadows the actual blitting.
	 * We call this a "rect overflow" and, at least for the time being,
	 * we use an empirically determined threshold.
	 * Usually, this does not happen unless the game makes a heavy
	 * use of particles (I'm looking at you, J.u.l.i.a.).
	 */
	bool gotDRectOverflow();
private:
#if ENABLE_BAILOUT
    /* Max input rects before we fall back to a single giant rect. */
	static const uint kMaxInputRects = MAX_INPUT_RECTS;
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
