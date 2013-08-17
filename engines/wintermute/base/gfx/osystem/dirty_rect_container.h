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

namespace Wintermute {
class DirtyRectContainer {
public:
	DirtyRectContainer();
	~DirtyRectContainer();
	void addDirtyRect(const Common::Rect &rect, const Common::Rect *clipRect);
	void reset();
	int getSize();
	bool _disableDirtyRects;
	Common::Rect *getRect(int id);
	Common::Array<Common::Rect *> getOptimized();
	Common::Array<Common::Rect *> getFallback();
private:
	Common::Array<Common::Rect *> _rectArray;
	Common::Rect *_clipRect;
};
} // End of namespace Wintermute

#endif
