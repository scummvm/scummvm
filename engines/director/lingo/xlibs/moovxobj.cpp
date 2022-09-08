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
 * Xobject to play Quicktime Videos
 *
 * USED IN:
 * C.H.A.O.S. Continuum
 *
 *************************************/

/*
--moovXobj 10.22.93 <<gzr>>
moovXobj
I		mNew							--Creates a new instance of the XObject.
X		mDispose						--Disposes of XObject instance.
S		mName							--Returns the XObject name (moovobj).
I		mMovieInit						--Initialize QTW.
I		mMovieKill						--Dispose of QTW.
I		mFondler						--Movie idle task.
ISII	mPlayMovie name,left,top		--Play movie at designated location.
I		mPauseMovie						--Pause active movie.
II		mSoundMovie						--Turn movie sound on or off.
I		mStopMovie						--Stops active movie.
I		mMovieDone						--Returns true if movie done.

ScummVM Note: mMovieDone returns true when the movie is _not_ done.

 */

#include "video/qt_decoder.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/moovxobj.h"


namespace Director {

const char *MoovXObj::xlibName = "moovxobj";
const char *MoovXObj::fileNames[] = {
	"moovxobj",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "new",					MoovXObj::m_new,			    0,	0,	300 },	// D3
	{ "Dispose",				MoovXObj::m_dispose,		    0,	0,	300 },	// D3
	{ "Name",				    MoovXObj::m_name,		        0,	0,	300 },	// D4
	{ "MovieInit",				MoovXObj::m_movieInit,		    0,	0,	300 },	// D4
	{ "MovieKill",				MoovXObj::m_movieKill,		    0,	0,	300 },	// D4
	{ "Fondler",				MoovXObj::m_fondler,	    	0,	0,	300 },	// D4
	{ "PlayMovie",				MoovXObj::m_playMovie,		    3,	3,	300 },	// D4
	{ "PauseMovie",				MoovXObj::m_pauseMovie,	    	0,	0,	300 },	// D4
	{ "SoundMovie",				MoovXObj::m_soundMovie,	    	0,	0,	300 },	// D4
	{ "StopMovie",				MoovXObj::m_stopMovie,	    	0,	0,	300 },	// D4
	{ "MovieDone",				MoovXObj::m_movieDone,		    0,	0,	300 },	// D4

	{ nullptr, nullptr, 0, 0, 0 }
};

void MoovXObj::open(int type) {
	if (type == kXObj) {
		MoovXObject::initMethods(xlibMethods);
		MoovXObject *xobj = new MoovXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void MoovXObj::close(int type) {
	if (type == kXObj) {
		MoovXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

MoovXObject::MoovXObject(ObjectType ObjectType) :Object<MoovXObject>("MoovXObj") {
	_objType = ObjectType;
	_video = nullptr;
	_x = 0;
	_y = 0;
}

MoovXObject::~MoovXObject() {
	if (_video) {
		delete _video;
		_video = nullptr;
	}
}

void MoovXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_currentMe);
}

void MoovXObj::m_dispose(int nargs) {
	debug(5, "MoovXObj::m_dispose");
	MoovXObject *me = static_cast<MoovXObject *>(g_lingo->_currentMe.u.obj);
	if (me->_video) {
		delete me->_video;
		me->_video = nullptr;
	}
}

void MoovXObj::m_name(int nargs) {
	// unused in C.H.A.O.S.
	g_lingo->printSTUBWithArglist("MoovXObj::m_name", nargs);
}

void MoovXObj::m_movieInit(int nargs) {
	// called in C.H.A.O.S. ScummVMs setup happens in playMovie
	g_lingo->printSTUBWithArglist("MoovXObj::m_movieInit", nargs);
}

void MoovXObj::m_movieKill(int nargs) {
	debug(5, "MoovXObj::m_movieKill");
	MoovXObject *me = static_cast<MoovXObject *>(g_lingo->_currentMe.u.obj);

	if (me->_video)
		me->_video->stop();
}

void MoovXObj::m_fondler(int nargs) {
	MoovXObject *me = static_cast<MoovXObject *>(g_lingo->_currentMe.u.obj);

	debug(10, "MoovXObj::m_fondler");
	Graphics::Surface const *frame;

	if (me->_video && me->_video->needsUpdate()) {
		frame = me->_video->decodeNextFrame();
		if (frame) {
			g_system->copyRectToScreen(frame->getPixels(), frame->pitch, me->_x, me->_y, frame->w, frame->h);
			g_system->updateScreen();
		}
	}
}

void MoovXObj::m_playMovie(int nargs) {
	MoovXObject *me = static_cast<MoovXObject *>(g_lingo->_currentMe.u.obj);

	me->_y = g_lingo->pop().asInt();
	me->_x = g_lingo->pop().asInt();
	Common::String filename = g_lingo->pop().asString();

	debug(5, "MoovXObj::m_playMovie: name: %s, x: %i y: %i", filename.c_str(), me->_x, me->_y);

	me->_video = new Video::QuickTimeDecoder();
	bool result = me->_video->loadFile(Common::Path(filename, g_director->_dirSeparator));
	if (result && g_director->_pixelformat.bytesPerPixel == 1) {
		// Director supports playing back RGB and paletted video in 256 colour mode.
		// In both cases they are dithered to match the Director palette.
		byte palette[256 * 3];
		g_system->getPaletteManager()->grabPalette(palette, 0, 256);
		me->_video->setDitheringPalette(palette);
	}
	me->_video->start();
}

void MoovXObj::m_pauseMovie(int nargs) {
	// unused in C.H.A.O.S.
	g_lingo->printSTUBWithArglist("MoovXObj::m_pauseMovie", nargs);
}

void MoovXObj::m_soundMovie(int nargs) {
	// unused in C.H.A.O.S.
	g_lingo->printSTUBWithArglist("MoovXObj::m_soundMovie", nargs);
}

void MoovXObj::m_stopMovie(int nargs) {
	// unused in C.H.A.O.S.
	g_lingo->printSTUBWithArglist("MoovXObj::m_stopMovie", nargs);
}

void MoovXObj::m_movieDone(int nargs) {
	MoovXObject *me = static_cast<MoovXObject *>(g_lingo->_currentMe.u.obj);
	debug(10, "MoovXObj::m_movieDone");
	bool result = (me->_video && !me->_video->endOfVideo());
	g_lingo->push(result);
}

}
// End of namespace Director
