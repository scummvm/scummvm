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

#ifndef DIRECTOR_LINGO_XLIBS_M_MAZEXOBJ_H
#define DIRECTOR_LINGO_XLIBS_M_MAZEXOBJ_H

namespace Director {

class MazeXObject : public Object<MazeXObject> {
public:
	MazeXObject(ObjectType objType);
};

namespace MazeXObj {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_userClicked(int nargs);
void m_checkClick(int nargs);
void m_getKeySettings(int nargs);
void m_setKeySettings(int nargs);
void m_getLocSettings(int nargs);
void m_setLocSettings(int nargs);
void m_getTemplate(int nargs);
void m_getKeyAdded(int nargs);
void m_setCDPath(int nargs);
void m_setHDPath(int nargs);
void m_setMovieBox(int nargs);
void m_updateScreen(int nargs);
void m_fixScreen(int nargs);
void m_getLevel(int nargs);
void m_getKeyState(int nargs);
void m_setKeyState(int nargs);
void m_setPreclickOption(int nargs);
void m_disposeMem(int nargs);

} // End of namespace MazeXObj

} // End of namespace Director

#endif
