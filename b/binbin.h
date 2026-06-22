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

#ifndef DIRECTOR_LINGO_XTRAS_B_BINBIN_H
#define DIRECTOR_LINGO_XTRAS_B_BINBIN_H

namespace Director {

class BinbinXtraObject : public Object<BinbinXtraObject> {
public:
	BinbinXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BinbinXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_Bin2Dec(int nargs);
void m_Dec2Bin(int nargs);
void m_BinBin(int nargs);
void m_DecBin(int nargs);
void m_DecDec(int nargs);
void m_BinDec(int nargs);
void m_BinShift(int nargs);
void m_DecShift(int nargs);
void m_GetCarry(int nargs);
void m_ClearCarry(int nargs);
void m_"(int nargs);

} // End of namespace BinbinXtra

} // End of namespace Director

#endif
