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

#ifndef DIRECTOR_LINGO_XTRAS_B_BUDDY_TRAYICON_H
#define DIRECTOR_LINGO_XTRAS_B_BUDDY_TRAYICON_H

namespace Director {

class Buddy_trayiconXtraObject : public Object<Buddy_trayiconXtraObject> {
public:
	Buddy_trayiconXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace Buddy_trayiconXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_btiCreateTrayIcon(int nargs);
void m_btiModifyTrayIcon(int nargs);
void m_btiDeleteTrayIcon(int nargs);
void m_btiDeleteAllIcons(int nargs);
void m_btiRestoreProgram(int nargs);
void m_btiMinimiseProgram(int nargs);
void m_btiHideProgram(int nargs);
void m_btiShowMenu(int nargs);
void m_btiAbout(int nargs);
void m_"(int nargs);

} // End of namespace Buddy_trayiconXtra

} // End of namespace Director

#endif
