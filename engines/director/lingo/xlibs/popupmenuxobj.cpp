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

/*************************************
 *
 * USED IN:
 * The Apartment
 *
 *************************************/

/*
 * --PopMenu, Tool, 1.0, 4/1/90
 * --© 1989, 1990 MacroMind, Inc.
 * -- by Jeff Tanner
 * ------------------------------------------------
 * ------------------------------------------------
 * --  IMPORTANT NOTE:  In Lingo, build PopMenus after using
 * --    the command installMenu.  On the Mac, Pop-up menus are
 * --    extensions of the menu bar.  When the command installMenu
 * --    is called, this will remove all menus currently on
 * --    the menubar (including pop-up menus) and only install those
 * --    menus defined in the text window referenced by the
 * --    castNum parameter.  If installMenu is used after
 * --    creating PopMenus, these PopMenus must be disposed of
 * --    and then recreated.
 * ------------------------------------------------
 * --  MENULIST NOTE: In Lingo, there are several ways
 * --      to build the menu list for a PopUp menu.
 * --
 * --      - a menulist can be a continuous string
 * --          with items separated by semicolons.
 * --          example: "item1;item2;item3"
 * --
 * --      - a menulist can be a set of strings, each
 * --          representing an item, separated by
 * --          &return& and ending with &return
 * --          example: "item1"&return&"item2"&return
 * --
 * --      - a menulist could come from a cast member with
 * --          each menu item separated by a carrage return.
 * --          example: set menulist to the text of cast A31
 * ------------------------------------------------
 * --  MENUITEM NOTE: Use only alphanumeric characters,
 * --      0 - 9 and A - Z.  Avoid dashes.  Indicate
 * --      style by "item1;item2<B;item3<U"
 * --      To change an item's style, add < followed by a
 * --      character at the end of the menu item text:
 * --          <B  Bold
 * --          <U  Underlined
 * --          <I  Italics
 * --          <S  Shadowed
 * --          <O  Outlined
 * ------------------------------------------------
 * ------------------------------------------------
 * --=METHODS=--
 * XSI     mNew, menuList, menuID --Creates a new instance of the XObject.
 * --   menuItemList - "item1;item2<B;item3<U"
 * --      Separate all items with semicolons.
 * --      Maximum character length of menulist is 256.
 * --   menuID - to avoid resourse conflict with Director,
 * --      use a menu ID between 100 and 1000.
 * --
 * X   mDispose --Disposes of the XObject instance.
 * S   mName --Returns the name of the XObject.
 * ------------------------------------------------
 * ------------------------------------------------
 * XS  mAppendMenu, menuList --Adds items to menuList.
 * XI  mDisableItem, itemNum --Disables item in pop up menu.
 * XI  mEnableItem, itemNum --Enables item in pop up menu.
 * SI  mGetItem, itemNum --Returns item in pop up menu.
 * I   mGetMenuID --Returns the assigned Menu ID number.
 * ------------------------------------------------
 * ------------------------------------------------
 * IIII    mPopNum, Left, Top, itemNum --Returns selected item's number.
 * SIII    mPopText, Left, Top, itemNum --Returns selected item's text.
 * ------------------------------------------------
 * ------------------------------------------------
 * XIS mSetItem, itemNum, newItemText --Sets changes to an item in pop up menu
 * XI  mSetItemMark, markNum --Sets marker for pop up menu: default is check.
 * XI  mSmart, TrueOrFalse --Remembers last selection if itemNum is 0
 * XII mSetItemIcon, itemNum, iconID --Attaches an icon to menu item, id# 257 - 511
 * ------------------------------------------------
 * ------------------------------------------------
 */

#include "director/director.h"
#include "director/window.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/popupmenuxobj.h"

#include "graphics/macgui/macpopupmenu.h"

namespace Director {

const char *const PopUpMenuXObj::xlibName = "PopMenu";
const XlibFileDesc PopUpMenuXObj::fileNames[] = {
	{ "PopMenu",			nullptr },
	{ "PopUp Menu XObj",	nullptr },
	{ nullptr,				nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				PopUpMenuXObj::m_new,			 2, 2,	200 },	// D2
	{ "AppendMenu",			PopUpMenuXObj::m_appendMenu,	 1, 1,	200 },	// D2
	{ "DisableItem",		PopUpMenuXObj::m_disableItem,	 1, 1,	200 },	// D2
	{ "EnableItem",			PopUpMenuXObj::m_enableItem,	 1, 1,	200 },	// D2
	{ "GetItem",			PopUpMenuXObj::m_getItem,		 1, 1,  200 },	// D2
	{ "GetMenuID",			PopUpMenuXObj::m_getMenuID,		 0, 0,  200 },	// D2
	{ "PopNum",				PopUpMenuXObj::m_popNum,		 3, 3,  200 },	// D2
	{ "PopText",			PopUpMenuXObj::m_popText,		 3, 3,  200 },	// D2
	{ "SetItem",			PopUpMenuXObj::m_setItem,		 2, 2,  200 },	// D2
	{ "SetItemMark",		PopUpMenuXObj::m_setItemMark,	 1, 1,  200 },	// D2
	{ "Smart",				PopUpMenuXObj::m_smart,			 1, 1,  200 },	// D2
	{ "SetItemIcon",		PopUpMenuXObj::m_setItemIcon,	 2, 2,  200 },	// D2
	{ nullptr, nullptr, 0, 0, 0 }
};

void PopUpMenuXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		PopUpMenuXObject::initMethods(xlibMethods);
		PopUpMenuXObject *xobj = new PopUpMenuXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void PopUpMenuXObj::close(ObjectType type) {
	if (type == kXObj) {
		PopUpMenuXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


PopUpMenuXObject::PopUpMenuXObject(ObjectType ObjectType) : Object<PopUpMenuXObject>("PopMenu") {
	_objType = ObjectType;
}

void PopUpMenuXObj::m_new(int nargs) {
	PopUpMenuXObject *me = static_cast<PopUpMenuXObject *>(g_lingo->_state->me.u.obj);

	int menuId = g_lingo->pop().asInt();
	Common::String menuList = g_lingo->pop().asString();

	new Graphics::MacPopUp(menuId, g_director->_wm->getScreenBounds(), g_director->_wm, menuList.c_str());
	me->_menuId = menuId;

	g_lingo->push(g_lingo->_state->me);
}

void PopUpMenuXObj::m_popNum(int nargs) {
	PopUpMenuXObject *me = static_cast<PopUpMenuXObject *>(g_lingo->_state->me.u.obj);

	int itemNum = g_lingo->pop().asInt();
	int top = g_lingo->pop().asInt();
	int left = g_lingo->pop().asInt();

	// Convert window coordinates to screen coordinates
	Common::Rect windowRect = g_director->getCurrentWindow()->getInnerDimensions();
	int screenTop = top + windowRect.top - 1;
	int screenLeft = left + windowRect.left - 1;

	Graphics::MacPopUp *menu = static_cast<Graphics::MacPopUp *>(g_director->_wm->getMenu(me->_menuId));
	int selected = menu->drawAndSelectMenu(screenLeft, screenTop, itemNum);
	g_lingo->push(Datum(selected));
}

void PopUpMenuXObj::m_popText(int nargs) {
	PopUpMenuXObject *me = static_cast<PopUpMenuXObject *>(g_lingo->_state->me.u.obj);

	int itemNum = g_lingo->pop().asInt();
	int top = g_lingo->pop().asInt();
	int left = g_lingo->pop().asInt();

	// Convert window coordinates to screen coordinates
	Common::Rect windowRect = g_director->getCurrentWindow()->getInnerDimensions();
	int screenTop = top + windowRect.top - 1;
	int screenLeft = left + windowRect.left - 1;

	Graphics::MacPopUp *menu = static_cast<Graphics::MacPopUp *>(g_director->_wm->getMenu(me->_menuId));
	int selected = menu->drawAndSelectMenu(screenLeft, screenTop, itemNum);
	Common::String selectedText = menu->getItemText(selected);

	g_lingo->push(Datum(selectedText));
}

void PopUpMenuXObj::m_smart(int nargs) {
	PopUpMenuXObject *me = static_cast<PopUpMenuXObject *>(g_lingo->_state->me.u.obj);
	bool isSmart = g_lingo->pop().asInt() != 0;

	Graphics::MacPopUp *menu = static_cast<Graphics::MacPopUp *>(g_director->_wm->getMenu(me->_menuId));
	menu->setSmart(isSmart);
}

XOBJSTUBNR(PopUpMenuXObj::m_appendMenu)
XOBJSTUBNR(PopUpMenuXObj::m_disableItem)
XOBJSTUBNR(PopUpMenuXObj::m_enableItem)
XOBJSTUB(PopUpMenuXObj::m_getItem, "")
XOBJSTUB(PopUpMenuXObj::m_getMenuID, 0)
XOBJSTUBNR(PopUpMenuXObj::m_setItem)
XOBJSTUBNR(PopUpMenuXObj::m_setItemMark)
XOBJSTUBNR(PopUpMenuXObj::m_setItemIcon)

} // End of namespace Director
