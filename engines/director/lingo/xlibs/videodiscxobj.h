/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DIRECTOR_LINGO_XLIBS_VIDEODISCXOBJ_H
#define DIRECTOR_LINGO_XLIBS_VIDEODISCXOBJ_H

namespace Director {

class VideodiscXObject : public Object<VideodiscXObject> {
public:
	VideodiscXObject(ObjectType objType);
};

namespace VideodiscXObj {

extern const char *xlibName;
extern const char *fileNames[];

void open(int type);
void close(int type);

void m_new(int nargs);
void m_name(int nargs);
void m_player(int nargs);
void m_play(int nargs);
void m_playRev(int nargs);
void m_fastFwd(int nargs);
void m_fastRev(int nargs);
void m_slowFwd(int nargs);
void m_slowRev(int nargs);
void m_stepFwd(int nargs);
void m_stepRev(int nargs);
void m_playJog(int nargs);
void m_playSpeed(int nargs);
void m_playSegment(int nargs);
void m_pause(int nargs);
void m_stop(int nargs);
void m_eject(int nargs);
void m_stopAtFrame(int nargs);
void m_searchWait(int nargs);
void m_readPos(int nargs);
void m_showDisplay(int nargs);
void m_clear(int nargs);
void m_videoControl(int nargs);
void m_audioControl(int nargs);
void m_status(int nargs);

} // End of namespace VideodiscXObj

} // End of namespace Director

#endif
