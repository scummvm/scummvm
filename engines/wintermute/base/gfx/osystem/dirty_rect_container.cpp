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
	_disableDirtyRects = true;
	_clipRect = nullptr;
}

DirtyRectContainer::~DirtyRectContainer() {
	reset();
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


	uint target = getSize();

	if (target > kMaxInputRects) {
		_disableDirtyRects = true;
		return;
	} else if (rect.width() == 0 || rect.height() == 0) {
		return;
	} else {
		Common::Rect *tmp = new Common::Rect(rect);
		_disableDirtyRects = false;
		_rectArray.insert_at(target, tmp);
		_rectArray[target]->clip(*clipRect);
	}

}

void DirtyRectContainer::reset() {
	for (int i = _rectArray.size() - 1; i >= 0; i--) {
		delete _rectArray[i];
		_rectArray.remove_at(i);
	}

	for (int i = _cleanMe.size() - 1; i >= 0; i--) {
		delete _cleanMe[i];
		_cleanMe.remove_at(i);
	}

	_disableDirtyRects = true;
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
	if (_clipRect == nullptr) {
		return singleret;
	}
	Common::Rect *temp = new Common::Rect(*_clipRect);
	singleret.insert_at(0, temp);
	_cleanMe.insert_at(_cleanMe.size(), temp);
	return singleret;
}

bool DirtyRectContainer::isHuge(const Common::Rect *rect) {
	// It's huge if it exceeds kHuge[Height|Width]Fixed
	// or is within kHuge[Width|Height]PErcent of the cliprect

	assert(rect != nullptr);
	assert(_clipRect);

	if (rect->width() > kHugeWidthFixed && rect->height() > kHugeHeightFixed) {
		return true;
	}

	int wThreshold = _clipRect->width() * (kHugeWidthPercent) / 100;
	int hThreshold = _clipRect->height() * (kHugeHeigthPercent) / 100;

	if (rect->width() > wThreshold && rect->height() > hThreshold) {
		return true;
	}

	return false;
}

Common::Array<Common::Rect *> DirtyRectContainer::getOptimized() {
	if (_disableDirtyRects) {
		return getFallback();
	}

	assert(_cleanMe.size() == 0);

	Common::Array<Common::Rect *> ret;
	Common::Array<Common::Rect *> queue;

	for (uint i = 0; i < _rectArray.size(); i++) {
		queue.insert_at(queue.size(), _rectArray[i]);
	}

	uint j = 0;
	while (j < queue.size()) {

		Common::Rect *candidate = queue[j];

		if (candidate->width() == 0 || candidate->height() == 0) {
			// We have no use for this
			queue.remove_at(j);
			continue;
		}

		if (ret.size() > kMaxOutputRects) {
			_disableDirtyRects = true;
			return getFallback();
		}

		bool discard = false;

		// See if it's contained or containes
		for (uint i = 0; i < ret.size() && !discard; i++) {
			assert(!discard);

			Common::Rect *existing = ret[i];

			assert(existing->width() != 0 && existing->height() != 0);
			// We don't want to put useless garbage in here.
			/* These are the 'simple' contains/contained cases, like:
			*     A A A A 
			 *    A A A A 
			 *    A A B B 
			 *    A A B B 
			 *     
			 *    B B B B 
			 *    B A A B
			 *    B B B B 
			 * In these cases we simply discard the "contained".
			 */

			if (existing->contains(*candidate)) {
				discard = true;
				continue;
			}

			if (candidate->contains(*(existing))) {
				// Contains an existing one.
				// Extend the pre-existing one and discard this.
				existing->extend(*candidate);
				discard = true;
				continue;
			}

			/* 
			 * Okay, now we work on intersections.
			 * We want to minimize the number of pixels shared by more than one rect.
			 * We are interested in the following cases:
			 * a. SE
			 *    A A A A 
			 *    A A A A 
			 *    A A B B B
			 *    A A B B B
			 *        B B B 
			 * In this case we keep B as it is and split A:
			 *
			 *
			 *    A1 A1 A1 A1
			 *    A1 A1 A1 A1
			 *    A2 A2  B  B  B
			 *    A2 A2  B  B  B
			 *           B  B  B
			 *
			 * A is then deleted and 
			 * A1, A2 are enqueued for processing as new rects.
			 *
			 * b, c, d: SW, NW, NE, same but with different corners.
			 *
			 * e, f: cross-shaped intersections like:
			 *
			 *    AA 
			 *    AA
			 * BB BA BB
			 *    AA
			 *    AA
			 * 
			 * and the other way around.
			 * We split like:
			 *
			 *    A1
			 *    A1
			 * BB BB BB 
			 *    A2
			 *    A2
			 *
			 * We keep B and enqueue A1, A2
			 *
			 */
			Common::Rect intersecting = existing->findIntersectingRect(*candidate);

			if (intersecting.width() >= kMaxSplicingX &&
			        intersecting.height() >= kMaxSplicingY
			   ) {
				if (intersecting.top >= candidate->top &&
				        intersecting.left >= candidate->left &&
				        intersecting.bottom == candidate->bottom &&
				        intersecting.right == candidate->right
				   ) { // SE case
					Common::Rect *neSlice = new Common::Rect(*candidate);
					_cleanMe.insert_at(_cleanMe.size(), neSlice);

					neSlice->bottom = intersecting.top;
					neSlice->left = intersecting.left;
					assert(neSlice->isValidRect());
					if (neSlice->width() != 0 && neSlice->height() != 0) {
						queue.insert_at(queue.size(), neSlice);
					}
					candidate->right = intersecting.left;
					assert(neSlice->isValidRect());
					if (candidate->width() == 0) {
						discard = true;
					}
					assert(candidate->height() != 0);
					assert(candidate->isValidRect());
				} else if (intersecting.top == candidate->top &&
				           intersecting.left >= candidate->left &&
				           intersecting.bottom <= candidate->bottom &&
				           intersecting.right == candidate->right
				          ) { // NE
					Common::Rect *seSlice = new Common::Rect(*candidate);
					_cleanMe.insert_at(_cleanMe.size(), seSlice);
					seSlice->top = intersecting.bottom;
					seSlice->left = intersecting.left;
					assert(seSlice->isValidRect());
					assert(seSlice->width() != 0 && seSlice->height() != 0);
					if (seSlice->width() != 0 && seSlice->height() != 0) {
						queue.insert_at(queue.size(), seSlice);
					}
					candidate->right = intersecting.left;
					if (candidate->width() == 0) {
						discard = true;
					}
					assert(candidate->height() != 0);

					assert(candidate->isValidRect());
				} else if (intersecting.top == candidate->top &&
				           intersecting.left == candidate->left &&
				           intersecting.bottom <= candidate->bottom &&
				           intersecting.right <= candidate->right
				          ) {     // NW
					Common::Rect *swSlice = new Common::Rect(*candidate);
					_cleanMe.insert_at(_cleanMe.size(), swSlice);
					swSlice->top = intersecting.bottom;
					swSlice->right = intersecting.right;
					assert(swSlice->isValidRect());
					assert(candidate->isValidRect());
					if (swSlice->width() != 0 && swSlice->height() != 0) {
						queue.insert_at(queue.size(), swSlice);
					}
					candidate->left = intersecting.right;
					if (candidate->width() == 0) {
						discard = true;
					}
					assert(candidate->height() != 0);
					assert(candidate->isValidRect());
				} else if (intersecting.top >= candidate->top &&
				           intersecting.left == candidate->left &&
				           intersecting.bottom == candidate->bottom &&
				           intersecting.right <= candidate->right
				          ) { // SW
					Common::Rect *nwSlice = new Common::Rect(*candidate);
					_cleanMe.insert_at(_cleanMe.size(), nwSlice);
					nwSlice->bottom = intersecting.top;
					nwSlice->right = intersecting.right;
					assert(nwSlice->isValidRect());
					assert(candidate->isValidRect());
					if (nwSlice->width() != 0 && nwSlice->height() != 0) {
						queue.insert_at(queue.size(), nwSlice);
					}
					candidate->left = intersecting.right;
					assert(candidate->isValidRect());
					if (candidate->width() == 0) {
						discard = true;
					}
					assert(candidate->height() != 0);
				} else if (existing->left <= candidate->left &&
				           existing->right >= candidate->right &&
				           existing->top >= candidate->top &&
				           existing->bottom <= candidate->bottom) { // Cross shaped intersections
					Common::Rect *topSlice = new Common::Rect(*candidate);
					Common::Rect *bottomSlice = new Common::Rect(*candidate);
					_cleanMe.insert_at(_cleanMe.size(), topSlice);
					_cleanMe.insert_at(_cleanMe.size(), bottomSlice);
					topSlice->bottom = existing->top;
					bottomSlice->top = existing->bottom;

					if (topSlice->height() > 0 && topSlice->width() > 0) {
						queue.insert_at(queue.size(), topSlice);
					}

					if (bottomSlice->height() > 0 && bottomSlice->width() > 0) {
						queue.insert_at(queue.size(), bottomSlice);
					}

					discard = true;
					continue;


				} else if (existing->left >= candidate->left &&
				           existing->right <= candidate->right &&
				           existing->top <= candidate->top &&
				           existing->bottom >= candidate->bottom) {
					Common::Rect *leftSlice = new Common::Rect(*candidate);
					_cleanMe.insert_at(_cleanMe.size(), leftSlice);
					leftSlice->right = existing->left;
					if (leftSlice->height() > 0 && leftSlice->width() > 0)
						queue.insert_at(queue.size(), leftSlice);

					Common::Rect *rightSlice = new Common::Rect(*candidate);
					_cleanMe.insert_at(_cleanMe.size(), rightSlice);

					rightSlice->right = existing->left;
					if (rightSlice->height() > 0 && rightSlice->width() > 0)
						queue.insert_at(queue.size(), rightSlice);

					discard = true;
					continue;
				}
			} // End of intersecting test

		} // End loop
		if (discard) {
			queue.remove_at(j);
		} else {
			assert(candidate->width() > 0 && candidate->height() > 0);
			if (isHuge(candidate)) {
				_disableDirtyRects = true;
				return getFallback();
			}
			ret.insert_at(ret.size(), candidate);
			j++;
		}
	} // End loop

	return ret;
}
} // End of namespace Wintermute

