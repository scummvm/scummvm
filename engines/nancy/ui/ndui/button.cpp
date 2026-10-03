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

#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"

#include "engines/nancy/ui/ndui/button.h"
#include "engines/nancy/ui/ndui/panel.h"
#include "engines/nancy/ui/ndui/manager.h"

namespace Nancy {
namespace UI {
namespace NDUI {

void Button::readExtra(Common::SeekableReadStream &stream) {
	Common::String textureName;
	Common::Rect src;
	readFilename(stream, textureName);
	readExclusiveRect(stream, src);

	_elements.resize(2);
	_elements[1].setTexture(textureName, src);

	_pressedOffset.x = stream.readSint32LE();
	_pressedOffset.y = stream.readSint32LE();
}

ControlState Button::getButtonState() const {
	if (!_visible) {
		return kStateHidden;
	}

	ControlState state = getState();
	if (state != kStateNormal) {
		return state;
	}

	if (_isPressed) {
		return kStatePressed;
	}

	return _isMouseOver ? kStateMouseOver : kStateNormal;
}

// Buttons keep being drawn after they get hidden, until they have faded out
bool Button::update(float seconds) {
	ControlState state = getButtonState();
	float rate = state == kStatePressed ? 0.0f : kControlBlendRate;

	bool changed = _elements[0].blend(state, seconds, rate);
	changed |= _elements[1].blend(state, seconds, rate);
	return changed;
}

void Button::draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const {
	Common::Rect dest = _bounds;
	dest.translate(offset.x, offset.y);
	if (_isPressed) {
		dest.translate(_pressedOffset.x, _pressedOffset.y);
	}

	drawSprite(surface, 0, dest);
	_elements[0].drawText(surface, _text, dest, false);
	drawSprite(surface, 1, dest);
	_elements[1].drawText(surface, _text, dest, false);
}

bool Button::onMouseDown(const Common::Point &pos) {
	_isPressed = true;
	return true;
}

void Button::onMouseUp(const Common::Point &pos) {
	_isPressed = false;
	if (_visible && _enabled && _bounds.contains(pos)) {
		sendEvent(kEventClicked);
	}
}

bool Button::doAction(const Action &action) {
	switch (action.type) {
	case kActionAddText:
		setText(_text + action.text);
		return true;
	case kActionSetText:
		setText(action.text);
		return true;
	default:
		return Control::doAction(action);
	}
}

bool Button::getValue(int32 eventType, double &value, Common::String &text) const {
	if (eventType != kEventClicked) {
		return false;
	}

	value = _id;
	text = _name;
	return true;
}

void InvButton::readExtra(Common::SeekableReadStream &stream) {
	Button::readExtra(stream);

	if (g_nancy->getGameType() <= kGameTypeNancy16) {
		_itemID = stream.readSint16LE();
		_kind = stream.readSint32LE();
		_scene = stream.readSint32LE();
		_valueIndex = stream.readSint16LE();
	}

	// The count gets drawn over the item picture
	_elements[0].moveFontTo(_elements[1]);
}

void InvButton::setItem(const INVD::Item &item) {
	_name = item.name;
	_itemID = item.id;
	_kind = item.kind;
	_valueIndex = item.valueIndex;

	// The scene is named after its number, e.g. "s6200"
	const Common::String sceneName = item.sceneName.toString();
	_scene = sceneName.size() > 1 ? atoi(sceneName.c_str() + 1) : 0;

	_elements[1].setTexture(_elements[1]._textureName, item.sourceRect);
}

bool InvButton::doAction(const Action &action) {
	if (action.type == kActionSetValue) {
		setText(Common::String::format(" %d ", (int)action.value));
		return true;
	}

	return Button::doAction(action);
}

bool InvButton::getValue(int32 eventType, double &value, Common::String &text) const {
	if (eventType != kEventClicked) {
		return false;
	}

	value = _itemID;
	text = _name;
	return true;
}

void CheckBox::readCheckTexture(Common::SeekableReadStream &stream) {
	Common::String textureName;
	Common::Rect src;
	readFilename(stream, textureName);
	readExclusiveRect(stream, src);

	_elements.resize(2);
	_elements[1].setTexture(textureName, src);
}

void CheckBox::readExtra(Common::SeekableReadStream &stream) {
	readCheckTexture(stream);
	_checked = stream.readByte() == 1;
}

// The box is as large as its picture, at the left of the control, and the text
// follows it
int CheckBox::getTextWidth() const {
	return _bounds.width() - (int)(1.25f * _elements[0]._src.width());
}

Common::Rect CheckBox::getBoxBounds() const {
	const Common::Rect &src = _elements[0]._src;
	Common::Rect box(src.width(), src.height());
	box.moveTo(_bounds.left, _bounds.top + (_bounds.height() - src.height()) / 2);
	return box;
}

bool CheckBox::update(float seconds) {
	ControlState state = kStateHidden;
	if (_visible) {
		state = getState();
		if (state == kStateNormal) {
			if (_isPressed || _checked) {
				state = kStatePressed;
			} else if (_isMouseOver) {
				state = kStateMouseOver;
			}
		}
	}

	float rate = state == kStatePressed ? 0.0f : kControlBlendRate;
	bool changed = _elements[0].blend(state, seconds, rate);
	changed |= _elements[1].blend(_checked ? state : kStateHidden, seconds, rate);
	return changed;
}

void CheckBox::draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const {
	Common::Rect box = getBoxBounds();
	box.translate(offset.x, offset.y);

	Common::Rect textBounds = _bounds;
	textBounds.translate(offset.x, offset.y);
	textBounds.left += (int)(1.25f * box.width());

	drawSprite(surface, 0, box);
	_elements[0].drawText(surface, _text, textBounds, true);
	drawSprite(surface, 1, box);
}

bool CheckBox::onMouseDown(const Common::Point &pos) {
	_isPressed = true;
	return true;
}

void CheckBox::onMouseUp(const Common::Point &pos) {
	_isPressed = false;
	if (_visible && _enabled && _bounds.contains(pos)) {
		onClicked();
	}
}

void CheckBox::onClicked() {
	_checked = !_checked;
	sendEvent(_checked ? kEventChecked : kEventUnchecked);
}

bool CheckBox::doAction(const Action &action) {
	switch (action.type) {
	case kActionSetText:
		setText(action.text);
		return true;
	case kActionSetValue:
		_checked = action.value != 0;
		_panel->setDirty();
		return true;
	default:
		return Control::doAction(action);
	}
}

bool CheckBox::getValue(int32 eventType, double &value, Common::String &text) const {
	if (eventType != kEventChecked && eventType != kEventUnchecked) {
		return false;
	}

	value = _checked ? 1 : 0;
	text = _name;
	return true;
}

bool CheckBox::isOwnEvent(int32 eventType) const {
	return eventType == (_checked ? kEventChecked : kEventUnchecked);
}

void RadioButton::readExtra(Common::SeekableReadStream &stream) {
	readCheckTexture(stream);

	// Buttons read as part of a group already belong to it
	Common::String group;
	readFilename(stream, group);
	if (!group.empty()) {
		_group = group;
	}
}

void RadioButton::onClicked() {
	if (!_checked) {
		_panel->uncheckRadioGroup(_group);
		_checked = true;
		sendEvent(kEventRadioChanged);
	}
}

bool RadioButton::doAction(const Action &action) {
	if (action.type == kActionSetValue) {
		if (action.value != 0) {
			_panel->uncheckRadioGroup(_group);
		}

		_checked = action.value != 0;
		_panel->setDirty();
		return true;
	}

	return CheckBox::doAction(action);
}

// The value is the position of the checked button in its group
bool RadioButton::getValue(int32 eventType, double &value, Common::String &text) const {
	if (eventType != kEventRadioChanged) {
		return false;
	}

	value = (uint32)_panel->getCheckedRadioIndex(_group);
	text = _name;
	return true;
}

void RadioGroup::readExtra(Common::SeekableReadStream &stream) {
	Common::String selected;
	readFilename(stream, selected);

	int16 numButtons = stream.readSint16LE();
	for (int16 i = 0; i < numButtons; ++i) {
		stream.skip(4); // control type
		RadioButton *button = new RadioButton(_panel);
		button->setGroup(_name);
		button->read(stream);
		button->setChecked(button->getName().equalsIgnoreCase(selected));
		_panel->addRadioButton(button);
	}
}

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy
