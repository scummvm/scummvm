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

#ifndef ULTIMA4_VIEWS_MENU_H
#define ULTIMA4_VIEWS_MENU_H

#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/views/menuitem.h"
#include "ultima/ultima4/core/observable.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

class Menu;
class TextView;

class MenuEvent {
public:
	enum Type {
		ACTIVATE,
		INCREMENT,
		DECREMENT,
		SELECT,
		RESET
	};

	MenuEvent(const Menu *menu, Type type, const MenuItem *item = nullptr) {
		this->_menu = menu;
		this->_type = type;
		this->_item = item;
	}

	const Menu *getMenu() {
		return _menu;
	}
	Type getType() {
		return _type;
	}
	const MenuItem *getMenuItem() {
		return _item;
	}

private:
	const Menu *_menu;
	Type _type;
	const MenuItem *_item;
};

/**
 * Menu class definition
 */
class Menu : public Observable<Menu *, MenuEvent &> {
public:
	typedef Common::List<MenuItem *> MenuItemList;

public:
	Menu();
	~Menu();

	void removeAll();

	/**
	 * Adds an item to the menu list and returns the menu
	 */
	void add(int id, Common::String text, short x, short y, int shortcutKey = -1);

	MenuItem *add(int id, MenuItem *item);
	void addShortcutKey(int id, int shortcutKey);
	void setClosesMenu(int id);

	/**
	 * Returns the menu item that is currently selected/highlighted
	 */
	MenuItemList::iterator getCurrent();

	/**
	 * Sets the current menu item to the one indicated by the iterator
	 */
	void setCurrent(MenuItemList::iterator i);
	void setCurrent(int id);
	void show(TextView *view);

	/**
	 * Checks the menu to ensure that there is at least 1 visible
	 * item in the list.  Returns true if there is at least 1 visible
	 * item, false if nothing is visible.
	 */
	bool isVisible();

	/**
	 * Sets the selected iterator to the next visible menu item and highlights it
	 */
	void next();

	/**
	 * Sets the selected iterator to the previous visible menu item and highlights it
	 */
	void prev();

	/**
	 * Highlights a single menu item, un-highlighting any others
	 */
	void highlight(MenuItem *item);

	/**
	 * Returns an iterator pointing to the first menu item
	 */
	MenuItemList::iterator begin();

	/**
	 * Returns an iterator pointing just past the last menu item
	 */
	MenuItemList::iterator end();

	/**
	 * Returns an iterator pointing to the first visible menu item
	 */
	MenuItemList::iterator begin_visible();

	/**
	 * 'Resets' the menu.  This does the following:
	 *      - un-highlights all menu items
	 *      - highlights the first menu item
	 *      - selects the first visible menu item
	 */
	void reset(bool highlightFirst = true);

	/**
	 * Returns an iterator pointing to the item associated with the given 'id'
	 */
	MenuItemList::iterator getById(int id);

	/**
	 * Returns the menu item associated with the given 'id'
	 */
	MenuItem *getItemById(int id);

	/**
	 * Activates any menu item at a given position
	 */
	void activateItemAtPos(TextView *view, const Common::Point &pt);

	/**
	 * Activates the menu item given by 'id', using 'action' to
	 * activate it.  If the menu item cannot be activated using
	 * 'action', then it is not activated.  This also un-highlights
	 * the menu item given by 'menu' and highlights the new menu
	 * item that was found for 'id'.
	 */
	void activateItem(int id, MenuEvent::Type action);

	/**
	 * Activates a menu item by it's shortcut key.  True is returned if a
	 * menu item get activated, false otherwise.
	 */
	bool activateItemByShortcut(int key, MenuEvent::Type action);

	/**
	 * Returns true if the menu has been closed.
	 */
	bool getClosed() const;

	/**
	 * Update whether the menu has been closed.
	 */
	void setClosed(bool closed);

	void setTitle(const Common::String &text, int x, int y);

private:
	MenuItemList _items;
	MenuItemList::iterator _current;
	MenuItemList::iterator _selected;
	bool _closed;
	Common::String _title;
	int _titleX, _titleY;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
