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

#ifndef DIRECTOR_LINGO_XLIBS_SOUNDJAM_H
#define DIRECTOR_LINGO_XLIBS_SOUNDJAM_H

namespace Director {

class SoundJamObject : public Object<SoundJamObject> {
public:
	Common::HashMap<int, CastMemberID> _soundMap;

public:
	SoundJamObject(ObjectType objType);
};

namespace SoundJam {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_defineFileSound(int nargs);
void m_defineCastSound(int nargs);
void m_undefineSound(int nargs);
void m_readSome(int nargs);
void m_startSound(int nargs);
void m_switchNew(int nargs);
void m_switchParallel(int nargs);
void m_hasSwitchHappened(int nargs);
void m_toggleMute(int nargs);
void m_stop(int nargs);

} // End of namespace SoundJam

} // End of namespace Director

#endif
