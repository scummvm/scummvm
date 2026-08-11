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

#ifndef DIRECTOR_LINGO_XTRAS_M_MBOX_H
#define DIRECTOR_LINGO_XTRAS_M_MBOX_H

namespace Director {

class MBoxXtraObject : public Object<MBoxXtraObject> {
public:
	MBoxXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace MBoxXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_forget(int nargs);
void m_OKMsgBox(int nargs);
void m_YESNOMsgBox(int nargs);
void m_OKCANCELMsgBox(int nargs);
void m_RETRYCANCELMsgBox(int nargs);
void m_YESNOCANCELMsgBox(int nargs);
void m_DiskSpaceAvailable(int nargs);
void m_WindowsFileLimit(int nargs);
void m_WindowsFileAttr(int nargs);

} // End of namespace MBoxXtra

} // End of namespace Director

#endif
