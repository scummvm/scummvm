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


#ifndef DIRECTOR_LINGO_XLIBS_JWXINI_H
#define DIRECTOR_LINGO_XLIBS_JWXINI_H

namespace Director {

class JourneyWareXINIXObject : public Object<JourneyWareXINIXObject> {
public:
	JourneyWareXINIXObject(ObjectType objType);
};

namespace JourneyWareXINIXObj {

extern const char *xlibName;
extern const char *fileNames[];

void open(int type);
void close(int type);

void m_new(int nargs);
void m_clear(int nargs);

void m_GetPrivateProfileInt(int nargs);
void m_GetPrivateProfileString(int nargs);
void m_GetProfileInt(int nargs);
void m_GetProfileString(int nargs);
void m_WritePrivateProfileString(int nargs);
void m_WriteProfileString(int nargs);

} // End of namespace JourneyWareXINIXObj

} // End of namespace Director

#endif
