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

#ifndef DIRECTOR_LINGO_XTRAS_G_GLU32_H
#define DIRECTOR_LINGO_XTRAS_G_GLU32_H

namespace Director {

class GLU32XtraObject : public Object<GLU32XtraObject> {
public:
	GLU32XtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;

	Common::String _dll;
	Common::String _func;
};

namespace GLU32Xtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_GLUNew(int nargs);
void m_GLUCall(int nargs);
void m_GLUGetResults(int nargs);
void m_GLUDispose(int nargs);
void m_GLUGetErrorString(int nargs);
void m_GLUGetLastError(int nargs);
void m_GLURegister(int nargs);
void m_GLUGetProp(int nargs);
void m_GLUSetProp(int nargs);

} // End of namespace GLU32Xtra

} // End of namespace Director

#endif
