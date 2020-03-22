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

#include "ultima/ultima4/game/menu.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/game/textview.h"

namespace Ultima {
namespace Ultima4 {

Menu::Menu() :
	_closed(false),
	_title(""),
	_titleX(0),
	_titleY(0) {
}

Menu::~Menu() {
	for (MenuItemList::iterator i = _items.begin(); i != _items.end(); i++)
		delete *i;
}

void Menu::removeAll() {
	_items.clear();
}

/**
 * Adds an item to the menu list and returns the menu
 */
void Menu::add(int id, Common::String text, short x, short y, int sc) {
	MenuItem *item = new MenuItem(text, x, y, sc);
	item->setId(id);
	_items.push_back(item);
}

MenuItem *Menu::add(int id, MenuItem *item) {
	item->setId(id);
	_items.push_back(item);
	return item;
}

void Menu::addShortcutKey(int id, int shortcutKey) {
	for (MenuItemList::iterator i = _items.begin(); i != _items.end(); i++) {
		if ((*i)->getId() == id) {
			(*i)->addShortcutKey(shortcutKey);
			break;
		}
	}
}

void Menu::setClosesMenu(int id) {
	for (MenuItemList::iterator i = _items.begin(); i != _items.end(); i++) {
		if ((*i)->getId() == id) {
			(*i)->setClosesMenu(true);
			break;
		}
	}
}

/**
 * Returns the menu item that is currently selected/highlighted
 */
Menu::MenuItemList::iterator Menu::getCurrent() {
	return _selected;
}

/**
 * Sets the current menu item to the one indicated by the iterator
 */
void Menu::setCurrent(MenuItemList::iterator i) {
	_selected = i;
	highlight(*_selected);

	MenuEvent event(this, MenuEvent::SELECT);
	setChanged();
	notifyObservers(event);
}

void Menu::setCurrent(int id) {
	setCurrent(getById(id));
}

void Menu::show(TextView *view) {
	if (_title.size() > 0)
		view->textAt(_titleX, _titleY, "%s", _title.c_str());

	for (_current = _items.begin(); _current != _items.end(); _current++) {
		MenuItem *mi = *_current;

		if (mi->isVisible()) {
			Common::String text(mi->getText());

			if (mi->isSelected()) {
				text.setChar('\010', 0);
			}

			if (mi->isHighlighted()) {
				view->textSelectedAt(mi->getX(), mi->getY(), view->colorizeString(text.c_str(), FG_YELLOW, mi->getScOffset(), 1).c_str());
				// hack for the custom U5 mix reagents menu
				// places cursor 1 column over, rather than 2.
				view->setCursorPos(mi->getX() - (view->getWidth() == 15 ? 1 : 2), mi->getY(), true);
				view->enableCursor();
			} else {
				view->textAt(mi->getX(), mi->getY(), "%s", view->colorizeString(text.c_str(), FG_YELLOW, mi->getScOffset(), 1).c_str());
			}
		}
	}
}

/**
 * Checks the menu to ensure that there is at least 1 visible
 * item in the list.  Returns true if there is at least 1 visible
 * item, false if nothing is visible.
 */
bool Menu::isVisible() {
	bool visible = false;

	for (_current = _items.begin(); _current != _items.end(); _current++) {
		if ((*_current)->isVisible())
			visible = true;
	}

	return visible;
}

/**
 * Sets the selected iterator to the next visible menu item and highlights it
 */
void Menu::next() {
	MenuItemList::iterator i = _selected;
	if (isVisible()) {
		if (++i == _items.end())
			i = _items.begin();
		while (!(*i)->isVisible()) {
			if (++i == _items.end())
				i = _items.begin();
		}
	}

	setCurrent(i);
}

/**
 * Sets the selected iterator to the previous visible menu item and highlights it
 */
void Menu::prev() {
	MenuItemList::iterator i = _selected;
	if (isVisible()) {
		if (i == _items.begin())
			i = _items.end();
		i--;
		while (!(*i)->isVisible()) {
			if (i == _items.begin())
				i = _items.end();
			i--;
		}
	}

	setCurrent(i);
}

/**
 * Highlights a single menu item, un-highlighting any others
 */
void Menu::highlight(MenuItem *item) {
	// unhighlight all menu items first
	for (_current = _items.begin(); _current != _items.end(); _current++)
		(*_current)->setHighlighted(false);
	if (item)
		item->setHighlighted(true);
}

/**
 * Returns an iterator pointing to the first menu item
 */
Menu::MenuItemList::iterator Menu::begin() {
	return _items.begin();
}

/**
 * Returns an iterator pointing just past the last menu item
 */
Menu::MenuItemList::iterator Menu::end() {
	return _items.end();
}

/**
 * Returns an iterator pointing to the first visible menu item
 */
Menu::MenuItemList::iterator Menu::begin_visible() {
	if (!isVisible())
		return _items.end();

	_current = _items.begin();
	while (!(*_current)->isVisible() && _current != _items.end())
		_current++;

	return _current;
}

/**
 * 'Resets' the menu.  This does the following:
 *      - un-highlights all menu items
 *      - highlights the first menu item
 *      - selects the first visible menu item
 */
void Menu::reset(bool highlightFirst) {
	_closed = false;

	/* get the first visible menu item */
	_selected = begin_visible();

	/* un-highlight and deselect each menu item */
	for (_current = _items.begin(); _current != _items.end(); _current++) {
		(*_current)->setHighlighted(false);
		(*_current)->setSelected(false);
	}

	/* highlight the first visible menu item */
	if (highlightFirst)
		highlight(*_selected);

	MenuEvent event(this, MenuEvent::RESET);
	setChanged();
	notifyObservers(event);
}

/**
 * Returns an iterator pointing to the item associated with the given 'id'
 */
Menu::MenuItemList::iterator Menu::getById(int id) {
	if (id == -1)
		return getCurrent();

	for (_current = _items.begin(); _current != _items.end(); _current++) {
		if ((*_current)->getId() == id)
			return _current;
	}
	return _items.end();
}

/**
 * Returns the menu item associated with the given 'id'
 */
MenuItem *Menu::getItemById(int id) {
	_current = getById(id);
	if (_current != _items.end())
		return *_current;
	return NULL;
}

/**
 * Activates the menu item given by 'id', using 'action' to
 * activate it.  If the menu item cannot be activated using
 * 'action', then it is not activated.  This also un-highlights
 * the menu item given by 'menu' and highlights the new menu
 * item that was found for 'id'.
 */
void Menu::activateItem(int id, MenuEvent::Type action) {
	MenuItem *mi;

	/* find the given menu item by id */
	if (id >= 0)
		mi = getItemById(id);
	/* or use the current item */
	else mi = *getCurrent();

	if (!mi)
		errorFatal("Error: Unable to find menu item with id '%d'", id);

	/* make sure the action given will activate the menu item */
	if (mi->getClosesMenu())
		setClosed(true);

	MenuEvent event(this, (MenuEvent::Type) action, mi);
	mi->activate(event);
	setChanged();
	notifyObservers(event);
}

/**
 * Activates a menu item by it's shortcut key.  True is returned if a
 * menu item get activated, false otherwise.
 */
bool Menu::activateItemByShortcut(int key, MenuEvent::Type action) {
	for (MenuItemList::iterator i = _items.begin(); i != _items.end(); ++i) {
		const Std::set<int> &shortcuts = (*i)->getShortcutKeys();
		if (shortcuts.find(key) != shortcuts.end()) {
			activateItem((*i)->getId(), action);
			// if the selection doesn't close the menu, highlight the selection
			if (!(*i)->getClosesMenu())
				setCurrent((*i)->getId());
			return true;
		}
	}
	return false;
}

/**
 * Returns true if the menu has been closed.
 */
bool Menu::getClosed() const {
	return _closed;
}

/**
 * Update whether the menu has been closed.
 */
void Menu::setClosed(bool closed) {
	this->_closed = closed;
}

void Menu::setTitle(const Common::String &text, int x, int y) {
	_title = text;
	_titleX = x;
	_titleY = y;
}

MenuController::MenuController(Menu *menu, TextView *view) {
	this->_menu = menu;
	this->_view = view;
}

bool MenuController::keyPressed(int key) {
	bool handled = true;
	bool cursorOn = _view->getCursorEnabled();

	if (cursorOn)
		_view->disableCursor();

	switch (key) {
	case U4_UP:
		_menu->prev();
		break;
	case U4_DOWN:
		_menu->next();
		break;
	case U4_LEFT:
	case U4_RIGHT:
	case U4_ENTER: {
		MenuEvent::Type action = MenuEvent::ACTIVATE;

		if (key == U4_LEFT)
			action = MenuEvent::DECREMENT;
		else if (key == U4_RIGHT)
			action = MenuEvent::INCREMENT;
		_menu->activateItem(-1, action);
	}
	break;
	default:
		handled = _menu->activateItemByShortcut(key, MenuEvent::ACTIVATE);
	}

	_menu->show(_view);

	if (cursorOn)
		_view->enableCursor();
	_view->update();

	if (_menu->getClosed())
		doneWaiting();

	return handled;
}

} // End of namespace Ultima4
} // End of namespace Ultima
