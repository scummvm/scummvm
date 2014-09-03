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
	_tempDisableDRects = false;
	_clipRect = nullptr;
}

DirtyRectContainer::~DirtyRectContainer() {
	reset();
	delete _clipRect;
}

void DirtyRectContainer::safeEnqueue(Common::Rect *slice, Common::Array<Common::Rect *> *queue) {
	if (slice->width() != 0 && slice->height() != 0) {
		assert(slice->isValidRect());
		queue->push_back(slice);
		_cleanMe.push_back(slice);
	} else {
		delete slice;
	}
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

#if ENABLE_BAILOUT
	if (_rectArray.size() > kMaxInputRects) {
		// This should not really happen in real world usage,
		// but there is a possibility of being flooded with rects.
		// At which point, we bail out.
		// This is, basically, the 'unrealistic' case, something went wrong.
		_tempDisableDRects = true;
		warning("Too many input rects: %d disabling dirty rects for this frame.", _rectArray.size());
	} else {
		Common::Rect *tmp = new Common::Rect(rect);
		tmp->clip(clipRect);
		if (tmp->width() != 0 && tmp->height() != 0) {
			_rectArray.push_back(tmp);
		}
	}
#else
	Common::Rect *tmp = new Common::Rect(rect);
	tmp->clip(clipRect);
	if (tmp->width() != 0 && tmp->height() != 0) {
		_rectArray.push_back(tmp);
	}
#endif
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
	_tempDisableDRects = false;
	delete _clipRect;
	_clipRect = nullptr;
}

#if ENABLE_BAILOUT
Common::Array<Common::Rect *> DirtyRectContainer::getFallback() {
	/*
	 * Fallback case for when we temporarily disable
	 * dirty rects somewhere.
	 * We return one giant rect that's as big as the clipRect.
	 * This should not really happen except in extreme cases.
	 */
	assert(_tempDisableDRects);
	assert(_clipRect != nullptr);
	Common::Array<Common::Rect *> singleret;
	warning("Drawing to whole cliprect!");

	// We return a 1-rect list where the rect is the whole cliprect.

	Common::Rect *temp = new Common::Rect(*_clipRect);
	singleret.push_back(temp);
	_cleanMe.push_back(temp);

	return singleret;
}
#endif

Common::Array<Common::Rect *> DirtyRectContainer::getOptimized() {
#if ENABLE_BAILOUT
	if (_tempDisableDRects) {
		return getFallback();
	}
#endif

	Common::Array<Common::Rect *> ret;
	Common::Array<Common::Rect *> queue;

	if (_clipRect == nullptr) {
		/*
		 * We get clipRect together with the rects for historical reasons.
		 * No cliprect means no rects, which means nothing was dirtied.
		 * We return an empty list with a clear conscience.
		 */
		assert(_rectArray.size() == 0);
		return ret;
	}

	assert(ret.size() == 0);
	assert(queue.size() == 0);
	for (uint i = 0; i < _rectArray.size(); i++) {
		assert(_clipRect->contains(*_rectArray[i]));
		queue.push_back(_rectArray[i]);
	}

	assert(queue.size() == _rectArray.size());

#if CONSISTENCY_CHECK
	int targetPixels = _clipRect->width() *_clipRect->height();
	int filledPixels = 0;
#endif

	while (queue.size()) {
		/* We iterate through the rect list and we see what to do with them.
		 * We take the one at the bottom and compare it with the existing rects.
		 * If we are really lucky, there is no overlap.
		 * Otherwise, we will end up in one of the cases in the giant if.
		 */

		Common::Rect *candidate = queue[0];
		assert(_clipRect->contains(*candidate));

		bool discard = false;

		assert(candidate->width() != 0 || candidate->height() != 0);
		assert(candidate->isValidRect());

		for (uint i = 0; i < ret.size() && !discard; i++) {
#if ENABLE_BAILOUT
			/*
			 * This is where we temporarily disable/bail out of dirty rects for this frame.
			 * In real-world usage actual blitting seems to vastly overshadow any time spent in here, though,
			 * so it's best used as a safeguard.
			 */
			if (
					(queue.size() >= kMaxQueuedRects)
				) {
				warning("Bailing out of dirty rect, queue size, input size: %d, %d", queue.size(), _rectArray.size());
				_tempDisableDRects = true;
				return getFallback();
			}
#endif

			Common::Rect *existing = ret[i];
			assert(_clipRect->contains(*existing));
			assert(existing->width() != 0 && existing->height() != 0);
			Common::Rect intersecting = existing->findIntersectingRect(*candidate);
			if ((intersecting.width() == 0) && (intersecting.height() == 0)) {
				// Wonderful. candidate has no intersection with existing.
				// We can move on and compare it with the next existing one.
			} else if (existing->contains(*candidate)) {
				// candidate is entirely contained in an existing rect.
				// We throw it away.
				discard = true;
			} else if (candidate->contains(*(existing))) {
				/*
				 * This is the other way around, it looks like
				 *
				 *  C C C C C C   (C = candidate, X = eXisting)
				 *  C X X X X C
				 *  C C C C C C
				 *
				 * We slice it to
				 *
				 *    n n n n n
				 *  w X X X X X e
				 *    s s s s s
				 *
				 * And put n,s,w,e back in the queue.
				 *
				 */

				Common::Rect *nSlice = new Common::Rect(*candidate);
				nSlice->bottom = existing->top;
				safeEnqueue(nSlice, &queue);

				Common::Rect *sSlice = new Common::Rect(*candidate);
				sSlice->top = existing->bottom;
				safeEnqueue(sSlice, &queue);

				Common::Rect *eSlice = new Common::Rect(*candidate);
				eSlice->bottom = existing->bottom;
				eSlice->top = existing->top;
				eSlice->left = existing->right;
				safeEnqueue(eSlice, &queue);

				Common::Rect *wSlice = new Common::Rect(*candidate);
				wSlice->bottom = existing->bottom;
				wSlice->top = existing->top;
				wSlice->right = existing->left;
				safeEnqueue(wSlice, &queue);

				discard = true;

			} else if (existing->left <= candidate->left &&
				   existing->right >= candidate->right &&
				   existing->top >= candidate->top &&
				   existing->bottom <= candidate->bottom) { // Cross shaped intersections
				/*
				 * Cross-shaped intersections like:
				 *
				 *     C C
				 *     C C
				 * X X X X X X
				 *     C C
				 *     C C
				 *
				 * and the other way around.
				 * We split them like:
				 *
				 *     C C
				 *     C C
				 * X X X X X X
				 *     X X
				 *     X X
				 *
				 * We hold on to C and enqueue b for later processing.
				 *
				 */


				Common::Rect *topSlice = new Common::Rect(*candidate);
				Common::Rect *bottomSlice = new Common::Rect(*candidate);

				topSlice->bottom = existing->top;
				bottomSlice->top = existing->bottom;

				safeEnqueue(topSlice, &queue);
				safeEnqueue(bottomSlice, &queue);

				discard = true;

			} else if (existing->left >= candidate->left &&
					   existing->right <= candidate->right &&
					   existing->top <= candidate->top &&
					   existing->bottom >= candidate->bottom) {

				/*
				 * Inverse version of the above
				 */
				Common::Rect *rightSlice = new Common::Rect(*candidate);
				Common::Rect *leftSlice = new Common::Rect(*candidate);

				rightSlice->left = existing->right;
				leftSlice->right = existing->left;

				safeEnqueue(rightSlice, &queue);
				safeEnqueue(leftSlice, &queue);

				discard = true;




			} else if (candidate->right <= existing->right &&
					candidate->top >= existing->top &&
					candidate->bottom <= existing->bottom &&
					candidate->left <= existing->left) {
				/*
				 *      X X
				 *  C C C X
				 *  C C C X
				 *      X X
				 */

				candidate->right = existing->left;

			} else if (candidate->right >= existing->right &&
					candidate->top >= existing->top &&
					candidate->bottom <= existing->bottom &&
					candidate->left >= existing->left) {
				/*
				 *  X X
				 *  X C C C C
				 *  X C C C C
				 *  X X
				 */

				candidate->left = existing->right;

			} else if (candidate->right <= existing->right &&
					candidate->top <= existing->top &&
					candidate->bottom <= existing->bottom &&
					candidate->left >= existing->left) {
				/*
				 *    C C
				 *    C C
				 *  X C C X
				 *  X X X X
				 */

				candidate->bottom = existing->top;

			} else if (candidate->right <= existing->right &&
					candidate->top >= existing->top &&
					candidate->bottom >= existing->bottom &&
					candidate->left >= existing->left) {
				/*
				 *  X X X X
				 *  X C C X
				 *    C C
				 *    C C
				 */

				candidate->top = existing->bottom;

			} else if (candidate->right >= existing->right &&
					candidate->top >= existing->top &&
					candidate->bottom >= existing->bottom &&
					candidate->left <= existing->left) {
				/*
				 *    X X
				 *  C X X C
				 *  C C C C
				 *  C C C C
				 */
				Common::Rect *nwSlice = new Common::Rect(*candidate);
				Common::Rect *neSlice = new Common::Rect(*candidate);
				Common::Rect *sSlice = new Common::Rect(*candidate);

				sSlice->top = existing->bottom;

				nwSlice->bottom = existing->bottom;
				nwSlice->right = existing->left;

				neSlice->bottom = existing->bottom;
				neSlice->left= existing->right;

				safeEnqueue(sSlice, &queue);
				safeEnqueue(nwSlice, &queue);
				safeEnqueue(neSlice, &queue);

				discard = true;

			} else if (candidate->right >= existing->right &&
					candidate->top >= existing->top &&
					candidate->bottom >= existing->bottom &&
					candidate->left <= existing->left) {
				/*
				 *
				 *  C C C C
				 *  C C C C
				 *  C X X C
				 *    X X
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

				safeEnqueue(nSlice, &queue);
				safeEnqueue(swSlice, &queue);
				safeEnqueue(seSlice, &queue);

				discard = true;

			} else if (candidate->top <= existing->top &&
					candidate->right >= existing->right &&
					candidate->left >= existing->left &&
					candidate->bottom >= existing->bottom) {
				/*
				 *   C C C C
				 * X X C C C
				 * X X C C C
				 *   C C C C
				 */
				Common::Rect *nwSlice = new Common::Rect(*candidate);
				Common::Rect *swSlice = new Common::Rect(*candidate);
				Common::Rect *eSlice = new Common::Rect(*candidate);

				eSlice->left = existing->right;

				nwSlice->bottom = existing->top;
				nwSlice->right = existing->right;

				swSlice->top = existing->bottom;
				swSlice->right = existing->right;

				safeEnqueue(eSlice, &queue);
				safeEnqueue(nwSlice, &queue);
				safeEnqueue(swSlice, &queue);

				discard = true;

			} else if (candidate->top <= existing->top &&
					candidate->right >= existing->right &&
					candidate->left >= existing->left &&
					candidate->bottom >= existing->bottom) {
				/*
				 * C C C C
				 * C C X X X
				 * C C X X X
				 * C C C C
				 */
				Common::Rect *neSlice = new Common::Rect(*candidate);
				Common::Rect *seSlice = new Common::Rect(*candidate);
				Common::Rect *wSlice = new Common::Rect(*candidate);

				wSlice->right = existing->left;

				neSlice->bottom = existing->top;
				neSlice->left = existing->left;

				seSlice->top = existing->bottom;
				seSlice->left = existing->left;

				safeEnqueue(wSlice, &queue);
				safeEnqueue(neSlice, &queue);
				safeEnqueue(seSlice, &queue);

				discard = true;

			} else if (intersecting.top >= candidate->top &&
					intersecting.left >= candidate->left &&
					intersecting.bottom == candidate->bottom &&
					intersecting.right == candidate->right
			   ) { // SE case

				/*
				 *  C C C
				 *  C C C
				 *  C C I X X
				 *      X X X
				 *      X X X
				 *
				 */
				assert(
						!(intersecting.left == candidate->left &&
						 intersecting.right == candidate->right &&
						 intersecting.top == candidate->top &&
						 intersecting.bottom == candidate->bottom)
				);
				assert((intersecting.width() != 0) && (intersecting.height() != 0));

				Common::Rect *nSlice = new Common::Rect(*candidate);
				Common::Rect *swSlice = new Common::Rect(*candidate);

				nSlice->bottom = intersecting.top;

				safeEnqueue(nSlice, &queue);

				swSlice->right = intersecting.left;
				swSlice->top = intersecting.top;

				assert(intersecting.findIntersectingRect(*swSlice).width() == 0 ||
						intersecting.findIntersectingRect(*swSlice).height() == 0);

				assert(intersecting.findIntersectingRect(*nSlice).width() == 0 ||
						intersecting.findIntersectingRect(*nSlice).height() == 0);


				safeEnqueue(swSlice, &queue);

				discard = true;

			} else if (intersecting.top >= candidate->top &&
					intersecting.left == candidate->left &&
					intersecting.bottom == candidate->bottom &&
					intersecting.right <= candidate->right
			   ) { // SW case

				/*
				 *      C C C
				 *      C C C
				 *  X X I C C
				 *  X X X
				 *  X X X
				 *
				 */
				assert(
						!(intersecting.left == candidate->left &&
						 intersecting.right == candidate->right &&
						 intersecting.top == candidate->top &&
						 intersecting.bottom == candidate->bottom)
				);
				assert((intersecting.width() != 0) && (intersecting.height() != 0));

				Common::Rect *nSlice = new Common::Rect(*candidate);
				Common::Rect *seSlice = new Common::Rect(*candidate);

				nSlice->bottom = intersecting.top;

				safeEnqueue(nSlice, &queue);

				seSlice->left = intersecting.right;
				seSlice->top = intersecting.top;

				assert(intersecting.findIntersectingRect(*seSlice).width() == 0 ||
						intersecting.findIntersectingRect(*seSlice).height() == 0);

				assert(intersecting.findIntersectingRect(*nSlice).width() == 0 ||
						intersecting.findIntersectingRect(*nSlice).height() == 0);

				safeEnqueue(seSlice, &queue);

				discard = true;

			} else if (intersecting.top == candidate->top &&
					intersecting.left == candidate->left &&
					intersecting.bottom <= candidate->bottom &&
					intersecting.right <= candidate->right
			   ) { // NW case

				/*
				 *
				 *  X X X
				 *  X X X
				 *  X X I C C
				 *      C C C
				 *      C C C
				 *
				 */
				assert(
						!(intersecting.left == candidate->left &&
						 intersecting.right == candidate->right &&
						 intersecting.top == candidate->top &&
						 intersecting.bottom == candidate->bottom)
				);
				assert((intersecting.width() != 0) && (intersecting.height() != 0));

				Common::Rect *sSlice = new Common::Rect(*candidate);
				Common::Rect *neSlice = new Common::Rect(*candidate);

				sSlice->top = intersecting.bottom;

				safeEnqueue(sSlice, &queue);

				neSlice->left = intersecting.right;
				neSlice->bottom = intersecting.bottom;

				assert(intersecting.findIntersectingRect(*neSlice).width() == 0 ||
						intersecting.findIntersectingRect(*neSlice).height() == 0);

				assert(intersecting.findIntersectingRect(*sSlice).width() == 0 ||
						intersecting.findIntersectingRect(*sSlice).height() == 0);

				safeEnqueue(neSlice, &queue);

				discard = true;

			} else if (intersecting.top == candidate->top &&
					intersecting.left >= candidate->left &&
					intersecting.bottom <= candidate->bottom &&
					intersecting.right == candidate->right) { // NE case

				/*
				 *
				 *      X X X
				 *      X X X
				 *  C C I X X
				 *  C C C
				 *  C C C
				 *
				 */
				assert(
						!(intersecting.left == candidate->left &&
						 intersecting.right == candidate->right &&
						 intersecting.top == candidate->top &&
						 intersecting.bottom == candidate->bottom)
				);
				assert((intersecting.width() != 0) && (intersecting.height() != 0));

				Common::Rect *sSlice = new Common::Rect(*candidate);
				Common::Rect *nwSlice = new Common::Rect(*candidate);

				sSlice->top = intersecting.bottom;

				safeEnqueue(sSlice, &queue);

				nwSlice->right = intersecting.left;
				nwSlice->bottom = intersecting.bottom;

				assert(intersecting.findIntersectingRect(*nwSlice).width() == 0 ||
						intersecting.findIntersectingRect(*nwSlice).height() == 0);

				assert(intersecting.findIntersectingRect(*sSlice).width() == 0 ||
						intersecting.findIntersectingRect(*sSlice).height() == 0);

				safeEnqueue(nwSlice, &queue);

				discard = true;

			}

		} // End loop

		/*
		 * At this point we should have taken out of candidate all bits that
		 * overlap with existing rects and/or bits that we have decided to
		 * push back in the queue for later processing.
		 * We assert that whatever's left of candidate has no overlap with
		 * anything in the return list.
		 *
		 * If marked for discard we ignore it altogether (consider
		 * marking for discard the equivalent of replacing it with a 0x0 rect after
		 * having verified each pixel is contained elsewhere and/or having sliced
		 * off and enqueued the 'useful' parts).
		 *
		 */

		if (!discard) {
			assert(candidate->isValidRect());
			// There should not be anything that makes the rect invalid
			// (worse that can happen, 0x0), so this is probably worth
			// investigating if it ever happens.
			if (candidate->width() > 0 && candidate->height() > 0) {
				ret.push_back(candidate);
#if CONSISTENCY_CHECK
				filledPixels += candidate->width() * candidate->height();
#endif
			}
		}

		queue.remove_at(0);
	} // End while loop

#if CONSISTENCY_CHECK
	assert(_tempDisableDRects == false);
	assert(_clipRect != nullptr);
	int naivePx = consistencyCheck(ret);
	// How many pixels we would have drawn if overlaps were not treated
	int gain = (((filledPixels * 128) / (naivePx)) * 100 ) / 128;
	warning("%d/%d/%dpx filled (%d percent), %d/%d rects", filledPixels, naivePx, targetPixels, gain, _rectArray.size(), ret.size());
#endif

#if DEBUG_COUNT_RECTS
	warning("%d rects to %d", _rectArray.size(), ret.size());
#endif

	return ret;
}

#if CONSISTENCY_CHECK
#define SENTINEL -255
int DirtyRectContainer::consistencyCheck(Common::Array<Common::Rect *> &optimized) {

	assert(_clipRect != nullptr);

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

			bool isDirty = false;

			for (uint i = 0; i < _rectArray.size(); i++) {
				Common::Rect *rect = _rectArray[i];
				if (rect->width() != 0 && rect->height() != 0) {
					if(rect->contains(Common::Point(x,y))) {
						if (isDirty) {
							duplicatePx++;
						}
						isDirty = true;
						totalPx++;
					}
				}
			}

			bool staysDirty = false;

			for (uint i = 0; i < optimized.size(); i++) {
				Common::Rect *rect = optimized[i];
				if(rect->contains(Common::Point(x,y))) {
					staysDirty = true;
				}
			}

			assert(diff[x][y] == SENTINEL);
			diff[x][y] = (int)staysDirty - (int)isDirty;

			assert(diff[x][y] >= 0);

			if (diff[x][y] == 1)
				dirtied++;

			if (diff[x][y] == -1)
				cleaned++;
		}
	}

	assert(cleaned == 0);

	if (cleaned) {
		warning("%d pixels have been cleaned", cleaned);
		/*
		 *	This should /never/ be triggered, it means that
		 *	some pixels were dirtied but somehow got un-dirtied
		 *	during the process.
		 */

	}

	if (dirtied) {
		warning("%d pixels have been dirtied", dirtied);
	}

	return totalPx;
}
#endif

} // End of namespace Wintermute

