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

#include "common/textconsole.h"

#include "m4/m4_views.h"
#include "m4/events.h"
#include "m4/font.h"
#include "m4/globals.h"
#include "m4/m4.h"

namespace M4 {

GUIInventory::GUIInventory(View *owner, MadsM4Engine *vm, const Common::Rect &bounds, int horizCells,
		   int vertCells, int cellWidth, int cellHeight, int tag): GUIRect(owner, bounds, tag) {

	_vm = vm;
	_cellCount.x = horizCells;
	_cellCount.y = vertCells;
	_cellSize.x = cellWidth;
	_cellSize.y = cellHeight;

	// Validate the cell settings
	if ((_cellCount.x * _cellSize.x > _bounds.width()) ||
		(_cellCount.y * _cellSize.y > _bounds.height()))
		error("Cell settings for inventory display exceeded control size");

	_visible = true;
	_scrollPosition = 0;
	_scrollable = false;
	_highlightedIndex = -1;
	_selectedIndex = -1;
}

void GUIInventory::onRefresh() {
	_parent->fillRect(_bounds, _vm->_palette->BLACK);
	//_parent->frameRect(_bounds, _vm->_palette->LIGHT_GRAY);

	if (_visible) {
		//kernel_trigger_dispatch(kernel_trigger_create(TRIG_INV_CLICK));

		_scrollable = false;

		// Get to the starting inventory position for display
		ItemsIterator i = _inventoryItems.begin();
		int index = _scrollPosition;
		while (index-- > 0) ++i;

		// Loop through displaying entries
		for (index = 0; (i != _inventoryItems.end()) && (index < _cellCount.x * _cellCount.y); ++index, ++i) {
			GUIInventoryItem *item = (*i).get();
			const Common::Point cellPos = getCellPosition(index);
/*			Common::Rect cellBounds(_bounds.left + cellPos.x + xOffset,
				_bounds.top + cellPos.y + yOffset,
				_bounds.left + cellPos.x + xOffset + _cellSize.x,
				_bounds.top + cellPos.y + _cellSize.y);*/
			Common::Rect cellBounds(_bounds.left + cellPos.x, _bounds.top + cellPos.y,
				_bounds.left + cellPos.x + _cellSize.x, _bounds.top + cellPos.y + _cellSize.y);

			Common::Point iconPt(
				cellBounds.left + (cellBounds.width() - item->icon->width()) / 2,
				cellBounds.top + (cellBounds.height() - item->icon->height()) / 2);

			item->icon->copyTo(_parent, iconPt.x, iconPt.y, 0);

			if (_highlightedIndex == index)
				_parent->frameRect(Common::Rect(iconPt.x - 2, iconPt.y - 2,
				iconPt.x + item->icon->width() + 2, iconPt.y + item->icon->height() + 2),
				_vm->_palette->LIGHT_GRAY);
		}
	}
}

bool GUIInventory::onEvent(M4EventType eventType, int32 param, int x, int y, GUIObject *&currentItem) {
	bool result = false;
	int overIndex = getInsideIndex(x, y);
	bool isPressed = (eventType == MEVENT_LEFT_CLICK) || (eventType == MEVENT_LEFT_HOLD) ||
		(eventType == MEVENT_LEFT_DRAG);
	ItemsIterator curItem = _inventoryItems.begin();

	if (isPressed) {
		if (_selectedIndex == -1 && overIndex != -1) {
			setHighlight(overIndex);
			_selectedIndex = overIndex;
			for (int i = 0; i < _scrollPosition + _selectedIndex; i++)
				++curItem;
			if (_scrollPosition + _selectedIndex < (int)_inventoryItems.size())
				_vm->_mouse->setCursorNum(curItem->get()->iconIndex);
			result = true;
		} else {
			// We are over something being tracked
			if (_selectedIndex == overIndex) {
				setHighlight(overIndex);
				result = true;
			} else {
				// Otherwise reset highlighting
				setHighlight(-1);
				result = false;
			}
		}
	} else {
		// No button pressed
		if (_selectedIndex == overIndex) {
			result = (_selectedIndex != -1);
		} else {
			result = (overIndex + _scrollPosition < (int)_inventoryItems.size());
			if (result) {
				for (int i = 0; i < overIndex + _scrollPosition; i++)
					++curItem;
				_m4Vm->scene()->getInterface()->setStatusText(curItem->get()->name);
			}
		}

		// Stop tracking anything
		setHighlight(overIndex);
	}

	return result;
}

void GUIInventory::add(const char *name, const char *verb, M4Surface *icon, int iconIndex) {
	// First scan through the list to prevent duplicate objects
	for (ItemsIterator i = _inventoryItems.begin(); i != _inventoryItems.end(); ++i) {
		if (!strcmp(name, ((*i).get())->name))
			return;
	}

	_inventoryItems.push_back(InventoryList::value_type(new GUIInventoryItem(name, verb, icon, iconIndex)));
}

bool GUIInventory::remove(const char *name) {
	for (ItemsIterator i = _inventoryItems.begin(); i != _inventoryItems.end(); ++i) {
		if (!strcmp(name, ((*i).get())->name)) {
			_inventoryItems.erase(i);
			_scrollPosition = 0;
			return true;
		}
	}

	return false;
}

int GUIInventory::getInsideIndex(int x, int y) {
	if (!_bounds.contains(x, y))
		return -1;

	int localX = x - _bounds.left;
	int localY = y - _bounds.top;
	return (localX / _cellSize.x) * _cellCount.y + (localY / _cellSize.y);
}

const char *GUIInventory::getSelectedObjectName() {
	if (_selectedIndex != -1) {
		ItemsIterator curItem = _inventoryItems.begin();
		for (int i = 0; i < _selectedIndex; i++)
			++curItem;
		return curItem->get()->name;
	} else {
		return NULL;
	}
}

const Common::Point &GUIInventory::getCellPosition(int index) {
	static Common::Point result;

	if (_cellCount.x > _cellCount.y) {
		// Horizontal orientation
		result.x = (index / _cellCount.y) * _cellSize.x;
		result.y = (index % _cellCount.y) * _cellSize.x;
	} else {
		// Vertical orientation
		result.x = (index / _cellCount.x) * _cellSize.y;
		result.y = (index % _cellCount.x) * _cellSize.y;
	}

	return result;
}

void GUIInventory::setHighlight(int index) {
	if (_highlightedIndex == index)
		return;

	_highlightedIndex = index;
}

void GUIInventory::setScrollPosition(int value) {
	if (value < 0)
		return;
	else if (value >= (int)_inventoryItems.size() - (_cellCount.x * _cellCount.y))
		return;

	_scrollPosition = value;
}

//--------------------------------------------------------------------------

const char *INTERFACE_SERIES = "999intr";

#define SPR(x) _sprites->getFrame(x)

M4InterfaceView::M4InterfaceView(MadsM4Engine *vm):
		GameInterfaceView(vm, Common::Rect(0, vm->_screen->height() - INTERFACE_HEIGHT,
				vm->_screen->width(), vm->_screen->height())),
		_statusText(GUITextField(this, Common::Rect(200, 1, 450, 21))),
		_inventory(GUIInventory(this, vm, Common::Rect(188, 22, 539, 97), 9, 1, 39, 75, 3)) {

	_screenType = VIEWID_INTERFACE;
	_screenFlags.layer = LAYER_INTERFACE;
	_screenFlags.visible = false;
	_screenFlags.get = SCREVENT_MOUSE;
	_highlightedIndex = -1;
	_selected = false;

	Common::SeekableReadStream *data = _vm->res()->get(INTERFACE_SERIES);
	RGB8 *palette;

	_sprites = new SpriteAsset(_vm, data, data->size(), INTERFACE_SERIES);
	palette = _sprites->getPalette();

	//Palette.setPalette(palette, 0, _sprites->getColorCount());

	_vm->res()->toss(INTERFACE_SERIES);

	// Setup the interface buttons

	_buttons.push_back(ButtonList::value_type(new GUIButton(this, Common::Rect(15, 35, 47, 66), 0, SPR(0), SPR(1), SPR(2))));   // look
	_buttons.push_back(ButtonList::value_type(new GUIButton(this, Common::Rect(60, 35, 92, 66), 1, SPR(3), SPR(4), SPR(5))));   // take
	_buttons.push_back(ButtonList::value_type(new GUIButton(this, Common::Rect(105, 35, 137, 66), 2, SPR(6), SPR(7), SPR(8)))); // manipulate

	_buttons.push_back(ButtonList::value_type(new GUIButton(this, Common::Rect(580, 10, 620, 69), 3, SPR(69), SPR(70), SPR(71))));  // abduction
	_buttons.push_back(ButtonList::value_type(new GUIButton(this, Common::Rect(582, 70, 619, 105), 4, SPR(76), SPR(77), SPR(78)))); // menu

	_buttons.push_back(ButtonList::value_type(new GUIButton(this, Common::Rect(168, 22, 188, 97), 5, SPR(60), SPR(61), SPR(62))));   // Scroll left
	_buttons.push_back(ButtonList::value_type(new GUIButton(this, Common::Rect(539, 22, 559, 97), 6, SPR(64), SPR(65), SPR(66))));   // Scroll right
}

#undef SPR

M4InterfaceView::~M4InterfaceView() {
	delete _sprites;
}

void M4InterfaceView::setHighlightedButton(int index) {
	if (index == _highlightedIndex)
		return;

	_selected = (index == -1);
	_highlightedIndex = index;
}

bool M4InterfaceView::onEvent(M4EventType eventType, int32 param, int x, int y, bool &captureEvents) {
	static bool selectionFlag = false;
	if (eventType == MEVENT_LEFT_RELEASE)
		selectionFlag = false;

	captureEvents = isInside(x, y);
	if (!captureEvents)
		return false;

	int localX = x - _coords.left;
	int localY = y - _coords.top;
	GUIObject *currentItem;

	_statusText.onEvent(eventType, param, localX, localY, currentItem);
	_inventory.onEvent(eventType, param, localX, localY, currentItem);

	if (_vm->_mouse->getCursorNum() != CURSOR_LOOK &&
		_vm->_mouse->getCursorNum() != CURSOR_TAKE &&
		_vm->_mouse->getCursorNum() != CURSOR_USE &&
		_m4Vm->scene()->getInterface()->_inventory.getSelectedIndex() == -1) {
		if (_vm->_mouse->getCursorNum() != 0)
			_vm->_mouse->setCursorNum(0);
	}

	for (ButtonsIterator i = _buttons.begin(); i != _buttons.end(); ++i) {
		GUIButton *btn = (*i).get();
		btn->onEvent(eventType, param, localX, localY, currentItem);
		if ((btn->getState() == BUTTON_PRESSED) && !selectionFlag) {
			selectionFlag = true;
			_highlightedIndex = btn->getTag();

			switch (_highlightedIndex) {
			case 0:
				_vm->_mouse->setCursorNum(CURSOR_LOOK);
				break;
			case 1:
				_vm->_mouse->setCursorNum(CURSOR_TAKE);
				break;
			case 2:
				_vm->_mouse->setCursorNum(CURSOR_USE);
				break;
			case 3:
				// TODO: Jump to abduction
				break;
			case 4:
				_vm->loadMenu(GAME_MENU);
				break;
			case 5:
				_inventory.scrollLeft();
				break;
			case 6:
				_inventory.scrollRight();
				break;
			default:
				break;
			}
		}
	}

	return true;
}

void M4InterfaceView::onRefresh(RectList *rects, M4Surface *destSurface) {
	clear();

	_statusText.onRefresh();
	_inventory.onRefresh();
	for (ButtonsIterator i = _buttons.begin(); i != _buttons.end(); ++i)
		((*i).get())->onRefresh();

	View::onRefresh(rects, destSurface);
}


} // End of namespace M4
