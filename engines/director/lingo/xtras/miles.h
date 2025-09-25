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

#ifndef DIRECTOR_LINGO_XTRAS_MILES_H
#define DIRECTOR_LINGO_XTRAS_MILES_H

namespace Director {

class MilesXtraObject : public Object<MilesXtraObject> {
public:
	MilesXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace MilesXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_MilesQuickPlay(int nargs);
void m_MilesQuickStream(int nargs);
void m_MilesStartup(int nargs);
void m_MilesReleaseHandles(int nargs);
void m_MilesReacquireHandles(int nargs);
void m_MilesShutdown(int nargs);
void m_MilesHookToSmacker(int nargs);
void m_MilesSetWaveOutput(int nargs);
void m_MilesGetWaveDescription(int nargs);
void m_MilesOpenWave(int nargs);
void m_MilesOpenWaveCastmember(int nargs);
void m_MilesOpenXMIDI(int nargs);
void m_MilesOpenStream(int nargs);
void m_MilesOpenCD(int nargs);
void m_MilesOpenCopy(int nargs);
void m_MilesCopyFrom(int nargs);
void m_MilesPlay(int nargs);
void m_MilesPause(int nargs);
void m_MilesResume(int nargs);
void m_MilesClose(int nargs);
void m_MilesGetStatus(int nargs);
void m_MilesGetVolume(int nargs);
void m_MilesGetPosition(int nargs);
void m_MilesGetSpeed(int nargs);
void m_MilesGetLoopCount(int nargs);
void m_MilesGetPan(int nargs);
void m_MilesSetVolume(int nargs);
void m_MilesSetPosition(int nargs);
void m_MilesSetSpeed(int nargs);
void m_MilesSetLoopCount(int nargs);
void m_MilesSetPan(int nargs);
void m_MilesGetCDTracks(int nargs);
void m_MilesPlayCDTrack(int nargs);
void m_MilesPlayCDMS(int nargs);
void m_MilesGetCDStartTrackMS(int nargs);
void m_MilesGetCDEndTrackMS(int nargs);
void m_MilesEjectCD(int nargs);
void m_MilesRetractCD(int nargs);

} // End of namespace MilesXtra

} // End of namespace Director

#endif
