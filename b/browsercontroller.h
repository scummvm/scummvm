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

#ifndef DIRECTOR_LINGO_XTRAS_B_BROWSERCONTROLLER_H
#define DIRECTOR_LINGO_XTRAS_B_BROWSERCONTROLLER_H

namespace Director {

class BrowsercontrollerXtraObject : public Object<BrowsercontrollerXtraObject> {
public:
	BrowsercontrollerXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BrowsercontrollerXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_BrowserControllerVerifyKey(int nargs);
void m_startProgram(int nargs);
void m_closeProgram(int nargs);
void m_stillRunningProgram(int nargs);
void m_getPathFromRegistry(int nargs);
void m_findBrowserExecutable(int nargs);
void m_getNetscapeInstallDir(int nargs);
void m_browserGoToURL(int nargs);
void m_initDDE(int nargs);
void m_closeDDE(int nargs);
void m_"(int nargs);

} // End of namespace BrowsercontrollerXtra

} // End of namespace Director

#endif
