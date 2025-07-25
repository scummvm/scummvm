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

#ifndef DIRECTOR_LINGO_XLIBS_VOYAGERXSOUND_H
#define DIRECTOR_LINGO_XLIBS_VOYAGERXSOUND_H

#include "director/sound.h"

namespace Director {

struct VoyagerChannel {
	int channelID;
};

class VoyagerXSoundXObject : public Object<VoyagerXSoundXObject> {
public:
	VoyagerXSoundXObject(ObjectType objType);
	~VoyagerXSoundXObject();

	int open(int monoStereo, int numChan);
	void close();
	int status(int chan);
	int playfile(int chan, Common::String &path, int tstart, int tend);
	int fade(int chan, int endvol, int duration, bool autostop);
	void stop(int chan);
	void volume(int chan, int vol);
	void leftrightvol(int chan, uint8 lvol, uint8 rvol);
	void frequency(int chan, int percent);
	void pan(int chan, int percent);

	DirectorSound *_soundManager;

	Common::HashMap<int, VoyagerChannel *> _channels;
};

namespace VoyagerXSoundXObj {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_init(int nargs);
void m_open(int nargs);
void m_close(int nargs);
void m_bufsize(int nargs);
void m_exists(int nargs);
void m_status(int nargs);
void m_path(int nargs);
void m_duration(int nargs);
void m_curtime(int nargs);
void m_playfile(int nargs);
void m_loadfile(int nargs);
void m_unloadfile(int nargs);
void m_playsnd(int nargs);
void m_extplayfile(int nargs);
void m_stop(int nargs);
void m_volume(int nargs);
void m_leftrightvol(int nargs);
void m_fade(int nargs);
void m_frequency(int nargs);
void m_pan(int nargs);
void m_startrecord(int nargs);
void m_stoprecord(int nargs);
void m_recordpath(int nargs);

} // End of namespace VoyagerXSoundXObj

} // End of namespace Director

#endif
