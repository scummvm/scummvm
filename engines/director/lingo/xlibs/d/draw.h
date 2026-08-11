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

#ifndef DIRECTOR_LINGO_XLIBS_DRAWXOBJ_H
#define DIRECTOR_LINGO_XLIBS_DRAWXOBJ_H

namespace Director {

class DrawXObject : public Object<DrawXObject> {
public:
	DrawXObject(ObjectType objType);
};

namespace DrawXObj {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_line(int nargs);
void m_lineBrush(int nargs);
void m_lineBrushTrans(int nargs);
void m_lineBrushCol(int nargs);
void m_filterBMP(int nargs);
void m_filterDIB(int nargs);
void m_filterBMP128(int nargs);
void m_filterDIB128(int nargs);
void m_filterBMPMakeGhostImage(int nargs);
void m_filterDIBMakeGhostImage(int nargs);
void m_emptyClipboard(int nargs);
void m_fill(int nargs);
void m_getColor(int nargs);
void m_drawRect(int nargs);
void m_drawFrame(int nargs);

} // End of namespace DrawXObj

} // End of namespace Director

#endif
