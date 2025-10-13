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

#ifndef DIRECTOR_LINGO_XLIBS_MISCX_H
#define DIRECTOR_LINGO_XLIBS_MISCX_H

namespace Director {

class MiscXObject : public Object<MiscXObject> {
public:
	MiscXObject(ObjectType objType);
};

namespace MiscX {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_bootName(int nargs);
void m_windowsDirectory(int nargs);
void m_fileExists(int nargs);
void m_copyFile(int nargs);
void m_folderExists(int nargs);
void m_insureFolder(int nargs);
void m_prefsFolder(int nargs);
void m_deleteFolder(int nargs);
void m_fileList(int nargs);
void m_ask(int nargs);
void m_answer(int nargs);
void m_spaceOnVol(int nargs);
void m_deleteGroup(int nargs);

} // End of namespace MiscX

} // End of namespace Director

#endif
