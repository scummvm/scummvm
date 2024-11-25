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

#include "common/file.h"
#include "common/savefile.h"
#include "common/str.h"
#include "common/system.h"
#include "gui/filebrowser-dialog.h"
#include "video/qt_decoder.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/mmovie.h"

/**************************************************
 *
 * USED IN:
 * Virtual Nightclub
 *
 **************************************************/

/*
Multi Movie XObject by Mediamation Ltd. Copyright © Trip Media Ltd 1995-1996.
--MMovie
I       mNew
X       mDispose
IS      mOpenMMovie, fileName
II      mCloseMMovie, movieIndex
ISSSSS  mPlaySegment, segmentName, restoreOpt, abortOpt, purgeOpt, asyncOpt
ISSSSS  mPlaySegLoop, segmentName, restoreOpt, abortOpt, purgeOpt, asyncOpt
I       mIdleSegment
I       mStopSegment
IS      mSeekSegment, segmentName
II      mSetSegmentTime, segTime
IIIII   mSetDisplayBounds, left, top, right, bottom
II      mGetMovieNormalWidth, movieIndex
II      mGetMovieNormalHeight, movieIndex
II      mGetSegCount, movieIndex
SII     mGetSegName, movieIndex, segmentIndex
I       mGetMovieRate
II      mSetMovieRate, ratePercent
I       mFlushEvents
IIIII   mInvalidateRect, left, top, right, bottom
SSI     mReadFile, fileName, scramble
SSSI    mWriteFile, fileName, data, scramble
ISS     mCopyFile, sourceFileName, destFileName
I       mCopyFileCont
IS      mFreeSpace, driveLetter
IS      mDeleteFile, fileName
S       mVolList
 */

namespace Director {

const char *const MMovieXObj::xlibName = "MMovie";
const XlibFileDesc MMovieXObj::fileNames[] = {
	{ "MMovie",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "Movie",				MMovieXObj::m_Movie,		 4, 4,	400 },
	{ "new",				MMovieXObj::m_new,		 0, 0,	400 },
	{ "dispose",				MMovieXObj::m_dispose,		 0, 0,	400 },
	{ "openMMovie",				MMovieXObj::m_openMMovie,		 1, 1,	400 },
	{ "closeMMovie",				MMovieXObj::m_closeMMovie,		 1, 1,	400 },
	{ "playSegment",				MMovieXObj::m_playSegment,		 5, 5,	400 },
	{ "playSegLoop",				MMovieXObj::m_playSegLoop,		 5, 5,	400 },
	{ "idleSegment",				MMovieXObj::m_idleSegment,		 0, 0,	400 },
	{ "stopSegment",				MMovieXObj::m_stopSegment,		 0, 0,	400 },
	{ "seekSegment",				MMovieXObj::m_seekSegment,		 1, 1,	400 },
	{ "setSegmentTime",				MMovieXObj::m_setSegmentTime,		 1, 1,	400 },
	{ "setDisplayBounds",				MMovieXObj::m_setDisplayBounds,		 4, 4,	400 },
	{ "getMovieNormalWidth",				MMovieXObj::m_getMovieNormalWidth,		 1, 1,	400 },
	{ "getMovieNormalHeight",				MMovieXObj::m_getMovieNormalHeight,		 1, 1,	400 },
	{ "getSegCount",				MMovieXObj::m_getSegCount,		 1, 1,	400 },
	{ "getSegName",				MMovieXObj::m_getSegName,		 2, 2,	400 },
	{ "getMovieRate",				MMovieXObj::m_getMovieRate,		 0, 0,	400 },
	{ "setMovieRate",				MMovieXObj::m_setMovieRate,		 1, 1,	400 },
	{ "flushEvents",				MMovieXObj::m_flushEvents,		 0, 0,	400 },
	{ "invalidateRect",				MMovieXObj::m_invalidateRect,		 4, 4,	400 },
	{ "readFile",				MMovieXObj::m_readFile,		 2, 2,	400 },
	{ "writeFile",				MMovieXObj::m_writeFile,		 3, 3,	400 },
	{ "copyFile",				MMovieXObj::m_copyFile,		 2, 2,	400 },
	{ "copyFileCont",				MMovieXObj::m_copyFileCont,		 0, 0,	400 },
	{ "freeSpace",				MMovieXObj::m_freeSpace,		 1, 1,	400 },
	{ "deleteFile",				MMovieXObj::m_deleteFile,		 1, 1,	400 },
	{ "volList",				MMovieXObj::m_volList,		 0, 0,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

MMovieXObject::MMovieXObject(ObjectType ObjectType) :Object<MMovieXObject>("MMovie") {
	_objType = ObjectType;
}

MMovieXObject::~MMovieXObject() {
	_lastFrame.free();
	for (auto &it : _movies) {
		if (it._value._video) {
			delete it._value._video;
			it._value._video = nullptr;
		}
	}
}

int MMovieXObject::playSegment(int movieIndex, int segIndex, bool looping, bool restore, bool shiftAbort, bool abortOnClick, bool purge, bool async) {
	int result = MMovieError::MMOVIE_INVALID_MOVIE_INDEX;
	if (_movies.contains(movieIndex)) {
		MMovieFile &movie = _movies.getVal(movieIndex);
		result = MMovieError::MMOVIE_INDEX_OUT_OF_RANGE;
		if (segIndex <= (int)movie.segments.size() && segIndex > 0) {
			MMovieSegment &segment = movie.segments[segIndex - 1];
			_currentMovieIndex = movieIndex;
			_currentSegmentIndex = segIndex;
			_looping = looping;
			_restore = restore;
			_shiftAbort = shiftAbort;
			_abortOnClick = abortOnClick;
			_purge = purge;
			_async = async;
			debugC(5, kDebugXObj, "MMovieXObject::playSegment(): hitting play on movie %s (%d) segment %s (%d) - %d", movie._path.toString().c_str(), movieIndex, segment._name.c_str(), segIndex, segment._start);
			movie._video->seek(Audio::Timestamp(0, segment._start, movie._video->getTimeScale()));
			movie._video->start();
			result = MMovieError::MMOVIE_NONE;
			if (!_async) {
				result = updateScreenBlocking();
			}
		}
	}
	return result;
}

bool MMovieXObject::stopSegment() {
	if (_currentMovieIndex && _currentSegmentIndex) {
		MMovieFile &movie = _movies.getVal(_currentMovieIndex);
		MMovieSegment &seg = movie.segments[_currentSegmentIndex - 1];
		debugC(5, kDebugXObj, "MMovieXObject::stopSegment(): hitting stop on movie %s (%d) segment %s (%d) - %d", movie._path.toString().c_str(), _currentMovieIndex, seg._name.c_str(), _currentSegmentIndex, seg._start);
		if (movie._video) {
			movie._video->stop();
		}
		_currentMovieIndex = 0;
		_currentSegmentIndex = 0;
		return true;
	}
	return false;
}

int MMovieXObject::updateScreenBlocking() {
	MMovieError result = MMovieError::MMOVIE_PLAYBACK_FINISHED;
	while (_currentMovieIndex && _currentSegmentIndex) {
		Common::Event event;
		bool keepPlaying = true;
		if (g_director->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				g_director->processEventQUIT();
				// fallthrough
			case Common::EVENT_KEYDOWN:
			case Common::EVENT_RBUTTONDOWN:
			case Common::EVENT_LBUTTONDOWN:
				if (_abortOnClick) {
					result = MMovieError::MMOVIE_ABORT_DOUBLE_CLICK;
					keepPlaying = false;
				} else if (_shiftAbort) {
					if (event.type == Common::EVENT_RBUTTONDOWN ||
						(event.type == Common::EVENT_LBUTTONDOWN &&
						(g_system->getEventManager()->getModifierState() & Common::KBD_SHIFT))) {
						result = MMovieError::MMOVIE_ABORT_DOUBLE_CLICK;
						keepPlaying = false;
					}
				}
				break;
			default:
				break;
			}
			// pass event through to window manager.
			// this is required so that e.g. the stillDown is kept up to date
			g_director->_wm->processEvent(event);
		}
		if (!keepPlaying)
			break;
		updateScreen();
	}
	return result;
}

int MMovieXObject::updateScreen() {
	int result = MMovieError::MMOVIE_CONTINUE_WITHOUT_PLAYING;
	if (_currentMovieIndex) {
		MMovieFile &movie = _movies.getVal(_currentMovieIndex);
		if (_currentSegmentIndex) {
			MMovieSegment &seg = movie.segments[_currentSegmentIndex - 1];
			result = getTicks();
			if (movie._video && movie._video->isPlaying() && movie._video->needsUpdate()) {
				const Graphics::Surface *frame = movie._video->decodeNextFrame();
				if (frame && !_bounds.isEmpty()) {
					debugC(8, kDebugXObj, "MMovieXObject: rendering movie %s (%d), ticks %d", movie._path.toString().c_str(), _currentMovieIndex, getTicks());
					Graphics::Surface *temp1 = frame->scale(_bounds.width(), _bounds.height(), false);
					Graphics::Surface *temp2 = temp1->convertTo(g_director->_pixelformat, movie._video->getPalette());
					_lastFrame.copyFrom(*temp2);
					temp2->free();
					delete temp2;
					temp1->free();
					delete temp1;
				}
			}
			if (!_bounds.isEmpty())
				g_system->copyRectToScreen(_lastFrame.getPixels(), _lastFrame.pitch, _bounds.left, _bounds.top, _bounds.width(), _bounds.height());
			// do a time check
			uint32 endTime = Audio::Timestamp(0, seg._length + seg._start, movie._video->getTimeScale()).msecs();
			debugC(8, kDebugXObj, "MMovieXObject::updateScreen(): time: %d, endTime: %d, ticks: %d, endTicks: %d", movie._video->getTime(), endTime, getTicks(), seg._length + seg._start);
			if (movie._video->getTime() >= endTime) {
				if (_looping) {
					debugC(5, kDebugXObj, "MMovieXObject::updateScreen(): rewinding loop on %s (%d), time: %d, ticks: %d", movie._path.toString().c_str(), _currentMovieIndex, movie._video->getTime(), getTicks());
					movie._video->seek(Audio::Timestamp(0, seg._start, movie._video->getTimeScale()));
				} else {
					debugC(5, kDebugXObj, "MMovieXObject::updateScreen(): stopping %s (%d), time: %d, ticks: %d", movie._path.toString().c_str(), _currentMovieIndex, movie._video->getTime(), getTicks());
					stopSegment();
					result = MMovieError::MMOVIE_PLAYBACK_FINISHED;
				}
			}
		}
	}
	g_system->updateScreen();
	g_director->delayMillis(10);
	return result;
}

int MMovieXObject::getTicks() {
	if (_currentMovieIndex && _currentSegmentIndex) {
		MMovieFile &movie = _movies.getVal(_currentMovieIndex);
		MMovieSegment &segment = movie.segments[_currentSegmentIndex - 1];
		if (movie._video) {
			_lastTicks = movie._video->getTime() - Audio::Timestamp(0, segment._start, movie._video->getTimeScale()).msecs();
			_lastTicks = _lastTicks * 60 / 1000;
		}
	}
	return _lastTicks;
}

void MMovieXObj::open(ObjectType type, const Common::Path &path) {
    MMovieXObject::initMethods(xlibMethods);
    MMovieXObject *xobj = new MMovieXObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void MMovieXObj::close(ObjectType type) {
    MMovieXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void MMovieXObj::m_new(int nargs) {
	g_lingo->printArgs("MMovieXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(MMovieXObj::m_Movie, 0)
XOBJSTUBNR(MMovieXObj::m_dispose)

void MMovieXObj::m_openMMovie(int nargs) {
	g_lingo->printArgs("MMovieXObj::m_openMMovie", nargs);
	if (nargs != 1) {
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(-1));
		return;
	}
	MMovieXObject *me = static_cast<MMovieXObject *>(g_lingo->_state->me.u.obj);
	Common::String basename = g_lingo->pop().asString();
	Common::Path path = findPath(basename);
	if (path.empty()) {
		g_lingo->push(MMovieError::MMOVIE_INVALID_OFFSETS_FILE);
		return;
	}
	Common::Path offsetsPath = findPath(basename.substr(0, basename.size()-4) + ".ofs");
	if (offsetsPath.empty()) {
		g_lingo->push(MMovieError::MMOVIE_INVALID_OFFSETS_FILE);
		return;
	}
	if (me->_moviePathMap.contains(basename)) {
		g_lingo->push(MMovieError::MMOVIE_MOVIE_ALREADY_OPEN);
		return;
	}
	Common::File offsetsFile;
	if (!offsetsFile.open(offsetsPath)) {
		g_lingo->push(MMovieError::MMOVIE_INVALID_OFFSETS_FILE);
		return;
	}

	MMovieFile movie(path);
	movie._video = new Video::QuickTimeDecoder();
	if (!movie._video->loadFile(path)) {
		warning("MMovieXObj::m_openMMovie(): unable to open QT file %s", path.toString().c_str());
		delete movie._video;
		movie._video = nullptr;
	}
	uint32 offsetCount = offsetsFile.readUint32BE();
	offsetsFile.skip(0x3c); // rest of header should be blank
	debugC(5, kDebugXObj, "MMovieXObj:m_openMMovie(): opening movie %s (index %d)", path.toString().c_str(), me->_lastIndex);
	for (uint32 i = 0; i < offsetCount; i++) {
		Common::String name = offsetsFile.readString(' ', 0x10);
		uint32 start = offsetsFile.readUint32BE();
		uint32 length = offsetsFile.readUint32BE();
		debugC(5, kDebugXObj, "MMovieXObj:m_openMMovie(): adding segment %s (index %d): start %d (%dms) length %d (%dms)", name.c_str(), movie.segments.size(), start, Audio::Timestamp(0, start, movie._video->getTimeScale()).msecs(), length, Audio::Timestamp(0, length, movie._video->getTimeScale()).msecs());
		movie.segments.push_back(MMovieSegment(name, start, length));
		movie.segLookup.setVal(name, movie.segments.size());
	}
	me->_movies.setVal(me->_lastIndex, movie);
	me->_moviePathMap.setVal(basename, me->_lastIndex);
	g_lingo->push(me->_lastIndex);
	me->_lastIndex += 1;
}

void MMovieXObj::m_closeMMovie(int nargs) {
	g_lingo->printArgs("MMovieXObj::m_closeMMovie", nargs);
	if (nargs != 1) {
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(MMovieError::MMOVIE_INVALID_MOVIE_INDEX));
		return;
	}
	MMovieXObject *me = static_cast<MMovieXObject *>(g_lingo->_state->me.u.obj);
	int index = g_lingo->pop().asInt();
	if (!me->_movies.contains(index)) {
		warning("MMovieXObj::m_closeMMovie(): movie index %d not found", index);
		g_lingo->push(Datum(MMovieError::MMOVIE_INVALID_MOVIE_INDEX));
		return;
	}
	for (auto &it : me->_moviePathMap) {
		if (it._value == index) {
			me->_moviePathMap.erase(it._key);
			break;
		}
	}
	MMovieFile &file = me->_movies.getVal(index);
	debugC(5, kDebugXObj, "MMovieXObj:m_openMMovie(): closing movie %s (index %d)", file._path.toString().c_str(), me->_lastIndex);
	if (file._video) {
		delete file._video;
		file._video = nullptr;
	}
	me->_movies.erase(index);
	g_lingo->push(Datum(MMovieError::MMOVIE_NONE));
}

void MMovieXObj::m_playSegment(int nargs) {
	g_lingo->printArgs("MMovieXObj::m_playSegment", nargs);
	if (nargs != 5) {
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(MMovieError::MMOVIE_INVALID_SEGMENT_NAME));
		return;
	}

	Common::String asyncOpt = g_lingo->pop().asString();
	Common::String purgeOpt = g_lingo->pop().asString();
	Common::String abortOpt = g_lingo->pop().asString();
	Common::String restoreOpt = g_lingo->pop().asString();
	Common::String segmentName = g_lingo->pop().asString();

	bool restore = restoreOpt.equalsIgnoreCase("restore");
	bool shiftAbort = abortOpt.equalsIgnoreCase("shiftAbort");
	bool abortOnClick = abortOpt.equalsIgnoreCase("abortOnClick");
	bool purge = purgeOpt.equalsIgnoreCase("purge");
	bool async = asyncOpt.equalsIgnoreCase("async");
	MMovieXObject *me = static_cast<MMovieXObject *>(g_lingo->_state->me.u.obj);
	for (auto &it : me->_movies) {
		if (it._value.segLookup.contains(segmentName)) {
			int segIndex = it._value.segLookup.getVal(segmentName);
			int result = me->playSegment(it._key, segIndex, false, restore, shiftAbort, abortOnClick, purge, async);
			int ticks = me->getTicks();
			debugC(5, kDebugXObj, "MMovieXObj::m_playSegment: ticks: %d, result: %d", ticks, result);
			g_lingo->push(Datum(result));
			return;
		}
	}
	g_lingo->push(Datum(MMovieError::MMOVIE_INVALID_SEGMENT_NAME));
	return;
}

void MMovieXObj::m_playSegLoop(int nargs) {
	g_lingo->printArgs("MMovieXObj::m_playSegLoop", nargs);
	if (nargs != 5) {
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(MMovieError::MMOVIE_INVALID_SEGMENT_NAME));
		return;
	}

	Common::String asyncOpt = g_lingo->pop().asString();
	Common::String purgeOpt = g_lingo->pop().asString();
	Common::String abortOpt = g_lingo->pop().asString();
	Common::String restoreOpt = g_lingo->pop().asString();
	Common::String segmentName = g_lingo->pop().asString();

	bool restore = restoreOpt.equalsIgnoreCase("restore");
	bool shiftAbort = abortOpt.equalsIgnoreCase("shiftAbort");
	bool abortOnClick = abortOpt.equalsIgnoreCase("abortOnClick");
	bool purge = abortOpt.equalsIgnoreCase("purge");
	bool async = asyncOpt.equalsIgnoreCase("async");

	MMovieXObject *me = static_cast<MMovieXObject *>(g_lingo->_state->me.u.obj);
	for (auto &it : me->_movies) {
		if (it._value.segLookup.contains(segmentName)) {
			int segIndex = it._value.segLookup.getVal(segmentName);
			int result = me->playSegment(it._key, segIndex, true, restore, shiftAbort, abortOnClick, purge, async);
			int ticks = me->getTicks();
			debugC(5, kDebugXObj, "MMovieXObj::m_playSegLoop: ticks: %d, result: %d", ticks, result);
			g_lingo->push(Datum(result));
			return;
		}
	}
	g_lingo->push(Datum(MMovieError::MMOVIE_INVALID_SEGMENT_NAME));
	return;
}

void MMovieXObj::m_idleSegment(int nargs) {
	if (nargs != 0) {
		g_lingo->dropStack(nargs);
	}
	MMovieXObject *me = static_cast<MMovieXObject *>(g_lingo->_state->me.u.obj);
	int result = me->updateScreen();
	int ticks = me->getTicks();
	debugC(5, kDebugXObj, "MMovieXObj::m_idleSegment(): ticks: %d, result: %d", ticks, result);

	g_lingo->push(Datum(result));
}

void MMovieXObj::m_stopSegment(int nargs) {
	g_lingo->printArgs("MMovieXObj::m_stopSegment", nargs);
	if (nargs != 0) {
		g_lingo->dropStack(nargs);
	}
	MMovieXObject *me = static_cast<MMovieXObject *>(g_lingo->_state->me.u.obj);
	me->stopSegment();
	g_lingo->push(0);
}

void MMovieXObj::m_seekSegment(int nargs) {
	g_lingo->printArgs("MMovieXObj::m_seekSegment", nargs);
	if (nargs != 1) {
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(MMovieError::MMOVIE_INVALID_SEGMENT_NAME));
		return;
	}
	Common::String segmentName = g_lingo->pop().asString();
	MMovieXObject *me = static_cast<MMovieXObject *>(g_lingo->_state->me.u.obj);
	for (auto &it : me->_movies) {
		if (it._value.segLookup.contains(segmentName)) {
			g_lingo->push(Datum(MMovieError::MMOVIE_NONE));
			return;
		}
	}
	g_lingo->push(Datum(MMovieError::MMOVIE_INVALID_SEGMENT_NAME));
}


XOBJSTUB(MMovieXObj::m_setSegmentTime, 0)

void MMovieXObj::m_setDisplayBounds(int nargs) {
	g_lingo->printArgs("MMovieXObj::m_setDisplayBounds", nargs);
	if (nargs != 4) {
		warning("MMovieXObj::m_setDisplayBounds: expecting 4 arguments!");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(0));
		return;
	}
	MMovieXObject *me = static_cast<MMovieXObject *>(g_lingo->_state->me.u.obj);

	Datum bottom = g_lingo->pop();
	Datum right = g_lingo->pop();
	Datum top = g_lingo->pop();
	Datum left = g_lingo->pop();
	me->_bounds = Common::Rect((int16)left.asInt(), (int16)top.asInt(), (int16)right.asInt(), (int16)bottom.asInt());
	me->_lastFrame.free();
	me->_lastFrame.create(me->_bounds.width(), me->_bounds.height(), g_director->_pixelformat);
	Common::Rect screen = g_director->getCurrentMovie()->_movieRect;
	me->_bounds.clip(Common::Rect(screen.width(), screen.height()));
	g_lingo->push(Datum(0));
}

XOBJSTUB(MMovieXObj::m_getMovieNormalWidth, 0)
XOBJSTUB(MMovieXObj::m_getMovieNormalHeight, 0)

void MMovieXObj::m_getSegCount(int nargs) {
	g_lingo->printArgs("MMovieXObj::m_getSegCount", nargs);
	if (nargs != 1) {
		g_lingo->dropStack(nargs);
		g_lingo->push(MMovieError::MMOVIE_INVALID_MOVIE_INDEX);
		return;
	}
	MMovieXObject *me = static_cast<MMovieXObject *>(g_lingo->_state->me.u.obj);
	int movieIndex = g_lingo->pop().asInt();
	if (!me->_movies.contains(movieIndex)) {
		g_lingo->push(MMovieError::MMOVIE_INVALID_MOVIE_INDEX);
		return;
	}
	g_lingo->push((int)me->_movies.getVal(movieIndex).segments.size());
}

void MMovieXObj::m_getSegName(int nargs) {
	if (nargs != 2) {
		g_lingo->dropStack(nargs);
		g_lingo->push(Common::String(""));
		return;
	}
	MMovieXObject *me = static_cast<MMovieXObject *>(g_lingo->_state->me.u.obj);
	int segmentIndex = g_lingo->pop().asInt();
	int movieIndex = g_lingo->pop().asInt();
	if (!me->_movies.contains(movieIndex)) {
		g_lingo->push(Common::String(""));
		return;
	}
	if (segmentIndex > (int)me->_movies.getVal(movieIndex).segments.size() ||
			segmentIndex <= 0) {
		g_lingo->push(Common::String(""));
		return;
	}
	Common::String result = me->_movies.getVal(movieIndex).segments[segmentIndex - 1]._name;
	debugC(5, kDebugXObj, "MMovieXObj::m_getSegName(%d, %d): %s", movieIndex, segmentIndex, result.c_str());
	g_lingo->push(result);
}

void MMovieXObj::m_getMovieRate(int nargs) {
	g_lingo->printArgs("MMovieXObj::m_getMovieRate", nargs);
	if (nargs != 0) {
		g_lingo->dropStack(nargs);
	}
	MMovieXObject *me = static_cast<MMovieXObject *>(g_lingo->_state->me.u.obj);
	g_lingo->push(Datum(me->_rate));
}

void MMovieXObj::m_setMovieRate(int nargs) {
	g_lingo->printArgs("MMovieXObj::m_setMovieRate", nargs);
	if (nargs != 1) {
		warning("MMovieXObj::m_setMovieRate: expecting 4 arguments");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(0));
		return;
	}
	MMovieXObject *me = static_cast<MMovieXObject *>(g_lingo->_state->me.u.obj);
	me->_rate = g_lingo->pop().asInt();
	g_lingo->push(Datum(me->_rate));
}

XOBJSTUB(MMovieXObj::m_flushEvents, 0)
XOBJSTUB(MMovieXObj::m_invalidateRect, 0)

void MMovieXObj::m_readFile(int nargs) {
	g_lingo->printArgs("MMovieXObj::m_readFile", nargs);
	if (nargs != 2) {
		warning("MMovieXObj::m_readFile(): expecting 2 arguments");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(""));
		return;
	}
	Common::SaveFileManager *saves = g_system->getSavefileManager();
	bool scramble = g_lingo->pop().asInt() != 0;
	Common::String origPath = g_lingo->pop().asString();
	Common::String path = origPath;

	Common::String prefix = savePrefix();
	Common::String result;
	if (origPath.empty()) {
		path = getFileNameFromModal(false, Common::String(), Common::String(), "txt");
		if (path.empty()) {
			debugC(5, kDebugXObj, "MMovieXObj::m_readFile(): read cancelled by modal");
			g_lingo->push(result);
			return;
		}
	} else {
		path = lastPathComponent(origPath, g_director->_dirSeparator);
		if (!path.hasSuffixIgnoreCase(".txt"))
			path += ".txt";
	}
	if (!path.hasPrefixIgnoreCase(prefix)) {
		path = prefix + path;
	}

	Common::SeekableReadStream *stream = saves->openForLoading(path);
	if (stream) {
		debugC(5, kDebugXObj, "MMovieXObj::m_readFile(): opening file %s as %s from the saves dir", origPath.c_str(), path.c_str());
	} else {
		Common::File *f = new Common::File;
		Common::Path location = findPath(origPath);
		if (!location.empty() && f->open(location)) {
			debugC(5, kDebugXObj, "MMovieXObj::m_readFile(): opening file %s from the game dir", origPath.c_str());
			stream = (Common::SeekableReadStream *)f;
		} else {
			delete f;
		}
	}

	if (stream) {
		while (!stream->eos() && !stream->err()) {
			byte ch = stream->readByte();
			if (scramble) // remove unbreakable encryption
				ch ^= 0xa5;
			result += ch;
		}
		delete stream;
	} else {
		warning("MMovieXObj::m_readFile(): file %s not found", origPath.c_str());
	}

	g_lingo->push(result);
}

void MMovieXObj::m_writeFile(int nargs) {
	g_lingo->printArgs("MMovieXObj::m_writeFile", nargs);
	if (nargs != 3) {
		warning("MMovieXObj::m_writeFile(): expecting 3 arguments");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(""));
		return;
	}
	Common::SaveFileManager *saves = g_system->getSavefileManager();
	bool scramble = g_lingo->pop().asInt() != 0;
	Common::String data = g_lingo->pop().asString();
	Common::String origPath = g_lingo->pop().asString();
	Common::String path = origPath;
	Common::String result;

	Common::String prefix = savePrefix();
	if (origPath.empty()) {
		path = getFileNameFromModal(true, Common::String(), Common::String(), "txt");
		if (path.empty()) {
			debugC(5, kDebugXObj, "MMovieXObj::m_writeFile(): read cancelled by modal");
			g_lingo->push(result);
			return;
		}
	} else {
		path = lastPathComponent(origPath, g_director->_dirSeparator);
		// Virtual Nightclub makes autosaves at random intervals.
		// Intercept and give them a sensible name.
		if (path.hasSuffixIgnoreCase(".VNC") && path.hasPrefixIgnoreCase("VNC_")) {
			path = Common::String::format("Autosave.txt");
		}

		if (!path.hasSuffixIgnoreCase(".txt"))
			path += ".txt";
	}
	if (!path.hasPrefixIgnoreCase(prefix)) {
		path = prefix + path;
	}

	Common::SeekableWriteStream *stream = saves->openForSaving(path, false);

	if (stream) {
		debugC(5, kDebugXObj, "MMovieXObj::m_writeFile(): opening file %s as %s from the saves dir", origPath.c_str(), path.c_str());
		for (auto &it : data) {
			byte ch = it;
			if (scramble) // apply world's greatest encryption
				ch ^= 0xa5;
			stream->writeByte(ch);
		}
		stream->finalize();
		delete stream;
	} else {
		warning("MMovieXObj::m_writeFile(): file %s not found", origPath.c_str());
	}

	g_lingo->push(result);
}

XOBJSTUB(MMovieXObj::m_copyFile, 0)
XOBJSTUB(MMovieXObj::m_copyFileCont, 0)
XOBJSTUB(MMovieXObj::m_freeSpace, 0)
XOBJSTUB(MMovieXObj::m_deleteFile, 0)
XOBJSTUB(MMovieXObj::m_volList, "")

}
