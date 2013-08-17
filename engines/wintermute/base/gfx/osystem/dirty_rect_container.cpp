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

const int kMaxOutputRects = 32;
const int kMaxInputRects = 64;
const int kMaxAcceptableWaste = 15;
const int kMinAcceptableWaste = 5;
const int kThresholdWidthPercent = 90;
const int kThresholdHeigthPercent = 90;

DirtyRectContainer::DirtyRectContainer() {
	_disableDirtyRects = false;
	_clipRect = nullptr;
}

DirtyRectContainer::~DirtyRectContainer() {
	delete _clipRect;
}

void DirtyRectContainer::addDirtyRect(const Common::Rect &rect, const Common::Rect *clipRect) {
	// TODO: Ignore 0-size rects?
	// TODO: Maybe check if really big (like == viewport)
	// and avoid the whole dance altogether.

	assert(clipRect != nullptr);
	if (_clipRect == nullptr) {
		_clipRect = new Common::Rect(*clipRect);
	} else {
		assert (clipRect->equals(*_clipRect));
	}

	Common::Rect *tmp = new Common::Rect(rect);
	int target = getSize();
	if (target > kMaxInputRects) {
		_disableDirtyRects = true;
	}
	_rectArray.insert_at(target, tmp);
	_rectArray[target]->clip(*clipRect);
	// TODO: Upper limit?
}

void DirtyRectContainer::reset() {
	for (int i = _rectArray.size() - 1; i >= 0; i--) {
		delete _rectArray[i];
		_rectArray.remove_at(i);
	}
	_disableDirtyRects = false;
	delete _clipRect;
	_clipRect = nullptr;
}

int DirtyRectContainer::getSize() {
	return _rectArray.size();
}

Common::Rect *DirtyRectContainer::getRect(int id) {
	return _rectArray[id];
}

Common::Array<Common::Rect *> DirtyRectContainer::getFallback() {
	Common::Array<Common::Rect *> singleret;
	singleret.insert_at(0, _clipRect);
	return singleret;
}

Common::Array<Common::Rect *> DirtyRectContainer::getOptimized() {

	Common::Array<Common::Rect *> ret;

	for (int i = 0; i < getSize(); i++) {

		if (_disableDirtyRects) {
			return getFallback();
		}

		if (i > kMaxOutputRects) {
			return getFallback();
		}

		Common::Rect *lastModified = nullptr;
		Common::Rect *candidate = _rectArray[i];

		if (candidate->width() == 0 || candidate->height() == 0) {
			// We have no use for this
			continue;
		}
		
		// TODO: if really big then just use a single rect and avoid 
		// all these unnecessary calculations (e.g. Rosemary fades)

		if (ret.size() > kMaxOutputRects) {
			// Simply extend one (hack: the first one) and avoid rect soup & calculations;
			ret[0]->extend(*candidate);
			lastModified = ret[0];
		}

		// See if it's contained or containes
		for (uint j = 0; j < ret.size() && lastModified == nullptr; j++) {

			if (ret[j]->contains(*candidate) || ret[j]->equals(*candidate)) {
				// It's contained - continue!
				lastModified = ret[j];
				continue;
			}

			if (candidate->contains(*(ret[j]))) {
				// Contains an existing one.
				// Extend the pre-existing one and discard this.
				ret[j]->extend(*candidate);
				lastModified = ret[j];
				continue;
			}
		}

		// See if we can extend an existing one wasting < kMaxWaste percent space

		int extendable = -1;
		int bestRatio = 999999;

		for (uint j = 0; j < ret.size()  && lastModified == nullptr; j++) {
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
					break;
				}
			}

		}

		if (extendable != -1) {
			ret[extendable]->extend(*candidate);
			lastModified = ret[extendable];
		}
		// Do checks on lastModified

		int hugeXThreshold = 800; // Hack
		int hugeYThreshold = 600; // Hack

		if (lastModified != nullptr && lastModified->width() >= hugeXThreshold &&
			lastModified->height() >= hugeYThreshold) {
				// This one is so huge that we can as well redraw the entire screen
				_disableDirtyRects = true;
				return getFallback();
				break;
		}

		if (candidate->width() >= hugeXThreshold &&
			candidate->height() >= hugeYThreshold) {
				// This one is so huge that we can as well redraw the entire screen
				_disableDirtyRects = true;
				return getFallback();
				break;
		}
		// If we ended up here, there's really nothing we can do
		int target = ret.size();
		ret.insert_at(target, candidate);
	}
	return ret;
}
} // End of namespace Wintermute

