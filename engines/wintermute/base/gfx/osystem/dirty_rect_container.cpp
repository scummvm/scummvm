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
#define SAFE_ENQUEUE(slice)\
				if (slice->width() != 0 && slice->height() != 0) {\
					assert(slice->isValidRect());\
					queue.insert_at(queue.size(), slice);\
					_cleanMe.insert_at(_cleanMe.size(), slice);\
				} else {\
					delete slice;\
				}\

namespace Wintermute {

DirtyRectContainer::DirtyRectContainer() {
	_disableDirtyRects = true;
	_clipRect = nullptr;
}

DirtyRectContainer::~DirtyRectContainer() {
	reset();
	delete _clipRect;
}

void DirtyRectContainer::addDirtyRect(const Common::Rect &rect, const Common::Rect &clipRect) {
	// We get the clipping rect information along with rects for legacy reasons.
	// We assume it does not change mid-frame and, if we don't have  one, we store it.
	// This is later relevant if getOptimized gets called without being fed rects first;

	if (_clipRect == nullptr) {
		_clipRect = new Common::Rect(clipRect);
	} else {
		assert(clipRect.equals(*_clipRect));
	}

	assert(_rectArray.size() < INPUT_RECTS_HARD_LIMIT);

	if (_rectArray.size() > kMaxInputRects) {
		// This should not really happen in real world usage,
		// but there is a possibility of being flooded with rects.
		// At which point, we bail out.
		// This is, basically, the 'unrealistic' case, something went wrong.
		_disableDirtyRects = true;
		warning ("Too many rects, disabling dirty rects for this frame.");
	} else {
		Common::Rect *tmp = new Common::Rect(rect);
		tmp->clip(clipRect);
		if (tmp->width() != 0 && tmp->height() != 0) {
			_disableDirtyRects = false;
			_rectArray.push_back(tmp);
		}
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

Common::Array<Common::Rect *> DirtyRectContainer::getFallback() {
#if ENABLE_BAILOUT == false
	assert(_disableDirtyRects || false);
#endif
	Common::Array<Common::Rect *> singleret;
	if (_clipRect == nullptr) {
		/* This is the reasonable case;
		 * We have been fed no dirty rects, therefore we have no clip rect,
		 * we return a 0-rect, which means '0 pixels dirtied'.
		 */
		return singleret;
	} else {
		/*
		 * This is the actual fallback case for when we temporarily disable
		 * dirty rects somewhere.
		 * We return one giant rect that's as big as the clipRect.
		 * This should not really happen except in extreme cases.
		 */
		assert(_disableDirtyRects);
		warning ("Drawing to whole cliprect!");
		Common::Rect *temp = new Common::Rect(*_clipRect);
		singleret.insert_at(0, temp);
		_cleanMe.insert_at(_cleanMe.size(), temp);
	}
	return singleret;
}

Common::Array<Common::Rect *> DirtyRectContainer::getOptimized() {
	if (_disableDirtyRects) {
#if not ENABLE_BAILOUT
		// If we have not enabled bailout, this should be really the only reason
		assert(_clipRect == nullptr);
#endif
		return getFallback();
	}

	assert(_cleanMe.size() == 0);

	Common::Array<Common::Rect *> ret;
	Common::Array<Common::Rect *> queue;

	for (uint i = 0; i < _rectArray.size(); i++) {
		assert(_clipRect->contains(*_rectArray[i]));
		queue.insert_at(queue.size(), _rectArray[i]);
	}

#if CONSISTENCY_CHECK
	int targetPixels = _clipRect->width() *_clipRect->height();
	int filledPixels = 0;
#endif

	while (queue.size()) {
		assert(queue.size() <= QUEUE_HARD_LIMIT);
		assert(ret.size() <= RETURN_HARD_LIMIT);
		assert(_cleanMe.size() <= CLEAN_HARD_LIMIT);

#if ENABLE_BAILOUT
		if (
				(filledPixels * 128 >= (targetPixels * PIXEL_BAILOUT_LIMIT)) ||
				(queue.size() >= QUEUE_BAILOUT_LIMIT)
			) {
			warning("Bailing out of dirty rect, filled %d pixels out of %d, queue size: %d", filledPixels, targetPixels, queue.size());
			_disableDirtyRects = true;
			return getFallback();
		}
#endif

		Common::Rect *candidate = queue[0];
		assert(_clipRect->contains(*candidate));

		bool discard = false;

		assert (candidate->width() != 0 || candidate->height() != 0);
		assert (candidate->isValidRect());

		for (uint i = 0; i < ret.size() && !discard; i++) {

			Common::Rect *existing = ret[i];
			assert(_clipRect->contains(*existing));
			assert(existing->width() != 0 && existing->height() != 0);
			Common::Rect intersecting = existing->findIntersectingRect(*candidate);
			if ((intersecting.width() == 0) && (intersecting.height() == 0)) {
				continue;
			}
			if (existing->contains(*candidate)) {
				discard = true;
			} else if (candidate->contains(*(existing))) {
				// Contains an existing one.

				Common::Rect *nSlice = new Common::Rect(*candidate);
				nSlice->bottom = existing->top;
				SAFE_ENQUEUE(nSlice);

				Common::Rect *sSlice = new Common::Rect(*candidate);
				sSlice->top = existing->bottom;
				SAFE_ENQUEUE(sSlice);

				Common::Rect *eSlice = new Common::Rect(*candidate);
				eSlice->bottom = existing->bottom;
				eSlice->top = existing->top;
				eSlice->left = existing->right;
				SAFE_ENQUEUE(eSlice);

				Common::Rect *wSlice = new Common::Rect(*candidate);
				wSlice->bottom = existing->bottom;
				wSlice->top = existing->top;
				wSlice->right = existing->left;
				SAFE_ENQUEUE(wSlice);

				discard = true;

			} else if (existing->left <= candidate->left &&
				   existing->right >= candidate->right &&
				   existing->top >= candidate->top &&
				   existing->bottom <= candidate->bottom) { // Cross shaped intersections
				/*
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


				Common::Rect *topSlice = new Common::Rect(*candidate);
				Common::Rect *bottomSlice = new Common::Rect(*candidate);

				topSlice->bottom = existing->top;
				bottomSlice->top = existing->bottom;

				SAFE_ENQUEUE(topSlice);
				SAFE_ENQUEUE(bottomSlice);

				discard = true;

			} else if (existing->left >= candidate->left &&
					   existing->right <= candidate->right &&
					   existing->top <= candidate->top &&
					   existing->bottom >= candidate->bottom) {

				Common::Rect *rightSlice = new Common::Rect(*candidate);
				Common::Rect *leftSlice = new Common::Rect(*candidate);

				rightSlice->left = existing->right;
				leftSlice->right = existing->left;

				SAFE_ENQUEUE(rightSlice);
				SAFE_ENQUEUE(leftSlice);

				discard = true;




			} else if (candidate->right <= existing->right &&
					candidate->top >= existing->top &&
					candidate->bottom <= existing->bottom &&
					candidate->left <= existing->left) {
				/*
				 *      B B
				 *  A A A B
				 *  A A A B
				 *      B B
				 */

				candidate->right = existing->left;

			} else if (candidate->right >= existing->right &&
					candidate->top >= existing->top &&
					candidate->bottom <= existing->bottom &&
					candidate->left >= existing->left) {
				/*
				 *  B B
				 *  B A A A A
				 *  B A A A A
				 *  B B
				 */

				candidate->left = existing->right;

			} else if (candidate->right <= existing->right &&
					candidate->top <= existing->top &&
					candidate->bottom <= existing->bottom &&
					candidate->left >= existing->left) {
				/*
				 *    A A
				 *    A A
				 *  B A A B
				 *  B B B B
				 */

				candidate->bottom = existing->top;

			} else if (candidate->right <= existing->right &&
					candidate->top >= existing->top &&
					candidate->bottom >= existing->bottom &&
					candidate->left >= existing->left) {
				/*
				 *  B B B B
				 *  B A A B
				 *    A A
				 *    A A
				 */

				candidate->top = existing->bottom;

			} else if (candidate->right >= existing->right &&
					candidate->top >= existing->top &&
					candidate->bottom >= existing->bottom &&
					candidate->left <= existing->left) {
				/*
				 *    B B
				 *  A B B A
				 *  A A A A
				 *  A A A A
				 */
				Common::Rect *nwSlice = new Common::Rect(*candidate);
				Common::Rect *neSlice = new Common::Rect(*candidate);
				Common::Rect *sSlice = new Common::Rect(*candidate);

				sSlice->top = existing->bottom;

				nwSlice->bottom = existing->bottom;
				nwSlice->right = existing->left;

				neSlice->bottom = existing->bottom;
				neSlice->left= existing->right;

				SAFE_ENQUEUE(sSlice);
				SAFE_ENQUEUE(nwSlice);
				SAFE_ENQUEUE(neSlice);

				discard = true;

			} else if (candidate->right >= existing->right &&
					candidate->top >= existing->top &&
					candidate->bottom >= existing->bottom &&
					candidate->left <= existing->left) {
				/*
				 *
				 *  A A A A
				 *  A A A A
				 *  A B B A
				 *    B B
				 *
				 */
				Common::Rect *swSlice = new Common::Rect(*candidate);
				Common::Rect *seSlice = new Common::Rect(*candidate);
				Common::Rect *nSlice = new Common::Rect(*candidate);

				nSlice->bottom = existing->bottom;

				swSlice->top = existing->top;
				swSlice->right = existing->left;

				seSlice->top = existing->top;
				seSlice->left= existing->right;

				SAFE_ENQUEUE(nSlice);
				SAFE_ENQUEUE(swSlice);
				SAFE_ENQUEUE(seSlice);

				discard = true;

			} else if (candidate->top <= existing->top &&
					candidate->right >= existing->right &&
					candidate->left >= existing->left &&
					candidate->bottom >= existing->bottom) {
				/*
				 *   A A A A
				 * B B A A A
				 * B B A A A
				 *   A A A A
				 */
				Common::Rect *nwSlice = new Common::Rect(*candidate);
				Common::Rect *swSlice = new Common::Rect(*candidate);
				Common::Rect *eSlice = new Common::Rect(*candidate);

				eSlice->left = existing->right;

				nwSlice->bottom = existing->top;
				nwSlice->right = existing->right;

				swSlice->top = existing->bottom;
				swSlice->right = existing->right;

				SAFE_ENQUEUE(eSlice);
				SAFE_ENQUEUE(nwSlice);
				SAFE_ENQUEUE(swSlice);

				discard = true;

			} else if (candidate->top <= existing->top &&
					candidate->right >= existing->right &&
					candidate->left >= existing->left &&
					candidate->bottom >= existing->bottom) {
				/*
				 * A A A A
				 * A A B B B
				 * A A B B B
				 * A A A A
				 */
				Common::Rect *neSlice = new Common::Rect(*candidate);
				Common::Rect *seSlice = new Common::Rect(*candidate);
				Common::Rect *wSlice = new Common::Rect(*candidate);

				wSlice->right = existing->left;

				neSlice->bottom = existing->top;
				neSlice->left = existing->left;

				seSlice->top = existing->bottom;
				seSlice->left = existing->left;

				SAFE_ENQUEUE(wSlice);
				SAFE_ENQUEUE(neSlice);
				SAFE_ENQUEUE(seSlice);

				discard = true;

			} else if (intersecting.top >= candidate->top &&
					intersecting.left >= candidate->left &&
					intersecting.bottom == candidate->bottom &&
					intersecting.right == candidate->right
			   ) { // SE case

				/*
				 *  A A A      A = candidate
				 *  A A A
				 *  A A I B B  B = existing
				 *      B B B
				 *      B B B
				 *
				 */
				assert (
						!(intersecting.left == candidate->left &&
						 intersecting.right == candidate->right &&
						 intersecting.top == candidate->top &&
						 intersecting.bottom == candidate->bottom)
				);
				assert ((intersecting.width() != 0) && (intersecting.height() != 0));

				Common::Rect *nSlice = new Common::Rect(*candidate);
				Common::Rect *swSlice = new Common::Rect(*candidate);

				nSlice->bottom = intersecting.top;

				SAFE_ENQUEUE(nSlice);

				swSlice->right = intersecting.left;
				swSlice->top = intersecting.top;

				assert (intersecting.findIntersectingRect(*swSlice).width() == 0 ||
						intersecting.findIntersectingRect(*swSlice).height() == 0);

				assert (intersecting.findIntersectingRect(*nSlice).width() == 0 ||
						intersecting.findIntersectingRect(*nSlice).height() == 0);


				SAFE_ENQUEUE(swSlice);

				discard = true;

			} else if (intersecting.top >= candidate->top &&
					intersecting.left == candidate->left &&
					intersecting.bottom == candidate->bottom &&
					intersecting.right <= candidate->right
			   ) { // SW case

				/*
				 *      A A A  A = candidate
				 *      A A A
				 *  B B I A A  B = existing
				 *  B B B
				 *  B B B
				 *
				 */
				assert (
						!(intersecting.left == candidate->left &&
						 intersecting.right == candidate->right &&
						 intersecting.top == candidate->top &&
						 intersecting.bottom == candidate->bottom)
				);
				assert ((intersecting.width() != 0) && (intersecting.height() != 0));

				Common::Rect *nSlice = new Common::Rect(*candidate);
				Common::Rect *seSlice = new Common::Rect(*candidate);

				nSlice->bottom = intersecting.top;

				SAFE_ENQUEUE(nSlice);

				seSlice->left = intersecting.right;
				seSlice->top = intersecting.top;

				assert (intersecting.findIntersectingRect(*seSlice).width() == 0 ||
						intersecting.findIntersectingRect(*seSlice).height() == 0);

				assert (intersecting.findIntersectingRect(*nSlice).width() == 0 ||
						intersecting.findIntersectingRect(*nSlice).height() == 0);

				SAFE_ENQUEUE(seSlice);

				discard = true;

			} else if (intersecting.top == candidate->top &&
					intersecting.left == candidate->left &&
					intersecting.bottom <= candidate->bottom &&
					intersecting.right <= candidate->right
			   ) { // NW case

				/*
				 *
				 *  B B B
				 *  B B B
				 *  B B I A A  A = candidate
				 *      A A A
				 *      A A A  B = existing
				 *
				 */
				assert (
						!(intersecting.left == candidate->left &&
						 intersecting.right == candidate->right &&
						 intersecting.top == candidate->top &&
						 intersecting.bottom == candidate->bottom)
				);
				assert ((intersecting.width() != 0) && (intersecting.height() != 0));

				Common::Rect *sSlice = new Common::Rect(*candidate);
				Common::Rect *neSlice = new Common::Rect(*candidate);

				sSlice->top = intersecting.bottom;

				SAFE_ENQUEUE(sSlice);

				neSlice->left = intersecting.right;
				neSlice->bottom = intersecting.bottom;

				assert (intersecting.findIntersectingRect(*neSlice).width() == 0 ||
						intersecting.findIntersectingRect(*neSlice).height() == 0);

				assert (intersecting.findIntersectingRect(*sSlice).width() == 0 ||
						intersecting.findIntersectingRect(*sSlice).height() == 0);

				SAFE_ENQUEUE(neSlice);

				discard = true;

			} else if (intersecting.top == candidate->top &&
					intersecting.left >= candidate->left &&
					intersecting.bottom <= candidate->bottom &&
					intersecting.right == candidate->right) { // NE case

				/*
				 *
				 *      B B B
				 *      B B B
				 *  A A I B B   A = candidate
				 *  A A A
				 *  A A A       B = existing
				 *
				 */
				assert (
						!(intersecting.left == candidate->left &&
						 intersecting.right == candidate->right &&
						 intersecting.top == candidate->top &&
						 intersecting.bottom == candidate->bottom)
				);
				assert ((intersecting.width() != 0) && (intersecting.height() != 0));

				Common::Rect *sSlice = new Common::Rect(*candidate);
				Common::Rect *nwSlice = new Common::Rect(*candidate);

				sSlice->top = intersecting.bottom;

				SAFE_ENQUEUE(sSlice);

				nwSlice->right = intersecting.left;
				nwSlice->bottom = intersecting.bottom;

				assert (intersecting.findIntersectingRect(*nwSlice).width() == 0 ||
						intersecting.findIntersectingRect(*nwSlice).height() == 0);

				assert (intersecting.findIntersectingRect(*sSlice).width() == 0 ||
						intersecting.findIntersectingRect(*sSlice).height() == 0);

				SAFE_ENQUEUE(nwSlice);

				discard = true;

			}

		} // End loop

		if (!discard) {
			assert(candidate->isValidRect());
			if (candidate->width() > 0 && candidate->height() > 0) {
				ret.insert_at(ret.size(), candidate);
#if CONSISTENCY_CHECK
				filledPixels += candidate->width() * candidate->height();
#endif
			}
		}

		queue.remove_at(0);
	} // End while loop

#if CONSISTENCY_CHECK
	assert (_disableDirtyRects == false);
	int naivePx = consistencyCheck(ret);
	int gain = (((filledPixels * 128) / (naivePx)) * 100 ) / 128;
	warning ("%d/%d/%dpx filled (%d percent), %d/%d rects", filledPixels, naivePx, targetPixels, gain, _rectArray.size(), ret.size());
#endif
	return ret;
}

#if CONSISTENCY_CHECK
#define SENTINEL -255
int DirtyRectContainer::consistencyCheck(Common::Array<Common::Rect *> &optimized) {
	Common::Array<Common::Array<int> > diff;

	for (int x = _clipRect->left; x < _clipRect->right; x++) {
		Common::Array<int> col;
		for (int y = _clipRect->top; y < _clipRect->bottom; y++) {
			col.insert_at(y, SENTINEL);
		}
		diff.insert_at(x, col);
	}

	int dirtied = 0;
	int cleaned = 0;
	int totalPx = 0;
	int duplicatePx = 0;

	for (int x = _clipRect->left; x < _clipRect->right; x++) {
		for (int y = _clipRect->top; y < _clipRect->bottom; y++) {

			bool is_dirty = false;

			for (uint i = 0; i < _rectArray.size(); i++) {
				Common::Rect *rect = _rectArray[i];
				if (rect->width() != 0 && rect->height() != 0) {
					if(rect->contains(Common::Point(x,y))) {
						if (is_dirty) {
							duplicatePx++;
						}
						is_dirty = true;
						totalPx++;
					}
				}
			}

			bool stays_dirty = false;

			for (uint i = 0; i < optimized.size(); i++) {
				Common::Rect *rect = optimized[i];
				if(rect->contains(Common::Point(x,y))) {
					stays_dirty = true;
				}
			}

			assert (diff[x][y] == SENTINEL);
			diff[x][y] = (int)stays_dirty - (int)is_dirty;

			assert (diff[x][y] >= 0);

			if (diff[x][y] == 1)
				dirtied++;

			if (diff[x][y] == -1)
				cleaned++;
		}
	}

	assert (cleaned == 0);

	if (cleaned) {
		warning("%d pixels have been cleaned", cleaned);
	}

	if (dirtied) {
		warning("%d pixels have been dirtied", dirtied);
	}

	return totalPx;
}
#endif

} // End of namespace Wintermute

