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

#ifndef DIRECTOR_LINGO_XTRAS_SCRNXTRA_H
#define DIRECTOR_LINGO_XTRAS_SCRNXTRA_H

namespace Director {

class ScrnxtraXtraObject : public Object<ScrnxtraXtraObject> {
public:
	ScrnxtraXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace ScrnxtraXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_StageToMember(int nargs);
void m_StageResizeToMember(int nargs);
void m_StageToClipboard(int nargs);
void m_StageResizeToClipboard(int nargs);
void m_StageToFile(int nargs);
void m_StageResizeToFile(int nargs);
void m_ScreenToMember(int nargs);
void m_ScreenResizeToMember(int nargs);
void m_ScreenToClipboard(int nargs);
void m_ScreenResizeToClipboard(int nargs);
void m_ScreenToFile(int nargs);
void m_ScreenResizeToFile(int nargs);

} // End of namespace ScrnxtraXtra

} // End of namespace Director

#endif
