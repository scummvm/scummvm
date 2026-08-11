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

#ifndef DIRECTOR_LINGO_XLIBS_XWIN_H
#define DIRECTOR_LINGO_XLIBS_XWIN_H

namespace Director {

class XWINXObject : public Object<XWINXObject> {
public:
	XWINXObject(ObjectType objType);
};

namespace XWINXObj {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_writeChar(int nargs);
void m_writeString(int nargs);
void m_readChar(int nargs);
void m_readWord(int nargs);
void m_readLine(int nargs);
void m_readFile(int nargs);
void m_readToken(int nargs);
void m_getPosition(int nargs);
void m_setPosition(int nargs);
void m_getLength(int nargs);
void m_setFinderInfo(int nargs);
void m_getFinderInfo(int nargs);
void m_fileName(int nargs);
void m_delete(int nargs);
void m_status(int nargs);
void m_error(int nargs);
void m_readPICT(int nargs);
void m_nativeFileName(int nargs);
void m_hwnd(int nargs);

} // End of namespace XWINXObj

} // End of namespace Director

#endif
