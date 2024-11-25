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

#include "common/util.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/movutils.h"

/**************************************************
 *
 * USED IN:
 * AMBER: Journeys Beyond
 * Gahan Wilson's Ultimate Haunted House
 * Momi no Ki no Shita de: The Day of St. Claus
 * Virtual Nightclub
 *
 **************************************************/

/*
-- MovieUtilities External Factory, v. 1.0.7  by David Jackson Shields
--MovUtils
I      mNew                  --Create new XObject instance
X      mDispose              --Dispose of XObject instance
S      mName                 --Return the XObject name (MovUtils)
SS     mGetVolName           --Return name of disk drive (volume) from path
S      mGetSystemPath        --Return system directory path as a string
S      mGetWindowsPath       --Return Windows directory path as a string
IS     mSetDefaultPath       --Set current drive:irectory path
SS     mGetChildWindowNames  --Return string of all Child window names,
--                                     given a Parent window name
IS     mGetNamedWindowHdl    --Return the ID handle to the named window
--                                     (needed for MCI commands)
XSLLII mDrawLine             --Draw line in the named window from point A
--                                     to point B, at specified pen width and color
ISLI   mDrawOval             --Draw filled oval (ellipse) in the named window,
--                                     the bounds RECT, using specified fill color
ISLI   mDrawRect             --Draw filled RECT in the named window, within the bounds
--                                     RECT, using specified fill color
ISLII  mDrawRoundRect        --Draw filled rounded RECT in the named window, within bounds
--                                     RECT, with specified curvature and fill color
ISLI   mDrawPoly             --Draw filled polygon in named window from a linear
--                                     list of coordinates, using specified fill color
ISLIII mDrawPie              --Draw filled arc in named window, within bounds RECT, from
--                                     start angle to arc angle, using specified fill color
I      mPrintLandscape       --Print the Stage in Landscape orientation using dithered grays
SS     mNoPunct              --Remove all punctuation chars from text
SS     mToUpperCase          --Convert text to all upperCase chars
SS     mToLowerCase          --Convert text to all lowerCase chars
SS     mTrimWhiteChars       --Remove leading and trailing 'white space' chars from text
SS     mDollarFormat         --Convert number string to US currency format
ISI    mGetWordStart         --Find number of chars to start of specified word
ISI    mGetWordEnd           --Find number of chars to end   of specified word
ISI    mGetLineStart         --Find number of chars to start of specified line
ISI    mGetLineEnd           --Find number of chars to end   of specified line
IS     mIsAlphaNum           --Return Boolean whether char is alphaNumeric
IS     mIsAlpha              --Return Boolean whether char is alphabetic
IS     mIsUpper              --Return Boolean whether char is upperCase alphabetic
IS     mIsLower              --Return Boolean whether char is lowerCase alphabetic
IS     mIsDigit              --Return Boolean whether char is a decimal digit
IS     mIsPunctuation        --Return Boolean whether char is punctuation
IS     mIsWhiteSpace         --Return Boolean whether char is a 'white space' char
IS     mIsPrintable          --Return Boolean whether char is printable
IS     mIsGraphic            --Return Boolean whether char is graphic
IS     mIsControl            --Return Boolean whether char is a control char
IS     mIsHex                --Return Boolean whether char is a hexadecimal digit
III    mBitSet               --Set   specified bit within a long integer
III    mBitTest              --Test  specified bit within a long integer
III    mBitClear             --Clear specified bit within a long integer
II     mBitShiftL            --Shift specified bit left  within a long integer
II     mBitShiftR            --Shift specified bit right within a long integer
III    mBitAnd               --Perform logical AND operation of two long integers
III    mBitOr                --Perform logical OR  operation of two long integers
III    mBitXOr               --Perform logical XOR operation of two long integers
II     mBitNot               --Perform logical NOT operation on a long integer
IS     mBitStringToNumber    --Translate string of '1's and '0's to a long integer
PL     mStageToCast          --Returns a picture handle to the image in the Rect on Stage
ISL    mStageToDIB           --Save, to a named bitmap file, the image in the Rect on Stage
ISL    mStageToPICT          --Save, to a Mac PICT file, the image in the Rect on Stage
SS     mCRtoCRLF             --Add a LineFeed to each Return char within Macintosh text
SS     mCRLFtoCR             --Remove LineFeed from each end of line within Windows text
III    mGetMessage           --Get mouse/key messages from the application message queue
 */

namespace Director {

const char *const MovUtilsXObj::xlibName = "MovUtils";
const XlibFileDesc MovUtilsXObj::fileNames[] = {
	{ "MOVUTILS",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				MovUtilsXObj::m_new,		 0, 0,	400 },
	{ "dispose",				MovUtilsXObj::m_dispose,		 0, 0,	400 },
	{ "name",				MovUtilsXObj::m_name,		 0, 0,	400 },
	{ "getVolName",				MovUtilsXObj::m_getVolName,		 1, 1,	400 },
	{ "getSystemPath",				MovUtilsXObj::m_getSystemPath,		 0, 0,	400 },
	{ "getWindowsPath",				MovUtilsXObj::m_getWindowsPath,		 0, 0,	400 },
	{ "setDefaultPath",				MovUtilsXObj::m_setDefaultPath,		 1, 1,	400 },
	{ "getChildWindowNames",				MovUtilsXObj::m_getChildWindowNames,		 1, 1,	400 },
	{ "getNamedWindowHdl",				MovUtilsXObj::m_getNamedWindowHdl,		 1, 1,	400 },
	{ "drawLine",				MovUtilsXObj::m_drawLine,		 5, 5,	400 },
	{ "drawOval",				MovUtilsXObj::m_drawOval,		 3, 3,	400 },
	{ "drawRect",				MovUtilsXObj::m_drawRect,		 3, 3,	400 },
	{ "drawRoundRect",				MovUtilsXObj::m_drawRoundRect,		 4, 4,	400 },
	{ "drawPoly",				MovUtilsXObj::m_drawPoly,		 3, 3,	400 },
	{ "drawPie",				MovUtilsXObj::m_drawPie,		 5, 5,	400 },
	{ "printLandscape",				MovUtilsXObj::m_printLandscape,		 0, 0,	400 },
	{ "noPunct",				MovUtilsXObj::m_noPunct,		 1, 1,	400 },
	{ "toUpperCase",				MovUtilsXObj::m_toUpperCase,		 1, 1,	400 },
	{ "toLowerCase",				MovUtilsXObj::m_toLowerCase,		 1, 1,	400 },
	{ "trimWhiteChars",				MovUtilsXObj::m_trimWhiteChars,		 1, 1,	400 },
	{ "dollarFormat",				MovUtilsXObj::m_dollarFormat,		 1, 1,	400 },
	{ "getWordStart",				MovUtilsXObj::m_getWordStart,		 2, 2,	400 },
	{ "getWordEnd",				MovUtilsXObj::m_getWordEnd,		 2, 2,	400 },
	{ "getLineStart",				MovUtilsXObj::m_getLineStart,		 2, 2,	400 },
	{ "getLineEnd",				MovUtilsXObj::m_getLineEnd,		 2, 2,	400 },
	{ "isAlphaNum",				MovUtilsXObj::m_isAlphaNum,		 1, 1,	400 },
	{ "isAlpha",				MovUtilsXObj::m_isAlpha,		 1, 1,	400 },
	{ "isUpper",				MovUtilsXObj::m_isUpper,		 1, 1,	400 },
	{ "isLower",				MovUtilsXObj::m_isLower,		 1, 1,	400 },
	{ "isDigit",				MovUtilsXObj::m_isDigit,		 1, 1,	400 },
	{ "isPunctuation",				MovUtilsXObj::m_isPunctuation,		 1, 1,	400 },
	{ "isWhiteSpace",				MovUtilsXObj::m_isWhiteSpace,		 1, 1,	400 },
	{ "isPrintable",				MovUtilsXObj::m_isPrintable,		 1, 1,	400 },
	{ "isGraphic",				MovUtilsXObj::m_isGraphic,		 1, 1,	400 },
	{ "isControl",				MovUtilsXObj::m_isControl,		 1, 1,	400 },
	{ "isHex",				MovUtilsXObj::m_isHex,		 1, 1,	400 },
	{ "bitSet",				MovUtilsXObj::m_bitSet,		 2, 2,	400 },
	{ "bitTest",				MovUtilsXObj::m_bitTest,		 2, 2,	400 },
	{ "bitClear",				MovUtilsXObj::m_bitClear,		 2, 2,	400 },
	{ "bitShiftL",				MovUtilsXObj::m_bitShiftL,		 1, 1,	400 },
	{ "bitShiftR",				MovUtilsXObj::m_bitShiftR,		 1, 1,	400 },
	{ "bitAnd",				MovUtilsXObj::m_bitAnd,		 2, 2,	400 },
	{ "bitOr",				MovUtilsXObj::m_bitOr,		 2, 2,	400 },
	{ "bitXOr",				MovUtilsXObj::m_bitXOr,		 2, 2,	400 },
	{ "bitNot",				MovUtilsXObj::m_bitNot,		 1, 1,	400 },
	{ "bitStringToNumber",				MovUtilsXObj::m_bitStringToNumber,		 1, 1,	400 },
	{ "stageToCast",				MovUtilsXObj::m_stageToCast,		 1, 1,	400 },
	{ "stageToDIB",				MovUtilsXObj::m_stageToDIB,		 2, 2,	400 },
	{ "stageToPICT",				MovUtilsXObj::m_stageToPICT,		 2, 2,	400 },
	{ "cRtoCRLF",				MovUtilsXObj::m_cRtoCRLF,		 1, 1,	400 },
	{ "cRLFtoCR",				MovUtilsXObj::m_cRLFtoCR,		 1, 1,	400 },
	{ "getMessage",				MovUtilsXObj::m_getMessage,		 2, 2,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

MovUtilsXObject::MovUtilsXObject(ObjectType ObjectType) :Object<MovUtilsXObject>("MovUtils") {
	_objType = ObjectType;
}

void MovUtilsXObj::open(ObjectType type, const Common::Path &path) {
    MovUtilsXObject::initMethods(xlibMethods);
    MovUtilsXObject *xobj = new MovUtilsXObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void MovUtilsXObj::close(ObjectType type) {
    MovUtilsXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void MovUtilsXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MovUtilsXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(MovUtilsXObj::m_dispose)
XOBJSTUB(MovUtilsXObj::m_name, "")
XOBJSTUB(MovUtilsXObj::m_getVolName, "")
XOBJSTUB(MovUtilsXObj::m_getSystemPath, "")
XOBJSTUB(MovUtilsXObj::m_getWindowsPath, "")
XOBJSTUB(MovUtilsXObj::m_setDefaultPath, 0)
XOBJSTUB(MovUtilsXObj::m_getChildWindowNames, "")
XOBJSTUB(MovUtilsXObj::m_getNamedWindowHdl, 0)
XOBJSTUBNR(MovUtilsXObj::m_drawLine)
XOBJSTUB(MovUtilsXObj::m_drawOval, 0)
XOBJSTUB(MovUtilsXObj::m_drawRect, 0)
XOBJSTUB(MovUtilsXObj::m_drawRoundRect, 0)
XOBJSTUB(MovUtilsXObj::m_drawPoly, 0)
XOBJSTUB(MovUtilsXObj::m_drawPie, 0)
XOBJSTUB(MovUtilsXObj::m_printLandscape, 0)
XOBJSTUB(MovUtilsXObj::m_noPunct, "")
XOBJSTUB(MovUtilsXObj::m_toUpperCase, "")
XOBJSTUB(MovUtilsXObj::m_toLowerCase, "")
XOBJSTUB(MovUtilsXObj::m_trimWhiteChars, "")
XOBJSTUB(MovUtilsXObj::m_dollarFormat, "")
XOBJSTUB(MovUtilsXObj::m_getWordStart, 0)
XOBJSTUB(MovUtilsXObj::m_getWordEnd, 0)
XOBJSTUB(MovUtilsXObj::m_getLineStart, 0)
XOBJSTUB(MovUtilsXObj::m_getLineEnd, 0)

void MovUtilsXObj::m_isAlphaNum(int nargs) {
	Datum result(0);
	if (nargs != 1) {
		warning("MovUtilsXObj::m_isAlphaNum(): expected 1 arg");
		g_lingo->dropStack(nargs);
	} else {
		char test = g_lingo->pop().asString().firstChar();
		result = Datum(Common::isAlnum(test) ? 1 : 0);
	}
	g_lingo->push(result);
}

void MovUtilsXObj::m_isAlpha(int nargs) {
	Datum result(0);
	if (nargs != 1) {
		warning("MovUtilsXObj::m_isAlpha(): expected 1 arg");
		g_lingo->dropStack(nargs);
	} else {
		char test = g_lingo->pop().asString().firstChar();
		result = Datum(Common::isAlpha(test) ? 1 : 0);
	}
	g_lingo->push(result);
}

void MovUtilsXObj::m_isUpper(int nargs) {
	Datum result(0);
	if (nargs != 1) {
		warning("MovUtilsXObj::m_isUpper(): expected 1 arg");
		g_lingo->dropStack(nargs);
	} else {
		char test = g_lingo->pop().asString().firstChar();
		result = Datum(Common::isUpper(test) ? 1 : 0);
	}
	g_lingo->push(result);
}

void MovUtilsXObj::m_isLower(int nargs) {
	Datum result(0);
	if (nargs != 1) {
		warning("MovUtilsXObj::m_isLower(): expected 1 arg");
		g_lingo->dropStack(nargs);
	} else {
		char test = g_lingo->pop().asString().firstChar();
		result = Datum(Common::isLower(test) ? 1 : 0);
	}
	g_lingo->push(result);
}

void MovUtilsXObj::m_isDigit(int nargs) {
	Datum result(0);
	if (nargs != 1) {
		warning("MovUtilsXObj::m_isDigit(): expected 1 arg");
		g_lingo->dropStack(nargs);
	} else {
		char test = g_lingo->pop().asString().firstChar();
		result = Datum(Common::isDigit(test) ? 1 : 0);
	}
	g_lingo->push(result);
}

void MovUtilsXObj::m_isPunctuation(int nargs) {
	Datum result(0);
	if (nargs != 1) {
		warning("MovUtilsXObj::m_isPunctuation(): expected 1 arg");
		g_lingo->dropStack(nargs);
	} else {
		char test = g_lingo->pop().asString().firstChar();
		result = Datum(Common::isPunct(test) ? 1 : 0);
	}
	g_lingo->push(result);
}

void MovUtilsXObj::m_isWhiteSpace(int nargs) {
	Datum result(0);
	if (nargs != 1) {
		warning("MovUtilsXObj::m_isWhiteSpace(): expected 1 arg");
		g_lingo->dropStack(nargs);
	} else {
		char test = g_lingo->pop().asString().firstChar();
		result = Datum(Common::isSpace(test) ? 1 : 0);
	}
	g_lingo->push(result);
}

void MovUtilsXObj::m_isPrintable(int nargs) {
	Datum result(0);
	if (nargs != 1) {
		warning("MovUtilsXObj::m_isPrintable(): expected 1 arg");
		g_lingo->dropStack(nargs);
	} else {
		char test = g_lingo->pop().asString().firstChar();
		result = Datum(Common::isPrint(test) ? 1 : 0);
	}
	g_lingo->push(result);
}

void MovUtilsXObj::m_isGraphic(int nargs) {
	Datum result(0);
	if (nargs != 1) {
		warning("MovUtilsXObj::m_isGraphic(): expected 1 arg");
		g_lingo->dropStack(nargs);
	} else {
		char test = g_lingo->pop().asString().firstChar();
		result = Datum(Common::isGraph(test) ? 1 : 0);
	}
	g_lingo->push(result);
}

void MovUtilsXObj::m_isControl(int nargs) {
	Datum result(0);
	if (nargs != 1) {
		warning("MovUtilsXObj::m_isControl(): expected 1 arg");
		g_lingo->dropStack(nargs);
	} else {
		char test = g_lingo->pop().asString().firstChar();
		result = Datum(Common::isCntrl(test) ? 1 : 0);
	}
	g_lingo->push(result);
}

void MovUtilsXObj::m_isHex(int nargs) {
	Datum result(0);
	if (nargs != 1) {
		warning("MovUtilsXObj::m_isHex(): expected 1 arg");
		g_lingo->dropStack(nargs);
	} else {
		char test = g_lingo->pop().asString().firstChar();
		result = Datum(Common::isXDigit(test) ? 1 : 0);
	}
	g_lingo->push(result);
}

XOBJSTUB(MovUtilsXObj::m_bitSet, 0)
XOBJSTUB(MovUtilsXObj::m_bitTest, 0)
XOBJSTUB(MovUtilsXObj::m_bitClear, 0)
XOBJSTUB(MovUtilsXObj::m_bitShiftL, 0)
XOBJSTUB(MovUtilsXObj::m_bitShiftR, 0)
XOBJSTUB(MovUtilsXObj::m_bitAnd, 0)
XOBJSTUB(MovUtilsXObj::m_bitOr, 0)
XOBJSTUB(MovUtilsXObj::m_bitXOr, 0)
XOBJSTUB(MovUtilsXObj::m_bitNot, 0)
XOBJSTUB(MovUtilsXObj::m_bitStringToNumber, 0)
XOBJSTUB(MovUtilsXObj::m_stageToCast, 0)
XOBJSTUB(MovUtilsXObj::m_stageToDIB, 0)
XOBJSTUB(MovUtilsXObj::m_stageToPICT, 0)
XOBJSTUB(MovUtilsXObj::m_cRtoCRLF, "")
XOBJSTUB(MovUtilsXObj::m_cRLFtoCR, "")
XOBJSTUB(MovUtilsXObj::m_getMessage, 0)

}
