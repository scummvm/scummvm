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

#include "common/array.h"
#include "common/rect.h"
#include <limits.h>

#define CONSISTENCY_CHECK 0
#define ENABLE_BAILOUT 0
#define DEBUG_COUNT_RECTS 0

namespace Wintermute {
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
	 * @brief Returns on optimized list of rects where duplicates and
	 * intersections are eschewed.
	 * The returned rect list is (unless BAIL_OUT is enabled, see above)
	 * supposed to contain only and all of the pixels contained in the input
	 * rects.
	 *
	 * Profiling shows this to be insignificant in comparison
	 * to the actual blitting, but if blitting gets a significant boost in the
	 * future (eg hardware acceleration) this may benefit from optimization
	 * (or for an appropriate degree of sloppiness in computation).
	 */
	Common::Array<Common::Rect *> getOptimized();
private:
#if ENABLE_BAILOUT
	Common::Array<Common::Rect *> getFallback();
#endif
	/**
	 * @brief Returns the whole clipping_Rect.
	 * Usually this is not to be used, unless bailout is enabled.
	 */
	static const uint kMaxQueuedRects = UINT_MAX;
	/* We have convened that we are not worried about lotsa rects
	 * anymore thanks to wjp's patch... but overflow is still a remote risk.
	 */
	static const uint kMaxInputRects = UINT_MAX;
	/* Max input rects before we fall back to a single giant rect.
	 * We assume this to be an unrealistic case, if we get here something wrong has
	 * probably happened somewhere.
	 * Profiling shows that for 'reasonable' input sizes the getOptimized algorithm,
	 * which is quadratic, is irrelevant in comparison to the actual blitting, so
	 * as long as we have realistic inputs we want to save pixels, not rects.
	 */
	Common::Array<Common::Rect *> _rectArray;
	/**
	 * List of temporary rects created by the class to be delete()d
	 * when the renderer is done with them (not before!).
	 * This is performed by reset()
	 */
	Common::Array<Common::Rect *> _cleanMe;
	/**
	 * Safe enqueue utility.
	 * Good for leaks, good for readability.
	 */
	void safeEnqueue(Common::Rect *slice, Common::Array<Common::Rect *> *queue);
	Common::Rect *_clipRect;
	/**
	 * True if DR are temporarily disabled.
	 * Only the DirtyRectContainer, single point of handling of all matters dirtyrect,
	 * may decide to do so.
	 */
	bool _tempDisableDRects;

#if CONSISTENCY_CHECK
	/**
	 * Double-checks consistency of output pixel by pixel.
	 * Returns the number of pixels that would have been drawn
	 * if overlaps were not treated, including duplicates.
	 */
	int consistencyCheck(Common::Array<Common::Rect *> &optimized);
#endif

};
} // End of namespace Wintermute

#endif
