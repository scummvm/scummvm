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

#ifndef DIRECTOR_LINGO_XTRAS_B_BLINKER_H
#define DIRECTOR_LINGO_XTRAS_B_BLINKER_H

namespace Director {

class BlinkerXtraObject : public Object<BlinkerXtraObject> {
public:
	BlinkerXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BlinkerXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_blinkUpdate(int nargs);
void m_blink(int nargs);
void m_blinkSetDefaults(int nargs);
void m_blinkKill(int nargs);
void m_blinkKillAll(int nargs);
void m_blinkSuspend(int nargs);
void m_blinkSuspendAll(int nargs);
void m_blinkResume(int nargs);
void m_blinkResumeAll(int nargs);
void m_blinking(int nargs);
void m_blinkSuspended(int nargs);
void m_blinkRegister(int nargs);
void m_blinkVersion(int nargs);
void m_"(int nargs);

} // End of namespace BlinkerXtra

} // End of namespace Director

#endif
