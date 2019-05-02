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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "titanic/pet_control/pet_slider.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

CPetSlider::CPetSlider() {
	_orientation = 0;
	_thumbWidth = 0;
	_thumbHeight = 0;
	_sliderOffset = 0;
	_thumbFocused = false;
}

Rect CPetSlider::clearDirtyArea() {
	Rect result = _dirtyArea;
	_dirtyArea.clear();
	return result;
}

bool CPetSlider::checkThumb(const Point &pt) {
	_thumbFocused = thumbContains(pt);
	if (_thumbFocused)
		return true;
	else
		return containsPt(pt);
}

bool CPetSlider::resetThumbFocus() {
	bool result = _thumbFocused;
	_thumbFocused = false;
	return result;
}

bool CPetSlider::MouseDragMoveMsg(const Point &pt) {
	int newOffset = calcSliderOffset(pt);
	setOffsetPixels(newOffset);
	return true;
}

bool CPetSlider::MouseButtonUpMsg(const Point &pt) {
	if (thumbContains(pt))
		return true;
	if (!containsPt(pt))
		return false;

	int newOffset = calcSliderOffset(pt);
	setOffsetPixels(newOffset);
	return true;
}

bool CPetSlider::contains(const Point &pt) const {
	return thumbContains(pt) || containsPt(pt);
}

double CPetSlider::getOffsetPixels() const {
	int maxVal = 0, minVal = 0;
	if (_orientation & ORIENTATION_HORIZONTAL) {
		maxVal = _slidingRect.right;
		minVal = _slidingRect.left;
	}

	if (_orientation & ORIENTATION_VERTICAL) {
		maxVal = _slidingRect.bottom;
		minVal = _slidingRect.top;
	}

	if (minVal == maxVal)
		return 0.0;

	return (double)_sliderOffset / (maxVal - minVal);
}

void CPetSlider::setSliderOffset(double offset) {
	if (_orientation & ORIENTATION_HORIZONTAL)
		_sliderOffset = (int)(offset * (_slidingRect.right - _slidingRect.left));

	if (_orientation & ORIENTATION_VERTICAL)
		_sliderOffset = (int)(offset * (_slidingRect.bottom - _slidingRect.top));
}

void CPetSlider::setOffsetPixels(int offset) {
	// Add the slider's old position to the dirty area
	Rect tempRect = getThumbRect();
	_dirtyArea.combine(tempRect);

	// Set the new offset
	_sliderOffset = offset;

	// Add the thumb's new location to the dirty area
	tempRect = getThumbRect();
	_dirtyArea.combine(tempRect);
}

Point CPetSlider::getBackgroundDrawPos() {
	return Point(_bounds.left, _bounds.top);
}

Point CPetSlider::getThumbDrawPos() {
	Point thumbPos = getThumbCentroidPos();
	thumbPos -= Point(_thumbWidth / 2, _thumbHeight / 2);
	return thumbPos;
}

Point CPetSlider::getThumbCentroidPos() const {
	Point pt;

	if (_orientation & ORIENTATION_HORIZONTAL) {
		pt = Point(_slidingRect.left + _sliderOffset,
			_slidingRect.top + _slidingRect.height() / 2);
	}

	if (_orientation & ORIENTATION_VERTICAL) {
		pt = Point(_slidingRect.left + _slidingRect.width() / 2,
			_slidingRect.top + _sliderOffset);
	}

	return pt;
}

bool CPetSlider::thumbContains(const Point &pt) const {
	return getThumbRect().contains(pt);
}

Rect CPetSlider::getThumbRect() const {
	Rect thumbRect(0, 0, _thumbWidth, _thumbHeight);
	Point centroid = getThumbCentroidPos();
	thumbRect.moveTo(centroid.x - _thumbWidth / 2, centroid.y - _thumbHeight / 2);

	return thumbRect;
}

int CPetSlider::calcSliderOffset(const Point &pt) const {
	int result = 0;

	if (_orientation & ORIENTATION_HORIZONTAL) {
		result = CLIP(pt.x, _slidingRect.left, _slidingRect.right) - _slidingRect.left;
	}

	if (_orientation & ORIENTATION_VERTICAL) {
		result = CLIP(pt.y, _slidingRect.top, _slidingRect.bottom) - _slidingRect.top;
	}

	return result;
}

void CPetSlider::setOrientation(SliderOrientation orientation) {
	_orientation |= orientation;
}

void CPetSlider::stepPosition(int direction) {
	double val = getOffsetPixels();

	if (direction == -1) {
		val = MAX<double>(val - 0.1, 0.0);
	} else if (direction == 1) {
		val = MIN<double>(val + 0.1, 1.0);
	}

	setSliderOffset(val);
}

/*------------------------------------------------------------------------*/

void CPetSoundSlider::setupBackground(const CString &name, CPetControl *petControl) {
	if (petControl) {
		_background = petControl->getHiddenObject(name);
	}
}

void CPetSoundSlider::setupThumb(const CString &name, CPetControl *petControl) {
	if (petControl) {
		_thumb = petControl->getHiddenObject(name);
	}
}

void CPetSoundSlider::setupBackground2(const CString &name, CPetControl *petControl) {
	if (petControl) {
		CString numStr = "3";
		int mode = petControl->getPassengerClass();
		if (mode <= 3) {
			numStr = CString(mode);
		} else if (mode == 4) {
			mode = petControl->getPriorClass();
			if (mode == 1) {
				numStr = CString(mode);
			}
		}

		CString fullName = numStr + name;
		setupBackground(fullName, petControl);
	}
}

void CPetSoundSlider::setupThumb2(const CString &name, CPetControl *petControl) {
	if (petControl) {
		CString numStr = "3";
		int mode = petControl->getPassengerClass();
		if (mode <= 3) {
			numStr = CString(mode);
		} else if (mode == 4) {
			mode = petControl->getPriorClass();
			if (mode == 1) {
				numStr = CString(mode);
			}
		}

		CString fullName = numStr + name;
		setupThumb(fullName, petControl);
	}
}

void CPetSoundSlider::draw(CScreenManager *screenManager) {
	if (_background) {
		Point pt = getBackgroundDrawPos();
		_background->draw(screenManager, pt);
	}

	if (_thumb) {
		Point pt = getThumbDrawPos();
		_thumb->draw(screenManager, pt);
	}
}

} // End of namespace Titanic
