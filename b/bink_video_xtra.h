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

#ifndef DIRECTOR_LINGO_XTRAS_BINKVIDEOXTRA_H
#define DIRECTOR_LINGO_XTRAS_BINKVIDEOXTRA_H

namespace Director {

class BinkXtraObject : public Object<BinkXtraObject> {
public:
	BinkXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BinkXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_binkVersion(int nargs);
void m_binkSetInterfaceKeys(int nargs);
void m_binkSetPosition(int nargs);
void m_binkUseDirectSound(int nargs);
void m_binkUseMiles(int nargs);
void m_binkQuickPlay(int nargs);
void m_binkOpen(int nargs);
void m_binkClose(int nargs);
void m_binkPlay(int nargs);
void m_binkGetLastKey(int nargs);
void m_binkGetFrameNum(int nargs);
void m_binkGetFrames(int nargs);
void m_binkPlayFrames(int nargs);
void m_binkGoto(int nargs);
void m_binkPlayNext(int nargs);
void m_binkSetUpdateStage(int nargs);
void m_binkMinorVersion(int nargs);
void m_binkMajorVersion(int nargs);
void m_binkAbout(int nargs);

} // End of namespace BinkXtra

} // End of namespace Director

#endif
