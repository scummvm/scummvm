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

#ifndef DIRECTOR_LINGO_XLIBS_CDROMXOBJ_H
#define DIRECTOR_LINGO_XLIBS_CDROMXOBJ_H

#include "backends/audiocd/audiocd.h"

namespace Director {

class CDROMXObject : public Object<CDROMXObject> {
public:
	CDROMXObject(ObjectType objType);
    AudioCDManager::Status _cdda_status;
};

namespace CDROMXObj {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_name(int nargs);
void m_play(int nargs);
void m_playTrack(int nargs);
void m_playName(int nargs);
void m_playAbsTime(int nargs);
void m_playSegment(int nargs);
void m_askPlay(int nargs);
void m_stepFwd(int nargs);
void m_stepBwd(int nargs);
void m_pause(int nargs);
void m_continue(int nargs);
void m_stop(int nargs);
void m_stopTrack(int nargs);
void m_stopAbsTime(int nargs);
void m_removeStop(int nargs);
void m_eject(int nargs);
void m_status(int nargs);
void m_playMode(int nargs);
void m_currentFormat(int nargs);
void m_currentTrack(int nargs);
void m_currentTime(int nargs);
void m_firstTrack(int nargs);
void m_lastTrack(int nargs);
void m_totalTime(int nargs);
void m_scanFwd(int nargs);
void m_scanBwd(int nargs);
void m_stopScan(int nargs);

} // End of namespace CDROMXObj

} // End of namespace Director

#endif
