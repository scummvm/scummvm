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
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_GUI_MENU_H
#define SCI_GUI_MENU_H

namespace Sci {

enum {
	SCI_MENU_ATTRIBUTE_SAID		= 0x6d,
	SCI_MENU_ATTRIBUTE_TEXT		= 0x6e,
	SCI_MENU_ATTRIBUTE_KEYPRESS	= 0x6f,
	SCI_MENU_ATTRIBUTE_ENABLED	= 0x70,
	SCI_MENU_ATTRIBUTE_TAG		= 0x71
};

enum {
	SCI_MENU_REPLACE_ONCONTROL	= 0x03,
	SCI_MENU_REPLACE_ONALT		= 0x02,
	SCI_MENU_REPLACE_ONFUNCTION	= 'F'
};

struct GuiMenuEntry {
	uint16 id;
	Common::String text;
	int16 textWidth;

	GuiMenuEntry(uint16 curId)
	 : id(curId), textWidth(0) { }
};
typedef Common::List<GuiMenuEntry *> GuiMenuList;

struct GuiMenuItemEntry {
	uint16 menuId;
	uint16 id;
	bool enabled;
	uint16 tag;
	uint16 keyPress;
	uint16 keyModifier;
	bool separatorLine;
	reg_t saidVmPtr;
	Common::String text;
	reg_t textVmPtr;
	int16 textWidth;
	Common::String textRightAligned;
	int16 textRightAlignedWidth;

	GuiMenuItemEntry(uint16 curMenuId, uint16 curId)
	 : menuId(curMenuId), id(curId),
		enabled(true), tag(0), keyPress(0), keyModifier(0), separatorLine(false), textWidth(0), textRightAlignedWidth(0) {
		saidVmPtr = NULL_REG;
		textVmPtr = NULL_REG;
	}
};
typedef Common::List<GuiMenuItemEntry *> GuiMenuItemList;

class SciGuiMenu {
public:
	SciGuiMenu(SegManager *segMan, SciGuiGfx *gfx, SciGuiText *text, SciGuiScreen *screen, SciGuiCursor *cursor);
	~SciGuiMenu();

	void init(GfxState *gfxstate);
	void reset();
	void add(Common::String title, Common::String content, reg_t contentVmPtr);
	void setAttribute(uint16 menuId, uint16 itemId, uint16 attributeId, reg_t value);
	reg_t getAttribute(uint16 menuId, uint16 itemId, uint16 attributeId);

	void drawBar();
	reg_t select(reg_t eventObject);

private:
	GuiMenuItemEntry *findItem(uint16 menuId, uint16 itemId);
	void calculateTextWidth();
	void drawMenu(uint16 oldMenuId, uint16 newMenuId);
	void invertMenuSelection(uint16 itemId);
	GuiMenuItemEntry *interactiveWithKeyboard();
	GuiMenuItemEntry *interactiveWithMouse();
	GuiMenuItemEntry *interactiveGetItem(uint16 menuId, uint16 itemId, bool menuChanged);

	GfxState *_gfxstate;
	SegManager *_segMan;
	SciGuiGfx *_gfx;
	SciGuiText *_text;
	SciGuiScreen *_screen;
	SciGuiCursor *_cursor;

	uint16 _listCount;
	GuiMenuList _list;
	GuiMenuItemList _itemList;

	uint16 _curMenuId;
	uint16 _curItemId;

	GuiPort *_oldPort;
	GuiMemoryHandle _barSaveHandle;
	GuiMemoryHandle _menuSaveHandle;
	Common::Rect _menuRect;
};

} // End of namespace Sci

#endif
