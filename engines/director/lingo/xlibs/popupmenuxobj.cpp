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

/*
 * mAppendMenu, menuList  adds items to menuList
 * mDisableItem, itemNum  disables item in pop up menu
 * mEnableItem, itemNum  enables item in pop up menu
 * mGetItem, itemNum  returns item in pop up menu
 * mGetMenuID  returns assigned menuID number
 * mPopNum, Left, Top, itemNum  returns selected
 *     item’s number
 * mPopText, Left, Top, itemNum  returns selected
 *     item’s text
 * mSetItem, itemNum, newItemText  sets changes to an
 *     item in pop up menu
 * mSetItemMark, markNum  sets marker for pop up menu,
 *     default is check
 * mSmart, TrueOrFalse  remembers last selection if itemNum
 *    is 0 (there was no item selected by user)
 * mSetItemIcon, itemNum, iconID  attaches an icon to
 *    menu item; icons should have IDs from 257 to 511
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/popupmenuxobj.h"


namespace Director {

const char *PopUpMenuXObj::xlibName = "PopMenu";
const char *PopUpMenuXObj::fileNames[] = {
	"PopMenu",
	"PopUp Menu XObj",
	nullptr
};

static MethodProto xlibMethods[] = {
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

void PopUpMenuXObj::open(int type) {
	if (type == kXObj) {
		PopUpMenuXObject::initMethods(xlibMethods);
		PopUpMenuXObject *xobj = new PopUpMenuXObject(kXObj);
		g_lingo->_globalvars[xlibName] = xobj;
	}
}

void PopUpMenuXObj::close(int type) {
	if (type == kXObj) {
		PopUpMenuXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


PopUpMenuXObject::PopUpMenuXObject(ObjectType ObjectType) :Object<PopUpMenuXObject>("PopMenu") {
	_objType = ObjectType;
}

void PopUpMenuXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("PopUpMenuXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_currentMe);
}

void PopUpMenuXObj::m_appendMenu(int nargs) {
	g_lingo->printSTUBWithArglist("PopUpMenuXObj::m_appendMenu", nargs);
	g_lingo->dropStack(nargs);
}

void PopUpMenuXObj::m_disableItem(int nargs) {
	g_lingo->printSTUBWithArglist("PopUpMenuXObj::m_disableItem", nargs);
	g_lingo->dropStack(nargs);
}

void PopUpMenuXObj::m_enableItem(int nargs) {
	g_lingo->printSTUBWithArglist("PopUpMenuXObj::m_enableItem", nargs);
	g_lingo->dropStack(nargs);
}

void PopUpMenuXObj::m_getItem(int nargs) {
	g_lingo->printSTUBWithArglist("PopUpMenuXObj::m_getItem", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void PopUpMenuXObj::m_getMenuID(int nargs) {
	g_lingo->printSTUBWithArglist("PopUpMenuXObj::m_getMenuID", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void PopUpMenuXObj::m_popNum(int nargs) {
	g_lingo->printSTUBWithArglist("PopUpMenuXObj::m_popNum", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void PopUpMenuXObj::m_popText(int nargs) {
	g_lingo->printSTUBWithArglist("PopUpMenuXObj::m_popText", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void PopUpMenuXObj::m_setItem(int nargs) {
	g_lingo->printSTUBWithArglist("PopUpMenuXObj::m_setItem", nargs);
	g_lingo->dropStack(nargs);
}

void PopUpMenuXObj::m_setItemMark(int nargs) {
	g_lingo->printSTUBWithArglist("PopUpMenuXObj::m_setItemMark", nargs);
	g_lingo->dropStack(nargs);
}

void PopUpMenuXObj::m_smart(int nargs) {
	g_lingo->printSTUBWithArglist("PopUpMenuXObj::m_smart", nargs);
	g_lingo->dropStack(nargs);
}

void PopUpMenuXObj::m_setItemIcon(int nargs) {
	g_lingo->printSTUBWithArglist("PopUpMenuXObj::m_setItemIcon", nargs);
	g_lingo->dropStack(nargs);
}

} // End of namespace Director
