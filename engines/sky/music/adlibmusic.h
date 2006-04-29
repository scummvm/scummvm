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

#ifndef ADLIBMUSIC_H
#define ADLIBMUSIC_H

#include "sky/music/musicbase.h"
#include "sound/audiostream.h"
#include "sound/fmopl.h"

namespace Audio {
	class Mixer;
}

namespace Sky {

class AdlibMusic : public Audio::AudioStream, public MusicBase {
public:
	AdlibMusic(Audio::Mixer *pMixer, Disk *pDisk);
	~AdlibMusic(void);

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) {
		premixerCall(buffer, numSamples / 2);
		return numSamples;
	}
	bool isStereo() const { return true; }
	bool endOfData() const { return false; }
	int getRate() const { return _sampleRate; }

private:
	FM_OPL *_opl;
	Audio::Mixer *_mixer;
	uint8 *_initSequence;
	uint32 _sampleRate, _nextMusicPoll;
	virtual void setupPointers(void);
	virtual void setupChannels(uint8 *channelData);
	virtual void startDriver(void);

	void premixerCall(int16 *buf, uint len);
};

} // End of namespace Sky

#endif //ADLIBMUSIC_H
