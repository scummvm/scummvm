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

#ifndef DIRECTOR_LINGO_XTRAS_B_BUDAPI_MAC_H
#define DIRECTOR_LINGO_XTRAS_B_BUDAPI_MAC_H

namespace Director {

class Budapi_macXtraObject : public Object<Budapi_macXtraObject> {
public:
	Budapi_macXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace Budapi_macXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_baFileCreator(int nargs);
void m_baFileType(int nargs);
void m_baSetFileInfo(int nargs);
void m_baMsgBoxButtons(int nargs);
void m_baGestalt(int nargs);
void m_baGestaltExists(int nargs);
void m_baIsBundle(int nargs);
void m_baFilePermissions(int nargs);
void m_baSetFilePermissions(int nargs);
void m_baReturnUnixNames(int nargs);
void m_baUnixName(int nargs);
void m_baHfsName(int nargs);
void m_baLanguage(int nargs);
void m_"(int nargs);

} // End of namespace Budapi_macXtra

} // End of namespace Director

#endif
