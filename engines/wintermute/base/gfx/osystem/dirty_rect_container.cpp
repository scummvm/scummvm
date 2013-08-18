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

const int kMaxOutputRects = 96;
const int kMaxInputRects = 512;
const int kMaxAcceptableWaste = 10;
const int kMinAcceptableWaste = 3;
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

bool DirtyRectContainer::isHuge(Common::Rect *rect) {
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

	Common::Array<Common::Rect *> ret;
	ret.clear();

	for (int i = 0; i < getSize(); i++) {

		if (_disableDirtyRects) {
			return getFallback();
		}

		if (ret.size() > kMaxOutputRects) {
			return getFallback();
		}

		Common::Rect *lastModified = nullptr;
		Common::Rect *candidate = _rectArray[i];

		if (candidate->width() == 0 || candidate->height() == 0) {
			// We have no use for this
			continue;
		}

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
				break;
			}

			if (candidate->contains(*(ret[j]))) {
				// Contains an existing one.
				// Extend the pre-existing one and discard this.
				ret[j]->extend(*candidate);
				lastModified = ret[j];
				break;
			}
		}

		// If we ended up here, there's really nothing we can do

		if (lastModified == nullptr) {
			int target = ret.size();
			ret.insert_at(target, candidate);
		}
	}

	// Exclude areas in common, if any.
	// TODO: Splice if intersection exists but removing it
	// Would result in non-rectangular shape

	Common::Array<Common::Rect *> new_ret;
	new_ret.clear();

	for (uint i = 0; i < ret.size(); i++) {
		Common::Rect *temp = ret[i];
		for (uint j = 0; j < new_ret.size(); j++) {
			if (new_ret[j]->left <= temp->left &&
			        new_ret[j]->top <= temp->top &&
			        new_ret[j]->bottom >= temp->bottom &&
			        new_ret[j]->right <= temp->right) {
				temp->left = new_ret[j]->right;
			}

			if (new_ret[j]->left >= temp->left &&
			        new_ret[j]->top <= temp->top &&
			        new_ret[j]->bottom >= temp->bottom &&
			        new_ret[j]->right >= temp->right) {
				temp->right = new_ret[j]->left;
			}

			if (new_ret[j]->left <= temp->left &&
			        new_ret[j]->top >= temp->top &&
			        new_ret[j]->bottom >= temp->bottom &&
			        new_ret[j]->right >= temp->right) {
				temp->bottom = new_ret[j]->top;
			}

			if (new_ret[j]->left <= temp->left &&
			        new_ret[j]->top <= temp->top &&
			        new_ret[j]->bottom <= temp->bottom &&
			        new_ret[j]->right >= temp->right) {
				temp->top = new_ret[j]->bottom;
			}
		}
		if (temp->width() > 0 && temp->height() > 0) {
			new_ret.insert_at(new_ret.size(), temp);
		}
	}

	return new_ret;
}
} // End of namespace Wintermute

