/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#ifndef CINE_SNDDRIVER_H_
#define CINE_SNDDRIVER_H_

#include "sound/audiostream.h"
#include "sound/fmopl.h"

namespace Audio {
	class Mixer;
}

namespace Cine {

#define ADLIB_REG_TIMER_1_DATA 2
#define ADLIB_REG_TIMER_CONTROL_FLAGS 4
#define ADLIB_REG_AM_VIBRATO_EG_KS 0x20
#define ADLIB_REG_KEY_SCALING_OPERATOR_OUTPUT 0x40
#define ADLIB_REG_ATTACK_RATE_DECAY_RATE 0x60
#define ADLIB_REG_SUSTAIN_LEVEL_RELEASE_RATE_0 0x80
#define ADLIB_REG_FREQUENCY_0 0xA0
#define ADLIB_REG_KEY_ON_OCTAVE_FREQUENCY_0 0xB0
#define ADLIB_REG_AM_VIBRATO_RHYTHM 0xBD
#define ADLIB_REG_FEEDBACK_STRENGTH_CONNECTION_TYPE 0xC0
#define ADLIB_REG_WAVE_SELECT 0xE0

struct sndDriverStruct {
	void (*setupChannel) (int channelNum, const uint8 * data, int instrumentNum);
	void (*setChannelFrequency) (int channelNum, int frequency);
	void (*stopChannel) (int channelNum);
	void (*playSound) (uint8 * data, int channelNum, int volume);
};

extern uint16 snd_fadeOutCounter, snd_songTicksCounter;
extern uint8 *snd_adlibInstrumentsTable[4];
extern sndDriverStruct snd_driver;

extern void snd_adlibDriverInit();
extern void snd_adlibDriverExit();
extern void snd_adlibDriverStopSong();
extern void snd_resetChannel(int channelNum);

class AdlibMusic : public AudioStream {
public:
	AdlibMusic(Audio::Mixer * pMixer);
	~AdlibMusic(void);
	virtual void setVolume(uint8 volume);

	FM_OPL *getOPL() {
		return _opl;
	}

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
	Audio::Mixer * _mixer;
	uint32 _sampleRate;

	void premixerCall(int16 *buf, uint len);
};

} // End of namespace Cine

#endif				/* CINE_SNDDRIVER_H_ */
