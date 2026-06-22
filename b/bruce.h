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

#ifndef DIRECTOR_LINGO_XTRAS_BRUCE_H
#define DIRECTOR_LINGO_XTRAS_BRUCE_H

namespace Director {

class BruceXtraObject : public Object<BruceXtraObject> {
public:
	BruceXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BruceXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_BruceCreate(int nargs);
void m_BruceDestroy(int nargs);
void m_BruceUseIndex(int nargs);
void m_BruceFind(int nargs);
void m_BruceFindNext(int nargs);
void m_BruceFindPrev(int nargs);
void m_BruceFindAll(int nargs);
void m_BruceFindTeledex(int nargs);
void m_BruceSeek(int nargs);
void m_BruceGet(int nargs);
void m_BruceGetChoices(int nargs);
void m_BruceGetAll(int nargs);
void m_BruceGetDataPos(int nargs);
void m_BruceSetDataPos(int nargs);
void m_BruceGetIndexPos(int nargs);
void m_BruceSetIndexPos(int nargs);
void m_BruceNextRec(int nargs);
void m_BruceSetPath(int nargs);

} // End of namespace BruceXtra

} // End of namespace Director

#endif
