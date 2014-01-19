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

void DirtyRectContainer::addDirtyRect(const Common::Rect &rect, const Common::Rect &clipRect) {
	// TODO: Ignore 0-size rects?
	// TODO: Maybe check if really big (like == viewport)
	// and avoid the whole dance altogether.

	if (_clipRect == nullptr) {
		_clipRect = new Common::Rect(clipRect);
	} else {
		assert(clipRect.equals(*_clipRect));
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
		_rectArray[target]->clip(clipRect);
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
		assert(_clipRect->contains(*_rectArray[i]));
		queue.insert_at(queue.size(), _rectArray[i]);
	}

	int targetPixels = _clipRect->width() *_clipRect->height();
	int filledPixels = 0;

	while (queue.size()) {
		assert(queue.size() <= RECT_QUEUE_LIMIT);
		assert(ret.size() <= RECT_LIMIT);
		assert(_cleanMe.size() <= 10*(RECT_LIMIT + RECT_QUEUE_LIMIT));
		if (0 && filledPixels * 100 >= (targetPixels * 85)) {
			// We have filled almost everything, let's just bail out.
			warning("Bailing out of dirty rect, filled %d pixels out of %d", filledPixels, targetPixels);
			_disableDirtyRects = true;
			return getFallback();
		}
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
				if (nSlice->width() != 0 && nSlice->height() != 0) {
					queue.insert_at(queue.size(), nSlice);
					_cleanMe.insert_at(_cleanMe.size(), nSlice);
				} else {
					delete nSlice;
				}

				Common::Rect *sSlice = new Common::Rect(*candidate);
				sSlice->top = existing->bottom;
				if (sSlice->width() != 0 && sSlice->height() != 0) {
					queue.insert_at(queue.size(), sSlice);
					_cleanMe.insert_at(_cleanMe.size(), sSlice);
				} else {
					delete sSlice;
				}

				Common::Rect *eSlice = new Common::Rect(*candidate);
				eSlice->bottom = existing->bottom;
				eSlice->top = existing->top;
				eSlice->left = existing->right;
				if (eSlice->width() != 0 && eSlice->height() != 0) {
					queue.insert_at(queue.size(), eSlice);
					_cleanMe.insert_at(_cleanMe.size(), eSlice);
				} else {
					delete eSlice;
				}

				Common::Rect *wSlice = new Common::Rect(*candidate);
				wSlice->bottom = existing->bottom;
				wSlice->top = existing->top;
				wSlice->right = existing->left;
				if (wSlice->width() != 0 && wSlice->height() != 0) {
					queue.insert_at(queue.size(), wSlice);
					_cleanMe.insert_at(_cleanMe.size(), wSlice);
				} else {
					delete wSlice;
				}

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

				if (topSlice->height() > 0 && topSlice->width() > 0) {
					queue.insert_at(queue.size(), topSlice);
					_cleanMe.insert_at(_cleanMe.size(), topSlice);
				} else {
					delete topSlice;
				}


				if (bottomSlice->height() > 0 && bottomSlice->width() > 0) {
					queue.insert_at(queue.size(), bottomSlice);
					_cleanMe.insert_at(_cleanMe.size(), bottomSlice);
				} else {
					delete bottomSlice;
				}

				discard = true;
			} else if (existing->left >= candidate->left &&
					   existing->right <= candidate->right &&
					   existing->top <= candidate->top &&
					   existing->bottom >= candidate->bottom) {

				Common::Rect *rightSlice = new Common::Rect(*candidate);
				Common::Rect *leftSlice = new Common::Rect(*candidate);

				rightSlice->left = existing->right;
				leftSlice->right = existing->left;

				if (rightSlice->height() > 0 && rightSlice->width() > 0) {
					queue.insert_at(queue.size(), rightSlice);
					_cleanMe.insert_at(_cleanMe.size(), rightSlice);
				} else {
					delete rightSlice;
				}

				if (leftSlice->height() > 0 && leftSlice->width() > 0) {
					queue.insert_at(queue.size(), leftSlice);
					_cleanMe.insert_at(_cleanMe.size(), leftSlice);
				} else {
					delete leftSlice;
				}

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
				Common::Rect *wSlice = new Common::Rect(*candidate);
				wSlice->right = existing->left;
				if (wSlice->width() != 0 && wSlice->height() != 0) {
					assert(wSlice->isValidRect());
					queue.insert_at(queue.size(), wSlice);
					_cleanMe.insert_at(_cleanMe.size(), wSlice);
				} else {
					delete wSlice;
				}
				discard = true;
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
				Common::Rect *eSlice = new Common::Rect(*candidate);
				eSlice->left = existing->right;
				if (eSlice->width() != 0 && eSlice->height() != 0) {
					assert(eSlice->isValidRect());
					queue.insert_at(queue.size(), eSlice);
					_cleanMe.insert_at(_cleanMe.size(), eSlice);
				} else {
					delete eSlice;
				}
				discard = true;
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
				Common::Rect *nSlice = new Common::Rect(*candidate);
				nSlice->bottom = existing->top;
				if (nSlice->width() != 0 && nSlice->height() != 0) {
					assert(nSlice->isValidRect());
					queue.insert_at(queue.size(), nSlice);
					_cleanMe.insert_at(_cleanMe.size(), nSlice);
				} else {
					delete nSlice;
				}
				discard = true;
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
				Common::Rect *sSlice = new Common::Rect(*candidate);
				sSlice->top = existing->bottom;
				if (sSlice->width() != 0 && sSlice->height() != 0) {
					assert(sSlice->isValidRect());
					queue.insert_at(queue.size(), sSlice);
					_cleanMe.insert_at(_cleanMe.size(), sSlice);
				} else {
					delete sSlice;
				}
				discard = true;
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

				if (sSlice->width() != 0 && sSlice->height() != 0) {
					assert(sSlice->isValidRect());
					queue.insert_at(queue.size(), sSlice);
					_cleanMe.insert_at(_cleanMe.size(), sSlice);
				} else {
					delete sSlice;
				}

				if (nwSlice->width() != 0 && nwSlice->height() != 0) {
					assert(nwSlice->isValidRect());
					queue.insert_at(queue.size(), nwSlice);
					_cleanMe.insert_at(_cleanMe.size(), nwSlice);
				} else {
					delete nwSlice;
				}

				if (neSlice->width() != 0 && neSlice->height() != 0) {
					assert(neSlice->isValidRect());
					queue.insert_at(queue.size(), neSlice);
					_cleanMe.insert_at(_cleanMe.size(), neSlice);
				} else {
					delete neSlice;
				}

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

				if (nSlice->width() != 0 && nSlice->height() != 0) {
					assert(nSlice->isValidRect());
					queue.insert_at(queue.size(), nSlice);
					_cleanMe.insert_at(_cleanMe.size(), nSlice);
				} else {
					delete nSlice;
				}

				if (swSlice->width() != 0 && swSlice->height() != 0) {
					assert(swSlice->isValidRect());
					queue.insert_at(queue.size(), swSlice);
					_cleanMe.insert_at(_cleanMe.size(), swSlice);
				} else {
					delete swSlice;
				}

				if (seSlice->width() != 0 && seSlice->height() != 0) {
					assert(seSlice->isValidRect());
					queue.insert_at(queue.size(), seSlice);
					_cleanMe.insert_at(_cleanMe.size(), seSlice);
				} else {
					delete seSlice;
				}

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

				if (eSlice->width() != 0 && eSlice->height() != 0) {
					assert(eSlice->isValidRect());
					queue.insert_at(queue.size(), eSlice);
					_cleanMe.insert_at(_cleanMe.size(), eSlice);
				} else {
					delete eSlice;
				}

				if (nwSlice->width() != 0 && nwSlice->height() != 0) {
					assert(nwSlice->isValidRect());
					queue.insert_at(queue.size(), nwSlice);
					_cleanMe.insert_at(_cleanMe.size(), nwSlice);
				} else {
					delete nwSlice;
				}

				if (swSlice->width() != 0 && swSlice->height() != 0) {
					assert(swSlice->isValidRect());
					queue.insert_at(queue.size(), swSlice);
					_cleanMe.insert_at(_cleanMe.size(), swSlice);
				} else {
					delete swSlice;
				}

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

				if (wSlice->width() != 0 && wSlice->height() != 0) {
					assert(wSlice->isValidRect());
					queue.insert_at(queue.size(), wSlice);
					_cleanMe.insert_at(_cleanMe.size(), wSlice);
				} else {
					delete wSlice;
				}

				if (neSlice->width() != 0 && neSlice->height() != 0) {
					assert(neSlice->isValidRect());
					queue.insert_at(queue.size(), neSlice);
					_cleanMe.insert_at(_cleanMe.size(), neSlice);
				} else {
					delete neSlice;
				}

				if (seSlice->width() != 0 && seSlice->height() != 0) {
					assert(seSlice->isValidRect());
					queue.insert_at(queue.size(), seSlice);
					_cleanMe.insert_at(_cleanMe.size(), seSlice);
				} else {
					delete seSlice;
				}

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

				assert(nSlice->isValidRect());

				if (nSlice->width() != 0 && nSlice->height() != 0) {
					queue.insert_at(queue.size(), nSlice);
					_cleanMe.insert_at(_cleanMe.size(), nSlice);
				} else {
					delete nSlice;
				}

				swSlice->right = intersecting.left;
				swSlice->top = intersecting.top;

				assert (intersecting.findIntersectingRect(*swSlice).width() == 0 ||
						intersecting.findIntersectingRect(*swSlice).height() == 0);

				assert (intersecting.findIntersectingRect(*nSlice).width() == 0 ||
						intersecting.findIntersectingRect(*nSlice).height() == 0);

				assert(swSlice->isValidRect());

				if (swSlice->width() != 0 && swSlice->height() != 0) {
					queue.insert_at(queue.size(), swSlice);
					_cleanMe.insert_at(_cleanMe.size(), swSlice);
				} else {
					delete swSlice;
				}

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

				assert(nSlice->isValidRect());

				if (nSlice->width() != 0 && nSlice->height() != 0) {
					queue.insert_at(queue.size(), nSlice);
					_cleanMe.insert_at(_cleanMe.size(), nSlice);
				} else {
					delete nSlice;
				}

				seSlice->left = intersecting.right;
				seSlice->top = intersecting.top;

				assert (intersecting.findIntersectingRect(*seSlice).width() == 0 ||
						intersecting.findIntersectingRect(*seSlice).height() == 0);

				assert (intersecting.findIntersectingRect(*nSlice).width() == 0 ||
						intersecting.findIntersectingRect(*nSlice).height() == 0);

				assert(seSlice->isValidRect());

				if (seSlice->width() != 0 && seSlice->height() != 0) {
					queue.insert_at(queue.size(), seSlice);
					_cleanMe.insert_at(_cleanMe.size(), seSlice);
				} else {
					delete seSlice;
				}

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

				assert(sSlice->isValidRect());

				if (sSlice->width() != 0 && sSlice->height() != 0) {
					queue.insert_at(queue.size(), sSlice);
					_cleanMe.insert_at(_cleanMe.size(), sSlice);
				} else {
					delete sSlice;
				}

				neSlice->left = intersecting.right;
				neSlice->bottom = intersecting.bottom;

				assert (intersecting.findIntersectingRect(*neSlice).width() == 0 ||
						intersecting.findIntersectingRect(*neSlice).height() == 0);

				assert (intersecting.findIntersectingRect(*sSlice).width() == 0 ||
						intersecting.findIntersectingRect(*sSlice).height() == 0);

				assert(neSlice->isValidRect());

				if (neSlice->width() != 0 && neSlice->height() != 0) {
					queue.insert_at(queue.size(), neSlice);
					_cleanMe.insert_at(_cleanMe.size(), neSlice);
				} else {
					delete neSlice;
				}

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

				assert(sSlice->isValidRect());

				if (sSlice->width() != 0 && sSlice->height() != 0) {
					queue.insert_at(queue.size(), sSlice);
					_cleanMe.insert_at(_cleanMe.size(), sSlice);
				} else {
					delete sSlice;
				}

				nwSlice->right = intersecting.left;
				nwSlice->bottom = intersecting.bottom;

				assert (intersecting.findIntersectingRect(*nwSlice).width() == 0 ||
						intersecting.findIntersectingRect(*nwSlice).height() == 0);

				assert (intersecting.findIntersectingRect(*sSlice).width() == 0 ||
						intersecting.findIntersectingRect(*sSlice).height() == 0);

				assert(nwSlice->isValidRect());

				if (nwSlice->width() != 0 && nwSlice->height() != 0) {
					queue.insert_at(queue.size(), nwSlice);
					_cleanMe.insert_at(_cleanMe.size(), nwSlice);
				} else {
					delete nwSlice;
				}

				discard = true;

			}

		} // End loop

		if (!discard) {
			assert(candidate->width() > 0 && candidate->height() > 0);
			ret.insert_at(ret.size(), candidate);
			filledPixels += candidate->width() * candidate->height();
		}

		queue.remove_at(0);
	} // End while loop

#if CONSISTENCY_CHECK == DO_CHECK
	assert (_disableDirtyRects == false);
	consistencyCheck(ret);
#endif
	warning ("%d from %d px, %d from %d rects", filledPixels, targetPixels, queue.size(), ret.size());
	return ret;
}

#if CONSISTENCY_CHECK == DO_CHECK
#define SENTINEL -255
void DirtyRectContainer::consistencyCheck(Common::Array<Common::Rect *> &optimized) {
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

	for (int x = _clipRect->left; x < _clipRect->right; x++) {
		for (int y = _clipRect->top; y < _clipRect->bottom; y++) {

			bool is_dirty = false;

			for (uint i = 0; i < _rectArray.size(); i++) {
				Common::Rect *rect = _rectArray[i];
				if (rect->width() != 0 && rect->height() != 0) {
					if(rect->contains(Common::Point(x,y))) {
						is_dirty = true;
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
}
#endif

} // End of namespace Wintermute

