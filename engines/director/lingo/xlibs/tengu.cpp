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
#include "director/lingo/xlibs/tengu.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
--Tengu External Factory. 1996
--Tengu
I                  mNew                        --Creates a new inctance of XObject
X                      mDispose                --Dispose of XObject instance
IIIIS          mSetMenu                --Setting Menu Movie Variable
IIII           mGameStart              --Setting Game Start Variable
IIS            mSetArea                --Setting Area Data
XIIII          mSetShikake             --Setting Shikake Data
XIII           mSetEsa                 --Setting Esa Data
III            mGetTime                --Getting Now Time
II                     mGetPal                 --Getting Pallete Data
II                     mGetAtari               --Getting Atari Data
XI                     mSetNode                --Setting Node Data
III            mGetUkiLoc              --Getting Uki Location Data
I                      mGetUkiTime             --Getting Uki Shinking and Floating Time
II                     mGetSaoDepth    --Getting Water Depth in Now Sao
XI                     mContinue               --Setting Data in Game Continueing
IIII           mGetValue               --Getting A Set Of Values
I                      mGetFuna                --Getting Funa Data
II                     mWriteData              --Writting Binary Data
II                     mReadData               --Reading Binary Data
IIS            mAlert                  --Setting Alert Mode
XII            mSetScore               --Setting Score Data
XII            mSetChoka               --Setting Choka Data
ISS            mGyotaku                --Writting Gyotaku data
II                     mESearch                --Searching Event Data
XIII           mTournament             --Setting Tournament Data
X                      mSetYudachi             --Setting Shower Mode
IIS            mSetEvent               --Setting Event Data
 */

namespace Director {

const char *TenguXObj::xlibName = "Tengu";
const XlibFileDesc TenguXObj::fileNames[] = {
	{ "TENGU",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				TenguXObj::m_new,		 0, 0,	400 },
	{ "dispose",				TenguXObj::m_dispose,		 0, 0,	400 },
	{ "setMenu",				TenguXObj::m_setMenu,		 4, 4,	400 },
	{ "gameStart",				TenguXObj::m_gameStart,		 3, 3,	400 },
	{ "setArea",				TenguXObj::m_setArea,		 2, 2,	400 },
	{ "setShikake",				TenguXObj::m_setShikake,		 4, 4,	400 },
	{ "setEsa",				TenguXObj::m_setEsa,		 3, 3,	400 },
	{ "getTime",				TenguXObj::m_getTime,		 2, 2,	400 },
	{ "getPal",				TenguXObj::m_getPal,		 1, 1,	400 },
	{ "getAtari",				TenguXObj::m_getAtari,		 1, 1,	400 },
	{ "setNode",				TenguXObj::m_setNode,		 1, 1,	400 },
	{ "getUkiLoc",				TenguXObj::m_getUkiLoc,		 2, 2,	400 },
	{ "getUkiTime",				TenguXObj::m_getUkiTime,		 0, 0,	400 },
	{ "getSaoDepth",				TenguXObj::m_getSaoDepth,		 1, 1,	400 },
	{ "continue",				TenguXObj::m_continue,		 1, 1,	400 },
	{ "getValue",				TenguXObj::m_getValue,		 3, 3,	400 },
	{ "getFuna",				TenguXObj::m_getFuna,		 0, 0,	400 },
	{ "writeData",				TenguXObj::m_writeData,		 1, 1,	400 },
	{ "readData",				TenguXObj::m_readData,		 1, 1,	400 },
	{ "alert",				TenguXObj::m_alert,		 2, 2,	400 },
	{ "setScore",				TenguXObj::m_setScore,		 2, 2,	400 },
	{ "setChoka",				TenguXObj::m_setChoka,		 2, 2,	400 },
	{ "gyotaku",				TenguXObj::m_gyotaku,		 2, 2,	400 },
	{ "eSearch",				TenguXObj::m_eSearch,		 1, 1,	400 },
	{ "tournament",				TenguXObj::m_tournament,		 3, 3,	400 },
	{ "setYudachi",				TenguXObj::m_setYudachi,		 0, 0,	400 },
	{ "setEvent",				TenguXObj::m_setEvent,		 2, 2,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

TenguXObject::TenguXObject(ObjectType ObjectType) :Object<TenguXObject>("Tengu") {
	_objType = ObjectType;
}

void TenguXObj::open(ObjectType type, const Common::Path &path) {
    TenguXObject::initMethods(xlibMethods);
    TenguXObject *xobj = new TenguXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void TenguXObj::close(ObjectType type) {
    TenguXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void TenguXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("TenguXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(TenguXObj::m_dispose)
XOBJSTUB(TenguXObj::m_setMenu, 0)
XOBJSTUB(TenguXObj::m_gameStart, 0)
XOBJSTUB(TenguXObj::m_setArea, 0)
XOBJSTUBNR(TenguXObj::m_setShikake)
XOBJSTUBNR(TenguXObj::m_setEsa)
XOBJSTUB(TenguXObj::m_getTime, 10)
XOBJSTUB(TenguXObj::m_getPal, 0)
XOBJSTUB(TenguXObj::m_getAtari, 0)
XOBJSTUBNR(TenguXObj::m_setNode)
XOBJSTUB(TenguXObj::m_getUkiLoc, 0)
XOBJSTUB(TenguXObj::m_getUkiTime, 0)
XOBJSTUB(TenguXObj::m_getSaoDepth, 0)
XOBJSTUBNR(TenguXObj::m_continue)
XOBJSTUB(TenguXObj::m_getValue, 0)
XOBJSTUB(TenguXObj::m_getFuna, 0)
XOBJSTUB(TenguXObj::m_writeData, 0)
XOBJSTUB(TenguXObj::m_readData, 0)
XOBJSTUB(TenguXObj::m_alert, 0)
XOBJSTUBNR(TenguXObj::m_setScore)
XOBJSTUBNR(TenguXObj::m_setChoka)
XOBJSTUB(TenguXObj::m_gyotaku, 0)
XOBJSTUB(TenguXObj::m_eSearch, 0)
XOBJSTUBNR(TenguXObj::m_tournament)
XOBJSTUBNR(TenguXObj::m_setYudachi)
XOBJSTUB(TenguXObj::m_setEvent, 0)

}
