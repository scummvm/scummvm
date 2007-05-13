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

#ifndef CINE_SOUNDDRIVER_H
#define CINE_SOUNDDRIVER_H

#include "sound/audiostream.h"
#include "sound/fmopl.h"
#include "sound/mixer.h"

namespace Cine {

class SoundDriver {
public:
	typedef void (*UpdateCallback)(void *);

	virtual ~SoundDriver() {}

	virtual void setupChannel(int channel, const byte *data, int instrument, int volume) = 0;
	virtual void setChannelFrequency(int channel, int frequency) = 0;
	virtual void stopChannel(int channel) = 0;
	virtual void playSound(const byte *data, int size, int channel, int volume) = 0;
	virtual void stopSound() = 0;
	virtual const char *getInstrumentExtension() const { return ""; }
	virtual void update() {}
	
	void setUpdateCallback(UpdateCallback upCb, void *ref);
	void resetChannel(int channel);
	void findNote(int freq, int *note, int *oct) const;

protected:
	UpdateCallback _upCb;
	void *_upRef;

	static const int _noteTable[];
	static const int _noteTableCount;
};

struct AdlibRegisterSoundInstrument {
	uint16 vibrato;
	uint16 attackDecay;
	uint16 sustainRelease;
	uint16 feedbackStrength;
	uint16 keyScaling;
	uint16 outputLevel;
	uint16 freqMod;
};

struct AdlibSoundInstrument {
	byte mode;
	byte channel;
	AdlibRegisterSoundInstrument regMod;
	AdlibRegisterSoundInstrument regCar;
	byte waveSelectMod;
	byte waveSelectCar;
	byte amDepth;
};
	
class AdlibSoundDriver : public SoundDriver, Audio::AudioStream {
public:
	AdlibSoundDriver(Audio::Mixer *mixer);
	virtual ~AdlibSoundDriver();

	// SoundDriver interface
	virtual void setupChannel(int channel, const byte *data, int instrument, int volume);
	virtual void stopChannel(int channel);
	virtual void stopSound();

	// AudioStream interface
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return false; }
	virtual bool endOfData() const { return false; }
	virtual int getRate() const { return _sampleRate; }

	void initCard();
	void update(int16 *buf, int len);
	void setupInstrument(const byte *data, int channel);
	void loadRegisterInstrument(const byte *data, AdlibRegisterSoundInstrument *reg);
	virtual void loadInstrument(const byte *data, AdlibSoundInstrument *asi) = 0;

protected:
	FM_OPL *_opl;
	int _sampleRate;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;

	byte _vibrato;
	int _channelsVolumeTable[4];
	AdlibSoundInstrument _instrumentsTable[4];

	static const int _freqTable[];
	static const int _freqTableCount;
	static const int _operatorsTable[];
	static const int _operatorsTableCount;
	static const int _voiceOperatorsTable[];
	static const int _voiceOperatorsTableCount;
};

// Future Wars adlib driver
class AdlibSoundDriverINS : public AdlibSoundDriver {
public:
	AdlibSoundDriverINS(Audio::Mixer *mixer) : AdlibSoundDriver(mixer) {}
	virtual const char *getInstrumentExtension() const { return ".INS"; }
	virtual void loadInstrument(const byte *data, AdlibSoundInstrument *asi);
	virtual void setChannelFrequency(int channel, int frequency);
	virtual void playSound(const byte *data, int size, int channel, int volume);
};

// Operation Stealth adlib driver
class AdlibSoundDriverADL : public AdlibSoundDriver {
public:
	AdlibSoundDriverADL(Audio::Mixer *mixer) : AdlibSoundDriver(mixer) {}
	virtual const char *getInstrumentExtension() const { return ".ADL"; }
	virtual void loadInstrument(const byte *data, AdlibSoundInstrument *asi);
	virtual void setChannelFrequency(int channel, int frequency);
	virtual void playSound(const byte *data, int size, int channel, int volume);
};

class PaulaSoundDriver : public SoundDriver {
public:
	PaulaSoundDriver(Audio::Mixer *mixer);
	
	virtual void setupChannel(int channel, const byte *data, int instrument, int volume);
	virtual void setChannelFrequency(int channel, int frequency);
	virtual void stopChannel(int channel);
	virtual void playSound(const byte *data, int size, int channel, int volume);
	virtual void stopSound();
	
	// Future Wars specific
	void queueSound(int channel, int frequency, const uint8 *data, int size, int volumeStep, int stepCount, int volume, int repeat);
	virtual void update();

	enum {
		PAULA_FREQ = 7093789,
		NUM_CHANNELS = 4,
		SPL_HDR_SIZE = 22
	};
	
	struct SoundQueue {
		int freq;
		const uint8 *data;
		int size;
		int volumeStep;
		int stepCount;
		int step;
		bool repeat;
		int volume;
	};

private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _channelsTable[NUM_CHANNELS];
	uint _channelsFreqTable[NUM_CHANNELS];
	SoundQueue _soundsQueue[NUM_CHANNELS];
};

extern SoundDriver *g_soundDriver; // TEMP

} // End of namespace Cine

#endif /* CINE_SOUNDDRIVER_H_ */
