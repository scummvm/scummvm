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

#ifndef DIRECTOR_LINGO_XLIBS_EDNOX_H
#define DIRECTOR_LINGO_XLIBS_EDNOX_H

namespace Director {

class EdnoxObject : public Object<EdnoxObject> {
public:
    EdnoxObject(ObjectType objType);
};

namespace Ednox {

extern const char *xlibName;
extern const char *fileNames[];

void open(int type);
void close(int type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_name(int nargs);
void m_status(int nargs);
void m_error(int nargs);
void m_lasterror(int nargs);
void m_playsoundx(int nargs);
void m_clearsoundx(int nargs);
void m_checksoundx(int nargs);
void m_drawbkgndx(int nargs);
void m_savex(int nargs);
void m_getpathx(int nargs);
void m_restorex(int nargs);
void m_setdrivex(int nargs);
void m_iscdx(int nargs);
void m_enabletaskswitch(int nargs);
void m_disabletaskswitch(int nargs);
void m_getdocumentname(int nargs);
void m_getdocumentfile(int nargs);
void m_savedocumentfile(int nargs);
void m_deletedocumentfile(int nargs);

} // End of namespace Ednox

} // End of namespace Director

#endif
