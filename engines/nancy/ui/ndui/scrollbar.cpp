/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/ui/ndui/scrollbar.h"
#include "engines/nancy/ui/ndui/panel.h"

namespace Nancy {
namespace UI {
namespace NDUI {

// The thumb of a scroll bar never gets smaller than this
static const int32 kMinThumbHeight = 8;

// Holding an arrow of a scroll bar keeps scrolling, after a delay
static const float kArrowRepeatDelay = 0.33f;
static const float kArrowRepeatInterval = 0.05f;

void ScrollBar::readExtra(Common::SeekableReadStream &stream) {
	// Up arrow, down arrow and thumb, after the track
	_elements.resize(4);
	for (uint i = 1; i < 4; ++i) {
		_elements[i].read(stream);
	}

	_pageSize = stream.readSint32LE();
	capPosition();
	updateThumb();
}

// The arrows are square
Common::Rect ScrollBar::getUpButton() const {
	return Common::Rect(_bounds.left, _bounds.top, _bounds.right, _bounds.top + _bounds.width());
}

Common::Rect ScrollBar::getDownButton() const {
	return Common::Rect(_bounds.left, _bounds.bottom - _bounds.width(), _bounds.right, _bounds.bottom);
}

Common::Rect ScrollBar::getTrack() const {
	return Common::Rect(_bounds.left, getUpButton().bottom, _bounds.right, getDownButton().top);
}

void ScrollBar::capPosition() {
	if (_position < 0 || _end <= _pageSize) {
		_position = 0;
	} else if (_position + _pageSize > _end) {
		_position = _end - _pageSize;
	}
}

// Every time the thumb gets placed, the scroll bar tells others about its position
void ScrollBar::updateThumb() {
	Common::Rect track = getTrack();
	_panel->setDirty();

	if (_pageSize < _end) {
		int height = MAX(track.height() * _pageSize / _end, kMinThumbHeight);
		int top = track.top + (track.height() - height) * _position / (_end - _pageSize);
		_thumb = Common::Rect(track.left, top, track.right, top + height);
		_showThumb = true;
		sendEvent(kEventScrolled);
	} else {
		_thumb = Common::Rect(track.left, track.top, track.right, track.top);
		_showThumb = false;
	}
}

void ScrollBar::setRange(int32 end) {
	_end = end;
	capPosition();
	updateThumb();
}

void ScrollBar::setPageSize(int32 pageSize) {
	_pageSize = pageSize;
	capPosition();
	updateThumb();
}

void ScrollBar::scroll(int32 delta) {
	_position += delta;
	capPosition();
	updateThumb();
}

bool ScrollBar::update(float seconds) {
	if (_arrow != kArrowNone) {
		_arrowHeldTime += seconds;
		if (_arrowHeldTime > kArrowRepeatDelay) {
			_arrowRepeatTime += seconds;
			while (_arrowRepeatTime >= kArrowRepeatInterval) {
				_arrowRepeatTime -= kArrowRepeatInterval;
				if (_arrow == kArrowUp && getUpButton().contains(_lastMouse)) {
					scroll(-1);
				} else if (_arrow == kArrowDown && getDownButton().contains(_lastMouse)) {
					scroll(1);
				}
			}
		}
	}

	ControlState state = kStateHidden;
	if (_visible) {
		state = getState();
		if (state == kStateNormal) {
			if (!_showThumb) {
				state = kStateDisabled;
			} else if (_isMouseOver) {
				state = kStateMouseOver;
			}
		}
	}

	bool changed = false;
	for (Element &element : _elements) {
		changed |= element.blend(state, seconds, kControlBlendRate);
	}

	return changed;
}

void ScrollBar::draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const {
	Common::Rect rects[4] = { getTrack(), getUpButton(), getDownButton(), _thumb };
	for (uint i = 0; i < 4; ++i) {
		rects[i].translate(offset.x, offset.y);
		drawSprite(surface, i, rects[i]);
	}
}

bool ScrollBar::onMouseDown(const Common::Point &pos) {
	_lastMouse = pos;
	_arrowHeldTime = 0;
	_arrowRepeatTime = 0;

	if (getUpButton().contains(pos)) {
		scroll(-1);
		_arrow = kArrowUp;
	} else if (getDownButton().contains(pos)) {
		scroll(1);
		_arrow = kArrowDown;
	} else if (_showThumb && _thumb.contains(pos)) {
		_isDragging = true;
		_dragOffset = pos.y - _thumb.top;
	} else if (_showThumb && pos.y < _thumb.top) {
		scroll(-(_pageSize - 1));
	} else if (_showThumb && pos.y >= _thumb.bottom) {
		scroll(_pageSize - 1);
	}

	return true;
}

void ScrollBar::onMouseMove(const Common::Point &pos) {
	_lastMouse = pos;
	if (!_isDragging) {
		return;
	}

	Common::Rect track = getTrack();
	int maxThumbTop = track.height() - _thumb.height();
	int maxFirst = _end - _pageSize + 1;
	if (maxThumbTop <= 0 || maxFirst <= 0) {
		return;
	}

	int thumbTop = CLIP<int>(pos.y - _dragOffset - track.top, 0, maxThumbTop);
	int32 position = (thumbTop + maxThumbTop / (maxFirst * 2)) * maxFirst / maxThumbTop;
	if (position != _position) {
		_position = position;
		capPosition();
		updateThumb();
	}
}

void ScrollBar::onMouseUp(const Common::Point &pos) {
	_isDragging = false;
	_arrow = kArrowNone;
	updateThumb();
}

bool ScrollBar::onWheel(int notches) {
	scroll(-notches * kWheelLines);
	return true;
}

bool ScrollBar::doAction(const Action &action) {
	switch (action.type) {
	case kActionSetRange:
		setRange((int32)action.value);
		return true;
	case kActionSetValue:
		scroll(-(int32)action.value);
		return true;
	default:
		return Control::doAction(action);
	}
}

bool ScrollBar::getValue(int32 eventType, double &value, Common::String &text) const {
	if (eventType != kEventScrolled) {
		return false;
	}

	value = _position;
	text = _name;
	return true;
}

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy
