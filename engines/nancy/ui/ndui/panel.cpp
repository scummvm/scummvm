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
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "common/memstream.h"

#include "engines/nancy/ui/ndui/panel.h"
#include "engines/nancy/ui/ndui/control.h"
#include "engines/nancy/ui/ndui/button.h"
#include "engines/nancy/ui/ndui/slider.h"
#include "engines/nancy/ui/ndui/scrollbar.h"
#include "engines/nancy/ui/ndui/listbox.h"
#include "engines/nancy/ui/ndui/editbox.h"
#include "engines/nancy/ui/ndui/manager.h"

namespace Nancy {
namespace UI {
namespace NDUI {

// Inventory buttons are numbered after their items
static const int kInvButtonFirstID = 10;

Panel::Panel(Manager *manager, uint16 z, byte kind) : RenderObject(z), _manager(manager), _kind(kind) {
	setNativeResolution(true);
}

Panel::~Panel() {
	delete _background;
	delete _tooltip;
	delete _objectFactory;

	for (Control *control : _controls) {
		delete control;
	}
}

void Panel::read(Common::SeekableReadStream &stream) {
	readHeader(stream);

	if (stream.readSint32LE() == kControlStatic) {
		readBackground(stream);
	}

	if (g_nancy->getGameType() >= kGameTypeNancy17 && stream.readSint32LE() == kControlTooltip) {
		_tooltip = new Tooltip(this);
		_tooltip->read(stream);
	}

	int16 numControls = stream.readSint16LE();
	for (int16 i = 0; i < numControls; ++i) {
		readControl(stream);
	}

	if (_kind == kObjectScrollPanel || _kind == kObjectInvPanel) {
		readScrolling(stream);
	}

	place();
}

void Panel::readHeader(Common::SeekableReadStream &stream) {
	stream.skip(33); // version

	_initialState = stream.readSint32LE();
	_arrangement = (Arrangement)stream.readSint32LE();
	stream.skip(4);
	_spacing.x = stream.readSint32LE();
	_spacing.y = stream.readSint32LE();
	readExclusiveRect(stream, _margins);
	_backgroundColor = stream.readUint32BE();
	stream.skip(g_nancy->getGameType() >= kGameTypeNancy17 ? 2 : 1);

	// The background is a static that also gives the panel its name, position and events
	_background = new Static(this);

	// Nothing gets drawn until the panel has a place on the screen
	setVisible(false);

	switch (_initialState) {
	case kStartVisible:
		_showCount = 1;
		break;
	case kStartDisabled:
		_showCount = 1;
		_enabled = false;
		break;
	default:
		break;
	}
}

void Panel::readScrolling(Common::SeekableReadStream &stream) {
	readFilename(stream, _scrollBarName);
	_scrollStep = MAX<int32>(stream.readSint32LE(), 1);
}

void Panel::readBackground(Common::SeekableReadStream &stream) {
	_background->read(stream);
}

void Panel::place() {
	Common::Rect screenPosition = _background->_bounds;
	_background->_bounds.moveTo(0, 0);

	moveTo(screenPosition);
	_drawSurface.create(screenPosition.width(), screenPosition.height(), g_nancy->_graphics->getTransparentPixelFormat());
	_isPlaced = true;

	setVisible(_showCount > 0);
	growToControls();
	arrangeControls();
}

// Controls aren't clipped to a panel that doesn't scroll, and some are placed past
// the right or the bottom of its background, like the meters of the hypothermia
// dialog, so the panel is as large as it takes to draw them. Panels that arrange
// their controls keep their width, since the rows are laid out by it.
void Panel::growToControls() {
	if (!_scrollBarName.empty() || _arrangement != kArrangeNone) {
		return;
	}

	Common::Rect screenPosition = _screenPosition;
	for (const Control *control : _controls) {
		Common::Rect drawBounds = control->getDrawBounds();
		screenPosition.right = MAX<int16>(screenPosition.right, screenPosition.left + drawBounds.right);
		screenPosition.bottom = MAX<int16>(screenPosition.bottom, screenPosition.top + drawBounds.bottom);
	}

	if (screenPosition != _screenPosition) {
		moveTo(screenPosition);
		_drawSurface.create(screenPosition.width(), screenPosition.height(), g_nancy->_graphics->getTransparentPixelFormat());
		_isDirty = true;
	}
}

static void writeTooltipName(Common::WriteStream &out, const char *name) {
	char buf[33] = {};
	Common::strlcpy(buf, name, sizeof(buf));
	out.write(buf, sizeof(buf));
}

void Panel::addBuiltInTooltip() {
	// A tooltip comes up after the mouse has been over a control for a second, and
	// stays for ten at most. Setting a flag keeps tooltips from showing.
	static const double kShowAfter = 1.0;
	static const double kHideAfter = 10.0;
	static const char *const kFontName = "UIFont";

	// White text with nothing behind it
	static const uint32 kBackgroundColor = 0x00000000;
	static const uint32 kTextColor = 0xFFFFFFFF;

	// Centered, and on a single line
	static const int32 kAlignCenter = 2;

	// Nancy20 ends the names of all flags in "_FL"
	const char *preventFlagName = g_nancy->getGameType() >= kGameTypeNancy20 ? "PreventTooltips_FL" : "PreventTooltips";

	int16 preventFlag = -1;
	const EVNT *flags = GetEngineData(EVNT);
	if (flags) {
		for (uint i = 0; i < flags->eventFlagNames.size(); ++i) {
			if (flags->eventFlagNames[i].equalsIgnoreCase(preventFlagName)) {
				preventFlag = (int16)(EVNT::kFirstEventFlag + i);
			}
		}
	}

	// Written out the way an NDUI chunk describes a tooltip
	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	writeTooltipName(data, "");			// version
	data.writeSint32LE(kStartVisible);
	writeTooltipName(data, "");			// name
	for (uint i = 0; i < 4; ++i) {
		data.writeSint32LE(0);			// bounds
	}

	writeTooltipName(data, "");			// text
	writeTooltipName(data, "");			// tooltip
	data.writeSint16LE(0);				// events

	writeTooltipName(data, "");			// texture
	for (uint i = 0; i < 4; ++i) {
		data.writeSint32LE(0);			// part of the texture
	}

	for (uint i = 0; i < 4; ++i) {
		data.writeUint32BE(kBackgroundColor);
	}

	writeTooltipName(data, kFontName);
	data.writeByte(0);					// bold
	data.writeSint32LE(kAlignCenter);
	data.writeSint32LE(kAlignCenter);
	data.writeByte(0);					// word wrap
	for (uint i = 0; i < 4; ++i) {
		data.writeUint32BE(kTextColor);
	}

	data.writeByte(0);

	data.writeDoubleLE(kShowAfter);
	data.writeDoubleLE(kHideAfter);
	data.writeSint16LE(preventFlag);

	Common::MemoryReadStream stream(data.getData(), data.size());
	delete _tooltip;
	_tooltip = new Tooltip(this);
	_tooltip->read(stream);
}

bool Panel::hasControl(const Common::String &name) const {
	for (const Control *control : _controls) {
		if (control->getName().equalsIgnoreCase(name)) {
			return true;
		}
	}

	return false;
}

bool Panel::createObject(const Common::String &name, const Common::String &templateName) {
	return _objectFactory && _objectFactory->createObject(name, templateName);
}

// The script adds a button like any other, which becomes the item's on its way in
void Panel::addItemButton(const INVD::Item &item) {
	_kind = kObjectInvPanel;
	_newItem = &item;

	if (!createObject(item.name, "InvButton")) {
		warning("Couldn't create the UI button of item %s", item.name.c_str());
	}

	_newItem = nullptr;
}

void Panel::readControl(Common::SeekableReadStream &stream) {
	int32 type = stream.readSint32LE();
	addNewControl(type, stream);
}

Control *Panel::addNewControl(int32 type, Common::SeekableReadStream &stream) {
	Control *control = nullptr;
	switch (type) {
	case kControlButton:
		if (_newItem) {
			InvButton *button = new InvButton(this);
			button->read(stream);
			button->setItem(*_newItem);
			button->setVisible(false);
			addControl(button);
			return button;
		}

		control = new Button(this);
		break;
	case kControlStatic:
		control = new Static(this);
		break;
	case kControlSlider:
		control = new Slider(this);
		break;
	case kControlRadioGroup:
		control = new RadioGroup(this);
		break;
	case kControlRadioButton: {
		RadioButton *button = new RadioButton(this);
		button->read(stream);
		addRadioButton(button);
		return button;
	}
	case kControlListBox:
		control = new ListBox(this);
		break;
	case kControlScrollBar:
		control = new ScrollBar(this);
		break;
	case kControlCheckBox:
		control = new CheckBox(this);
		break;
	case kControlInvButton:
		control = new InvButton(this);
		break;
	case kControlVarStatic:
		control = new VarStatic(this);
		break;
	case kControlEditBox: {
		EditBox *editBox = new EditBox(this);
		_editBoxes.push_back(editBox);
		control = editBox;
		break;
	}
	default:
		error("Unknown NDUI control type %d", type);
	}

	control->read(stream);
	addControl(control);
	return control;
}

void Panel::addItemButtons(const Common::Array<byte> &buttonTemplate) {
	const INVD *items = GetEngineData(INVD);
	if (!items) {
		return;
	}

	_kind = kObjectInvPanel;

	for (const INVD::Item &item : items->items) {
		Common::MemoryReadStream stream(buttonTemplate.data(), buttonTemplate.size());
		stream.skip(33 + 4); // version and control type

		InvButton *button = new InvButton(this);
		button->read(stream);
		button->setItem(item);
		button->setVisible(false);
		addControl(button);
	}

	arrangeControls();
}

// Controls are numbered in the order they get added; inventory buttons after their item
void Panel::addControl(Control *control) {
	if (_kind == kObjectInvPanel) {
		InvButton *button = (InvButton *)control;
		control->_id = button->getItemID() + kInvButtonFirstID;

		if (button->getKind() == InvButton::kItemOpenCountedScene) {
			_manager->registerValueDisplay(button->getValueIndex(), button->getName());
		}
	} else {
		control->_id = _nextID++;
	}

	_controls.push_back(control);
	_arrangeOrder.push_back(control);
}

void Panel::addRadioButton(RadioButton *button) {
	addControl(button);
	_radioButtons.push_back(button);
}

const Common::String &Panel::getName() const {
	return _background->getName();
}

void Panel::arrangeControls() {
	_isDirty = true;

	if (_arrangement == kArrangeNone) {
		return;
	}

	int width = _screenPosition.width();
	int rowStart = _arrangement == kArrangeLeftToRight ? _margins.left : width - _margins.right;
	int x = rowStart;
	int y = _margins.top;
	int rowBottom = y;

	// Controls go one after another along a row, and start a new row when they don't fit;
	// one too wide for a row still goes on it when the row is empty
	for (Control *control : _arrangeOrder) {
		if (!control->isVisible()) {
			continue;
		}

		int controlWidth = control->_bounds.width();
		int controlHeight = control->_bounds.height();

		if (_arrangement == kArrangeLeftToRight) {
			if (x != rowStart && x + controlWidth > width - _margins.right) {
				x = rowStart;
				y = rowBottom + _spacing.y;
			}

			control->_bounds.moveTo(x, y);
			x += controlWidth + _spacing.x;
		} else {
			if (x != rowStart && x - controlWidth < _margins.left) {
				x = rowStart;
				y = rowBottom + _spacing.y;
			}

			control->_bounds.moveTo(x - controlWidth, y);
			x -= controlWidth + _spacing.x;
		}

		rowBottom = MAX(rowBottom, y + controlHeight);
	}

	if (!_scrollBarName.empty()) {
		// Scroll panels can share a scroll bar, which covers what the controls
		// of the visible one take up, to the end of the last scroll step
		int contentHeight = rowBottom;
		int overflow = contentHeight - _screenPosition.height();
		if (overflow <= 0) {
			_scrollOffset = 0;
		} else if (overflow % _scrollStep) {
			contentHeight += _scrollStep - overflow % _scrollStep;
		}

		if (isVisible()) {
			_manager->takeAction(kActionSetRange, _scrollBarName, (double)contentHeight);
		}
	} else if (rowBottom > _drawSurface.h) {
		// Controls aren't clipped to a panel that doesn't scroll, so it grows to hold them
		Common::Rect screenPosition = _screenPosition;
		screenPosition.bottom = screenPosition.top + rowBottom;
		moveTo(screenPosition);
		_drawSurface.create(screenPosition.width(), screenPosition.height(), g_nancy->_graphics->getTransparentPixelFormat());
	}
}

bool Panel::takeAction(const Action &action) {
	// A panel whose script never got to its background has nothing to act on
	if (!_isPlaced) {
		return false;
	}

	if (!getName().equalsIgnoreCase(action.target)) {
		for (Control *control : _controls) {
			if (control->takeAction(action)) {
				return true;
			}
		}

		return false;
	}

	switch (action.type) {
	case kActionShow:
		show();
		break;
	case kActionHide:
		hide();
		break;
	case kActionToggle:
		if (isVisible()) {
			hide();
		} else {
			show();
		}

		break;
	case kActionEnable:
		_enabled = true;
		fireEvent(kEventEnable);
		break;
	case kActionDisable:
		_enabled = false;
		fireEvent(kEventDisable);
		break;
	case kActionExpand:
		setCollapsed(false);
		break;
	case kActionCollapse:
		setCollapsed(true);
		break;
	case kActionToggleCollapsed:
		setCollapsed(!_isCollapsed);
		break;
	case kActionSetValue:
		// A scroll panel follows its scroll bar, a whole step at a time
		if (_scrollBarName.empty()) {
			return false;
		}

		_scrollOffset = ((int)action.value + _scrollStep / 2) / _scrollStep * _scrollStep;

		// The scroll bar starts over when something else moves the panel
		if (!action.sender.equalsIgnoreCase(_scrollBarName)) {
			_manager->takeAction(kActionSetRange, _scrollBarName, action.value);
		}

		break;
	case kActionShowItem:
	case kActionHideItem:
		if (_kind != kObjectInvPanel) {
			return false;
		}

		showItem((int16)action.value, action.type == kActionShowItem);
		break;
	default:
		return false;
	}

	_isDirty = true;
	return true;
}

// The newest item goes first
void Panel::showItem(int16 itemID, bool show) {
	for (uint i = 0; i < _arrangeOrder.size(); ++i) {
		Control *control = _arrangeOrder[i];
		if (((InvButton *)control)->getItemID() != itemID) {
			continue;
		}

		if (show) {
			_arrangeOrder.remove_at(i);
			_arrangeOrder.insert_at(0, control);
		}

		control->setVisible(show);
		arrangeControls();
		return;
	}
}

void Panel::saveState(Common::Array<UIStateData::Change> &changes) const {
	if (!_savesState || !_isPlaced) {
		return;
	}

	// Which inventory buttons are shown follows from the items of the character
	if (_kind != kObjectInvPanel) {
		for (const Control *control : _controls) {
			addStateChanges(changes, control->getName(), control->getInitialState(), control->isVisible(), control->isEnabled());
		}
	}

	addStateChanges(changes, getName(), _initialState, isVisible(), _enabled);
}

void Panel::finishBlending() {
	if (!_isPlaced) {
		return;
	}

	_background->update(kFinishBlendingTime);
	for (Control *control : _controls) {
		control->update(kFinishBlendingTime);
	}

	_isDirty = true;
}

// Every show needs a matching hide before the panel goes away
void Panel::show() {
	++_showCount;
	if (_isCollapsed) {
		_isShownWhenExpanded = true;
	} else {
		setVisible(true);
	}

	arrangeControls();
	fireEvent(kEventShow);
}

void Panel::setCollapsed(bool isCollapsed) {
	if (isCollapsed == _isCollapsed) {
		return;
	}

	_isCollapsed = isCollapsed;
	if (isCollapsed) {
		_isShownWhenExpanded = isVisible();
		setVisible(false);
		if (_capture) {
			_capture->_isPressed = false;
			_capture = nullptr;
		}
	} else {
		setVisible(_isShownWhenExpanded);
	}
}

void Panel::hide() {
	--_showCount;
	if (_showCount == 0) {
		setVisible(false);
		_isShownWhenExpanded = false;
		if (_capture) {
			_capture->_isPressed = false;
			_capture = nullptr;
		}

		fireEvent(kEventHide);
	} else if (_showCount < 0) {
		_showCount = 0;
	}
}

void Panel::fireEvent(int32 eventType) {
	_background->fireEvents(eventType, false, 0, getName());

	for (Control *control : _controls) {
		control->fireEvents(eventType, false, 0, control->getName());
	}
}

bool Panel::removeControl(const Common::String &name) {
	for (uint i = 0; i < _controls.size(); ++i) {
		Control *control = _controls[i];
		if (!control->getName().equalsIgnoreCase(name)) {
			continue;
		}

		_controls.remove_at(i);

		for (uint j = 0; j < _arrangeOrder.size(); ++j) {
			if (_arrangeOrder[j] == control) {
				_arrangeOrder.remove_at(j);
				break;
			}
		}

		for (uint j = 0; j < _radioButtons.size(); ++j) {
			if (_radioButtons[j] == control) {
				_radioButtons.remove_at(j);
				break;
			}
		}

		for (uint j = 0; j < _editBoxes.size(); ++j) {
			if (_editBoxes[j] == control) {
				_editBoxes.remove_at(j);
				break;
			}
		}

		if (_capture == control) {
			_capture = nullptr;
		}

		delete control;
		arrangeControls();
		return true;
	}

	return false;
}

EditBox *Panel::getKeyboardFocus() const {
	if (!isVisible() || !_enabled) {
		return nullptr;
	}

	for (EditBox *editBox : _editBoxes) {
		if (editBox->isVisible() && editBox->isEnabled()) {
			return editBox;
		}
	}

	return nullptr;
}

bool Panel::isItemButton(const Common::String &name) const {
	if (_kind != kObjectInvPanel) {
		return false;
	}

	for (const Control *control : _controls) {
		if (control->getName().equalsIgnoreCase(name)) {
			return true;
		}
	}

	return false;
}

InvButton *Panel::findItemButton(int16 itemID) const {
	if (_kind != kObjectInvPanel) {
		return nullptr;
	}

	for (Control *control : _controls) {
		InvButton *button = (InvButton *)control;
		if (button->getItemID() == itemID) {
			return button;
		}
	}

	return nullptr;
}

void Panel::uncheckRadioGroup(const Common::String &group) {
	for (RadioButton *button : _radioButtons) {
		if (button->getGroup().equalsIgnoreCase(group)) {
			button->setChecked(false);
		}
	}

	_isDirty = true;
}

int Panel::getCheckedRadioIndex(const Common::String &group) const {
	int index = 0;
	for (RadioButton *button : _radioButtons) {
		if (!button->getGroup().equalsIgnoreCase(group)) {
			continue;
		}

		if (button->isChecked()) {
			return index;
		}

		++index;
	}

	return -1;
}

void Panel::updateGraphics() {
	uint32 now = g_nancy->getTotalPlayTime();
	if (_lastUpdateTime == 0) {
		_lastUpdateTime = now;
	}

	float seconds = (now - _lastUpdateTime) / 1000.0f;
	_lastUpdateTime = now;

	if (!isVisible()) {
		return;
	}

	for (Control *control : _controls) {
		control->fireEvents(kEventUpdate, false, 0, control->getName());
	}

	bool changed = _background->update(seconds);
	for (Control *control : _controls) {
		changed |= control->update(seconds);
	}

	if (changed || _isDirty) {
		redraw();
	}
}

void Panel::redraw() {
	_drawSurface.clear(0);

	if (_backgroundColor >> 24) {
		_drawSurface.fillRect(_background->_bounds, argbToColor(_drawSurface.format, _backgroundColor));
	}

	_background->draw(_drawSurface, Common::Point());
	for (Control *control : _controls) {
		control->draw(_drawSurface, Common::Point(0, -_scrollOffset));
	}

	_isDirty = false;
	_needsRedraw = true;
}

Control *Panel::getControlAt(const Common::Point &pos) const {
	for (Control *control : _controls) {
		if (control->isVisible() && control->isEnabled() && control->containsPoint(pos)) {
			return control;
		}
	}

	return nullptr;
}

bool Panel::handleInput(NancyInput &input, bool canHover, bool &isOverControl) {
	isOverControl = false;
	Common::Point pos(input.screenMousePos.x - _screenPosition.left, input.screenMousePos.y - _screenPosition.top + _scrollOffset);

	// A control that got pressed keeps the mouse until it is released
	if (_capture) {
		Control *capture = _capture;
		if (input.input & NancyInput::kLeftMouseButtonHeld) {
			capture->onMouseMove(pos);
		} else {
			_capture = nullptr;
			capture->onMouseUp(pos);
		}

		_isDirty = true;
		isOverControl = true;
		return true;
	}

	// The panel can be larger than its background, to hold controls that reach past it;
	// the mouse is only over the panel when it is over the background or one of those
	bool isInside = canHover && isVisible() && _screenPosition.contains(input.screenMousePos);
	Control *hovered = (isInside && _enabled) ? getControlAt(pos) : nullptr;
	if (isInside && !hovered) {
		Common::Point unscrolledPos(pos.x, pos.y - _scrollOffset);
		isInside = _capturesClicks && _background->_bounds.contains(unscrolledPos);
	}

	for (Control *control : _controls) {
		if (control->_isMouseOver != (control == hovered)) {
			control->_isMouseOver = control == hovered;
			_isDirty = true;
		}
	}

	if (!isInside) {
		return false;
	}

	if (hovered && _tooltip && !hovered->getTooltip().empty()) {
		_manager->setHoveredTooltip(hovered, _tooltip);
	}

	int notches = 0;
	if (input.input & NancyInput::kMouseWheelUp) {
		notches = 1;
	} else if (input.input & NancyInput::kMouseWheelDown) {
		notches = -1;
	}

	if (hovered) {
		isOverControl = true;
		hovered->onMouseHover(pos);

		if ((input.input & NancyInput::kLeftMouseButtonDown) && hovered->onMouseDown(pos)) {
			_capture = hovered;
		}

		if (notches && hovered->onWheel(notches)) {
			notches = 0;
		}

		_isDirty = true;
	} else if (_enabled && (input.input & NancyInput::kLeftMouseButtonUp)) {
		// A click on the panel itself reports no value
		_background->fireEvents(kEventClicked, false, -1, getName());
	}

	// The wheel scrolls a scroll panel by one step
	if (notches && !_scrollBarName.empty()) {
		_manager->takeAction(kActionSetValue, _scrollBarName, (double)(notches * _scrollStep));
	}

	return true;
}

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy
