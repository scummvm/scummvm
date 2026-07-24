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

#ifndef MADS_NEBULAR_ASOUND_H
#define MADS_NEBULAR_ASOUND_H

#include "mads/core/sound_manager.h"

namespace MADS {
namespace RexNebular {

class ASound;

/**
 * Represents the data for a channel on the Adlib
 */
class AdlibChannel {
public:
	ASound *_owner = nullptr;

	int _activeCount = 0;
	int _pitchBend = 0;         // signed pitch-bend offset added to frequency by updateFNumber()
	int _volumeFadeStep = 0;    // signed per-period volume delta applied to _volumeOffset
	int _attenFadeStep = 0;     // signed per-period delta applied to _patchAttenuation
	int _note = 0;              // note byte read from the sound-data stream
	int _sampleIndex = 0;
	int _volume = 0;
	int _noteOffset = 0;        // subtracted from _activeCount to derive _keyOnDelay
	int _keyOnDelay = 0;        // countdown before the key-on bit is cleared (gate time)
	int _volumeFadeCounter = 0; // counts down to 0 before applying _volumeFadeStep
	int _volumeFadeReload = 0;  // reload value for _volumeFadeCounter
	uint8 _attenFadeCounter = 0;// counts down to 0 before applying _attenFadeStep
	int _attenFadeReload = 0;   // reload value for _attenFadeCounter
	int _patchAttenuation = 0;  // per-note attenuation offset added on top of the patch TL
	int _pendingStop = 0;       // non-zero while the channel is fading out to silence
	byte *_ptr1 = nullptr;
	byte *_pSrc = nullptr;
	byte *_innerLoopPtr = nullptr;	// inner-loop restart address (opcode 0)
	byte *_outerLoopPtr = nullptr;	// outer-loop restart address (opcode 1)
	int _innerLoopCount = 0;    // remaining inner-loop iterations (opcode 0)
	int _outerLoopCount = 0;    // remaining outer-loop iterations (opcode 1)
	byte *_soundData = nullptr;
	int _transpose = 0;         // fine-tune offset added into the frequency table lookup
	int _volumeOffset = 0;
	int _octaveTranspose = 0;   // added to _note before the octave/semitone split

	// Extra static per-channel volume trim, confirmed via disassembly
	// comparison to be a third additive term in updateActiveChannel()'s
	// volume sum (volume + volumeOffset + channelAttenuation, clamped to
	// [0, 63]). Only asound.003/asound.004's copy of the shared runtime
	// reads this = 0; it is otherwise always 0, so summing it in unconditionally
	// is harmless for every other driver.
	byte _channelAttenuation = 0;

public:
	static bool _channelsEnabled;
public:
	AdlibChannel() {}

	void reset();
	void enable(int flag);
	void setPtr2(byte *pData);
	void load(byte *pData);
	void check(byte *nullPtr);
};

class AdlibChannelData {
public:
	int _hasNoiseMode; // non-zero if this sample drives the 2-voice noise generator
	int _freqMask;
	int _freqBase;
	int _freqStep;      // per-tick frequency-sweep increment
};

class AdlibSample {
public:
	int _attackRate;
	int _decayRate;
	int _sustainLevel;
	int _releaseRate;
	bool _egTyp;
	bool _ksr;
	int _totalLevel;
	int _scalingLevel;
	int _waveformSelect;
	int _freqMultiple;
	int _feedback;
	bool _ampMod;
	int _vib;
	int _alg;
	int _noiseMode;    // copied into AdlibChannelData::_hasNoiseMode by loadSample()
	int _freqMask;
	int _freqBase;
	int _freqStep;     // copied into AdlibChannelData::_freqStep by loadSample()

	AdlibSample() {
	}
	AdlibSample(Common::SeekableReadStream &s);
};

struct RegisterValue {
	uint8 _regNum;
	uint8 _value;

	RegisterValue(int regNum, int value) {
		_regNum = regNum; _value = value;
	}
};

#define ADLIB_CHANNEL_COUNT 9
#define ADLIB_CHANNEL_MIDWAY 5

/**
 * Base class for the sound player resource files
 */
class ASound : public SoundDriver {
private:
	uint16 _randomSeed;
	int _masterVolume;

	/**
	 * Does the initial Adlib initialisation
	 */
	void adlibInit();

	/**
	 * Does on-going processing for the Adlib sounds being played
	 */
	void update();

	/**
	 * Polls each of the channels for updates
	 */
	void pollChannels();

	/**
	 * Checks the status of the channels
	 */
	void checkChannels();

	/**
	 * Polls the currently active channel
	 */
	void pollActiveChannel();

	/**
	 * Updates the octave of the currently active channel
	 */
	void updateOctave();

	void updateChannelState();
	void updateActiveChannel();

	/**
	 * Writes out the data of the selected sample to the Adlib
	 */
	void processSample();

	void updateFNumber();

	/**
	 * Timer function for OPL
	 */
	void onTimer();

protected:
	int _chanCommandCount;
	int _commandParam;

	virtual void channelCommand(byte *&pSrc, bool &updateFlag) = 0;

	/**
	 * Hook called once per update() frame, immediately after the disabled
	 * check and before the frame counter/channel polling. Only ASound9's
	 * driver data makes use of a recurring deferred-callback timer (the
	 * word_1949E/word_194A0/_soundPtr trio in the original disassembly);
	 * every other driver leaves this as a no-op.
	 */
	virtual void tickCallback() {
	}

	/**
	 * Returns data for the specified offset
	 */
	byte *loadData(int offset) {
		return &_soundData[offset];
	}

	int getDataOffset(byte *ptr) const {
		return ptr - &_soundData[0];
	}

	/**
	 * Loads up the specified sample
	 */
	void loadSample(int sampleIndex);

	/**
	 * Queue a byte for an Adlib register
	 */
	void write(int reg, int val);

	/**
	 * Queue a byte for an Adlib register, and store it in the _ports array
	 */
	int write2(int state, int reg, int val);

	/**
	 * Flush any pending Adlib register values to the OPL driver
	 */
	void flush();

	/**
	 * Turn a channel on
	 */
	void channelOn(int reg, int volume);

	/**
	 * Turn a channel off
	 */
	void channelOff(int reg);

	/**
	 * Checks for whether a poll result needs to be set
	 */
	void resultCheck();

	/**
	 * Play the specified sound, using any free channel from 5 to 8.
	 * @param offset	Offset of sound data within sound player data segment
	 */
	void playSound(int offset);

	/**
	 * Play the specified sound using any channel from 0 to 8.
	 * @param offset	Offset of sound data within sound player data segment
	 */
	void playSoundAny(int offset) {
		playSoundData(loadData(offset), 0);
	}

	/**
	 * Play the specified raw sound data
	 * @param pData		Pointer to data block containing sound data
	 * @param startingChannel	Channel to start scan from
	 */
	void playSoundData(byte *pData, int startingChannel = ADLIB_CHANNEL_MIDWAY);

	/**
	 * Checks to see whether the given block of data is already loaded into a channel.
	 */
	bool isSoundActive(byte *pData);

	/**
	 * Sets the frequency for a given channel.
	 */
	void setFrequency(int channel, int freq);

	/**
	 * Returns a 16-bit random number
	 */
	int getRandomNumber();

	/**
	 * Converts a 16-bit near pointer (data-segment offset) to a C++ byte pointer
	 * by searching the data cache for the matching block.
	 */
	byte *getDataPtr(int nearPtr);

	virtual int command0();
	int command1();
	int command2();
	int command3();
	int command4();
	int command5();
	int command6();
	int command7();
	int command8();

	int nullCommand() {
		return 0;
	}
public:
	AdlibChannel _channels[ADLIB_CHANNEL_COUNT];
	AdlibChannel *_activeChannelPtr;
	AdlibChannelData _channelData[11];
	Common::Array<AdlibSample> _samples;
	AdlibSample *_samplePtr;
	Common::Queue<RegisterValue> _queue;
	int _frameCounter;
	bool _isDisabled;
	int _noiseTicks1;       // remaining duration for noise voice 1 (byte_11F86)
	int _noiseTicks2;       // remaining duration for noise voice 2 (byte_11F87)
	int _activeChannelNumber;
	int _freqMask1;
	int _freqMask2;
	int _freqBase1;
	int _freqBase2;
	int _noiseChannel1, _noiseChannel2;
	int _noiseFreqStep1;    // per-tick frequency-sweep step for noise voice 1 (word_11F8A)
	int _noiseFreqStep2;    // per-tick frequency-sweep step for noise voice 2 (word_11F8C)
	int _savedNoiseTicks1;  // _noiseTicks1 saved across command6/7 (byte_194B0)
	int _savedNoiseTicks2;  // _noiseTicks2 saved across command6/7 (byte_194B1)
	int _pollResult;
	int _resultFlag;
	byte _nullData[2];
	int _ports[256];
	bool _stateFlag;
	int _activeChannelReg;
	int _outputReg;         // scratch OPL operator register offset used within loadSample()
	bool _amDep, _vibDep, _splitPoint;
public:
	/**
	 * Constructor
	 * @param mixer			Mixer
	 * @param opl			OPL
	 * @param filename		Specifies the adlib sound player file to use
	 * @param dataOffset	Offset in the file of the data segment
	 * @param dataSize		Size of the data segment
	 */
	ASound(Audio::Mixer *mixer, OPL::OPL *opl, const Common::Path &filename,
		int dataOffset, int dataSize);

	/**
	 * Destructor
	 */
	~ASound() override {
	}

	/**
	 * Validates the Adlib sound files
	 */
	static void validate();

	/**
	 * Stop all currently playing sounds
	 */
	int stop() override;

	/**
	 * Main poll method to allow sounds to progress
	 */
	int poll() override;

	/**
	 * General noise/note output
	 */
	void noise() override;

	/**
	 * Return the current frame counter
	 */
	int getFrameCounter() {
		return _frameCounter;
	}

	/**
	 * Set the volume
	 */
	void setVolume(int volume) override;
};

} // namespace RexNebular
} // namespace MADS

#endif
