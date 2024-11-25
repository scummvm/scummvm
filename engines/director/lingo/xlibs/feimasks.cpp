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
 * -- FEIMasks External Factory.
 * FEIMasks
 * IPIIII mNew                --Creates a new instance of the XObject
 * X      mDispose            --Disposes of XObject instance
 * S      mName               --Returns the XObject name (FEIMask)
 * III    mMaskID             -- h,v
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/feimasks.h"


namespace Director {

const char *const FEIMasksXObj::xlibName = "FEIMasks";
const XlibFileDesc FEIMasksXObj::fileNames[] = {
	{ "FEIMasks",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",   FEIMasksXObj::m_new,					0,	0,	400 },	// D4
	{ "MaskID",   FEIMasksXObj::m_MaskID,			2,	2,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void FEIMasksXObj::open(ObjectType type, const Common::Path &path) {
   if (type == kXObj) {
		FEIMasksXObject::initMethods(xlibMethods);
		FEIMasksXObject *xobj = new FEIMasksXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
   }
}

void FEIMasksXObj::close(ObjectType type) {
   if (type == kXObj) {
		FEIMasksXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
   }
}

FEIMasksXObject::FEIMasksXObject(ObjectType ObjectType) : Object<FEIMasksXObject>("FEIMasks") {
	_objType = ObjectType;
}

void FEIMasksXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("FEIMasksXObj::new", nargs);
	g_lingo->push(g_lingo->_state->me);
}

void FEIMasksXObj::m_MaskID(int nargs) {
	g_lingo->printSTUBWithArglist("FEIMasksXObj::MaskID", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

} // End of namespace Director
