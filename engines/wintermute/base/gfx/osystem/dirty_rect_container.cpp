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

const int kMaxOutputRects = 256;
const int kMaxInputRects = 512;
const int kMaxAcceptableWaste = 10;
const int kMinAcceptableWaste = 3;
const int kMaxSplicingX = 5;
const int kMaxSplicingY = 5;
const int kHugeWidthPercent = 90;
const int kHugeHeigthPercent = 90;
const int kHugeWidthFixed = 1024;
const int kHugeHeightFixed = 768;

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
		assert(clipRect->equals(*_clipRect));
	}

	Common::Rect *tmp = new Common::Rect(rect);
	int target = getSize();

	if (_disableDirtyRects) {
		return;
	}
	if (target > kMaxInputRects) {
		_disableDirtyRects = true;
		return;
	}

	if (isHuge(&rect)) {
		_disableDirtyRects = true;
		return;
	}

	if (rect.width() == 0 || rect.height() == 0) {
		return;
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

bool DirtyRectContainer::isHuge(const Common::Rect *rect) {
	// It's huge if it exceeds kHuge[Height|Width]Fixed
	// or is within kHuge[Width|Height]PErcent of the cliprect

	assert(rect != nullptr);

	if (rect->width() > kHugeWidthFixed && rect->height() > kHugeHeightFixed) {
		return true;
	}

	int wThreshold = _clipRect->width() * (kHugeWidthFixed * 10) / 100;
	int hThreshold = _clipRect->height() * (kHugeHeightFixed * 10) / 100;

	if (rect->width() * 10 > wThreshold && rect->height() * 10 > hThreshold) {
		return true;
	}

	return false;
}
Common::Array<Common::Rect *> DirtyRectContainer::getOptimized() {

	if (_disableDirtyRects) {
		return getFallback();
	}

	Common::Array<Common::Rect *> ret;
	ret.clear();
	Common::Array<Common::Rect *> queue;

	for (int i = 0; i < _rectArray.size(); i++) {
		queue.insert_at(queue.size(), _rectArray[i]);
	}

	for (int j = 0; j < queue.size(); j++) {

		Common::Rect *candidate = queue[j];

		if (candidate->width() == 0 || candidate->height() == 0) {
			// We have no use for this
			continue;
		}

		if (ret.size() > kMaxOutputRects) {
			return getFallback();
		}

		// See if it's contained or containes
		for (uint i = 0; i < ret.size(); i++) {

			Common::Rect *existing = ret[i];

			assert(existing->width() != 0 && existing->height() != 0);
			// We don't want to put useless garbage in here.

			if (existing->contains(*candidate) || existing->equals(*candidate)) {
				break;
			}

			if (candidate->contains(*(existing))) {
				// Contains an existing one.
				// Extend the pre-existing one and discard this.
				existing->extend(*candidate);
				break;
			}

			// Okay, we now see if we have an overlapping corner and slice accordingly.
			Common::Rect intersecting = existing->findIntersectingRect(*candidate);
			// We have to remove intersecting and enqueue the rest
			// We know that it's not a simple contained rect, we know that
			// it's not a cross-shaped thing like above, we know that it's not
			// a Commodore-logo type of deal (where the C is the bigger rect)
			// So intersecting is either the NE, SE, SW or NW corner of candidate
			if (intersecting.width() >= kMaxSplicingX &&
			        intersecting.height() >= kMaxSplicingY
			   ) {
				if (intersecting.top >= candidate->top &&
				        intersecting.left >= candidate->left &&
				        intersecting.bottom == candidate->bottom &&
				        intersecting.right == candidate->right
				   ) { // SE case
					Common::Rect *neslice = new Common::Rect(*candidate);
					neslice->bottom = intersecting.top;
					neslice->left = intersecting.left;
					assert(neslice->isValidRect());
					if (neslice->width() != 0 && neslice->height() != 0) {
						queue.insert_at(queue.size(), neslice);
					}
					candidate->right = intersecting.left;
					assert(neslice->isValidRect());
					assert(candidate->isValidRect());
				} else if (intersecting.top == candidate->top &&
				           intersecting.left >= candidate->left &&
				           intersecting.bottom <= candidate->bottom &&
				           intersecting.right == candidate->right
				          ) { // NE
					Common::Rect *seslice = new Common::Rect(*candidate);
					seslice->top = intersecting.bottom;
					seslice->left = intersecting.left;
					assert(seslice->isValidRect());
					assert(seslice->width() != 0 && seslice->height() != 0);
					if (seslice->width() != 0 && seslice->height() != 0) {
						queue.insert_at(queue.size(), seslice);
					}
					candidate->right = intersecting.left;
					assert(candidate->isValidRect());
				} else if (intersecting.top == candidate->top &&
				           intersecting.left == candidate->left &&
				           intersecting.bottom <= candidate->bottom &&
				           intersecting.right <= candidate->right
				          ) {     // NW
					Common::Rect *swslice = new Common::Rect(*candidate);
					swslice->top = intersecting.bottom;
					swslice->right = intersecting.right;
					assert(swslice->isValidRect());
					assert(candidate->isValidRect());
					if (swslice->width() != 0 && swslice->height() != 0) {
						queue.insert_at(queue.size(), swslice);
					}
					candidate->left = intersecting.right;
					assert(candidate->isValidRect());
				} else if (intersecting.top >= candidate->top &&
				           intersecting.left == candidate->left &&
				           intersecting.bottom == candidate->bottom &&
				           intersecting.right <= candidate->right
				          ) { // SW
					Common::Rect *nwslice = new Common::Rect(*candidate);
					nwslice->bottom = intersecting.top;
					nwslice->right = intersecting.right;
					assert(nwslice->isValidRect());
					assert(candidate->isValidRect());
					if (nwslice->width() != 0 && nwslice->height() != 0) {
						queue.insert_at(queue.size(), nwslice);
					}
					candidate->left = intersecting.right;
					assert(candidate->isValidRect());
				} else if (existing->left <= candidate->left &&
				           existing->right >= candidate->right &&
				           existing->top >= candidate->top &&
				           existing->bottom <= candidate->bottom) { // Cross shaped intersections
					Common::Rect *top_slice = new Common::Rect(*candidate);
					Common::Rect *bottom_slice = new Common::Rect(*candidate);
					top_slice->bottom = existing->top;
					bottom_slice->top = existing->bottom;

					if (top_slice->height() > 0 && top_slice->width() > 0) {
						queue.insert_at(queue.size(), top_slice);
						candidate->left = 0;
						candidate->right = 0;
						candidate->bottom = 0;
						candidate->top = 0;
						continue;
					}

					if (bottom_slice->height() > 0 && bottom_slice->width() > 0) {
						queue.insert_at(queue.size(), bottom_slice);
						candidate->left = 0;
						candidate->right = 0;
						candidate->bottom = 0;
						candidate->top = 0;
						continue;
					}

				} else if (existing->left >= candidate->left &&
				           existing->right <= candidate->right &&
				           existing->top <= candidate->top &&
				           existing->bottom >= candidate->bottom) {
					Common::Rect *left_slice = new Common::Rect(*candidate);
					left_slice->right = existing->left;
					if (left_slice->height() > 0 && left_slice->width() > 0)
						queue.insert_at(queue.size(), left_slice);

					Common::Rect *right_slice = new Common::Rect(*candidate);
					right_slice->right = existing->left;
					if (right_slice->height() > 0 && right_slice->width() > 0)
						queue.insert_at(queue.size(), right_slice);

					candidate->left = 0;
					candidate->right = 0;
					candidate->bottom = 0;
					candidate->top = 0;
				}
			} // End of intersecting test

		} // End loop

		if (candidate->width() > 0 && candidate->height() > 0) {
			ret.insert_at(ret.size(), candidate);
		}
	} // End loop

	return ret;
}
} // End of namespace Wintermute

