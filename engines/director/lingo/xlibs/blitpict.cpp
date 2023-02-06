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

/**
 *  -- BlitPict effects factory. 29Jun94 RNB
 * BlitPict
 * I                mNew                --Creates a new instance of the XObject
 * X                mDispose            --Disposes of XObject instance
 * S                mName               --Returns the XObject name (BlitPict)
 * I                mStatus             --Returns an integer status code
 * SI               mError, code        --Returns an error string
 * S                mLastError          --Returns last error string
 * SSIIIII          mInit               --Initializer
 * SOIIII           mCopy               --Initializes from an existing object
 * IIIIIOIIIIIIII   mDraw               --Draws to a destinitation
 * IIIIIIIIIIII     mSparkle            --Draws a sparkle from a bitmap
 *
 * USED IN: teamxtreme2-win
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/blitpict.h"


namespace Director {

// The name is different from the obj filename.
const char *BlitPict::xlibName = "BlitPict";
const char *BlitPict::fileNames[] = {
	"BlitPict",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "new",		BlitPict::m_new,			 0, 0,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void BlitPict::open(int type) {
	if (type == kXObj) {
		BlitPictXObject::initMethods(xlibMethods);
		BlitPictXObject *xobj = new BlitPictXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void BlitPict::close(int type) {
	if (type == kXObj) {
		BlitPictXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


BlitPictXObject::BlitPictXObject(ObjectType ObjectType) : Object<BlitPictXObject>("BlitPict") {
	_objType = ObjectType;
}

void BlitPict::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

} // End of namespace Director
