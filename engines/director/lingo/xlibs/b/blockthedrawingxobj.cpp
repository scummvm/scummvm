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
#include "director/lingo/xlibs/b/blockthedrawingxobj.h"

/**************************************************
 *
 * USED IN:
 * wttf
 *
 **************************************************/

/*
--		BlockTheDrawingXObj v 1.0d2 (c) 1995 Samizdat Productions.
--		All Rights Reserved.
--		Written by Christopher P. Kelly.
I		mNew
IIIII	mSetRect
I		mInstallPatch
I		mRemovePatch
I		mDisposeMem
 */

namespace Director {

const char *BlockTheDrawingXObj::xlibName = "BlockTheDrawingXObj";
const XlibFileDesc BlockTheDrawingXObj::fileNames[] = {
	{ "BlockTheDrawingXObj",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BlockTheDrawingXObj::m_new,		 0, 0,	400 },
	{ "setRect",				BlockTheDrawingXObj::m_setRect,		 4, 4,	400 },
	{ "installPatch",				BlockTheDrawingXObj::m_installPatch,		 0, 0,	400 },
	{ "removePatch",				BlockTheDrawingXObj::m_removePatch,		 0, 0,	400 },
	{ "disposeMem",				BlockTheDrawingXObj::m_disposeMem,		 0, 0,	400 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BlockTheDrawingXObject::BlockTheDrawingXObject(ObjectType ObjectType) :Object<BlockTheDrawingXObject>("BlockTheDrawingXObj") {
	_objType = ObjectType;
}

void BlockTheDrawingXObj::open(ObjectType type, const Common::Path &path) {
    BlockTheDrawingXObject::initMethods(xlibMethods);
    BlockTheDrawingXObject *xobj = new BlockTheDrawingXObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BlockTheDrawingXObj::close(ObjectType type) {
    BlockTheDrawingXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BlockTheDrawingXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BlockTheDrawingXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BlockTheDrawingXObj::m_setRect, 0)
XOBJSTUB(BlockTheDrawingXObj::m_installPatch, 0)
XOBJSTUB(BlockTheDrawingXObj::m_removePatch, 0)
XOBJSTUB(BlockTheDrawingXObj::m_disposeMem, 0)

}
