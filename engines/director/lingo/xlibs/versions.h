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

#ifndef DIRECTOR_LINGO_XLIBS_VERSIONS_H
#define DIRECTOR_LINGO_XLIBS_VERSIONS_H

namespace Director {

class VersionsXObject : public Object<VersionsXObject> {
public:
	VersionsXObject(ObjectType objType);
};

namespace VersionsXObj {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_quickTimeVersion(int nargs);
void m_win32QuickTimeVersion(int nargs);
void m_fileVersion(int nargs);
void m_windowsDirectory(int nargs);
void m_win32WindowsDirectory(int nargs);
void m_systemDirectory(int nargs);
void m_win32SystemDirectory(int nargs);
void m_dOSVersion(int nargs);
void m_windowsVersion(int nargs);
void m_win32Version(int nargs);
void m_win32Platform(int nargs);
void m_win32Build(int nargs);
void m_winNTVersion(int nargs);
void m_getShortFileName(int nargs);
void m_getLongFileName(int nargs);

} // End of namespace VersionsXObj

} // End of namespace Director

#endif
