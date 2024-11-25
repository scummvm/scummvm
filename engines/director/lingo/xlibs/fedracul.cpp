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
* Dracula's Secret
*
*************************************/

/*
 * -- FEDracul External Factory.
 * FEDracul
 * I     mNew                --Creates a new instance of the XObject
 * X     mDispose            --Disposes of XObject instance
 * S     mName               --Returns the XObject name (FEIMask)
 * IP    mCreateInventory    -- hPictOfBackground
 * IPII  mAddToInventory     -- hPictOfItem,row,column
 * P     mGetInventory       -- retrieve hPictOfInventory
 * I     mDestroyInventory   --
 * IIPI  mSetCharData        -- letter,hpicLetter,letterWidth
 * I     mIsWindows95
 * I     mDisableScreenSaver
 * II    mEnableScreenSaver
 * I     mProgramStartup
 * II    mProgramShutdown
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/fedracul.h"

namespace Director {

const char *const FEDraculXObj::xlibName = "FEDracul";
const XlibFileDesc FEDraculXObj::fileNames[] = {
	{ "FEDracul",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				FEDraculXObj::m_new,				0,	0,	400 },	// D4
	{ "AddToInventory",		FEDraculXObj::m_AddToInventory,		3,	3,	400 },	// D4
	{ "CreateInventory",	FEDraculXObj::m_CreateInventory,	1,	1,	400 },	// D4
	{ "DestroyInventory",	FEDraculXObj::m_DestroyInventory,	0,	0,	400 },	// D4
	{ "GetInventory",		FEDraculXObj::m_GetInventory,		0,	0,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void FEDraculXObj::open(ObjectType type, const Common::Path &path) {
   if (type == kXObj) {
		FEDraculXObject::initMethods(xlibMethods);
		FEDraculXObject *xobj = new FEDraculXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
   }
}

void FEDraculXObj::close(ObjectType type) {
   if (type == kXObj) {
		FEDraculXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


FEDraculXObject::FEDraculXObject(ObjectType ObjectType) : Object<FEDraculXObject>("FEDracul") {
	_objType = ObjectType;
}

void FEDraculXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void FEDraculXObj::m_AddToInventory(int nargs) {
	g_lingo->printSTUBWithArglist("FEDraculXObj::AddToInventory", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void FEDraculXObj::m_CreateInventory(int nargs) {
	g_lingo->printSTUBWithArglist("FEDraculXObj::CreateInventory", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void FEDraculXObj::m_DestroyInventory(int nargs) {
	g_lingo->printSTUBWithArglist("FEDraculXObj:mDestroyInventory", nargs);
	g_lingo->push(g_lingo->_state->me);
}

void FEDraculXObj::m_GetInventory(int nargs) {
	g_lingo->printSTUBWithArglist("FEDraculXObj::GetInventory", nargs);
	g_lingo->push(g_lingo->_state->me);
}

} // End of namespace Director
