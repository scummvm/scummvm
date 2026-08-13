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

#include "common/events.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/managed_surface.h"
#include "graphics/primitives.h"
#include "mads/mads.h"
#include "mads/nebular/mac_dialogs.h"
#include "mads/nebular/mac_menus.h"
#include "mads/nebular/mac_resources.h"
#include "mads/nebular/nebular.h"

namespace MADS {
namespace RexNebular {

MacNebularDialog::Item::Item() : type(kUserItem), enabled(false),
		checked(false), listItem(false), hasUndo(false), listSelection(-1),
		listTop(0), caret(0), selectionStart(0), selectionEnd(0),
		maxLength(0) {
}

MacNebularDialog::MacNebularDialog(RexNebularEngine &engine,
		MacResourceProvider &resources, Graphics::ManagedSurface &screen,
		Graphics::MacWindowManager &windowManager, MacNebularMenu *menus) :
		_engine(engine),
		_resources(resources), _screen(screen), _windowManager(windowManager),
		_menus(menus), _font(resources.getDialogFont()), _focusItem(0),
		_pressedItem(0),
		_defaultItem(0), _cancelItem(0), _redraw(true),
		_inlineEditable(false) {
}

MacNebularDialog::Item *MacNebularDialog::getItem(int itemNumber) {
	if (itemNumber < 1 || (uint)itemNumber > _items.size())
		return nullptr;
	return &_items[itemNumber - 1];
}

const MacNebularDialog::Item *MacNebularDialog::getItem(
		int itemNumber) const {
	if (itemNumber < 1 || (uint)itemNumber > _items.size())
		return nullptr;
	return &_items[itemNumber - 1];
}

Common::Rect MacNebularDialog::getItemBounds(const Item &item) const {
	Common::Rect bounds(item.bounds);
	bounds.translate(_bounds.left, _bounds.top);
	return bounds;
}

bool MacNebularDialog::loadDialogResource(uint16 resourceID,
		uint16 &itemResourceID) {
	Common::SeekableReadStream *stream = _resources.openResource(
		MacResourceProvider::kApplicationContainer,
		MKTAG('D', 'L', 'O', 'G'), resourceID);
	if (!stream)
		return false;

	bool valid = stream->size() >= 21;
	if (valid) {
		_bounds.top = stream->readSint16BE();
		_bounds.left = stream->readSint16BE();
		_bounds.bottom = stream->readSint16BE();
		_bounds.right = stream->readSint16BE();
		stream->skip(10);
		itemResourceID = stream->readUint16BE();
		/* Common::String title = */ stream->readPascalString();
		valid = valid && !stream->err() && _bounds.isValidRect();
	}
	delete stream;
	return valid;
}

bool MacNebularDialog::loadDialogItems(uint16 resourceID) {
	Common::SeekableReadStream *stream = _resources.openResource(
		MacResourceProvider::kApplicationContainer,
		MKTAG('D', 'I', 'T', 'L'), resourceID);
	if (!stream)
		return false;

	if (stream->size() < 2) {
		delete stream;
		return false;
	}

	const uint itemCount = stream->readUint16BE() + 1;
	bool valid = true;
	for (uint itemNumber = 0; valid && itemNumber < itemCount; ++itemNumber) {
		if (stream->pos() + 14 > stream->size()) {
			valid = false;
			break;
		}

		Item item;
		stream->skip(4);
		item.bounds.top = stream->readSint16BE();
		item.bounds.left = stream->readSint16BE();
		item.bounds.bottom = stream->readSint16BE();
		item.bounds.right = stream->readSint16BE();
		const byte rawType = stream->readByte();
		const uint length = stream->readByte();
		if (stream->pos() + length > stream->size()) {
			valid = false;
			break;
		}

		item.type = rawType & 0x7f;
		item.enabled = (rawType & 0x80) == 0;
		if (item.type == kButton || item.type == kCheckbox ||
				item.type == kRadioButton || item.type == kStaticText ||
				item.type == kEditableText) {
			for (uint index = 0; index < length; ++index)
				item.text += (char)stream->readByte();
		} else {
			// User and help items carry binary data rather than Pascal text.
			stream->skip(length);
		}
		item.caret = item.text.size();
		item.selectionStart = item.caret;
		item.selectionEnd = item.caret;
		_items.push_back(item);

		if (length & 1)
			stream->skip(1);
	}

	valid = valid && !stream->err();
	delete stream;
	return valid;
}

bool MacNebularDialog::load(uint16 resourceID) {
	_items.clear();
	_inlineEditable = false;
	_focusItem = 0;
	_pressedItem = 0;
	_redraw = true;
	uint16 itemResourceID = 0;
	return _font && loadDialogResource(resourceID, itemResourceID) &&
		loadDialogItems(itemResourceID);
}

void MacNebularDialog::configureInlineEditable(const Common::Rect &bounds,
		const Common::String &text, uint maxLength) {
	_bounds = bounds;
	_items.clear();
	Item item;
	item.bounds = Common::Rect(0, 0, bounds.width(), bounds.height());
	item.text = maxLength && text.size() > maxLength ?
		text.substr(0, maxLength) : text;
	item.type = kEditableText;
	item.enabled = true;
	item.caret = item.text.size();
	item.selectionStart = item.caret;
	item.selectionEnd = item.caret;
	item.maxLength = maxLength;
	_items.push_back(item);
	_inlineEditable = true;
	_focusItem = 0;
	_pressedItem = 0;
	_redraw = true;
}

void MacNebularDialog::center() {
	_bounds.moveTo((_screen.w - _bounds.width()) / 2,
		(_screen.h - _bounds.height()) / 2);
}

void MacNebularDialog::setItemEnabled(int itemNumber, bool enabled) {
	Item *item = getItem(itemNumber);
	if (item) {
		item->enabled = enabled;
		_redraw = true;
	}
}

void MacNebularDialog::setItemChecked(int itemNumber, bool checked) {
	Item *item = getItem(itemNumber);
	if (item) {
		item->checked = checked;
		_redraw = true;
	}
}

bool MacNebularDialog::isItemChecked(int itemNumber) const {
	const Item *item = getItem(itemNumber);
	return item && item->checked;
}

void MacNebularDialog::setItemText(int itemNumber,
		const Common::String &text) {
	Item *item = getItem(itemNumber);
	if (!item)
		return;
	item->text = text;
	item->caret = text.size();
	item->selectionStart = item->caret;
	item->selectionEnd = item->caret;
	_redraw = true;
}

void MacNebularDialog::setItemMaxLength(int itemNumber, uint maxLength) {
	Item *item = getItem(itemNumber);
	if (!item)
		return;
	item->maxLength = maxLength;
	if (maxLength && item->text.size() > maxLength)
		setItemText(itemNumber, item->text.substr(0, maxLength));
}

Common::String MacNebularDialog::getItemText(int itemNumber) const {
	const Item *item = getItem(itemNumber);
	return item ? item->text : Common::String();
}

void MacNebularDialog::setList(int itemNumber,
		const Common::StringArray &entries, int selection) {
	Item *item = getItem(itemNumber);
	if (!item)
		return;
	item->listItem = true;
	item->enabled = true;
	item->list = entries;
	selectList(*item, selection);
	_redraw = true;
}

int MacNebularDialog::getListSelection(int itemNumber) const {
	const Item *item = getItem(itemNumber);
	return item && item->listItem ? item->listSelection : -1;
}

void MacNebularDialog::drawButton(const Item &item,
		const Common::Rect &bounds, bool pressed) {
	Graphics::Primitives &primitives = _windowManager.getDrawPrimitives();
	Graphics::MacPlotData plot(&_screen, nullptr,
		&_windowManager.getBuiltinPatterns(), 1, 0, 0,
		Common::Point(1, 1), _windowManager._colorWhite, false);
	primitives.drawRoundRect1(bounds, 4, _windowManager._colorBlack,
		false, &plot);
	if (pressed) {
		Common::Rect inside(bounds);
		inside.grow(-2);
		primitives.drawFilledRect1(inside, _windowManager._colorBlack, &plot);
	}
	const uint32 color = pressed ? _windowManager._colorWhite :
		_windowManager._colorBlack;
	const int textX = bounds.left +
		(bounds.width() - _font->getStringWidth(item.text)) / 2;
	const int textY = bounds.top +
		(bounds.height() - _font->getFontHeight()) / 2;
	_font->drawString(&_screen, item.text, textX, textY,
		bounds.width(), color);
}

void MacNebularDialog::drawCheckbox(const Item &item,
		const Common::Rect &bounds) {
	Graphics::Primitives &primitives = _windowManager.getDrawPrimitives();
	Graphics::MacPlotData plot(&_screen, nullptr,
		&_windowManager.getBuiltinPatterns(), 1, 0, 0,
		Common::Point(1, 1), _windowManager._colorWhite, false);
	const int boxTop = bounds.top + (bounds.height() - 12) / 2;
	const Common::Rect box(bounds.left, boxTop, bounds.left + 12, boxTop + 12);
	primitives.drawRect1(box, _windowManager._colorBlack, &plot);
	if (item.checked) {
		primitives.drawLine(box.left + 2, box.top + 2,
			box.right - 3, box.bottom - 3, _windowManager._colorBlack, &plot);
		primitives.drawLine(box.left + 2, box.bottom - 3,
			box.right - 3, box.top + 2, _windowManager._colorBlack, &plot);
	}
	const int textY = bounds.top +
		(bounds.height() - _font->getFontHeight()) / 2;
	_font->drawString(&_screen, item.text, bounds.left + 17, textY,
		bounds.width() - 17, _windowManager._colorBlack);
}

void MacNebularDialog::drawRadioButton(const Item &item,
		const Common::Rect &bounds) {
	Graphics::Primitives &primitives = _windowManager.getDrawPrimitives();
	Graphics::MacPlotData plot(&_screen, nullptr,
		&_windowManager.getBuiltinPatterns(), 1, 0, 0,
		Common::Point(1, 1), _windowManager._colorWhite, false);
	const int circleTop = bounds.top + (bounds.height() - 12) / 2;
	primitives.drawEllipse(bounds.left, circleTop, bounds.left + 11,
		circleTop + 11, _windowManager._colorBlack, false, &plot);
	if (item.checked)
		primitives.drawEllipse(bounds.left + 4, circleTop + 4,
			bounds.left + 7, circleTop + 7,
			_windowManager._colorBlack, true, &plot);
	const int textY = bounds.top +
		(bounds.height() - _font->getFontHeight()) / 2;
	_font->drawString(&_screen, item.text, bounds.left + 17, textY,
		bounds.width() - 17, _windowManager._colorBlack);
}

void MacNebularDialog::drawEditable(Item &item,
		const Common::Rect &bounds, bool focused) {
	Graphics::Primitives &primitives = _windowManager.getDrawPrimitives();
	Graphics::MacPlotData plot(&_screen, nullptr,
		&_windowManager.getBuiltinPatterns(), 1, 0, 0,
		Common::Point(1, 1), _windowManager._colorWhite, false);
	primitives.drawRect1(bounds, _windowManager._colorBlack, &plot);
	Common::Rect inside(bounds);
	inside.grow(-1);
	primitives.drawFilledRect1(inside, _windowManager._colorWhite, &plot);

	const int textX = bounds.left + 3;
	const int textY = bounds.top +
		(bounds.height() - _font->getFontHeight()) / 2;
	const uint selectionStart = MIN(item.selectionStart, item.selectionEnd);
	const uint selectionEnd = MAX(item.selectionStart, item.selectionEnd);
	if (selectionEnd > selectionStart) {
		const Common::String prefix = item.text.substr(0, selectionStart);
		const Common::String selected = item.text.substr(selectionStart,
			selectionEnd - selectionStart);
		const Common::String suffix = item.text.substr(selectionEnd);
		const int selectionX = textX + _font->getStringWidth(prefix);
		const int selectionWidth = _font->getStringWidth(selected);
		_font->drawString(&_screen, prefix, textX, textY,
			bounds.width() - 6, _windowManager._colorBlack);
		primitives.drawFilledRect1(Common::Rect(selectionX, bounds.top + 2,
			selectionX + selectionWidth, bounds.bottom - 2),
			_windowManager._colorBlack, &plot);
		_font->drawString(&_screen, selected, selectionX, textY,
			selectionWidth, _windowManager._colorWhite);
		_font->drawString(&_screen, suffix, selectionX + selectionWidth,
			textY, bounds.right - selectionX - selectionWidth - 3,
			_windowManager._colorBlack);
	} else {
		_font->drawString(&_screen, item.text, textX, textY,
			bounds.width() - 6, _windowManager._colorBlack);
	}
	if (focused) {
		const Common::String prefix = item.text.substr(0, item.caret);
		const int caretX = MIN(bounds.right - 2,
			textX + _font->getStringWidth(prefix));
		primitives.drawLine(caretX, bounds.top + 2, caretX,
			bounds.bottom - 3, _windowManager._colorBlack, &plot);
	}
}

void MacNebularDialog::drawList(const Item &item,
		const Common::Rect &bounds, bool focused) {
	Graphics::Primitives &primitives = _windowManager.getDrawPrimitives();
	Graphics::MacPlotData plot(&_screen, nullptr,
		&_windowManager.getBuiltinPatterns(), 1, 0, 0,
		Common::Point(1, 1), _windowManager._colorWhite, false);
	primitives.drawRect1(bounds, _windowManager._colorBlack, &plot);
	Common::Rect inside(bounds);
	inside.grow(-1);
	primitives.drawFilledRect1(inside, _windowManager._colorWhite, &plot);

	const int rowHeight = MAX(_font->getFontHeight() + 2, 12);
	const int visibleRows = MAX(1, (bounds.height() - 4) / rowHeight);
	for (int row = 0; row < visibleRows; ++row) {
		const int index = item.listTop + row;
		if (index < 0 || (uint)index >= item.list.size())
			break;
		const int rowTop = bounds.top + 2 + row * rowHeight;
		const bool selected = index == item.listSelection;
		if (selected) {
			primitives.drawFilledRect1(Common::Rect(bounds.left + 2, rowTop,
				bounds.right - 2, rowTop + rowHeight),
				_windowManager._colorBlack, &plot);
		}
		_font->drawString(&_screen, item.list[index], bounds.left + 4,
			rowTop, bounds.width() - 8, selected ?
			_windowManager._colorWhite : _windowManager._colorBlack);
	}
	if (focused)
		primitives.drawRect1(Common::Rect(bounds.left - 2, bounds.top - 2,
			bounds.right + 2, bounds.bottom + 2),
			_windowManager._colorBlack, &plot);
}

void MacNebularDialog::drawItem(Item &item, int itemNumber) {
	const Common::Rect bounds = getItemBounds(item);
	if (item.listItem) {
		drawList(item, bounds, _focusItem == itemNumber);
		return;
	}

	switch (item.type) {
	case kButton:
		drawButton(item, bounds, _pressedItem == itemNumber);
		break;
	case kCheckbox:
		drawCheckbox(item, bounds);
		break;
	case kRadioButton:
		drawRadioButton(item, bounds);
		break;
	case kStaticText: {
		Common::Array<Common::String> lines;
		_font->wordWrapText(item.text, bounds.width(), lines);
		int textY = bounds.top;
		for (uint line = 0; line < lines.size() &&
				textY + _font->getFontHeight() <= bounds.bottom; ++line) {
			_font->drawString(&_screen, lines[line], bounds.left, textY,
				bounds.width(), _windowManager._colorBlack);
			textY += _font->getFontHeight();
		}
		break;
	}
	case kEditableText:
		drawEditable(item, bounds, _focusItem == itemNumber);
		break;
	default:
		break;
	}
}

void MacNebularDialog::draw() {
	Graphics::Primitives &primitives = _windowManager.getDrawPrimitives();
	Graphics::MacPlotData plot(&_screen, nullptr,
		&_windowManager.getBuiltinPatterns(), 1, 0, 0,
		Common::Point(1, 1), _windowManager._colorWhite, false);
	if (!_inlineEditable) {
		primitives.drawFilledRect1(_bounds, _windowManager._colorWhite, &plot);
		primitives.drawRect1(_bounds, _windowManager._colorBlack, &plot);
	}
	for (uint index = 0; index < _items.size(); ++index)
		drawItem(_items[index], index + 1);
}

void MacNebularDialog::setFocus(int itemNumber) {
	if (_focusItem == itemNumber)
		return;
	_focusItem = itemNumber;
	_redraw = true;
}

void MacNebularDialog::focusNext(bool backwards) {
	if (_items.empty())
		return;
	int itemNumber = _focusItem;
	for (uint count = 0; count < _items.size(); ++count) {
		itemNumber += backwards ? -1 : 1;
		if (itemNumber < 1)
			itemNumber = _items.size();
		else if ((uint)itemNumber > _items.size())
			itemNumber = 1;
		const Item *item = getItem(itemNumber);
		if (item && item->enabled &&
				(item->type == kEditableText || item->listItem)) {
			setFocus(itemNumber);
			return;
		}
	}
}

void MacNebularDialog::selectRadio(int itemNumber) {
	for (uint index = 0; index < _items.size(); ++index) {
		if (_items[index].type == kRadioButton)
			_items[index].checked = index + 1 == (uint)itemNumber;
	}
	_redraw = true;
}

void MacNebularDialog::ensureListSelectionVisible(Item &item) {
	const int rowHeight = MAX(_font->getFontHeight() + 2, 12);
	const int visibleRows = MAX(1, (item.bounds.height() - 4) / rowHeight);
	if (item.listSelection < item.listTop)
		item.listTop = item.listSelection;
	else if (item.listSelection >= item.listTop + visibleRows)
		item.listTop = item.listSelection - visibleRows + 1;
	item.listTop = MAX(0, item.listTop);
}

void MacNebularDialog::selectList(Item &item, int selection) {
	if (item.list.empty()) {
		item.listSelection = -1;
		item.listTop = 0;
		return;
	}
	item.listSelection = CLIP<int>(selection, 0, item.list.size() - 1);
	ensureListSelectionVisible(item);
	_redraw = true;
}

void MacNebularDialog::moveCaret(Item &item, int position, bool extend) {
	const uint newPosition = CLIP<int>(position, 0, item.text.size());
	if (extend) {
		if (item.selectionStart == item.selectionEnd)
			item.selectionStart = item.caret;
		item.selectionEnd = newPosition;
	} else {
		item.selectionStart = newPosition;
		item.selectionEnd = newPosition;
	}
	item.caret = newPosition;
	_redraw = true;
}

void MacNebularDialog::deleteSelection(Item &item) {
	const uint start = MIN(item.selectionStart, item.selectionEnd);
	const uint end = MAX(item.selectionStart, item.selectionEnd);
	for (uint position = end; position > start; --position)
		item.text.deleteChar(position - 1);
	item.caret = start;
	item.selectionStart = start;
	item.selectionEnd = start;
}

void MacNebularDialog::rememberUndo(Item &item) {
	item.undoText = item.text;
	item.hasUndo = true;
}

void MacNebularDialog::insertCharacter(Item &item, char character) {
	const uint selected = MAX(item.selectionStart, item.selectionEnd) -
		MIN(item.selectionStart, item.selectionEnd);
	if (item.maxLength && item.text.size() - selected >= item.maxLength)
		return;
	rememberUndo(item);
	deleteSelection(item);
	item.text.insertChar(character, item.caret);
	moveCaret(item, item.caret + 1, false);
}

void MacNebularDialog::handleKey(const Common::Event &event,
		int &result, bool &done) {
	const bool shift = (event.kbd.flags & Common::KBD_SHIFT) != 0;
	if (event.kbd.keycode == Common::KEYCODE_TAB) {
		focusNext(shift);
		return;
	}
	if (event.kbd.keycode == Common::KEYCODE_ESCAPE && _cancelItem) {
		result = _cancelItem;
		done = true;
		return;
	}
	if ((event.kbd.keycode == Common::KEYCODE_RETURN ||
			event.kbd.keycode == Common::KEYCODE_KP_ENTER) && _defaultItem) {
		result = _defaultItem;
		done = true;
		return;
	}
	if (!_focusItem && (event.kbd.keycode == Common::KEYCODE_UP ||
			event.kbd.keycode == Common::KEYCODE_DOWN)) {
		int selected = 0;
		for (uint index = 0; index < _items.size(); ++index) {
			if (_items[index].type == kRadioButton && _items[index].checked)
				selected = index + 1;
		}
		const int direction = event.kbd.keycode == Common::KEYCODE_UP ? -1 : 1;
		for (uint count = 0; count < _items.size(); ++count) {
			selected += direction;
			if (selected < 1)
				selected = _items.size();
			else if ((uint)selected > _items.size())
				selected = 1;
			const Item *radio = getItem(selected);
			if (radio && radio->enabled && radio->type == kRadioButton) {
				selectRadio(selected);
				return;
			}
		}
	}

	Item *item = getItem(_focusItem);
	if (!item || !item->enabled)
		return;
	if (item->listItem) {
		int selection = item->listSelection;
		switch (event.kbd.keycode) {
		case Common::KEYCODE_UP:
			--selection;
			break;
		case Common::KEYCODE_DOWN:
			++selection;
			break;
		case Common::KEYCODE_PAGEUP:
			selection -= 5;
			break;
		case Common::KEYCODE_PAGEDOWN:
			selection += 5;
			break;
		case Common::KEYCODE_HOME:
			selection = 0;
			break;
		case Common::KEYCODE_END:
			selection = item->list.size() - 1;
			break;
		default:
			return;
		}
		selectList(*item, selection);
		return;
	}
	if (item->type != kEditableText)
		return;

	switch (event.kbd.keycode) {
	case Common::KEYCODE_LEFT:
		moveCaret(*item, item->caret - 1, shift);
		break;
	case Common::KEYCODE_RIGHT:
		moveCaret(*item, item->caret + 1, shift);
		break;
	case Common::KEYCODE_HOME:
		moveCaret(*item, 0, shift);
		break;
	case Common::KEYCODE_END:
		moveCaret(*item, item->text.size(), shift);
		break;
	case Common::KEYCODE_BACKSPACE:
		if (item->selectionStart != item->selectionEnd) {
			rememberUndo(*item);
			deleteSelection(*item);
		} else if (item->caret > 0) {
			rememberUndo(*item);
			item->text.deleteChar(item->caret - 1);
			moveCaret(*item, item->caret - 1, false);
		}
		break;
	case Common::KEYCODE_DELETE:
		if (item->selectionStart != item->selectionEnd) {
			rememberUndo(*item);
			deleteSelection(*item);
		} else if (item->caret < item->text.size()) {
			rememberUndo(*item);
			item->text.deleteChar(item->caret);
			_redraw = true;
		}
		break;
	default:
		if (event.kbd.ascii >= 32 && event.kbd.ascii < 127)
			insertCharacter(*item, (char)event.kbd.ascii);
		break;
	}
}

void MacNebularDialog::handleMouseDown(const Common::Point &point) {
	_pressedItem = 0;
	for (uint index = 0; index < _items.size(); ++index) {
		Item &item = _items[index];
		const int itemNumber = index + 1;
		const Common::Rect bounds = getItemBounds(item);
		if (!item.enabled || !bounds.contains(point))
			continue;

		if (item.type == kButton) {
			_pressedItem = itemNumber;
			_redraw = true;
		} else if (item.type == kCheckbox) {
			item.checked = !item.checked;
			_redraw = true;
		} else if (item.type == kRadioButton) {
			selectRadio(itemNumber);
		} else if (item.type == kEditableText) {
			setFocus(itemNumber);
			const int x = MAX(0, point.x - bounds.left - 3);
			uint caret = 0;
			while (caret < item.text.size() &&
					_font->getStringWidth(item.text.substr(0, caret + 1)) < x)
				++caret;
			moveCaret(item, caret, false);
		} else if (item.listItem) {
			setFocus(itemNumber);
			const int rowHeight = MAX(_font->getFontHeight() + 2, 12);
			selectList(item, item.listTop +
				(point.y - bounds.top - 2) / rowHeight);
		}
		return;
	}
}

void MacNebularDialog::handleMouseUp(const Common::Point &point,
		int &result, bool &done) {
	if (_pressedItem) {
		const Item *item = getItem(_pressedItem);
		if (item && getItemBounds(*item).contains(point)) {
			result = _pressedItem;
			done = true;
		}
		_pressedItem = 0;
		_redraw = true;
	}
}

bool MacNebularDialog::hasEditableFocus() const {
	const Item *item = getItem(_focusItem);
	return item && item->enabled && item->type == kEditableText;
}

bool MacNebularDialog::isEditCommandEnabled(
		MacDialogEditCommand command) const {
	const Item *item = getItem(_focusItem);
	if (!item || !item->enabled || item->type != kEditableText)
		return false;

	const bool hasSelection = item->selectionStart != item->selectionEnd;
	switch (command) {
	case kMacDialogUndo:
		return item->hasUndo;
	case kMacDialogCut:
	case kMacDialogCopy:
	case kMacDialogClear:
		return hasSelection;
	case kMacDialogPaste:
		return g_system->hasTextInClipboard();
	}
	return false;
}

bool MacNebularDialog::handleEditCommand(MacDialogEditCommand command) {
	Item *item = getItem(_focusItem);
	if (!item || !item->enabled || item->type != kEditableText)
		return false;

	const uint start = MIN(item->selectionStart, item->selectionEnd);
	const uint end = MAX(item->selectionStart, item->selectionEnd);
	Common::String selected;
	if (end > start)
		selected = item->text.substr(start, end - start);

	switch (command) {
	case kMacDialogUndo:
		if (!item->hasUndo)
			return false;
		item->text = item->undoText;
		item->hasUndo = false;
		moveCaret(*item, item->text.size(), false);
		break;
	case kMacDialogCut:
		if (selected.empty())
			return false;
		g_system->setTextInClipboard(Common::U32String(selected));
		rememberUndo(*item);
		deleteSelection(*item);
		break;
	case kMacDialogCopy:
		if (selected.empty())
			return false;
		g_system->setTextInClipboard(Common::U32String(selected));
		break;
	case kMacDialogPaste: {
		if (!g_system->hasTextInClipboard())
			return false;
		const Common::String text = g_system->getTextFromClipboard();
		rememberUndo(*item);
		deleteSelection(*item);
		for (uint index = 0; index < text.size(); ++index) {
			const char character = text[index];
			if (character >= 32 && character < 127) {
				if (item->maxLength &&
						item->text.size() >= item->maxLength)
					break;
				item->text.insertChar(character, item->caret);
				++item->caret;
			}
		}
		moveCaret(*item, item->caret, false);
		break;
	}
	case kMacDialogClear:
		if (selected.empty())
			return false;
		rememberUndo(*item);
		deleteSelection(*item);
		break;
	}
	_redraw = true;
	return true;
}

void MacNebularDialog::restore(const Graphics::ManagedSurface &saved,
		bool cursorWasVisible) {
	_screen.copyRectToSurface(saved.getPixels(), saved.pitch,
		_bounds.left, _bounds.top, _bounds.width(), _bounds.height());
	g_system->copyRectToScreen(_screen.getBasePtr(_bounds.left, _bounds.top),
		_screen.pitch, _bounds.left, _bounds.top,
		_bounds.width(), _bounds.height());
	g_system->updateScreen();
	_windowManager.popCursor();
	CursorMan.showMouse(cursorWasVisible);
}

int MacNebularDialog::runModal(int defaultItem, int cancelItem) {
	if (!_font || !_bounds.isValidRect())
		return 0;
	_defaultItem = defaultItem;
	_cancelItem = cancelItem;
	_redraw = true;
	_pressedItem = 0;
	_focusItem = 0;
	focusNext(false);

	Graphics::ManagedSurface saved;
	saved.create(_bounds.width(), _bounds.height(), _screen.format);
	saved.copyRectToSurface(_screen.getBasePtr(_bounds.left, _bounds.top),
		_screen.pitch, 0, 0, _bounds.width(), _bounds.height());

	const bool cursorWasVisible = CursorMan.isVisible();
	_windowManager.clearHandlingWidgets();
	_windowManager.pushCursor(Graphics::kMacCursorArrow);
	CursorMan.showMouse(true);

	int result = 0;
	bool done = false;
	while (!done) {
		if (_redraw) {
			draw();
			g_system->copyRectToScreen(_screen.getBasePtr(_bounds.left,
				_bounds.top), _screen.pitch, _bounds.left, _bounds.top,
				_bounds.width(), _bounds.height());
			_redraw = false;
		}

		Common::Event event;
		while (!done && g_system->getEventManager()->pollEvent(event)) {
			if (_menus && _menus->processDialogEvent(event))
				continue;
			switch (event.type) {
			case Common::EVENT_QUIT:
				_engine.quitGame();
				done = true;
				break;
			case Common::EVENT_KEYDOWN:
				handleKey(event, result, done);
				break;
			case Common::EVENT_LBUTTONDOWN:
				handleMouseDown(event.mouse);
				break;
			case Common::EVENT_LBUTTONUP:
				handleMouseUp(event.mouse, result, done);
				break;
			case Common::EVENT_WHEELUP: {
				Item *item = getItem(_focusItem);
				if (item && item->listItem)
					selectList(*item, item->listSelection - 1);
				break;
			}
			case Common::EVENT_WHEELDOWN: {
				Item *item = getItem(_focusItem);
				if (item && item->listItem)
					selectList(*item, item->listSelection + 1);
				break;
			}
			default:
				break;
			}
		}

		if (!done) {
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
	}

	restore(saved, cursorWasVisible);
	return result;
}

} // namespace RexNebular
} // namespace MADS
