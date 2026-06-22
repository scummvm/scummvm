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

#ifndef DIRECTOR_LINGO_XTRAS_B_BINFILEIO_H
#define DIRECTOR_LINGO_XTRAS_B_BINFILEIO_H

namespace Director {

class BinfileioXtraObject : public Object<BinfileioXtraObject> {
public:
	BinfileioXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BinfileioXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_openFile(int nargs);
void m_closeFile(int nargs);
void m_readNumeric(int nargs);
void m_writeNumeric(int nargs);
void m_readChars(int nargs);
void m_writeChars(int nargs);
void m_writeArray(int nargs);
void m_readArray(int nargs);
void m_setPosition(int nargs);
void m_getPosition(int nargs);
void m_size(int nargs);
void m_eof(int nargs);
void m_"(int nargs);

} // End of namespace BinfileioXtra

} // End of namespace Director

#endif
