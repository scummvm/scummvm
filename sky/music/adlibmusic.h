/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef ADLIBMUSIC_H
#define ADLIBMUSIC_H

#include "sky/music/musicbase.h"
#include "sound/fmopl.h"

class SoundMixer;

namespace Sky {

class AdlibMusic : public MusicBase {
public:
	AdlibMusic(SoundMixer *pMixer, Disk *pDisk, OSystem *system);
	~AdlibMusic(void);
	virtual void setVolume(uint8 volume);
private:
	FM_OPL *_opl;
	SoundMixer *_mixer;
	uint8 *_initSequence;
	uint32 _sampleRate, _nextMusicPoll;
	virtual void setupPointers(void);
	virtual void setupChannels(uint8 *channelData);
	virtual void startDriver(void);

	void premixerCall(int16 *buf, uint len);
	static void passMixerFunc(void *param, int16 *buf, uint len);
};

} // End of namespace Sky

#endif //ADLIBMUSIC_H
