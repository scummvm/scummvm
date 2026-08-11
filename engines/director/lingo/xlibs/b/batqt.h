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

#ifndef DIRECTOR_LINGO_XLIBS_BATQT_H
#define DIRECTOR_LINGO_XLIBS_BATQT_H

#include "common/rect.h"

namespace Video {
class QuickTimeDecoder;
}

namespace Director {

class BatQTXObject : public Object<BatQTXObject> {
public:
	BatQTXObject(ObjectType objType);
	~BatQTXObject();

	Video::QuickTimeDecoder *_video;
	Common::Rect _movieBox;
};

namespace BatQT {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_name(int nargs);
void m_status(int nargs);
void m_error(int nargs);
void m_lastError(int nargs);
void m_open(int nargs);
void m_play(int nargs);
void m_stop(int nargs);
void m_getTimeRange(int nargs);
void m_getMovieBox(int nargs);
void m_getTime(int nargs);
void m_setTime(int nargs);
void m_setVolume(int nargs);
void m_length(int nargs);
void m_setMovieBox(int nargs);
void m_setTimeRange(int nargs);
void m_addCallback(int nargs);
void m_removeCallback(int nargs);
void m_resetCallbacks(int nargs);
void m_setBatch(int nargs);

} // End of namespace BatQT

} // End of namespace Director

#endif
