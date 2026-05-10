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

#ifndef MADS_NEBULAR_CORE_ASOUND_H
#define MADS_NEBULAR_CORE_ASOUND_H

#include "mads/core/sound_manager.h"

namespace MADS {
namespace Nebular {

class ASound;

/**
 * Represents the data for a channel on the Adlib
 */
class AdlibChannel {
public:
	ASound *_owner;

	int _activeCount;
	int _field1;
	int _field2;
	int _field3;
	int _field4;
	int _sampleIndex;
	int _volume;
	int _field7;
	int _field8;
	int _field9;
	int _fieldA;
	uint8 _fieldB;
	int _fieldC;
	int _fieldD;
	int _fieldE;
	byte *_ptr1;
	byte *_pSrc;
	byte *_ptr3;
	byte *_ptr4;
	byte *_ptrEnd;
	int _field17;
	int _field19;
	byte *_soundData;
	int _field1D;
	int _volumeOffset;
	int _field1F;

	// TODO: Only used by asound.003. Figure out usage
	byte _field20;

	// Phantom-specific fields
	int _field26;        // pitch delta (set in case -14, zeroed in case -3)
	int _field28;        // zeroed in case -3
	int _field2A;        // set in case -18
	int _field2B;        // volume-cap flag (suppresses upward volume changes)
	int _field2C;        // frequency counter (used with _field7 in cases -9/-10)
public:
	static bool _channelsEnabled;
public:
	AdlibChannel();

	void reset();
	void enable(int flag);
	void setPtr2(byte *pData);
	void load(byte *pData);
	void check(byte *nullPtr);
};

class AdlibChannelData {
public:
	int _field0;
	int _freqMask;
	int _freqBase;
	int _field6;
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
	int _fieldE;
	int _freqMask;
	int _freqBase;
	int _field14;

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
protected:
	struct CachedDataEntry {
		byte *_dataStart = nullptr;
		byte *_dataEnd = nullptr;
		CachedDataEntry(byte *dataStart, size_t size) : _dataStart(dataStart),
			_dataEnd(dataStart + size - 1) {
		}
	};

private:
	Common::Array<CachedDataEntry> _dataCache;

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
	 * Returns data for the specified offset. It also caches the data size for that
	 * offset, for any future references that need it.
	 */
	byte *loadData(int offset, int size);

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
	 * Play the specified sound
	 * @param offset	Offset of sound data within sound player data segment
	 * @param size		Size of sound data block
	 */
	void playSound(int offset, int size);

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
	int _v1;
	int _v2;
	int _activeChannelNumber;
	int _freqMask1;
	int _freqMask2;
	int _freqBase1;
	int _freqBase2;
	int _channelNum1, _channelNum2;
	int _v7;
	int _v8;
	int _v9;
	int _v10;
	int _pollResult;
	int _resultFlag;
	byte _nullData[2];
	int _ports[256];
	bool _stateFlag;
	int _activeChannelReg;
	int _v11;
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
	 * Return the cached data block record for previously loaded sound data
	 */
	CachedDataEntry &getCachedData(byte *pData);

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

} // namespace Nebular
} // namespace MADS

#endif
