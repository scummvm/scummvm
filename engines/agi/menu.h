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

	int16 itemCount; // total number of menu items
	int16 firstItemNr; // first menu item number, points into _itemArray[]

	int16 selectedItemNr; // currently selected menu item

	int16 maxItemTextLen; // maximum text length of all menu items
};
typedef Common::Array<GuiMenuEntry *> GuiMenuArray;

struct GuiMenuItemEntry {
	Common::String text;
	int16 textLen;

	int16 row;
	int16 column;

	bool enabled; // enabled-state, set by scripts
	uint16 controllerSlot; // controller to trigger, when item is executed
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

	void keyPress(uint16 newKey);
	void mouseEvent(uint16 newKey);

	bool isAvailable();

	void accessAllow();
	void accessDeny();

	void delayedExecuteViaKeyboard();
	void delayedExecuteViaMouse();
	bool delayedExecuteActive();
	void execute();

private:
	void itemEnableDisable(uint16 controllerSlot, bool enabled);

	void drawMenuName(int16 menuNr, bool inverted);
	void drawItemName(int16 itemNr, bool inverted);
	void drawMenu(int16 selectedMenuNr, int16 selectedMenuItemNr);
	void removeActiveMenu(int16 selectedMenuNr);

	void mouseFindMenuSelection(int16 mouseRow, int16 mouseColumn, int16 &activeMenuNr, int16 &activeMenuItemNr);

	AgiEngine *_vm;
	GfxMgr *_gfx;
	PictureMgr *_picture;
	TextMgr *_text;

	bool _allowed;
	bool _submitted;
	bool _delayedExecuteViaKeyboard;
	bool _delayedExecuteViaMouse;

	// for initial setup of the menu
	int16 _setupMenuColumn;
	int16 _setupMenuItemColumn;

	GuiMenuArray _array;
	GuiMenuItemArray _itemArray;

	int16 _lastSelectedMenuNr; // only used for "via keyboard" mode

	int16 _drawnMenuNr;

	uint16 _drawnMenuHeight;
	uint16 _drawnMenuWidth;
	int16  _drawnMenuY;
	int16  _drawnMenuX;

	// Following variables are used in "via mouse" mode
	int16 _mouseModeItemNr;
};

} // End of namespace Agi

#endif /* AGI_MENU_H */
