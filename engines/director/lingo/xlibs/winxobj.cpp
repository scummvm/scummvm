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
 * Alice: An Interactive Museum
 *
 *************************************/

/*
 * RearWindow is a Mac only XObject. Its purpose is to cover the screen
 * with either a 1-bit pattern, indexed color, direct(RGB) color, bitmappedCastMember
 * or PICT file picture.
 *
 * It does this when the Stage size is smaller than the monitor screen.
 *
 * Implemented as a no-op, since ScummVM doesn't handle desktop backgrounds.
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/winxobj.h"


namespace Director {

const char *const RearWindowXObj::xlibName = "RearWindow";
const XlibFileDesc RearWindowXObj::fileNames[] = {
	{ "RearWindow",			nullptr },
	{ "RearWindow.Xobj",	nullptr },
	{ "RearWindow XObj",	nullptr },
	{ "RW.XOB",				nullptr },
	{ "winXObj",			nullptr },
	{ nullptr,				nullptr },
};

/*
-- RearWindow.XObj by David Jackson-Shields
--   vers. 1.0.2 (10/11/93)
--   © 1992-93 by David Jackson-Shields
--   All Rights Reserved.
--
--   Includes code from the XObject Developers Kit
--   © 1989-93 by Macromedia Inc.
--
-- Purpose of the XObject:
--   Covers the Finder desktop (behind the Director Stage) with a window
--   containing either a 1-bit pattern, indexed color, direct (RGB) color,
--   bitmapped castMember, or PICT file picture.
--
--   This XObject is for when the Stage size is be smaller than the monitor screen,
--   for covering the Finder Desktop behind the Stage. It requires system 6.0.5
--   or later with Director 3.0 or later. It also provides utility methods for
--   getting the monitor screen size (top, left, bottom, right). In cases where
--   there are multiple monitors, these utility methods return a Rect which contains
--   the minimum bounding rect which contains all monitors. Another utility method
--   returns the name of the current application. Subsequent methods create the Rear
--   Window and fill it with the appropriate color, 1-bit pattern, or image.
--
--	 NOTE: When using ResEdit to install this XObject in the resource fork of
--   the movie or projector, be sure to copy the RearWindow WDEF resource as well.
--   This custom Window Definition Procedure prevents accidental clicks on the
--   the RearWindow from bringing it forward, obscuring the Director Stage.
--
IS mNew -- creates the object in RAM. It only takes one argument.
--   (1) The argument specifies multiple or single screen devices to be covered.
--       Use either "M" for multiple, or "S" for single monitor coverage.
--       If you only have only one monitor, you can still use an "M" argument. In fact,
--       the only time a Single-Monitor would be specified would be if you expect a
--       low-memory situation, where the RearWindow plus the size of cast or PICT
--       image would take up more than the largest available freeBlock of memory.
--
--   The mNew method returns system error codes (if any)..otherwise it returns
--   the object handle (a memory address)...like all other XObjects.
--   Example of Lingo syntax:
--   global myObj
--   if objectP( myObj ) then myObj( mDispose )
--   -- [ "M" indicates multiple monitors.]
--   set myObj= RearWindow( mNew, "M" )
--   set resultCode = value( myObj )
--   if resultCode < 0 then
--     alert "System Error trying to create the RearWindow" && string( resultCode )
--   end if
--
S   mGetAppName -- returns name of current application, so you can test for either
--   "Macromedia Director 3.x", "MacroMind Player 3.x", or the name of your projector.
--   Example of Lingo syntax:
--   global myObj
--   if objectP( myObj ) then
--      put myObj( mGetAppName ) into returnStr
--   end if
--
I  mGetMemoryNeeded  -- Returns number of Bytes needed to create a RearWindow
--  for all screen devices. Compare this with the Lingo function 'the freeBlock'.
--  If the mNew method specified "Single" monitor configuration, then
--  this refers to the number of Bytes for only one monitor. See the
--  RearWindow Example Movie for how to use this with Lingo
--
--  Example of Lingo syntax:
--  global myObj
--  set memNeeded = myObj( mGetMemoryNeeded )
--
I  mGetScreenTop  -- Returns "top" pixel coordinate for all screens
--                  (refers to minimum rect surrounding multiple monitors)
--
--  Example of Lingo syntax:
--  global myObj
--  set theScreenTop = myObj( mGetScreenTop )
--
I  mGetScreenLeft -- Returns "left" pixel coordinate of all screen areas
--                  (refers to minimum rect surrounding multiple monitors)
--
--  Example of Lingo syntax:
--  global myObj
--  set theScreenLeft = myObj( mGetScreenLeft )
--
I  mGetScreenBottom -- Returns "bottom" pixel coordinate of all screen areas
--                  (refers to minimum rect surrounding multiple monitors)
--
--  Example of Lingo syntax:
--  global myObj
--  set theScreenBottom = myObj( mGetScreenBottom )
--
I  mGetScreenRight -- Returns "right" pixel coordinate of all screen areas
--                  (refers to minimum rect surrounding multiple monitors)
--
--  Example of Lingo syntax:
--  global myObj
--  set theScreenRight = myObj( mGetScreenRight )
--
II mPatToWindow -- Fills the window behind the Director stage with a particular
--  one-bit QuickDraw pattern, or the Finder desktop pattern. Returns a resultCode
--
--  Example of Lingo syntax:
--  global myObj
--  set resultCode = myObj( mPatToWindow,  -1 ) -- fills with a        white pattern
--  set resultCode = myObj( mPatToWindow,  -2 ) -- fills with a  light gray  pattern
--  set resultCode = myObj( mPatToWindow,  -3 ) -- fills with a middle gray  pattern
--  set resultCode = myObj( mPatToWindow,  -4 ) -- fills with a   dark gray  pattern
--  set resultCode = myObj( mPatToWindow,  -5 ) -- fills with a        black pattern
--  set resultCode = myObj( mPatToWindow, -99 ) -- any other negative number fills with
--                         --the Finder desktop pattern (whether color or black & white)
--
II mIndexColorToWindow -- In 256-color Monitor mode or less, fills the RearWindow
--  with a specified index color from the current palette. Returns resultCode
--
--  Example of Lingo syntax:
--  global myObj
--  --(int is an integer from 0 to 255:)
--  set resultCode = myObj( mIndexColorToWindow, int ) -- fills with an index color
--
--	NOTE: In direct-color display modes such as “thousands” or “millions”, using the
--  mIndexColorToWindow method will work, but produce unpredictable colors. In modes
--  lower than 256-colors, integers higher than the highest palette index will yield black.
--
IIII mRGBColorToWindow -- Fills the window behind the Director stage with a specified
--    RGB color. In 256-color Monitor mode or less, it produces the closest color in the
--    current indexed palette. Returns a resultCode
--
--  Example of Lingo syntax:
--  global myObj
--  --(red, green and blue are integers from 0 to 65535:)
--  set resultCode = myObj( mRGBColorToWindow, red, green, blue ) -- fills with an
--                  --RGB color or its closest equivalent in indexed palette modes
--
ISII mPICTToWindow -- Displays a PICT file in the window behind the Director stage
--   There are 3 arguments:
--   (1) the pathName and fileName -- a string
--   (2) the image placement code  -- an integer:
--        Ø = stretched across each monitor screen
--       -1 = positioned in the upper-left of each monitor screen (no stretch)
--        1 = centered within each monitor screen (no stretch)
--   (3) the background pattern (if any) -- an integer, same as mIndexToWindow
--  Returns a resultCode
--
--  Example of Lingo syntax:
--  global myObj
--  -- to find a file in the same folder as the movie, specify the pathName & «fileName»
--  -- otherwise, specify the full pathName beginning with the volume
--  set fileName = the pathName & "bkPictFile"
--  set resultCode = myObj( mPICTToWindow, fileName, -1, 112 )
--
IPII mCastToWindow -- Displays a movie castMember in the window behind the Stage
--   There are 3 arguments:
--   (1) the picture of a castMember
--   (2) the image placement code  -- an integer:
--        Ø = stretched across each monitor screen
--       -1 = positioned in the upper-left of each monitor screen (no stretch)
--        1 = centered within each monitor screen (no stretch)
--   (3) the background pattern (if any) -- an integer, same as mIndexToWindow
--  Returns resultCode
--
--  Example of Lingo syntax:
--  global myObj
--  set myPic = the picture of cast "bkPict"
--  set resultCode = myObj( mCastToWindow, myPic, 0, 0 )
--
X mDispose -- closes the RearWindow, releases its data, and the XObject itself from RAM
--
--  Example of Lingo syntax:
--  global myObj
--  if objectP( myObj ) then myObj( mDispose )
--
/X	  mIdle
/XI   mActivate, flag
/X	  mUpdate
/XII  mMouseUp, eventRecPtr, mdcode
/XII  mMouseDown, eventRecPtr, mdcode
/XI	  mKeyDown, theChar
/I    mMakeWindow
/I	  mInitDeviceArray
/I    mDevPixMapToWindow
*/

static const MethodProto xlibMethods[] = {
	{ "new",				RearWindowXObj::m_new,					1,	1,	400 },	// D4
	{ "getappname",			RearWindowXObj::m_getAppName,			0,	0,	400 },	// D4
	{ "GetMemoryNeeded",	RearWindowXObj::m_getMemoryNeeded,		0,	0,	400 },	// D4
	{ "GetScreenBottom",	RearWindowXObj::m_getScreenBottom,		0,	0,	400 },	// D4
	{ "GetScreenLeft",		RearWindowXObj::m_getScreenLeft,		0,	0,	400 },	// D4
	{ "GetScreenRight",		RearWindowXObj::m_getScreenRight,		0,	0,	400 },	// D4
	{ "GetScreenTop",		RearWindowXObj::m_getScreenTop,			0,	0,	400 },	// D4
	{ "IndexColorToWindow",	RearWindowXObj::m_indexColorToWindow,	1,	1,	400 },	// D4
	{ "PatToWindow",		RearWindowXObj::m_patToWindow,			1,	1,	400 },	// D4
	{ "RGBColorToWindow",	RearWindowXObj::m_rgbColorToWindow,		3,	3,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void RearWindowXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		RearWindowXObject::initMethods(xlibMethods);
		RearWindowXObject *xobj = new RearWindowXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void RearWindowXObj::close(ObjectType type) {
	if (type == kXObj) {
		RearWindowXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


RearWindowXObject::RearWindowXObject(ObjectType ObjectType) :Object<RearWindowXObject>("RearWindow") {
	_objType = ObjectType;
}

void RearWindowXObj::m_new(int nargs) {
	Datum d1 = g_lingo->pop();
	g_lingo->push(g_lingo->_state->me);
}

void RearWindowXObj::m_getMemoryNeeded(int nargs) {
	// No memory is needed for a stubbed XLib.
	g_lingo->push(Datum(0));
}

void RearWindowXObj::m_patToWindow(int nargs) {
	g_lingo->pop();
	g_lingo->push(Datum(0));
}

void RearWindowXObj::m_indexColorToWindow(int nargs) {
	g_lingo->pop();
}

void RearWindowXObj::m_getAppName(int nargs) {
	g_lingo->push(Datum(g_director->getStartMovie().startMovie));
}

void RearWindowXObj::m_rgbColorToWindow(int nargs) {
	Datum r = g_lingo->pop();
	Datum g = g_lingo->pop();
	Datum b = g_lingo->pop();
	Graphics::MacWindowManager *window = g_director->getMacWindowManager();

	window->setDesktopColor(r.asInt(), g.asInt(), b.asInt());
}

void RearWindowXObj::m_getScreenTop(int nargs) {
	g_lingo->push(Datum(0));
}

void RearWindowXObj::m_getScreenLeft(int nargs) {
	g_lingo->push(Datum(0));
}

void RearWindowXObj::m_getScreenBottom(int nargs) {
	g_lingo->push(Datum(g_director->getMacWindowManager()->getHeight()));
}

void RearWindowXObj::m_getScreenRight(int nargs) {
	g_lingo->push(Datum(g_director->getMacWindowManager()->getWidth()));
}

} // End of namespace Director
