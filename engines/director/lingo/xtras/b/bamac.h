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

#ifndef DIRECTOR_LINGO_XTRAS_B_BAMAC_H
#define DIRECTOR_LINGO_XTRAS_B_BAMAC_H

namespace Director {

class BudAPIMacXtraObject : public Object<BudAPIMacXtraObject> {
public:
	BudAPIMacXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BudAPIMacXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_baFileCreator(int nargs);
void m_baFileType(int nargs);
void m_baSetFileInfo(int nargs);
void m_baEjectDisk(int nargs);
void m_baDiskList(int nargs);
void m_baGestalt(int nargs);
void m_baGestaltExists(int nargs);

} // End of namespace BudAPIMacXtra

} // End of namespace Director

#endif
