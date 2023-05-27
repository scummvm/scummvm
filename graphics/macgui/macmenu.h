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

#ifndef GRAPHICS_MACGUI_MACMENU_H
#define GRAPHICS_MACGUI_MACMENU_H

#include "common/str-array.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindow.h"
#include "graphics/font.h"

namespace Common {
class U32String;
class MacResManager;
class PEResources;
}

namespace Graphics {

struct MacMenuItem;
struct MacMenuSubMenu;
typedef Common::Array<MacMenuItem *> ItemArray;

struct MacMenuSubMenu {
	ItemArray items;
	Common::Rect bbox;
	int highlight;

	MacMenuSubMenu() : highlight(-1) {}

	~MacMenuSubMenu();

	void enableAllItems();

	int ytoItem(int y, int itemHeight) { return MIN<int>((y - bbox.top) / itemHeight, items.size() - 1); }
};

struct MacMenuItem {
	Common::String text;
	Common::U32String unicodeText;
	bool unicode;
	int action;
	int style;
	char shortcut;
	int shortcutPos;
	bool enabled;
	bool checked;
	Common::Rect bbox;

	MacMenuSubMenu *submenu;

	MacMenuItem(const Common::String &t, int a = -1, int s = 0, char sh = 0, int sp = -1, bool e = true, bool c = false) :
			text(t), unicode(false), action(a), style(s), shortcut(sh),
			shortcutPos(sp), enabled(e), submenu(nullptr), checked(c) {}
	MacMenuItem(const Common::U32String &t, int a = -1, int s = 0, char sh = 0, int sp = -1, bool e = true, bool c = false) :
			unicodeText(t), unicode(true), action(a), style(s), shortcut(sh),
			shortcutPos(sp), enabled(e), submenu(nullptr), checked(c) {}

	~MacMenuItem() {
		if (submenu)
			delete submenu;
	}
};

struct MacMenuData {
	int menunum;
	const char *title;
	int action;
	byte shortcut;
	bool enabled;
};

class MacMenu : public BaseMacWindow {
public:
	MacMenu(int id, const Common::Rect &bounds, MacWindowManager *wm);
	~MacMenu();

	ManagedSurface *getBorderSurface() override { return nullptr; }
	const Common::Rect &getInnerDimensions() override { return _dims; }
	bool isDirty() override { return _contentIsDirty || _dimensionsDirty; }

	static Common::StringArray *readMenuFromResource(Common::SeekableReadStream *res);
	static MacMenu *createMenuFromPEexe(Common::PEResources *exe, MacWindowManager *wm);

	void setAlignment(Graphics::TextAlign align);

	void setCommandsCallback(void (*callback)(int, Common::String &, void *), void *data) { _ccallback = callback; _cdata = data; }
	void setCommandsCallback(void (*callback)(int, Common::U32String &, void *), void *data) { _unicodeccallback = callback; _cdata = data; }

	void addStaticMenus(const MacMenuData *data);
	void calcDimensions();

	int numberOfMenus();
	int numberOfMenuItems(MacMenuItem *menu);

	MacMenuItem *getMenuItem(int menuId);
	MacMenuItem *getMenuItem(const Common::String &menuId);
	MacMenuItem *getSubMenuItem(MacMenuItem *menu, int itemId);
	MacMenuItem *getSubMenuItem(MacMenuItem *menu, const Common::String &itemId);

	MacMenuSubMenu *addSubMenu(MacMenuSubMenu *submenu, int index = -1);
	int addMenuItem(MacMenuSubMenu *submenu, const Common::String &text, int action = -1, int style = 0, char shortcut = 0, bool enabled = true, bool checked = false);
	int addMenuItem(MacMenuSubMenu *submenu, const Common::U32String &text, int action = 0, int style = 0, char shortcut = 0, bool enabled = true, bool checked = false);
	void insertMenuItem(MacMenuSubMenu *submenu, const Common::String &text, uint pos, int action = -1, int style = 0, char shortcut = 0, bool enabled = true, bool checked = false);
	void insertMenuItem(MacMenuSubMenu *submenu, const Common::U32String &text, uint pos, int action = 0, int style = 0, char shortcut = 0, bool enabled = true, bool checked = false);
	void removeMenuItem(MacMenuSubMenu *submenu, uint pos);
	void loadMenuResource(Common::MacResManager *resFork, uint16 id);
	void loadMenuBarResource(Common::MacResManager *resFork, uint16 id);
	void createSubMenuFromString(int id, const char *string, int commandId);
	void clearSubMenu(int id);

	MacMenuSubMenu *getSubmenu(MacMenuSubMenu *submenu, int index);

	bool draw(ManagedSurface *g, bool forceRedraw = false) override;
	void eventLoop();
	bool mouseClick(int x, int y);
	bool draw(bool forceRedraw = false) override { return false; }
	void blit(ManagedSurface *g, Common::Rect &dest) override {}

	bool processEvent(Common::Event &event) override;

	void enableCommand(int menunum, int action, bool state);
	void enableCommand(const char *menuitem, const char *menuaction, bool state);
	void enableCommand(const Common::U32String &menuitem, const Common::U32String &menuaction, bool state);
	void enableAllMenus();
	void disableAllMenus();

	bool isVisible() { return _isVisible; }
	void setVisible(bool visible, bool silent = false) override { _isVisible = visible; _contentIsDirty = true; }

	void printMenu(int level = 0, MacMenuSubMenu *submenu = nullptr);

	virtual void closeMenu();

	bool checkIntersects(Common::Rect &rect);

	// macmenuItem operations
	void setCheckMark(MacMenuItem *menuItem, bool checkMark);
	bool getCheckMark(MacMenuItem *menuItem);

	void setEnabled(MacMenuItem *menuItem, bool enabled);
	bool getEnabled(MacMenuItem *menuItem);

	void setName(MacMenuItem *menuItem, const Common::String &name);
	Common::String getName(MacMenuItem *menuItem);

	void setAction(MacMenuItem *menuItem, int actionId);
	int getAction(MacMenuItem *menuItem);

	int getLastSelectedMenuItem() { return _lastActiveItem; };
	int getLastSelectedSubmenuItem() { return _lastActiveSubItem; };

protected:
	Common::Rect _bbox;
	ManagedSurface _screen;
	ItemArray _items;
	bool _isVisible;
	bool _dimensionsDirty;
	int _menuDropdownItemHeight;
	Common::Array<MacMenuSubMenu *> _menustack;

	int _activeItem;
	int _activeSubItem;

	void renderSubmenu(MacMenuSubMenu *menu, bool recursive = true);
	void calcSubMenuBounds(MacMenuSubMenu *menu, int x, int y);

private:
	ManagedSurface _tempSurface;
	TextAlign _align;
	int _menuLeftDropdownPadding;
	int _menuRightDropdownPadding;

private:
	bool checkCallback(bool unicode = false);
	const Font *getMenuFont(int slant = kMacFontRegular);
	const Common::String getAcceleratorString(MacMenuItem *item, const char *prefix);
	void processTabs();
	void processSubmenuTabs(MacMenuSubMenu *submenu);

	int calcSubMenuWidth(MacMenuSubMenu *menu);

	bool keyEvent(Common::Event &event);
	bool mouseRelease(int x, int y);
	bool mouseMove(int x, int y);

	bool processMenuShortCut(byte flags, uint16 ascii);

	void drawSubMenuArrow(ManagedSurface *dst, int x, int y, int color);
	bool contains(int x, int y);

	MacMenuItem *findMenuItem(const Common::String &menuId, const Common::String &itemId);
	MacMenuItem *findMenuItem(int menuId, int itemId);


	const Font *_font;
	Font *_loadedFont;

	int _lastActiveItem;
	int _lastActiveSubItem;

	void (*_ccallback)(int action, Common::String &text, void *data);
	void (*_unicodeccallback)(int action, Common::U32String &text, void *data);
	void *_cdata;
};

} // End of namespace Graphics

#endif
