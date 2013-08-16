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

#include "engines/wintermute/base/gfx/osystem/dirty_rect_container.h"

namespace Wintermute {
DirtyRectContainer::DirtyRectContainer() {
}

DirtyRectContainer::~DirtyRectContainer() {
}

void DirtyRectContainer::addDirtyRect(const Common::Rect &rect, const Common::Rect *clipRect) {
	Common::Rect *tmp = new Common::Rect(rect);
	if (_rectArray.size() == 0) {
		_rectArray.insert_at(0, tmp);
	} else {
		_rectArray[0]->extend(*tmp);
	}

	assert (clipRect != nullptr);
	
	_rectArray[_rectArray.size() - 1]->clip(*clipRect);
	
	// Stub to emulate single dirtyRect for the time being
}

void DirtyRectContainer::reset() {
	for (int i = _rectArray.size() - 1; i >= 0; i--) {
		delete _rectArray[i];
		_rectArray.remove_at(i);
	}
}

int DirtyRectContainer::getSize() {
	return _rectArray.size();
}

Common::Rect *DirtyRectContainer::getRect(int id) {
	return _rectArray[id];
}

} // End of namespace Wintermute

