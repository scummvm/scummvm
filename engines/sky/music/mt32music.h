/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef MT32MUSIC_H
#define MT32MUSIC_H

#include "sky/music/musicbase.h"

class MidiDriver;

namespace Sky {

class MT32Music : public MusicBase {
public:
	MT32Music(MidiDriver *pMidiDrv, Disk *pDisk);
	~MT32Music(void);
private:
	static void passTimerFunc(void *param);
	void timerCall(void);
	bool processPatchSysEx(uint8 *sysExData);
	virtual void setVolume(uint8 volume);

	uint32 _timerCount;
	uint8 *_sysExSequence;
	MidiDriver *_midiDrv;

	virtual void setupPointers(void);
	virtual void setupChannels(uint8 *channelData);
	virtual void startDriver(void);
};

} // End of namespace Sky

#endif //MT32MUSIC_H
