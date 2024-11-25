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
 * Majestic-mac
 * chopsuey-mac
 * Jewels of the Oracle - Mac
 *
 *************************************/

/*
 * Quicktime movies and PICT castmembers continually display
 * in wrong colors after an 8 palette switch
 * this XObject can be used to patch this problem
 * use mPatchIt message on the same frame as the palette switch
 * pass in the stage window coordinates when creating the XObject
 *
 * From: http://www.zeusprod.com/technote/patchpal.html
 * The FixPalette XObject is needed when using QuickTime movies with
 * more than one custom palette. If it not used on the PC, nor is it
 * needed if you are only using one custom palette. If the first
 * QuickTime you play looks fine, but the second QuickTime video
 * that is played looks funky or psychedelic, then there is a good
 * chance that the FixPaletet XObject will solve your problem.
 *
 * It's only necessary on Mac.
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/palxobj.h"


namespace Director {

// The name is different from the obj filename.
const char *const PalXObj::xlibName = "FixPalette";
const XlibFileDesc PalXObj::fileNames[] = {
	{ "PalXObj",		nullptr },
	{ "FixPalette",		nullptr },
	{ "FixPaletteXObj",	nullptr },
	{ "PALETTE.XOB",    nullptr }, //  Jewels of the Oracle - Mac
	{ nullptr,			nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				PalXObj::m_new,				 4, 4,	400 },	// D4
	{ "PatchIt",			PalXObj::m_patchIt,			 0, 0,  400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void PalXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		PalXObject::initMethods(xlibMethods);
		PalXObject *xobj = new PalXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void PalXObj::close(ObjectType type) {
	if (type == kXObj) {
		PalXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


PalXObject::PalXObject(ObjectType ObjectType) :Object<PalXObject>("FixPalette") {
	_objType = ObjectType;
}

void PalXObj::m_new(int nargs) {
	PalXObject *me = static_cast<PalXObject *>(g_lingo->_state->me.u.obj);

	Common::Rect rect;
	rect.bottom = g_lingo->pop().asInt();
	rect.right = g_lingo->pop().asInt();
	rect.top = g_lingo->pop().asInt();
	rect.left  = g_lingo->pop().asInt();
	me->_stageWindowCoordinates = rect;

	g_lingo->push(g_lingo->_state->me);
}

void PalXObj::m_patchIt(int nargs) {
	warning("STUB: PalXObj::m_patchIt");
}

} // End of namespace Director
