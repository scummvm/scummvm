/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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

#ifndef AGI_SOUND_H
#define AGI_SOUND_H

#include "agi/agi.h"

namespace Agi {

#define BUFFER_SIZE	410

#define SOUND_EMU_NONE	0
#define SOUND_EMU_PC	1
#define SOUND_EMU_TANDY	2
#define SOUND_EMU_MAC	3
#define SOUND_EMU_AMIGA	4

#define SOUND_PLAYING   0x01
#define WAVEFORM_SIZE   64
#define ENV_ATTACK	10000		/**< envelope attack rate */
#define ENV_DECAY       1000		/**< envelope decay rate */
#define ENV_SUSTAIN     100		/**< envelope sustain level */
#define ENV_RELEASE	7500		/**< envelope release rate */
#define NUM_CHANNELS    7		/**< number of sound channels */

/**
 * AGI engine sound driver structure.
 */
struct sound_driver {
	char *description;
	int (*init) (int16 * buffer);
	void (*deinit) (void);
};

/**
 * AGI sound resource structure.
 */
struct agi_sound {
	uint32 flen;		/**< size of raw data */
	uint8 *rdata;		/**< raw sound data */
	uint8 flags;		/**< sound flags */
	uint16 type;		/**< sound resource type */
};

/**
 * AGI sound note structure.
 */
struct agi_note {
	uint8 dur_lo;		/**< LSB of note duration */
	uint8 dur_hi;			/**< MSB of note duration */
	uint8 frq_0;			/**< LSB of note frequency */
	uint8 frq_1;			/**< MSB of note frequency */
	uint8 vol;			/**< note volume */
};

/**
 * AGI engine sound channel structure.
 */
struct channel_info {
#define AGI_SOUND_SAMPLE	0x0001
#define AGI_SOUND_MIDI		0x0002
#define AGI_SOUND_4CHN		0x0008
	uint32 type;
	struct agi_note *ptr;
	int16 *ins;
	int32 size;
	uint32 phase;
#define AGI_SOUND_LOOP		0x0001
#define AGI_SOUND_ENVELOPE	0x0002
	uint32 flags;
#define AGI_SOUND_ENV_ATTACK	3
#define AGI_SOUND_ENV_DECAY	2
#define AGI_SOUND_ENV_SUSTAIN	1
#define AGI_SOUND_ENV_RELEASE	0
	uint32 adsr;
	int32 timer;
	uint32 end;
	uint32 freq;
	uint32 vol;
	uint32 env;
};

void decode_sound(int);
void unload_sound(int);
void play_sound(void);
int init_sound(void);
void deinit_sound(void);
void start_sound(int, int);
void stop_sound(void);
uint32 mix_sound(void);
int load_instruments(char *fname);

extern struct sound_driver *snd;

#endif				/* AGI_SOUND_H */

} // End of namespace Agi

#include "sound/audiostream.h"

namespace Audio {
class Mixer;
} // End of namespace Audio

namespace Agi {

class AGIMusic : public Audio::AudioStream {
public:
	AGIMusic(Audio::Mixer * pMixer);
	~AGIMusic(void);
	virtual void setVolume(uint8 volume);

	// AudioStream API
	int readBuffer(int16 * buffer, const int numSamples) {
		premixerCall(buffer, numSamples / 2);
		return numSamples;
	}

	bool isStereo() const {
		return false;
	}

	bool endOfData() const {
		return false;
	}

	int getRate() const {
		// FIXME: Ideally, we should use _sampleRate.
		return 22050;
	}

private:
	Audio::Mixer * _mixer;
	uint32 _sampleRate;

	void premixerCall(int16 * buf, uint len);
};

} // End of namespace Agi
