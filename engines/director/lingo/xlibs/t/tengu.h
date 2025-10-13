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

#ifndef DIRECTOR_LINGO_XLIBS_TENGU_H
#define DIRECTOR_LINGO_XLIBS_TENGU_H

namespace Director {

class TenguXObject : public Object<TenguXObject> {
public:
	TenguXObject(ObjectType objType);
};

namespace TenguXObj {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_setMenu(int nargs);
void m_gameStart(int nargs);
void m_setArea(int nargs);
void m_setShikake(int nargs);
void m_setEsa(int nargs);
void m_getTime(int nargs);
void m_getPal(int nargs);
void m_getAtari(int nargs);
void m_setNode(int nargs);
void m_getUkiLoc(int nargs);
void m_getUkiTime(int nargs);
void m_getSaoDepth(int nargs);
void m_continue(int nargs);
void m_getValue(int nargs);
void m_getFuna(int nargs);
void m_writeData(int nargs);
void m_readData(int nargs);
void m_alert(int nargs);
void m_setScore(int nargs);
void m_setChoka(int nargs);
void m_gyotaku(int nargs);
void m_eSearch(int nargs);
void m_tournament(int nargs);
void m_setYudachi(int nargs);
void m_setEvent(int nargs);

} // End of namespace TenguXObj

} // End of namespace Director

#endif
