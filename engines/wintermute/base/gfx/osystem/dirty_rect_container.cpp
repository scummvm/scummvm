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

const int kMaxRects = 32;
const int kMaxPercentWaste = 10;

DirtyRectContainer::DirtyRectContainer() {
}

DirtyRectContainer::~DirtyRectContainer() {
}

void DirtyRectContainer::addDirtyRect(const Common::Rect &rect, const Common::Rect *clipRect) {
	assert(clipRect != nullptr);
	Common::Rect *tmp = new Common::Rect(rect);
	int target = getSize();
	_rectArray.insert_at(target, tmp);
	_rectArray[target]->clip(*clipRect);
	// TODO: Upper limit?
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

Common::Array<Common::Rect *> DirtyRectContainer::getOptimized() {

	Common::Array<Common::Rect *> ret;

	for (int i = 0; i < getSize(); i++) {
		// See if this rect is contained by another - if so, discard;
		bool contained = false;
		Common::Rect *candidate = _rectArray[i];

		for (uint j = 0; j < ret.size() && !contained; j++) {
			if (ret[j]->contains(*candidate) || ret[j]->equals(*candidate)) {
				contained = true;
			}
		}

		if (contained) {
			continue;
		}

		// See if this rect containes another - if so, extend the latter and discard this one;
		int contains = -1;

		for (uint j = 0; j < ret.size() && contains == -1; j++) {
			if (candidate->contains(*(ret[j]))) {
				contains = j;
			}
		}

		if (contains != -1) {
			ret[contains]->extend(*candidate);
			continue;
		}

		// TODO: Find close relative with < kMaxPercentWaste wasted
		for (int j = 0; false; j++) {
			// NOOP
		}

		// If we ended up here, there's really nothing we can do
		int target = ret.size();
		ret.insert_at(target, candidate);
	}
	return ret;
}
} // End of namespace Wintermute

