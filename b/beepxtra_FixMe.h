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

#ifndef DIRECTOR_LINGO_XTRAS_B_BEEPXTRA_H
#define DIRECTOR_LINGO_XTRAS_B_BEEPXTRA_H

namespace Director {

class BeepxtraXtraObject : public Object<BeepxtraXtraObject> {
public:
	BeepxtraXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BeepxtraXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_beepOne(int nargs);
void m_beepTwo(int nargs);
void m_beepThree(int nargs);
void m_modemCheck(int nargs);
void m_findWindow(int nargs);
void m_moveWindow(int nargs);
void m_topMost(int nargs);
void m_NoTopMost(int nargs);
void m_noMinMaxBox(int nargs);
void m_ModifyWSFlag(int nargs);
void m_showWindow(int nargs);
void m_SetFocus(int nargs);
void m_GetDesktopWindow(int nargs);
void m_GetWindowRect(int nargs);
void m_SetWindowText(int nargs);
void m_GetWindow(int nargs);
void m_RegQueryValue(int nargs);
void m_GetVersion(int nargs);
void m_WaitForProc(int nargs);
void m_SendMessage(int nargs);
void m_ReadFile(int nargs);
void m_SetWindowPos(int nargs);
void m_KillTimer(int nargs);
void m_"(int nargs);

} // End of namespace BeepxtraXtra

} // End of namespace Director

#endif
