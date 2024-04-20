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

namespace Director {

class MMovieXObject : public Object<MMovieXObject> {
public:
	MMovieXObject(ObjectType objType);
};

namespace MMovieXObj {

extern const char *xlibName;
extern const char *fileNames[];

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
