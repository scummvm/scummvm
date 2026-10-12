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

#include "engines/nancy/util.h"

#include "engines/nancy/ui/ndui/slider.h"
#include "engines/nancy/ui/ndui/panel.h"

namespace Nancy {
namespace UI {
namespace NDUI {

void Slider::readExtra(Common::SeekableReadStream &stream) {
	Common::String textureName;
	Common::Rect src;
	_elements.resize(3);

	readFilename(stream, textureName);
	readExclusiveRect(stream, src);
	_elements[1].setTexture(textureName, src);

	readFilename(stream, textureName);
	readExclusiveRect(stream, src);
	_hasFill = !textureName.empty();
	_elements[2].setTexture(textureName, src);

	_min = stream.readSint32LE();
	_max = MAX(stream.readSint32LE(), _min);
	_value = CLIP(stream.readSint32LE(), _min, _max);
}

int Slider::valueAt(int x) const {
	if (_bounds.width() == 0) {
		return _min;
	}

	return (int)(0.5f + _min + (float)(_max - _min) / _bounds.width() * (x - _bounds.left));
}

// The thumb is as wide as its picture, and centered on the position of the value
Common::Rect Slider::getThumbBounds() const {
	int width = _elements[1]._src.width();
	int x = _bounds.left - width / 2;
	if (_max > _min) {
		x += (_value - _min) * _bounds.width() / (_max - _min);
	}

	return Common::Rect(x, _bounds.top, x + width, _bounds.bottom);
}

// At either end of the track, half of the thumb sticks out
Common::Rect Slider::getDrawBounds() const {
	int overhang = (_elements[1]._src.width() + 1) / 2;
	return Common::Rect(_bounds.left - overhang, _bounds.top, _bounds.right + overhang, _bounds.bottom);
}

void Slider::setValue(int value, bool byPlayer) {
	value = CLIP<int>(value, _min, _max);
	if (value == _value) {
		return;
	}

	_value = value;
	_panel->setDirty();
	if (byPlayer) {
		sendEvent(kEventSliderChanged);
	}
}

bool Slider::update(float seconds) {
	ControlState state = kStateHidden;
	if (_visible) {
		state = getState();
		if (state == kStateNormal) {
			if (_isPressed) {
				state = kStatePressed;
			} else if (_isMouseOver) {
				state = kStateMouseOver;
			}
		}
	}

	float rate = state == kStatePressed ? 0.0f : kControlBlendRate;
	bool changed = _elements[0].blend(state, seconds, rate);
	if (_hasFill) {
		changed |= _elements[2].blend(state, seconds, rate);
	}

	changed |= _elements[1].blend(state, seconds, rate);
	return changed;
}

void Slider::draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const {
	Common::Rect track = _bounds;
	track.translate(offset.x, offset.y);
	Common::Rect thumb = getThumbBounds();
	thumb.translate(offset.x, offset.y);

	drawSprite(surface, 0, track);

	// The fill is cut off at the thumb instead of getting squeezed
	if (_hasFill && thumb.left > track.left) {
		Common::Rect src = _elements[2]._src;
		src.right = src.left + (thumb.left - track.left);
		drawSprite(surface, 2, src, Common::Rect(track.left, track.top, thumb.left, track.bottom));
	}

	drawSprite(surface, 1, thumb);
}

bool Slider::onMouseDown(const Common::Point &pos) {
	_isPressed = true;

	// Dragging the thumb keeps it where it was grabbed. A click elsewhere on the
	// track moves the value by one, and dragging from there follows the mouse.
	Common::Rect thumb = getThumbBounds();
	int valueX = thumb.left + thumb.width() / 2;
	if (thumb.contains(pos)) {
		_dragOffset = valueX - pos.x;
		return true;
	}

	_dragOffset = 0;
	if (pos.x > valueX) {
		setValue(_value + 1, true);
	} else if (pos.x < valueX) {
		setValue(_value - 1, true);
	}

	return true;
}

void Slider::onMouseMove(const Common::Point &pos) {
	setValue(valueAt(pos.x + _dragOffset), true);
}

void Slider::onMouseUp(const Common::Point &pos) {
	_isPressed = false;
	sendEvent(kEventSliderChanged);
	sendEvent(kEventSliderReleased);
}

bool Slider::doAction(const Action &action) {
	if (action.type == kActionSetValue) {
		setValue((int)action.value, false);
		return true;
	}

	return Control::doAction(action);
}

bool Slider::getValue(int32 eventType, double &value, Common::String &text) const {
	if (eventType != kEventSliderChanged && eventType != kEventSliderReleased) {
		return false;
	}

	value = _value;
	text = _name;
	return true;
}

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy
