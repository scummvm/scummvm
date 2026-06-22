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

#ifndef DIRECTOR_LINGO_XTRAS_B_BUDMENU_H
#define DIRECTOR_LINGO_XTRAS_B_BUDMENU_H

namespace Director {

class BudmenuXtraObject : public Object<BudmenuXtraObject> {
public:
	BudmenuXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BudmenuXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_bmMenu(int nargs);
void m_bmMenuAt(int nargs);
void m_bmCreateMenu(int nargs);
void m_bmAppendItem(int nargs);
void m_bmAppendItems(int nargs);
void m_bmInsertItem(int nargs);
void m_bmInsertItems(int nargs);
void m_bmInsertItemAt(int nargs);
void m_bmInsertItemsAt(int nargs);
void m_bmEnableItem(int nargs);
void m_bmDisableItem(int nargs);
void m_bmItemDisabled(int nargs);
void m_bmCheckItem(int nargs);
void m_bmUncheckItem(int nargs);
void m_bmItemChecked(int nargs);
void m_bmChangeItemText(int nargs);
void m_bmDeleteItem(int nargs);
void m_bmAttachSubMenu(int nargs);
void m_bmDetachSubMenu(int nargs);
void m_bmShowMenu(int nargs);
void m_bmShowMenuAt(int nargs);
void m_bmDestroyMenu(int nargs);
void m_"(int nargs);

} // End of namespace BudmenuXtra

} // End of namespace Director

#endif
