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
#include "director/lingo/xlibs/blitpict.h"

/**************************************************
 *
 * USED IN:
 * teamxtreme1
 * teamxtreme2
 *
 **************************************************/

/*
-- BlitPict effects factory. 29Jun94 RNB
BlitPict
I                mNew                --Creates a new instance of the XObject
X                mDispose            --Disposes of XObject instance
S                mName               --Returns the XObject name (BlitPict)
I                mStatus             --Returns an integer status code
SI               mError, code        --Returns an error string
S                mLastError          --Returns last error string
SSIIIII          mInit               --Initializer
SOIIII           mCopy               --Initializes from an existing object
IIIIIOIIIIIIII   mDraw               --Draws to a destinitation
IIIIIIIIIIII     mSparkle            --Draws a sparkle from a bitmap
 */

namespace Director {

const char *BlitPictXObj::xlibName = "blitpict";
const char *BlitPictXObj::fileNames[] = {
	"blitpict",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "new",				BlitPictXObj::m_new,		 0, 0,	400 },
	{ "dispose",				BlitPictXObj::m_dispose,		 0, 0,	400 },
	{ "name",				BlitPictXObj::m_name,		 0, 0,	400 },
	{ "status",				BlitPictXObj::m_status,		 0, 0,	400 },
	{ "error",				BlitPictXObj::m_error,		 1, 1,	400 },
	{ "lastError",				BlitPictXObj::m_lastError,		 0, 0,	400 },
	{ "init",				BlitPictXObj::m_init,		 6, 6,	400 },
	{ "copy",				BlitPictXObj::m_copy,		 5, 5,	400 },
	{ "draw",				BlitPictXObj::m_draw,		 13, 13,	400 },
	{ "sparkle",				BlitPictXObj::m_sparkle,		 11, 11,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

BlitPictXObject::BlitPictXObject(ObjectType ObjectType) :Object<BlitPictXObject>("BlitPictXObj") {
	_objType = ObjectType;
}

void BlitPictXObj::open(int type) {
	if (type == kXObj) {
		BlitPictXObject::initMethods(xlibMethods);
		BlitPictXObject *xobj = new BlitPictXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	} else if (type == kXtraObj) {
		// TODO - Implement Xtra
	}
}

void BlitPictXObj::close(int type) {
	if (type == kXObj) {
		BlitPictXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	} else if (type == kXtraObj) {
		// TODO - Implement Xtra
	}
}

void BlitPictXObj::m_new(int nargs) {
	if (nargs != 0) {
		warning("BlitPictXObj::m_new: expected 0 arguments");
		g_lingo->dropStack(nargs);
	}
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(BlitPictXObj::m_dispose)
XOBJSTUB(BlitPictXObj::m_name, "")
XOBJSTUB(BlitPictXObj::m_status, 0)
XOBJSTUB(BlitPictXObj::m_error, "")
XOBJSTUB(BlitPictXObj::m_lastError, "")
XOBJSTUB(BlitPictXObj::m_init, "")
XOBJSTUB(BlitPictXObj::m_copy, "")
XOBJSTUB(BlitPictXObj::m_draw, 0)
XOBJSTUB(BlitPictXObj::m_sparkle, 0)

}
