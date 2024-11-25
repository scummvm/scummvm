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

#ifndef DIRECTOR_LINGO_XLIBS_MOOVXOBJ_H
#define DIRECTOR_LINGO_XLIBS_MOOVXOBJ_H

namespace Video {
class QuickTimeDecoder;
}

namespace Director {

class MoovXObject : public Object<MoovXObject> {
public:
	MoovXObject(ObjectType objType);
	~MoovXObject();

public:
	Video::QuickTimeDecoder *_video;
	int _x;
	int _y;
};

namespace MoovXObj {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_name(int nargs);
void m_movieInit(int nargs);
void m_movieKill(int nargs);
void m_fondler(int nargs);
void m_playMovie(int nargs);
void m_pauseMovie(int nargs);
void m_soundMovie(int nargs);
void m_stopMovie(int nargs);
void m_movieDone(int nargs);

} // End of namespace MoovXObj

} // End of namespace Director

#endif
