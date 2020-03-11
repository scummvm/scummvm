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

#ifndef ULTIMA4_MENU_H
#define ULTIMA4_MENU_H

#include "ultima/ultima4/event.h"
#include "ultima/ultima4/menuitem.h"
#include "ultima/ultima4/observable.h"
#include "ultima/ultima4/types.h"
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

    MenuEvent(const Menu *menu, Type type, const MenuItem *item = NULL) {
        this->menu = menu;
        this->type = type;
        this->item = item;
    }

    const Menu *getMenu() { return menu; }
    Type getType() { return type; }
    const MenuItem *getMenuItem() { return item; }

private:
    const Menu *menu;
    Type type;
    const MenuItem *item;
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

    void                    removeAll();

    void                    add(int id, Common::String text, short x, short y, int shortcutKey = -1);
    MenuItem *              add(int id, MenuItem *item);
    void                    addShortcutKey(int id, int shortcutKey);
    void                    setClosesMenu(int id);
    MenuItemList::iterator  getCurrent();
    void                    setCurrent(MenuItemList::iterator i);
    void                    setCurrent(int id);
    void                    show(TextView *view);
    bool                    isVisible();
    void                    next();
    void                    prev();
    void                    highlight(MenuItem *item);
    MenuItemList::iterator  begin();
    MenuItemList::iterator  end();
    MenuItemList::iterator  begin_visible();
    void                    reset(bool highlightFirst = true);
    MenuItemList::iterator  getById(int id);
    MenuItem*               getItemById(int id);
    void                    activateItem(int id, MenuEvent::Type action);
    bool                    activateItemByShortcut(int key, MenuEvent::Type action);
    bool                    getClosed() const;
    void                    setClosed(bool closed);
    void                    setTitle(const Common::String &text, int x, int y);

private:    
    MenuItemList items;
    MenuItemList::iterator current;
    MenuItemList::iterator selected;
    bool closed;
    Common::String title;
    int titleX, titleY;
};

/**
 * This class controls a menu.  The value field of WaitableController
 * isn't used.
 */
class MenuController : public WaitableController<void *> {
public:
    MenuController(Menu *menu, TextView *view);
    bool keyPressed(int key);

protected:
    Menu *menu;
    TextView *view;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
