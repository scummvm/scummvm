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

#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/b/budmenu.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra BudMenu
new object me

-- Quick menu display --
* bmMenu list Items, integer Flags -- shows menu, at current mouse position
* bmMenuAt list Items, integer Flags, integer X, integer Y  -- shows menu, at specified position

-- Menu creation functions --
* bmCreateMenu -- creates new empty menu
* bmAppendItem integer Menu, string Item -- adds Item to end of Menu
* bmAppendItems integer Menu, list Items -- adds Items to end of Menu
* bmInsertItem integer Menu, string Item, string beforeItem -- inserts Item to Menu before beforeItem
* bmInsertItems integer Menu, list Items, string beforeItem -- inserts Items to Menu before beforeItem
* bmInsertItemAt integer Menu, string Item, integer Pos -- inserts Item to Menu at Pos
* bmInsertItemsAt integer Menu, list Items, integer Pos -- inserts Items to Menu at Pos

-- Menu modification functions --
* bmEnableItem integer Menu, string Item -- enables item
* bmDisableItem integer Menu, string Item -- disables item
* bmItemDisabled integer Menu, string Item -- returns true if item is disabled
* bmCheckItem integer Menu, string Item -- checks item
* bmUncheckItem integer Menu, string Item -- unchecks item
* bmItemChecked integer Menu, string Item -- returns true if item is checked
* bmChangeItemText integer Menu, string Item, string NewText -- changes item text
* bmDeleteItem integer Menu, string Item -- removes item

-- SubMenu functions --
* bmAttachSubMenu integer Menu, string Item, integer SubMenu -- attaches sub menu to item
* bmDetachSubMenu integer Menu, string Item -- detaches sub menu from item

-- Menu display functions --
* bmShowMenu integer Menu, integer Flags -- displays menu at current mouse position
* bmShowMenuAt integer Menu, integer X, integer Y  -- displays menu, at specified position
* bmDestroyMenu integer menu -- destroys Menu and releases memory
"
 */

namespace Director {

const char *BudmenuXtra::xlibName = "Budmenu";
const XlibFileDesc BudmenuXtra::fileNames[] = {
	{ "budmenu",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BudmenuXtra::m_new,		 0, 0,	500 },
	{ """,				BudmenuXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "bmMenu", BudmenuXtra::m_bmMenu, 2, 2, 500, HBLTIN },
	{ "bmMenuAt", BudmenuXtra::m_bmMenuAt, 4, 4, 500, HBLTIN },
	{ "bmCreateMenu", BudmenuXtra::m_bmCreateMenu, 0, 0, 500, HBLTIN },
	{ "bmAppendItem", BudmenuXtra::m_bmAppendItem, 2, 2, 500, HBLTIN },
	{ "bmAppendItems", BudmenuXtra::m_bmAppendItems, 2, 2, 500, HBLTIN },
	{ "bmInsertItem", BudmenuXtra::m_bmInsertItem, 3, 3, 500, HBLTIN },
	{ "bmInsertItems", BudmenuXtra::m_bmInsertItems, 3, 3, 500, HBLTIN },
	{ "bmInsertItemAt", BudmenuXtra::m_bmInsertItemAt, 3, 3, 500, HBLTIN },
	{ "bmInsertItemsAt", BudmenuXtra::m_bmInsertItemsAt, 3, 3, 500, HBLTIN },
	{ "bmEnableItem", BudmenuXtra::m_bmEnableItem, 2, 2, 500, HBLTIN },
	{ "bmDisableItem", BudmenuXtra::m_bmDisableItem, 2, 2, 500, HBLTIN },
	{ "bmItemDisabled", BudmenuXtra::m_bmItemDisabled, 2, 2, 500, HBLTIN },
	{ "bmCheckItem", BudmenuXtra::m_bmCheckItem, 2, 2, 500, HBLTIN },
	{ "bmUncheckItem", BudmenuXtra::m_bmUncheckItem, 2, 2, 500, HBLTIN },
	{ "bmItemChecked", BudmenuXtra::m_bmItemChecked, 2, 2, 500, HBLTIN },
	{ "bmChangeItemText", BudmenuXtra::m_bmChangeItemText, 3, 3, 500, HBLTIN },
	{ "bmDeleteItem", BudmenuXtra::m_bmDeleteItem, 2, 2, 500, HBLTIN },
	{ "bmAttachSubMenu", BudmenuXtra::m_bmAttachSubMenu, 3, 3, 500, HBLTIN },
	{ "bmDetachSubMenu", BudmenuXtra::m_bmDetachSubMenu, 2, 2, 500, HBLTIN },
	{ "bmShowMenu", BudmenuXtra::m_bmShowMenu, 2, 2, 500, HBLTIN },
	{ "bmShowMenuAt", BudmenuXtra::m_bmShowMenuAt, 3, 3, 500, HBLTIN },
	{ "bmDestroyMenu", BudmenuXtra::m_bmDestroyMenu, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BudmenuXtraObject::BudmenuXtraObject(ObjectType ObjectType) :Object<BudmenuXtraObject>("Budmenu") {
	_objType = ObjectType;
}

bool BudmenuXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BudmenuXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BudmenuXtra::xlibName);
	warning("BudmenuXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BudmenuXtra::open(ObjectType type, const Common::Path &path) {
    BudmenuXtraObject::initMethods(xlibMethods);
    BudmenuXtraObject *xobj = new BudmenuXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BudmenuXtra::close(ObjectType type) {
    BudmenuXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BudmenuXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BudmenuXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BudmenuXtra::m_bmMenu, 0)
XOBJSTUB(BudmenuXtra::m_bmMenuAt, 0)
XOBJSTUB(BudmenuXtra::m_bmCreateMenu, 0)
XOBJSTUB(BudmenuXtra::m_bmAppendItem, 0)
XOBJSTUB(BudmenuXtra::m_bmAppendItems, 0)
XOBJSTUB(BudmenuXtra::m_bmInsertItem, 0)
XOBJSTUB(BudmenuXtra::m_bmInsertItems, 0)
XOBJSTUB(BudmenuXtra::m_bmInsertItemAt, 0)
XOBJSTUB(BudmenuXtra::m_bmInsertItemsAt, 0)
XOBJSTUB(BudmenuXtra::m_bmEnableItem, 0)
XOBJSTUB(BudmenuXtra::m_bmDisableItem, 0)
XOBJSTUB(BudmenuXtra::m_bmItemDisabled, 0)
XOBJSTUB(BudmenuXtra::m_bmCheckItem, 0)
XOBJSTUB(BudmenuXtra::m_bmUncheckItem, 0)
XOBJSTUB(BudmenuXtra::m_bmItemChecked, 0)
XOBJSTUB(BudmenuXtra::m_bmChangeItemText, 0)
XOBJSTUB(BudmenuXtra::m_bmDeleteItem, 0)
XOBJSTUB(BudmenuXtra::m_bmAttachSubMenu, 0)
XOBJSTUB(BudmenuXtra::m_bmDetachSubMenu, 0)
XOBJSTUB(BudmenuXtra::m_bmShowMenu, 0)
XOBJSTUB(BudmenuXtra::m_bmShowMenuAt, 0)
XOBJSTUB(BudmenuXtra::m_bmDestroyMenu, 0)
XOBJSTUB(BudmenuXtra::m_", 0)

}
