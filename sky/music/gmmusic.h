/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
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
 * $Header$
 *
 */

#ifndef GMMUSIC_H
#define GMMUSIC_H

#include "sky/music/musicbase.h"

class MidiDriver;

namespace Sky {

class GmMusic : public MusicBase {
public:
	GmMusic(MidiDriver *pMidiDrv, Disk *pDisk);
	~GmMusic(void);
private:
	static void passTimerFunc(void *param);
	void timerCall(void);

	uint32 _timerCount;
	uint8 *_sysExSequence;
	MidiDriver *_midiDrv;
	static const byte _veloTab[128];

	virtual void setupPointers(void);
	virtual void setupChannels(uint8 *channelData);
	virtual void startDriver(void);
};

} // End of namespace Sky

#endif //GMMUSIC_H
