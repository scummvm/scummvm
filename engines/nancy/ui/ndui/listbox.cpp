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

#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"

#include "engines/nancy/ui/ndui/listbox.h"
#include "engines/nancy/ui/ndui/scrollbar.h"
#include "engines/nancy/ui/ndui/panel.h"
#include "engines/nancy/ui/ndui/manager.h"

namespace Nancy {
namespace UI {
namespace NDUI {

ListBox::~ListBox() {
	delete _scrollBar;
}

void ListBox::readExtra(Common::SeekableReadStream &stream) {
	// The selection highlight
	_elements.resize(2);
	_elements[1].read(stream);

	stream.skip(4); // control type of the scroll bar
	_scrollBar = new ScrollBar(_panel);
	_scrollBar->read(stream);

	_border = stream.readSint32LE();
	_margin = stream.readSint32LE();

	int16 numItems = stream.readSint16LE();
	for (int16 i = 0; i < numItems; ++i) {
		Common::String item;
		readFilename(stream, item);
		addItem(item, false);
	}

	Common::String selected;
	readFilename(stream, selected);
	for (uint i = 0; i < _items.size(); ++i) {
		if (_items[i].text.equalsIgnoreCase(selected)) {
			_selected = i;
			break;
		}
	}

	updateLayout();
}

void ListBox::addItem(const Common::String &text, bool atStart) {
	Item item;
	item.text = text;
	argbToChannels(_elements[0].getFontColor(kStateNormal), item.color);

	if (atStart) {
		_items.insert_at(0, item);
		if (_selected >= 0) {
			++_selected;
		}
	} else {
		_items.push_back(item);
	}

	_scrollBar->setRange(_items.size());
}

// Rows are as tall as the selection picture, or as the text when there is none
void ListBox::updateLayout() {
	_rowHeight = _elements[1]._src.height() > 0 ? _elements[1]._src.height() - 1 : _elements[0].getFontHeight();
	if (_rowHeight > 0) {
		_scrollBar->setPageSize(getTextBounds().height() / _rowHeight);
	}

	_scrollBar->setRange(_items.size());
}

Common::Rect ListBox::getSelectionBounds() const {
	Common::Rect bounds = _bounds;
	bounds.grow(-_border);
	return bounds;
}

Common::Rect ListBox::getTextBounds() const {
	Common::Rect bounds = getSelectionBounds();
	bounds.left += _margin;
	bounds.right -= _margin;
	return bounds;
}

int ListBox::getItemAt(const Common::Point &pos) const {
	if (_rowHeight <= 0) {
		return -1;
	}

	Common::Rect text = getTextBounds();
	int first = _scrollBar->getPosition();
	int numRows = text.height() / _rowHeight;
	int item = (pos.y - text.top) / _rowHeight + first;
	if (pos.y < text.top || item >= (int)_items.size() || item >= first + numRows) {
		return -1;
	}

	return item;
}

bool ListBox::update(float seconds) {
	bool changed = false;
	if (_visible) {
		changed |= _elements[0].blend(kStateNormal, seconds, kStaticBlendRate);
		changed |= _elements[1].blend(kStatePressed, seconds, kStaticBlendRate);
	}

	float amount = getBlendAmount(seconds, kStaticBlendRate);
	for (uint i = 0; i < _items.size(); ++i) {
		bool isHovered = _isMouseOver && (int)i == _hovered;
		changed |= blendColor(_items[i].color, _elements[0].getFontColor(isHovered ? kStateMouseOver : kStateNormal), amount);
	}

	changed |= _scrollBar->update(seconds);
	return changed;
}

void ListBox::draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const {
	if (!_visible) {
		return;
	}

	Common::Rect bounds = _bounds;
	bounds.translate(offset.x, offset.y);
	drawSprite(surface, 0, bounds);

	if (_rowHeight > 0) {
		Common::Rect selection = getSelectionBounds();
		selection.translate(offset.x, offset.y);
		Common::Rect text = getTextBounds();
		text.translate(offset.x, offset.y);

		Common::Rect row(text.left, text.top, text.right, text.top + _rowHeight);
		for (uint i = _scrollBar->getPosition(); i < _items.size() && row.bottom <= text.bottom; ++i) {
			if ((int)i == _selected) {
				drawSprite(surface, 1, Common::Rect(selection.left, row.top, selection.right, row.bottom));
				_elements[1].drawText(surface, _items[i].text, row, false);
			} else {
				bool isHovered = _isMouseOver && (int)i == _hovered;
				_elements[0].drawText(surface, _items[i].text, row, _items[i].color, isHovered ? kStateMouseOver : kStateNormal, false);
			}

			row.translate(0, _rowHeight);
		}
	}

	_scrollBar->draw(surface, offset);
}

void ListBox::onMouseHover(const Common::Point &pos) {
	Common::Rect text = getTextBounds();
	_hovered = (pos.x >= text.left && pos.x < text.right) ? getItemAt(pos) : -1;
}

bool ListBox::onMouseDown(const Common::Point &pos) {
	if (_scrollBar->containsPoint(pos)) {
		_isScrollBarDragging = _scrollBar->onMouseDown(pos);
		return true;
	}

	int item = getItemAt(pos);
	if (!getSelectionBounds().contains(pos) || item < 0) {
		return false;
	}

	// A second click on the same item, shortly after the first
	static const uint32 kDoubleClickTime = 500;

	const uint32 now = g_nancy->getTotalPlayTime();
	const bool isDoubleClick = item == _lastClicked && now - _lastClickTime <= kDoubleClickTime;
	_lastClicked = isDoubleClick ? -1 : item;
	_lastClickTime = now;

	_selected = item;
	_isDragging = true;
	_panel->setDirty();
	sendEvent(isDoubleClick ? kEventListDoubleClicked : kEventListSelected);
	return true;
}

// Dragging moves the selection, and scrolls when it goes past the visible rows
void ListBox::onMouseMove(const Common::Point &pos) {
	if (_isScrollBarDragging) {
		_scrollBar->onMouseMove(pos);
		return;
	}

	if (!_isDragging || _rowHeight <= 0 || _items.empty()) {
		return;
	}

	Common::Rect text = getTextBounds();
	int first = _scrollBar->getPosition();
	int numRows = text.height() / _rowHeight;
	int item = (pos.y - text.top) / _rowHeight + first;
	if (pos.y < text.top) {
		_scrollBar->scroll(-1);
		item = _scrollBar->getPosition();
	} else if (item >= first + numRows) {
		_scrollBar->scroll(1);
		item = _scrollBar->getPosition() + numRows - 1;
	}

	item = CLIP<int>(item, 0, (int)_items.size() - 1);
	if (item != _selected) {
		_selected = item;
		_panel->setDirty();
		sendEvent(kEventListSelected);
	}
}

void ListBox::onMouseUp(const Common::Point &pos) {
	if (_isScrollBarDragging) {
		_scrollBar->onMouseUp(pos);
	}

	_isScrollBarDragging = false;
	_isDragging = false;
}

bool ListBox::onWheel(int notches) {
	_scrollBar->scroll(-notches * kWheelLines);
	return true;
}

bool ListBox::doAction(const Action &action) {
	switch (action.type) {
	case kActionAddText:
		addItem(action.text, action.value < 0);
		_panel->setDirty();
		return true;
	// From Nancy20 on, "collapse" empties a list box, e.g. the list of saved games
	case kActionCollapse:
	case kActionSetText:
		_items.clear();
		_selected = -1;
		_hovered = -1;
		_scrollBar->setRange(0);
		_panel->setDirty();
		return true;
	case kActionSetValue:
		_selected = CLIP<int>((int)action.value, -1, (int)_items.size() - 1);
		_panel->setDirty();
		if (_selected != -1) {
			sendEvent(kEventListSelected);
		}

		return true;
	default:
		return Control::doAction(action);
	}
}

bool ListBox::getValue(int32 eventType, double &value, Common::String &text) const {
	if (eventType != kEventListSelected && eventType != kEventListDoubleClicked) {
		return false;
	}

	value = _selected;
	text = _selected >= 0 ? _items[_selected].text : Common::String();
	return true;
}

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy
