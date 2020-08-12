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

#ifndef GRAPHICS_MACGUI_MACMENU_H
#define GRAPHICS_MACGUI_MACMENU_H

#include "common/str-array.h"
#include "graphics/macgui/macfontmanager.h"
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

	virtual ManagedSurface *getBorderSurface() override { return nullptr; }
	virtual const Common::Rect &getInnerDimensions() override { return _dims; }
	virtual bool isDirty() override { return _contentIsDirty || _dimensionsDirty; }

	static Common::StringArray *readMenuFromResource(Common::SeekableReadStream *res);
	static MacMenu *createMenuFromPEexe(Common::PEResources *exe, MacWindowManager *wm);

	void setAlignment(Graphics::TextAlign align);

	void setCommandsCallback(void (*callback)(int, Common::String &, void *), void *data) { _ccallback = callback; _cdata = data; }
	void setCommandsCallback(void (*callback)(int, Common::U32String &, void *), void *data) { _unicodeccallback = callback; _cdata = data; }

	void addStaticMenus(const MacMenuData *data);
	void calcDimensions();

	MacMenuSubMenu *addSubMenu(MacMenuSubMenu *submenu, int index = -1);
	int addMenuItem(MacMenuSubMenu *submenu, const Common::String &text, int action = -1, int style = 0, char shortcut = 0, bool enabled = true);
	int addMenuItem(MacMenuSubMenu *submenu, const Common::U32String &text, int action = 0, int style = 0, char shortcut = 0, bool enabled = true);
	void loadMenuResource(Common::MacResManager *resFork, uint16 id);
	void loadMenuBarResource(Common::MacResManager *resFork, uint16 id);
	void createSubMenuFromString(int id, const char *string, int commandId);
	void clearSubMenu(int id);

	MacMenuSubMenu *getSubmenu(MacMenuSubMenu *submenu, int index);

	virtual bool draw(ManagedSurface *g, bool forceRedraw = false) override;
	virtual bool draw(bool forceRedraw = false) override { return false; }
	virtual void blit(ManagedSurface *g, Common::Rect &dest) override {}

	bool processEvent(Common::Event &event) override;

	void enableCommand(int menunum, int action, bool state);
	void enableCommand(const char *menuitem, const char *menuaction, bool state);
	void enableCommand(const Common::U32String &menuitem, const Common::U32String &menuaction, bool state);
	void disableAllMenus();

	bool isVisible() { return _isVisible; }
	void setVisible(bool visible, bool silent = false) override { _isVisible = visible; _contentIsDirty = true; }

	void printMenu(int level = 0, MacMenuSubMenu *submenu = nullptr);

	Common::Rect _bbox;

private:
	ManagedSurface _screen;
	ManagedSurface _tempSurface;
	TextAlign _align;

private:
	bool checkCallback(bool unicode = false);
	const Font *getMenuFont(int slant = kMacFontRegular);
	const Common::String getAcceleratorString(MacMenuItem *item, const char *prefix);
	void processTabs();
	void processSubmenuTabs(MacMenuSubMenu *submenu);

	int calcSubMenuWidth(MacMenuSubMenu *menu);
	void calcSubMenuBounds(MacMenuSubMenu *menu, int x, int y);
	void renderSubmenu(MacMenuSubMenu *menu, bool recursive = true);

	bool keyEvent(Common::Event &event);
	bool mouseClick(int x, int y);
	bool mouseRelease(int x, int y);
	bool mouseMove(int x, int y);

	bool processMenuShortCut(byte flags, uint16 ascii);

	void drawSubMenuArrow(ManagedSurface *dst, int x, int y, int color);

	void eventLoop();

	ItemArray _items;

	const Font *_font;

	bool _isVisible;

	bool _dimensionsDirty;

	int _activeItem;
	Common::Array<MacMenuSubMenu *> _menustack;
	int _activeSubItem;

	void (*_ccallback)(int action, Common::String &text, void *data);
	void (*_unicodeccallback)(int action, Common::U32String &text, void *data);
	void *_cdata;
};

} // End of namespace Graphics

#endif
