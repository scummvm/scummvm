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

#include "ultima/ultima4/views/menu.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/views/textview.h"

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

Menu::MenuItemList::iterator Menu::getCurrent() {
	return _selected;
}

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

bool Menu::isVisible() {
	bool visible = false;

	for (_current = _items.begin(); _current != _items.end(); _current++) {
		if ((*_current)->isVisible())
			visible = true;
	}

	return visible;
}

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

void Menu::highlight(MenuItem *item) {
	// unhighlight all menu items first
	for (_current = _items.begin(); _current != _items.end(); _current++)
		(*_current)->setHighlighted(false);
	if (item)
		item->setHighlighted(true);
}

Menu::MenuItemList::iterator Menu::begin() {
	return _items.begin();
}

Menu::MenuItemList::iterator Menu::end() {
	return _items.end();
}

Menu::MenuItemList::iterator Menu::begin_visible() {
	if (!isVisible())
		return _items.end();

	_current = _items.begin();
	while (!(*_current)->isVisible() && _current != _items.end())
		_current++;

	return _current;
}

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

Menu::MenuItemList::iterator Menu::getById(int id) {
	if (id == -1)
		return getCurrent();

	for (_current = _items.begin(); _current != _items.end(); _current++) {
		if ((*_current)->getId() == id)
			return _current;
	}
	return _items.end();
}

MenuItem *Menu::getItemById(int id) {
	_current = getById(id);
	if (_current != _items.end())
		return *_current;

	return nullptr;
}

void Menu::activateItemAtPos(TextView *view, const Common::Point &pt) {
	for (Menu::MenuItemList::iterator it = begin(); it != end(); ++it) {
		Common::Rect r = view->getTextBounds((*it)->getX(), (*it)->getY(),
			(*it)->getText().size());

		if (r.contains(pt)) {
			activateItem((*it)->getId(), MenuEvent::ACTIVATE);
		}
	}
}

void Menu::activateItem(int id, MenuEvent::Type action) {
	MenuItem *mi;

	/* find the given menu item by id */
	if (id >= 0)
		mi = getItemById(id);
	/* or use the current item */
	else
		mi = *getCurrent();

	if (!mi)
		error("Error: Unable to find menu item with id '%d'", id);

	/* make sure the action given will activate the menu item */
	if (mi->getClosesMenu())
		setClosed(true);

	MenuEvent event(this, (MenuEvent::Type)action, mi);
	mi->activate(event);
	setChanged();
	notifyObservers(event);
}

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

bool Menu::getClosed() const {
	return _closed;
}

void Menu::setClosed(bool closed) {
	this->_closed = closed;
}

void Menu::setTitle(const Common::String &text, int x, int y) {
	_title = text;
	_titleX = x;
	_titleY = y;
}

} // End of namespace Ultima4
} // End of namespace Ultima
