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

#ifndef DIRECTOR_LINGO_XTRAS_U_UIHELPER_H
#define DIRECTOR_LINGO_XTRAS_U_UIHELPER_H

namespace Director {

class UiHelperXtraObject : public Object<UiHelperXtraObject> {
public:
	UiHelperXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace UiHelperXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_setCurrentCast(int nargs);
void m_activateWindow(int nargs);
void m_activateMediaEditor(int nargs);
void m_getBehaviorMemRef(int nargs);
void m_getBehaviorInitializers(int nargs);
void m_activateScriptEditor(int nargs);
void m_sleep(int nargs);
void m_getByte(int nargs);
void m_executeCommand(int nargs);
void m_isNt(int nargs);
void m_getLanguage(int nargs);
void m_openInputFile(int nargs);
void m_translateInputFile(int nargs);
void m_copyInputToOutput(int nargs);
void m_getToken(int nargs);
void m_getTokenType(int nargs);
void m_openOutputFile(int nargs);
void m_putToken(int nargs);
void m_pushPosition(int nargs);
void m_popPosition(int nargs);
void m_tossPosition(int nargs);
void m_getPosition(int nargs);
void m_getHighWater(int nargs);
void m_stringsEqual(int nargs);
void m_calcOffset(int nargs);
void m_calcIntersect(int nargs);
void m_getDate(int nargs);
void m_setFileWritable(int nargs);
void m_dbgPropList(int nargs);
void m_showHelpTopic(int nargs);

} // End of namespace UiHelperXtra

} // End of namespace Director

#endif
