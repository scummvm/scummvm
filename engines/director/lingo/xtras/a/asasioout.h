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

#ifndef DIRECTOR_LINGO_XTRAS_A_ASASIOOUT_H
#define DIRECTOR_LINGO_XTRAS_A_ASASIOOUT_H

namespace Director {

class AsasiooutXtraObject : public Object<AsasiooutXtraObject> {
public:
	AsasiooutXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace AsasiooutXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_connect(int nargs);
void m_setGlobalVolume(int nargs);
void m_getGlobalVolume(int nargs);
void m_getChannelCount(int nargs);
void m_getError(int nargs);
void m_preloadSound(int nargs);
void m_unloadSound(int nargs);
void m_getSoundChannelCount(int nargs);
void m_playSound(int nargs);
void m_stopSound(int nargs);
void m_pauseSound(int nargs);
void m_getSoundState(int nargs);
void m_getSoundDuration(int nargs);
void m_getSoundElapsed(int nargs);
void m_setVolumes(int nargs);
void m_getVolumes(int nargs);
void m_"(int nargs);

} // End of namespace AsasiooutXtra

} // End of namespace Director

#endif
