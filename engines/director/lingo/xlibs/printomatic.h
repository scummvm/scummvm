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

#ifndef DIRECTOR_LINGO_XLIBS_PRINTOMATICXOBJ_H
#define DIRECTOR_LINGO_XLIBS_PRINTOMATICXOBJ_H

namespace Director {

class PrintOMaticXObject : public Object<PrintOMaticXObject> {
public:
	PrintOMaticXObject(ObjectType objType);
};

namespace PrintOMaticXObj {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_reset(int nargs);
void m_newPage(int nargs);
void m_setPrintableMargins(int nargs);
void m_getPageWidth(int nargs);
void m_getPageHeight(int nargs);
void m_picture(int nargs);
void m_stagePicture(int nargs);
void m_1bitStagePicture(int nargs);
void m_setLandscapeMode(int nargs);
void m_doPageSetup(int nargs);
void m_doJobSetup(int nargs);
void m_setProgressMsg(int nargs);
void m_printPreview(int nargs);
void m_printPicts(int nargs);
void m_print(int nargs);
void m_register(int nargs);

} // End of namespace PrintOMaticXObj

} // End of namespace Director

#endif
