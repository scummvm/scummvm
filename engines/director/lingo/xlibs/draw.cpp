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
 * מיץ פטל (Mitz Petel)
 *
 *************************************/

/* -- Draw XObject version 1.0 beta
 * Draw
 * -- (c) 1995 - Daniele Russo
 * -- New York University - Center for Digital Multimedia
 * I        mNew                --Creates a new instance of the XObject.
 * --
 * I        mDispose            --Disposes of an XObject instance.
 * --
 * IIIII    mLine               --Draws a line.
 * --
 * IIIII    mLineBrush          --Draws a line using a pen.
 * --
 * IIIIII   mLineBrushTrans     --Draws a line using a transparent, multicolored pen.
 * --
 * IIIIIII  mLineBrushCol       --Draws a line using a transparent pen.
 * --
 * III      mFilterBMP          --Gets a picture containing all the pixels in an
 * --                             image excluding those which are of a particular color.
 * --                             The image to filter must be on the clipboard (as a BITMAP).
 * --
 * III      mFilterDIB          --As above, but the image must be a DIB.
 * --
 * III      mFilterBMP128       --Gets a picture containing all the pixels in an
 * --                             image whose color is within the lower/upper
 * --                             128. The image to filter must be on the clipboard
 * --                             (as a BITMAP).
 * --
 * III      mFilterDIB128       --As above, but the image must be a DIB.
 * --
 * III      mFilterBMPMakeGhostImage
 * --                             Gets a picture which has all the pixels that don't
 * --                             have the background color changed to a given color,
 * --                             specified by the caller. The image to filter must be
 * --                             on the clipboard (as a BITMAP).
 * --
 * III      mFilterDIBMakeGhostImage
 * --                             As above, but the image must be a DIB.
 * --
 * I        mEmptyClipboard     --Empties the clipboard. This method MUST be called
 * --                             after using the result of mFilterBMP and mFilterBMP128.
 * --
 * IIII     mFill               --Fills an area starting at a specified pixel
 * --                             and defined by all the adjacent pixels with
 * --                             the same color.
 * --
 * III      mGetColor           --Gets the color of a pixel on the stage. The
 * --                             result is the index of the color in Director's
 * --                             palette.
 * --
 * IIIIII   mDrawRect           -- Draws a rectangle of a specified color.
 * --
 * IIIIII   mDrawFrame          -- Draws a frame using a dotted pen.
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/draw.h"


namespace Director {

const char *const DrawXObj::xlibName = "Draw";
const XlibFileDesc DrawXObj::fileNames[] = {
	{ "DRAW",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "New",						DrawXObj::m_new,						0,	0,	400 },	// D4
	{ "Dispose",					DrawXObj::m_dispose,					0,	0,	400 },	// D4
	{ "Line",						DrawXObj::m_line,						4,	4,	400 },	// D4
	{ "LineBrush",					DrawXObj::m_lineBrush,					4,	4,	400 },	// D4
	{ "LineBrushTrans",				DrawXObj::m_lineBrushTrans,				5,	5,	400 },	// D4
	{ "LineBrushCol",				DrawXObj::m_lineBrushCol,				6,	6,	400 },	// D4
	{ "FilterBMP",					DrawXObj::m_filterBMP,					2,	2,	400 },	// D4
	{ "FilterDIB",					DrawXObj::m_filterDIB,					2,	2,	400 },	// D4
	{ "FilterBMP128",				DrawXObj::m_filterBMP128,				2,	2,	400 },	// D4
	{ "FilterDIB128",				DrawXObj::m_filterDIB128,				2,	2,	400 },	// D4
	{ "FilterBMPMakeGhostImage",	DrawXObj::m_filterBMPMakeGhostImage,	2,	2,	400 },	// D4
	{ "FilterDIBMakeGhostImage",	DrawXObj::m_filterDIBMakeGhostImage,	2,	2,	400 },	// D4
	{ "EmptyClipboard",				DrawXObj::m_emptyClipboard,				0,	0,	400 },	// D4
	{ "Fill",						DrawXObj::m_fill,						3,	3,	400 },	// D4
	{ "GetColor",					DrawXObj::m_getColor,					2,	2,	400 },	// D4
	{ "DrawRect",					DrawXObj::m_drawRect,					5,	5,	400 },	// D4
	{ "DrawFrame",					DrawXObj::m_drawFrame,					5,	5,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void DrawXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		DrawXObject::initMethods(xlibMethods);
		DrawXObject *xobj = new DrawXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void DrawXObj::close(ObjectType type) {
	if (type == kXObj) {
		DrawXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


DrawXObject::DrawXObject(ObjectType ObjectType) :Object<DrawXObject>("Draw") {
	_objType = ObjectType;
}

void DrawXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(DrawXObj::m_dispose, 0)
XOBJSTUB(DrawXObj::m_line, 0)
XOBJSTUB(DrawXObj::m_lineBrush, 0)
XOBJSTUB(DrawXObj::m_lineBrushTrans, 0)
XOBJSTUB(DrawXObj::m_lineBrushCol, 0)
XOBJSTUB(DrawXObj::m_filterBMP, 0)
XOBJSTUB(DrawXObj::m_filterDIB, 0)
XOBJSTUB(DrawXObj::m_filterBMP128, 0)
XOBJSTUB(DrawXObj::m_filterDIB128, 0)
XOBJSTUB(DrawXObj::m_filterBMPMakeGhostImage, 0)
XOBJSTUB(DrawXObj::m_filterDIBMakeGhostImage, 0)
XOBJSTUB(DrawXObj::m_emptyClipboard, 0)
XOBJSTUB(DrawXObj::m_fill, 0)
XOBJSTUB(DrawXObj::m_getColor, 0)
XOBJSTUB(DrawXObj::m_drawRect, 0)
XOBJSTUB(DrawXObj::m_drawFrame, 0)

} // End of namespace Director
