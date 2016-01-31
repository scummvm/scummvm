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

#ifndef AGI_MENU_H
#define AGI_MENU_H

namespace Agi {

struct GuiMenuEntry {
	Common::String text;
	int16 textLen;

	int16 row;
	int16 column;

	int16 itemCount;
	int16 firstItemNr;
	int16 selectedItemNr;

	int16 maxItemTextLen;
};
typedef Common::Array<GuiMenuEntry *> GuiMenuArray;

struct GuiMenuItemEntry {
	Common::String text;
	int16 textLen;

	int16 row;
	int16 column;

	bool enabled;
	uint16 controllerSlot;
};
typedef Common::Array<GuiMenuItemEntry *> GuiMenuItemArray;

class GfxMenu {
public:
	GfxMenu(AgiEngine *vm, GfxMgr *gfx, PictureMgr *picture, TextMgr *text);
	~GfxMenu();

	void addMenu(const char *menuText);
	void addMenuItem(const char *menuText, uint16 controlCode);
	void submit();
	void itemEnable(uint16 controllerSlot);
	void itemDisable(uint16 controllerSlot);
	void itemEnableAll();
	void charPress(int16 newChar);

	bool isAvailable();

	void accessAllow();
	void accessDeny();

	void delayedExecute();
	bool delayedExecuteActive();
	void execute();

private:
	void itemEnableDisable(uint16 controllerSlot, bool enabled);

	void drawMenuName(int16 menuNr, bool inverted);
	void drawItemName(int16 itemNr, bool inverted);
	void drawActiveMenu();
	void removeActiveMenu();

	AgiEngine *_vm;
	GfxMgr *_gfx;
	PictureMgr *_picture;
	TextMgr *_text;

	bool _allowed;
	bool _submitted;
	bool _delayedExecute;

	// for initial setup of the menu
	int16 _setupMenuColumn;
	int16 _setupMenuItemColumn;

	GuiMenuArray _array;
	GuiMenuItemArray _itemArray;

	int16 _selectedMenuNr;

	uint16 _selectedMenuHeight;
	uint16 _selectedMenuWidth;
	int16  _selectedMenuRow;
	int16  _selectedMenuColumn;
};

#if 0
#define MENU_BG		0x0f	// White
#define MENU_DISABLED	0x07	// Grey

#define MENU_FG		0x00	// Black
#define MENU_LINE	0x00	// Black

struct AgiMenu;
struct AgiMenuOption;
typedef Common::List<AgiMenu *> MenuList;
typedef Common::List<AgiMenuOption *> MenuOptionList;

class GfxMgr;
class PictureMgr;

class Menu {
private:
	AgiEngine *_vm;
	GfxMgr *_gfx;
	PictureMgr *_picture;
	TextMgr *_text;

public:
	Menu(AgiEngine *vm, GfxMgr *gfx, PictureMgr *picture);
	~Menu();

	void add(const char *s);
	void addItem(const char *s, int code);
	void submit();
	void setItem(int event, int state);
	bool keyhandler(int key);
	void enableAll();

private:
	MenuList _menubar;

	int _hCurMenu;
	int _vCurMenu;

	int _hIndex;
	int _vIndex;
	int _hCol;
	int _hMaxMenu;
	int _vMaxMenu[10];

	AgiMenu* getMenu(int i);
	AgiMenuOption *getMenuOption(int i, int j);
	void drawMenuBar();
	void drawMenuHilite(int curMenu);
	void drawMenuOption(int hMenu);
	void drawMenuOptionHilite(int hMenu, int vMenu);
	void newMenuSelected(int i);
	bool mouseOverText(int line, int col, char *s);

};
#endif

} // End of namespace Agi

#endif /* AGI_MENU_H */
