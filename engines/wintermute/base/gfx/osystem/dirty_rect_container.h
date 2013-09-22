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

#include "common/rect.h"
#include "common/array.h"
#include <limits.h>

namespace Wintermute {
class DirtyRectContainer {
public:
	DirtyRectContainer();
	~DirtyRectContainer();
	/**
	 * @brief Add a dirty rect. To be called by the outside world.
	 * Doesn't necessarily result in a new dirty rect, it just informs the 
	 * DR container that an area is to be considered dirty.
	 * The DirtyRectContainer may then do what's optimal, which includes
	 * ignoring the rect altoegether (e.g. if a subset of an existing one)
	 * or disabling dirtyrects altogether.
	 */
	void addDirtyRect(const Common::Rect &rect, const Common::Rect *clipRect);
	/** 
	 * resets the DirtyRectContainer
	 */
	void reset();
	/**
	 * @brief returns on optimized list of rects where duplicates and intersections are eschewed.
	 */
	Common::Array<Common::Rect *> getOptimized();
	/**
	 * @brief returns the most naive but cheap solution - the whole viewport
	 */
	Common::Array<Common::Rect *> getFallback();
private:
	static const uint kMaxOutputRects = UINT_MAX;
	// We have convened that we are not worried about lotsa rects
	// anymore thanks to wjp's patch... but overflow is still a remote risk.
	static const uint kMaxInputRects = 512;
	// Max input rects before we fall back to a single giant rect.
	static const int kMaxSplicingX = 5;
	static const int kMaxSplicingY = 5;
	// Threshold for splicing.
	// Or, maximum allowed width and height of a region
	// shared by two rects, beyond which they are recomputed.
	// Lower this and less pixels are shared by two rects, so faster blitting.
	// Lower this too much and the optimizing algorithm takes up a lot of time.

	static const int kHugeWidthPercent = 90;
	static const int kHugeHeigthPercent = 90;
	// if a rect is > kHugeXXXPercent of the viewport 
	// it is basically the whole viewport, so we abort the loop, disable dirty rects altogether
	// and avoid further computations which could yield just a few saved pixels

	static const int kHugeWidthFixed = 1024;
	static const int kHugeHeightFixed = 768;
	// Same as above, but fixed.

	Common::Array<Common::Rect *> _rectArray;
	Common::Array<Common::Rect *> _cleanMe;
	// List of temporary rects created by the class to be delete()d
	// when the renderer is done with them.
	Common::Rect *_clipRect;
	bool isHuge(const Common::Rect *rect);
	/**
	 * How big has the DirtyRect input list grown?
	 */
	int getSize();
	/**
	 * True if DR are temporarily disabled.
	 * Only the DirtyRectContainer, single point of handling of all matters dirtyrect,
	 * may decide to do so.
	 */
	bool _disableDirtyRects;
	Common::Rect *getRect(int id);
};
} // End of namespace Wintermute

#endif
