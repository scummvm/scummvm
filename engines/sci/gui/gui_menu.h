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
	SCI_MENU_ATTRIBUTE_KEY		= 0x6f,
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

	GuiMenuEntry(uint16 id_)
	 : id(id_) { }
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
	Common::String said;
	Common::String text;
	Common::String textRightAligned;

	GuiMenuItemEntry(uint16 menuId_, uint16 id_)
	 : menuId(menuId_), id(id_),
		enabled(true), tag(0), keyPress(0), keyModifier(0), separatorLine(false) { }
};
typedef Common::List<GuiMenuItemEntry *> GuiMenuItemList;

class SciGuiMenu {
public:
	SciGuiMenu(SciGuiGfx *gfx, SciGuiText *text, SciGuiScreen *screen);
	~SciGuiMenu();

	void add(Common::String title, Common::String content);
	void setAttribute(uint16 menuId, uint16 itemId, uint16 attributeId, reg_t value);
	reg_t getAttribute(uint16 menuId, uint16 itemId, uint16 attributeId);

	void drawBar();

private:

	SciGuiGfx *_gfx;
	SciGuiText *_text;
	SciGuiScreen *_screen;

	uint16 _listCount;
	GuiMenuList _list;
	GuiMenuItemList _itemList;
};

} // End of namespace Sci

#endif
