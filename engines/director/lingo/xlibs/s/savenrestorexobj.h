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

#ifndef DIRECTOR_LINGO_XLIBS_S_SAVENRESTOREXOBJ_H
#define DIRECTOR_LINGO_XLIBS_S_SAVENRESTOREXOBJ_H

namespace Director {

class SaveNRestoreXObject : public Object<SaveNRestoreXObject> {
public:
	SaveNRestoreXObject(ObjectType objType);
};

namespace SaveNRestoreXObj {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_writeSettingsFile(int nargs);
void m_readSettingsFile(int nargs);
void m_setDirectorMovie(int nargs);
void m_setSubLocation1(int nargs);
void m_setSubLocation2(int nargs);
void m_setKeys(int nargs);
void m_setMazeKeys(int nargs);
void m_setMazeLocation(int nargs);
void m_getDirectorMovie(int nargs);
void m_getSubLocation1(int nargs);
void m_getSubLocation2(int nargs);
void m_getKeys(int nargs);
void m_getMazeKeys(int nargs);
void m_getMazeLocation(int nargs);

} // End of namespace SaveNRestoreXObj

} // End of namespace Director

#endif
