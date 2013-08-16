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
const int kMaxAcceptableWaste = 15;
const int kMinAcceptableWaste = 5;

DirtyRectContainer::DirtyRectContainer() {
}

DirtyRectContainer::~DirtyRectContainer() {
}

void DirtyRectContainer::addDirtyRect(const Common::Rect &rect, const Common::Rect *clipRect) {
	// TODO: Ignore 0-size rects?
	// TODO: Maybe check if really big (like == viewport)
	// and avoid the whole dance altogether.

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
		Common::Rect *candidate = _rectArray[i];

		if (candidate->width() == 0 || candidate->height() == 0) {
			// We have no use for this
			continue;
		}
		
		// TODO: if really big then just use a single rect and avoid 
		// all these unnecessary calculations (e.g. Rosemary fades)

		if (ret.size() > kMaxRects) {
			// Simply extend one (hack: the first one) and avoid rect soup & calculations;
			ret[0]->extend(*candidate);
			continue;
		}

		// See if it's contained or containes
		for (uint j = 0; j < ret.size(); j++) {

			if (ret[j]->contains(*candidate) || ret[j]->equals(*candidate)) {
				// It's contained - continue!
				continue;
			}

			if (candidate->contains(*(ret[j]))) {
				// Contains an existing one.
				// Extend the pre-existing one and discard this.
				ret[j]->extend(*candidate);
				continue;
			}
		}

		// See if we can extend an existing one wasting < kMaxWaste percent space

		int extendable = -1;
		int bestRatio = 999999;

		for (uint j = 0; j < ret.size(); j++) {
			Common::Rect original = *ret[j];
			Common::Rect extended = Common::Rect(original);
			extended.extend(*candidate);
			int originalArea = original.width() * original.height();
			int candidateArea = candidate->width() * candidate->height();
			int extendedArea = extended.width() * extended.height();
			int ratioScaled = extendedArea * 10 / (originalArea + candidateArea);
			if (ratioScaled <= kMaxAcceptableWaste / 10 &&
			        ratioScaled < bestRatio) {
				// This is a good candidate for extending.
				bestRatio = ratioScaled;
				extendable = j;
				if (ratioScaled <= kMinAcceptableWaste / 10) {
					// This is so good that we can actually
					// avoid traversing the rest of the array,
					// we would gain peanuts
					continue;
				}
			}

		}

		if (extendable != -1) {
			ret[extendable]->extend(*candidate);
			continue;
		}

		// If we ended up here, there's really nothing we can do
		int target = ret.size();
		ret.insert_at(target, candidate);
	}
	return ret;
}
} // End of namespace Wintermute

