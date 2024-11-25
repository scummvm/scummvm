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
 * teamxtreme1-win
 * teamxtreme2-win
 *
 *************************************/

/*
 * -- BatQt quicktime factory. 9Aug94 RNB
 * BatQt
 * I      mNew                    --Creates a new instance of the XObject
 * X      mDispose                --Disposes of XObject instance
 * S      mName                   --Returns the XObject name
 * I      mStatus                 --Returns an integer status code
 * SI     mError, code            --Returns an error string
 * S      mLastError              --Returns last error string
 * ISI    mOpen                   --Opens the specified movie
 * IIIS   mPlay                   --Play the movie, after setting parms
 * I      mStop                   --Stop the movie
 * S      mGetTimeRange           --Gets the current time range
 * S      mGetMovieBox            --Gets the current bounds box of the movie
 * I      mGetTime                --Gets the current time of the movie
 * SI     mSetTime                --Sets the current time of the movie
 * SI     mSetVolume              --Sets the volume of the movie
 * I      mLength                 --Gets the length of the movie
 * IIIII  mSetMovieBox            --Sets the bounding box of the movie
 * III    mSetTimeRange           -- Sets the active segment of the movie
 * II     mAddCallback            -- Adds a callback for the movie
 * II     mRemoveCallback         -- Removes a callback for the movie
 * I      mResetCallbacks         -- Resets the sent status of the callbacks
 * XS     mSetBatch               -- Applies a set of batch commands
 */

#include "graphics/paletteman.h"
#include "video/qt_decoder.h"
#include "director/director.h"
#include "director/util.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/batqt.h"


namespace Director {

// The name is different from the obj filename.
const char *const BatQT::xlibName = "batQT";
const XlibFileDesc BatQT::fileNames[] = {
	{ "batQT",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",			BatQT::m_new,			0, 0,	400 },	// D4
	{ "dispose",		BatQT::m_dispose,		1, 1,	400 },	// D4
	{ "name",			BatQT::m_name,			0, 0,	400 },	// D4
	{ "status",			BatQT::m_status,		0, 0,	400 },	// D4
	{ "error",			BatQT::m_error,			1, 1,	400 },	// D4
	{ "lastError",		BatQT::m_lastError,		0, 0,	400 },	// D4
	{ "open",			BatQT::m_open,			2, 2,	400 },	// D4
	{ "play",			BatQT::m_play,			3, 3,	400 },	// D4
	{ "stop",			BatQT::m_stop,			0, 0,	400 },  // D4
	{ "getTimeRange",	BatQT::m_getTimeRange,	0, 0,	400 },  // D4
	{ "getMovieBox",	BatQT::m_getMovieBox,	0, 0,	400 },  // D4
	{ "getTime",		BatQT::m_getTime,		0, 0,	400 },  // D4
	{ "setTime",		BatQT::m_setTime,		1, 1,	400 },  // D4
	{ "setVolume",		BatQT::m_setVolume,		1, 1,	400 },  // D4
	{ "length",			BatQT::m_length,		0, 0,	400 },  // D4
	{ "setMovieBox",	BatQT::m_setMovieBox,	4, 4,	400 },  // D4
	{ "setTimeRange",	BatQT::m_setTimeRange,	2, 2,	400 },  // D4
	{ "addCallback",	BatQT::m_addCallback,	1, 1,	400 },  // D4
	{ "removeCallback",	BatQT::m_removeCallback,1, 1,	400 },  // D4
	{ "resetCallbacks",	BatQT::m_resetCallbacks,0, 0,	400 },  // D4
	{ "setBatch",		BatQT::m_setBatch,		1, 1,	400 },  // D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void BatQT::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		BatQTXObject::initMethods(xlibMethods);
		BatQTXObject *xobj = new BatQTXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void BatQT::close(ObjectType type) {
	if (type == kXObj) {
		BatQTXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


BatQTXObject::BatQTXObject(ObjectType ObjectType) : Object<BatQTXObject>("BatQt") {
	_objType = ObjectType;
	_video = nullptr;
}

BatQTXObject::~BatQTXObject() {
	if (_video) {
		delete _video;
		_video = nullptr;
	}
}

void BatQT::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void BatQT::m_dispose(int nargs) {
	debug(5, "BatQT::m_dispose");
	BatQTXObject *me = static_cast<BatQTXObject *>(g_lingo->_state->me.u.obj);
	if (me->_video) {
		delete me->_video;
		me->_video = nullptr;
	}
}

XOBJSTUB(BatQT::m_name, "")
XOBJSTUB(BatQT::m_status, 0)
XOBJSTUB(BatQT::m_error, "")
XOBJSTUB(BatQT::m_lastError, "")

void BatQT::m_open(int nargs) {
	ARGNUMCHECK(2);
	Datum unk = g_lingo->pop();
	Datum path = g_lingo->pop();
	TYPECHECK(path, STRING);
	BatQTXObject *me = static_cast<BatQTXObject *>(g_lingo->_state->me.u.obj);
	Common::Path normPath = findPath(path.asString());
	if (!normPath.empty()) {
		me->_video = new Video::QuickTimeDecoder();
		debugC(5, kDebugXObj, "BatQT::m_open: Loading QT file %s", normPath.toString().c_str());
		if (me->_video->loadFile(normPath)) {
			me->_movieBox = Common::Rect(me->_video->getWidth(), me->_video->getHeight());
			 if (g_director->_pixelformat.bytesPerPixel == 1) {
				// Director supports playing back RGB and paletted video in 256 colour mode.
				// In both cases they are dithered to match the Director palette.
				byte palette[256 * 3];
				g_system->getPaletteManager()->grabPalette(palette, 0, 256);
				me->_video->setDitheringPalette(palette);
			 }
		} else {
			warning("BatQT::m_open: Could not load QT file %s", normPath.toString().c_str());
		}
	} else {
		warning("BatQT::m_open: Could not resolve path %s", path.asString().c_str());
	}
	g_lingo->push(0);
}

void BatQT::m_play(int nargs) {
	ARGNUMCHECK(3);
	Datum unk3 = g_lingo->pop();
	Datum unk2 = g_lingo->pop();
	Datum unk1 = g_lingo->pop();
	TYPECHECK(unk1, INT);
	TYPECHECK(unk2, INT);
	TYPECHECK(unk3, STRING);
	BatQTXObject *me = static_cast<BatQTXObject *>(g_lingo->_state->me.u.obj);
	if (me->_video) {
		debugC(5, kDebugXObj, "BatQT::m_play: Starting playback");
		me->_video->start();
	} else {
		warning("BatQT::m_play: No video loaded");
	}
	g_lingo->push(0);
}

void BatQT::m_stop(int nargs) {
	ARGNUMCHECK(0);
	BatQTXObject *me = static_cast<BatQTXObject *>(g_lingo->_state->me.u.obj);
	if (me->_video) {
		debugC(5, kDebugXObj, "BatQT::m_stop: Stopping playback");
		me->_video->stop();
	} else {
		warning("BatQT::m_stop: No video loaded");
	}
	g_lingo->push(0);
}

XOBJSTUB(BatQT::m_getTimeRange, "")

void BatQT::m_getMovieBox(int nargs) {
	BatQTXObject *me = static_cast<BatQTXObject *>(g_lingo->_state->me.u.obj);
	Common::String result = Common::String::format(
		"%d,%d,%d,%d",
		me->_movieBox.left,
		me->_movieBox.top,
		me->_movieBox.width(),
		me->_movieBox.height()
	);
	debugC(5, kDebugXObj, "BatQT::m_getMovieBox: %s", result.c_str());
	g_lingo->push(result);
}

void BatQT::m_getTime(int nargs) {
	ARGNUMCHECK(0);
	BatQTXObject *me = static_cast<BatQTXObject *>(g_lingo->_state->me.u.obj);
	Datum result(0);
	if (me->_video) {
		// Game uses a polling loop of m_getTime to measure progress,
		// therefore we need to render the frames in here
		if (me->_video->needsUpdate()) {
			const Graphics::Surface *frame = me->_video->decodeNextFrame();
			if (frame) {
				Graphics::Surface *temp = frame->scale(me->_movieBox.width(), me->_movieBox.height(), false);
				g_system->copyRectToScreen(temp->getPixels(), temp->pitch, me->_movieBox.left, me->_movieBox.top, temp->w, temp->h);
				g_system->updateScreen();
				delete temp;
			}
		}
		result = Datum(me->_video->getCurFrame() + 1);
		debugC(5, kDebugXObj, "BatQT::m_getTime: %d", result.asInt());
	} else {
		warning("BatQT::m_getTime: No video loaded");
	}
	g_lingo->push(result);
}

XOBJSTUB(BatQT::m_setTime, "")
XOBJSTUB(BatQT::m_setVolume, "")

void BatQT::m_length(int nargs) {
	ARGNUMCHECK(0);
	BatQTXObject *me = static_cast<BatQTXObject *>(g_lingo->_state->me.u.obj);
	Datum result(0);
	if (me->_video) {
		result = Datum((int)me->_video->getFrameCount());
		debugC(5, kDebugXObj, "BatQT::m_length: %d", result.asInt());
	}
	g_lingo->push(result);
}

void BatQT::m_setMovieBox(int nargs) {
	ARGNUMCHECK(4);
	Datum h = g_lingo->pop();
	Datum w = g_lingo->pop();
	Datum y = g_lingo->pop();
	Datum x = g_lingo->pop();
	TYPECHECK(h, INT);
	TYPECHECK(w, INT);
	TYPECHECK(y, INT);
	TYPECHECK(x, INT);

	BatQTXObject *me = static_cast<BatQTXObject *>(g_lingo->_state->me.u.obj);
	me->_movieBox.left = x.asInt();
	me->_movieBox.top = y.asInt();
	me->_movieBox.setWidth(w.asInt());
	me->_movieBox.setHeight(h.asInt());
	debugC(5, kDebugXObj, "BatQT::m_setMovieBox: %d,%d,%d,%d", me->_movieBox.left, me->_movieBox.top, me->_movieBox.width(), me->_movieBox.height());
	g_lingo->push(0);
}

XOBJSTUB(BatQT::m_setTimeRange, 0)
XOBJSTUB(BatQT::m_addCallback, 0)
XOBJSTUB(BatQT::m_removeCallback, 0)
XOBJSTUB(BatQT::m_resetCallbacks, 0)
XOBJSTUBNR(BatQT::m_setBatch)

} // End of namespace Director
