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

#include "engines/nancy/ui/ndui/control.h"
#include "engines/nancy/ui/ndui/panel.h"
#include "engines/nancy/ui/ndui/manager.h"

namespace Nancy {
namespace UI {
namespace NDUI {

void Control::read(Common::SeekableReadStream &stream) {
	stream.skip(33); // version

	_initialState = stream.readSint32LE();
	if (_initialState == kStartDisabled) {
		_enabled = false;
	} else if (_initialState == kStartHidden) {
		_visible = false;
	}

	readFilename(stream, _name);
	readExclusiveRect(stream, _bounds);
	readFilename(stream, _text);
	_autoHeight = _bounds.height() == 0;

	_events.resize(stream.readSint16LE());
	for (Action &event : _events) {
		event.read(stream);
	}

	_elements.resize(1);
	_elements[0].read(stream);

	readExtra(stream);

	if (_autoHeight) {
		fitHeightToText();
	}
}

bool Control::takeAction(const Action &action) {
	return _name.equalsIgnoreCase(action.target) && doAction(action);
}

bool Control::doAction(const Action &action) {
	switch (action.type) {
	case kActionShow:
		setVisible(true);
		break;
	case kActionHide:
		setVisible(false);
		break;
	case kActionToggle:
		setVisible(!_visible);
		break;
	case kActionEnable:
		setEnabled(true);
		break;
	case kActionDisable:
		setEnabled(false);
		break;
	case kActionToggleEnabled:
		setEnabled(!_enabled);
		break;
	default:
		return false;
	}

	return true;
}

void Control::setVisible(bool visible) {
	if (_visible != visible) {
		_visible = visible;
		_isMouseOver = false;
		_isPressed = false;
		_panel->arrangeControls();
	}
}

void Control::setEnabled(bool enabled) {
	if (_enabled != enabled) {
		_enabled = enabled;
		_panel->setDirty();
	}
}

void Control::setText(const Common::String &text) {
	if (_text == text) {
		return;
	}

	_text = text;
	_panel->setDirty();

	if (_autoHeight) {
		fitHeightToText();
		_panel->arrangeControls();
	}
}

void Control::fitHeightToText() {
	_bounds.bottom = _bounds.top + MAX(_elements[0].measureTextHeight(_text, getTextWidth()), getMinHeight());
}

void Control::fireEvents(int32 eventType, bool isOwnEvent, double value, const Common::String &text) {
	for (const Action &event : _events) {
		if (event.eventType != eventType && !(isOwnEvent && event.eventType == kEventOwn)) {
			continue;
		}

		Action action = event;
		action.sender = _name;
		if (!action.hasValue) {
			action.hasValue = true;
			action.value = value;
			action.text = text;
		}

		_panel->getManager()->takeAction(action);
	}
}

void Control::sendEvent(int32 eventType) {
	double value = 0;
	Common::String text;
	if (getValue(eventType, value, text)) {
		fireEvents(eventType, isOwnEvent(eventType), value, text);
	}
}

ControlState Control::getState() const {
	if (!_enabled || !_panel->isEnabled()) {
		return kStateDisabled;
	}

	return kStateNormal;
}

void Control::drawSprite(Graphics::ManagedSurface &surface, uint element, const Common::Rect &dest) const {
	drawSprite(surface, element, _elements[element]._src, dest);
}

void Control::drawSprite(Graphics::ManagedSurface &surface, uint element, const Common::Rect &src, const Common::Rect &dest) const {
	const Element &elem = _elements[element];
	if (elem._textureName.empty()) {
		return;
	}

	const Graphics::ManagedSurface *texture = _panel->getManager()->getTexture(elem._textureName);
	if (texture) {
		elem.drawSprite(surface, src, dest, *texture);
	}
}

bool Static::update(float seconds) {
	if (!_visible) {
		return false;
	}

	return _elements[0].blend(getState(), seconds, kStaticBlendRate);
}

void Static::draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const {
	if (!_visible) {
		return;
	}

	Common::Rect dest = _bounds;
	dest.translate(offset.x, offset.y);
	drawSprite(surface, 0, dest);
	_elements[0].drawText(surface, _text, dest, true);
}

bool Static::doAction(const Action &action) {
	switch (action.type) {
	case kActionAddText:
		setText(_text + action.text);
		return true;
	case kActionSetText:
		setText(action.text);
		return true;
	case kActionSetValue:
		setVisible(action.value != 0);
		return true;
	case kActionSetTexture:
		_elements[0].setTextureName(action.text);
		_panel->setDirty();
		return true;
	default:
		return Control::doAction(action);
	}
}

void VarStatic::readExtra(Common::SeekableReadStream &stream) {
	_width = stream.readSint32LE();
	_precision = stream.readSint32LE();
}

bool VarStatic::doAction(const Action &action) {
	if (action.type == kActionSetValue) {
		setText(Common::String::format("%0*.*f", (int)_width, (int)_precision, action.value));
		return true;
	}

	return Static::doAction(action);
}

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy
