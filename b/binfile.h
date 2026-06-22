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

#ifndef DIRECTOR_LINGO_XTRAS_B_BINFILE_H
#define DIRECTOR_LINGO_XTRAS_B_BINFILE_H

namespace Director {

class BinfileXtraObject : public Object<BinfileXtraObject> {
public:
	BinfileXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BinfileXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_bx_fopen(int nargs);
void m_bx_fclose(int nargs);
void m_bx_fread(int nargs);
void m_bx_fwrite(int nargs);
void m_bx_ftell(int nargs);
void m_bx_fseek(int nargs);
void m_bx_fsize(int nargs);
void m_bx_display_open(int nargs);
void m_bx_display_save(int nargs);
void m_bx_file_get_contents(int nargs);
void m_bx_file_put_contents(int nargs);
void m_bx_file_size(int nargs);
void m_bx_file_delete(int nargs);
void m_bx_file_exists(int nargs);
void m_bx_file_truncate(int nargs);
void m_"(int nargs);

} // End of namespace BinfileXtra

} // End of namespace Director

#endif
