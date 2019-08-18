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
#include "common/winexe_pe.h"

namespace Common {
class U32String;
class MacResManager;
}

namespace Graphics {

struct MacMenuItem;
struct MacMenuSubItem;

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

	static Common::StringArray *readMenuFromResource(Common::SeekableReadStream *res);
	static MacMenu *createMenuFromPEexe(Common::PEResources &exe, MacWindowManager *wm);

	void setCommandsCallback(void (*callback)(int, Common::String &, void *), void *data) { _ccallback = callback; _cdata = data; }
	void setCommandsCallback(void (*callback)(int, Common::U32String &, void *), void *data) { _unicodeccallback = callback; _cdata = data; }

	void addStaticMenus(const MacMenuData *data);
	void calcDimensions();

	int addMenuItem(const Common::String &name);
	int addMenuItem(const Common::U32String &name);
	void addMenuSubItem(int id, const Common::String &text, int action, int style = 0, char shortcut = 0, bool enabled = true);
	void addMenuSubItem(int id, const Common::U32String &text, int action, int style = 0, char shortcut = 0, bool enabled = true);
	void loadMenuResource(Common::MacResManager *resFork, uint16 id);
	void loadMenuBarResource(Common::MacResManager *resFork, uint16 id);
	void createSubMenuFromString(int id, const char *string, int commandId);
	void clearSubMenu(int id);

	bool draw(ManagedSurface *g, bool forceRedraw = false);
	bool processEvent(Common::Event &event);

	void enableCommand(int menunum, int action, bool state);
	void enableCommand(const char *menuitem, const char *menuaction, bool state);
	void enableCommand(const Common::U32String &menuitem, const Common::U32String &menuaction, bool state);
	void disableAllMenus();

	void setActive(bool active) { _menuActivated = active; }
	bool hasAllFocus() { return _menuActivated; }

	bool isVisible() { return _isVisible; }
	void setVisible(bool visible) { _isVisible = visible; _contentIsDirty = true; }

	Common::Rect _bbox;

private:
	ManagedSurface _screen;
	ManagedSurface _tempSurface;

private:
	const Font *getMenuFont();
	const Common::String getAcceleratorString(MacMenuSubItem *item, const char *prefix);
	int calculateMenuWidth(MacMenuItem *menu);
	void calcMenuBounds(MacMenuItem *menu);
	void renderSubmenu(MacMenuItem *menu);

	bool keyEvent(Common::Event &event);
	bool mouseClick(int x, int y);
	bool mouseRelease(int x, int y);
	bool mouseMove(int x, int y);

	bool processMenuShortCut(byte flags, uint16 ascii);

	Common::Array<MacMenuItem *> _items;

	const Font *_font;

	bool _menuActivated;
	bool _isVisible;

	int _activeItem;
	int _activeSubItem;

	void (*_ccallback)(int action, Common::String &text, void *data);
	void (*_unicodeccallback)(int action, Common::U32String &text, void *data);
	void *_cdata;
};

} // End of namespace Graphics

#endif
