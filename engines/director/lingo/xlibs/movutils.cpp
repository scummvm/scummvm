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
 * Gahan Wilson's Ultimate Haunted House
 * Momi no Ki no Shita de: The Day of St. Claus
 *
 *************************************/

/*
 * -- MovieUtilities External Factory, v. 1.0.7  by David Jackson Shields
 * MovUtils
 * I      mNew                  --Create new XObject instance
 * X      mDispose              --Dispose of XObject instance
 * S      mName                 --Return the XObject name (MovUtils)
 * SS     mGetVolName           --Return name of disk drive (volume) from path
 * S      mGetSystemPath        --Return system directory path as a string
 * S      mGetWindowsPath       --Return Windows directory path as a string
 * IS     mSetDefaultPath       --Set current drive:irectory path
 * SS     mGetChildWindowNames  --Return string of all Child window names,
 * --                                     given a Parent window name
 * IS     mGetNamedWindowHdl    --Return the ID handle to the named window
 * --                                     (needed for MCI commands)
 * XSLLII mDrawLine             --Draw line in the named window from point A
 * --                                     to point B, at specified pen width and color
 * ISLI   mDrawOval             --Draw filled oval (ellipse) in the named window,
 * --                                     the bounds RECT, using specified fill color
 * ISLI   mDrawRect             --Draw filled RECT in the named window, within the bounds
 * --                                     RECT, using specified fill color
 * ISLII  mDrawRoundRect        --Draw filled rounded RECT in the named window, within bounds
 * --                                     RECT, with specified curvature and fill color
 * ISLI   mDrawPoly             --Draw filled polygon in named window from a linear
 * --                                     list of coordinates, using specified fill color
 * ISLIII mDrawPie              --Draw filled arc in named window, within bounds RECT, from
 * --                                     start angle to arc angle, using specified fill color
 * I      mPrintLandscape       --Print the Stage in Landscape orientation using dithered grays
 * SS     mNoPunct              --Remove all punctuation chars from text
 * SS     mToUpperCase          --Convert text to all upperCase chars
 * SS     mToLowerCase          --Convert text to all lowerCase chars
 * SS     mTrimWhiteChars       --Remove leading and trailing 'white space' chars from text
 * SS     mDollarFormat         --Convert number string to US currency format
 * ISI    mGetWordStart         --Find number of chars to start of specified word
 * ISI    mGetWordEnd           --Find number of chars to end   of specified word
 * ISI    mGetLineStart         --Find number of chars to start of specified line
 * ISI    mGetLineEnd           --Find number of chars to end   of specified line
 * IS     mIsAlphaNum           --Return Boolean whether char is alphaNumeric
 * IS     mIsAlpha              --Return Boolean whether char is alphabetic
 * IS     mIsUpper              --Return Boolean whether char is upperCase alphabetic
 * IS     mIsLower              --Return Boolean whether char is lowerCase alphabetic
 * IS     mIsDigit              --Return Boolean whether char is a decimal digit
 * IS     mIsPunctuation        --Return Boolean whether char is punctuation
 * IS     mIsWhiteSpace         --Return Boolean whether char is a 'white space' char
 * IS     mIsPrintable          --Return Boolean whether char is printable
 * IS     mIsGraphic            --Return Boolean whether char is graphic
 * IS     mIsControl            --Return Boolean whether char is a control char
 * IS     mIsHex                --Return Boolean whether char is a hexadecimal digit
 * III    mBitSet               --Set   specified bit within a long integer
 * III    mBitTest              --Test  specified bit within a long integer
 * III    mBitClear             --Clear specified bit within a long integer
 * II     mBitShiftL            --Shift specified bit left  within a long integer
 * II     mBitShiftR            --Shift specified bit right within a long integer
 * III    mBitAnd               --Perform logical AND operation of two long integers
 * III    mBitOr                --Perform logical OR  operation of two long integers
 * III    mBitXOr               --Perform logical XOR operation of two long integers
 * II     mBitNot               --Perform logical NOT operation on a long integer
 * IS     mBitStringToNumber    --Translate string of '1's and '0's to a long integer
 * PL     mStageToCast          --Returns a picture handle to the image in the Rect on Stage
 * ISL    mStageToDIB           --Save, to a named bitmap file, the image in the Rect on Stage
 * ISL    mStageToPICT          --Save, to a Mac PICT file, the image in the Rect on Stage
 * SS     mCRtoCRLF             --Add a LineFeed to each Return char within Macintosh text
 * SS     mCRLFtoCR             --Remove LineFeed from each end of line within Windows text
 * III    mGetMessage           --Get mouse/key messages from the application message queue
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/movutils.h"


namespace Director {

const char *MovUtilsXObj::xlibNames[] = {
	"movutils",
	"MovieUtilities",
	nullptr
};

const char *MovUtilsXObj::fileNames[] = {
	"MOVUTILS",
	"MovieUtilities",
	"MovieUtilities.XObj",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "new",			MovUtilsXObj::m_new,			0,	0,	400 },	// D4
	{ "getSystemPath",	MovUtilsXObj::m_getsystempath,	0,	0,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void MovUtilsXObj::open(int type) {
	if (type == kXObj) {
		MovieUtilsXObject::initMethods(xlibMethods);
		MovieUtilsXObject *xobj = new MovieUtilsXObject(kXObj);
		for (uint i = 0; xlibNames[i]; i++) {
			g_lingo->exposeXObject(xlibNames[i], xobj);
		}
	}
}

void MovUtilsXObj::close(int type) {
	if (type == kXObj) {
		MovieUtilsXObject::cleanupMethods();
		for (uint i = 0; xlibNames[i]; i++) {
			g_lingo->_globalvars[xlibNames[i]] = Datum();
		}
	}
}

MovieUtilsXObject::MovieUtilsXObject(ObjectType ObjectType) :Object<MovieUtilsXObject>("MovUtilsXObj") {
	_objType = ObjectType;
}

void MovUtilsXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MovUtilsXObj::new", nargs);
	g_lingo->push(g_lingo->_state->me);
}

void MovUtilsXObj::m_getsystempath(int nargs) {
	g_lingo->dropStack(nargs);
	// An empty string ensures this just maps to the root of
	// ScummVM's save data path.
	g_lingo->push(Datum(""));
}

} // End of namespace Director
