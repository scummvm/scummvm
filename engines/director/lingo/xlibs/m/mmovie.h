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

#ifndef DIRECTOR_LINGO_XLIBS_MMOVIE_H
#define DIRECTOR_LINGO_XLIBS_MMOVIE_H

#include "common/hash-str.h"
#include "video/qt_decoder.h"
namespace Director {

// taken from shared:1124:mHandleError
enum MMovieError {
	MMOVIE_NONE = 0,
	MMOVIE_NO_STAGE = -1,
	MMOVIE_TOO_MANY_OPEN_FILES = -2,
	MMOVIE_MOVIE_ALREADY_OPEN = -3,
	MMOVIE_INVALID_MOVIE_INDEX = -4,
	MMOVIE_INVALID_OFFSETS_FILE = -5,
	MMOVIE_INVALID_SEGMENT_OFFSET = -6,
	MMOVIE_NO_MOVIES_OPEN = -7,
	MMOVIE_INVALID_SEGMENT_NAME = -8,
	MMOVIE_INDEX_OUT_OF_RANGE = -9,
	MMOVIE_CONTINUE_WITHOUT_PLAYING = -10,
	MMOVIE_ABORT_DOUBLE_CLICK = -11,
	MMOVIE_PLAYBACK_FINISHED = -12,
};

struct MMovieSegment {
	Common::String _name;
	uint32 _start = 0;
	uint32 _length = 0;
	MMovieSegment() {}
	MMovieSegment(Common::String name, uint32 start, uint32 length) : _name(name), _start(start), _length(length) {}
};

struct MMovieFile {
	int _lastIndex = 0;
	Common::Path _path;
	Common::Array<MMovieSegment> segments;
	Common::HashMap<Common::String, uint32, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> segLookup;
	Video::QuickTimeDecoder *_video = nullptr;
	MMovieFile() {}
	MMovieFile(Common::Path path) : _path(path) {}
};

class MMovieXObject : public Object<MMovieXObject> {
public:
	MMovieXObject(ObjectType objType);
	~MMovieXObject();

	int _rate = 100;
	Common::Rect _bounds;
	int _lastIndex = 1;
	int _currentMovieIndex = 0;
	int _currentSegmentIndex = 0;
	bool _looping = false;
	bool _restore = false;
	bool _shiftAbort = false;
	bool _abortOnClick = false;
	bool _purge = false;
	bool _async = false;
	int _lastTicks = -1;

	Common::HashMap<int, MMovieFile> _movies;
	Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _moviePathMap;

	Graphics::Surface _lastFrame;

	int playSegment(int movieIndex, int segIndex, bool looping, bool restore, bool shiftAbort, bool abortOnClick, bool purge, bool async);
	bool stopSegment();
	int updateScreenBlocking();
	int updateScreen();
	int getTicks();
};

namespace MMovieXObj {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_Movie(int nargs);
void m_new(int nargs);
void m_dispose(int nargs);
void m_openMMovie(int nargs);
void m_closeMMovie(int nargs);
void m_playSegment(int nargs);
void m_playSegLoop(int nargs);
void m_idleSegment(int nargs);
void m_stopSegment(int nargs);
void m_seekSegment(int nargs);
void m_setSegmentTime(int nargs);
void m_setDisplayBounds(int nargs);
void m_getMovieNormalWidth(int nargs);
void m_getMovieNormalHeight(int nargs);
void m_getSegCount(int nargs);
void m_getSegName(int nargs);
void m_getMovieRate(int nargs);
void m_setMovieRate(int nargs);
void m_flushEvents(int nargs);
void m_invalidateRect(int nargs);
void m_readFile(int nargs);
void m_writeFile(int nargs);
void m_copyFile(int nargs);
void m_copyFileCont(int nargs);
void m_freeSpace(int nargs);
void m_deleteFile(int nargs);
void m_volList(int nargs);

} // End of namespace MMovieXObj

} // End of namespace Director

#endif
