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

#ifndef DIRECTOR_LINGO_XTRAS_A_ATLAS-OSUTILS_H
#define DIRECTOR_LINGO_XTRAS_A_ATLAS-OSUTILS_H

namespace Director {

class UtilsXtraObject : public Object<UtilsXtraObject> {
public:
	UtilsXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace UtilsXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_XRegister(int nargs);
void m_XRequirements(int nargs);
void m_XLanguageCode(int nargs);
void m_XSetLanguageCode(int nargs);
void m_XSetCursor(int nargs);
void m_XSetStageRect(int nargs);
void m_XFindApplicationPath(int nargs);
void m_XSetApplicationPath(int nargs);
void m_XSetAdSParam(int nargs);
void m_XAdSParam(int nargs);
void m_XText(int nargs);
void m_XMac(int nargs);
void m_XMac_X(int nargs);
void m_XMac_9(int nargs);
void m_XWin(int nargs);
void m_XVersionOS(int nargs);
void m_XVersionCarbon(int nargs);
void m_XVersionQT(int nargs);
void m_"(int nargs);

} // End of namespace UtilsXtra

} // End of namespace Director

#endif
