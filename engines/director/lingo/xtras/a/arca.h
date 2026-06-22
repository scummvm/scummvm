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

#ifndef DIRECTOR_LINGO_XTRAS_A_ARCA_H
#define DIRECTOR_LINGO_XTRAS_A_ARCA_H

namespace Director {

class ArcaXtraObject : public Object<ArcaXtraObject> {
public:
	ArcaXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace ArcaXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_createdb(int nargs);
void m_opendb(int nargs);
void m_closedb(int nargs);
void m_compactdb(int nargs);
void m_getdbschema(int nargs);
void m_executesql(int nargs);
void m_createselection(int nargs);
void m_getrows(int nargs);
void m_getfield(int nargs);
void m_getlastinsertrowid(int nargs);
void m_freeselection(int nargs);
void m_arcaregister(int nargs);
void m_explainerror(int nargs);
void m_changekey(int nargs);
void m_setencoding(int nargs);
void m_"(int nargs);

} // End of namespace ArcaXtra

} // End of namespace Director

#endif
