/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MADS_SOUND_NEBULAR_H
#define MADS_SOUND_NEBULAR_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/mutex.h"
#include "common/queue.h"

namespace Audio {
class Mixer;
}

namespace Common {
class SeekableReadStream;
}

namespace OPL {
class OPL;
}

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

	AdlibSample() {}
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
#define CALLBACKS_PER_SECOND 60

struct CachedDataEntry {
	int _offset;
	byte *_data;
	byte *_dataEnd;
};

/**
 * Base class for the sound player resource files
 */
class ASound {
private:
	Common::List<CachedDataEntry> _dataCache;
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
	 * Loads up the specified sample
	 */
	void loadSample(int sampleIndex);

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
	int _commandParam;

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
	 * Loads a data block from the sound file, caching the result for any future
	 * calls for the same data
	 */
	byte *loadData(int offset, int size);

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

	virtual int command0();
	int command1();
	int command2();
	int command3();
	int command4();
	int command5();
	int command6();
	int command7();
	int command8();

	int nullCommand() { return 0; }
public:
	Audio::Mixer *_mixer;
	OPL::OPL *_opl;
	AdlibChannel _channels[ADLIB_CHANNEL_COUNT];
	AdlibChannel *_activeChannelPtr;
	AdlibChannelData _channelData[11];
	Common::Array<AdlibSample> _samples;
	AdlibSample *_samplePtr;
	Common::File _soundFile;
	Common::Queue<RegisterValue> _queue;
	Common::Mutex _driverMutex;
	int _dataOffset;
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
	 */
	ASound(Audio::Mixer *mixer, OPL::OPL *opl, const Common::String &filename, int dataOffset);

	/**
	 * Destructor
	 */
	virtual ~ASound();

	/**
	 * Validates the Adlib sound files
	 */
	static void validate();

	/**
	 * Execute a player command. Most commands represent sounds to play, but some
	 * low number commands also provide control operations.
	 * @param commandId		Player ommand to execute.
	 * @param param			Optional parameter used by a few commands
	 */
	virtual int command(int commandId, int param) = 0;

	/**
	 * Stop all currently playing sounds
	 */
	int stop();

	/**
	 * Main poll method to allow sounds to progress
	 */
	int poll();

	/**
	 * General noise/note output
	 */
	void noise();

	/**
	 * Return the current frame counter
	 */
	int getFrameCounter() { return _frameCounter; }

	/**
	 * Return the cached data block record for previously loaded sound data
	 */
	CachedDataEntry &getCachedData(byte *pData);

	/**
	 * Set the volume
	 */
	void setVolume(int volume);
};

class ASound1 : public ASound {
private:
	typedef int (ASound1::*CommandPtr)();
	static const CommandPtr _commandList[42];
	bool _cmd23Toggle;

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();
	int command40();
	int command41();

	void command111213();
	int command2627293032();
public:
	ASound1(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound2 : public ASound {
private:
	byte _command12Param;
private:
	typedef int (ASound2::*CommandPtr)();
	static const CommandPtr _commandList[44];

	int command0() override;
	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();
	int command40();
	int command41();
	int command42();
	int command43();

	void command9Randomize();
	void command9Apply(byte *data, int val, int incr);
public:
	ASound2(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound3 : public ASound {
private:
	bool _command39Flag;

	typedef int (ASound3::*CommandPtr)();
	static const CommandPtr _commandList[61];

	int command9();
	int command10();
	int command11();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();
	int command40();
	int command41();
	int command42();
	int command43();
	int command44();
	int command45();
	int command46();
	int command47();
	int command49();
	int command50();
	int command51();
	int command57();
	int command59();
	int command60();

	void command9Randomize();
	void command9Apply(byte *data, int val, int incr);
public:
	ASound3(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound4 : public ASound {
private:
	typedef int (ASound4::*CommandPtr)();
	static const CommandPtr _commandList[61];

	int command10();
	int command12();
	int command19();
	int command20();
	int command21();
	int command24();
	int command27();
	int command30();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command43();
	int command52();
	int command53();
	int command54();
	int command55();
	int command56();
	int command57();
	int command58();
	int command59();
	int command60();

	void method1();
public:
	ASound4(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound5 : public ASound {
private:
	typedef int (ASound5::*CommandPtr)();
	static const CommandPtr _commandList[42];

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();
	int command40();
	int command41();
	int command42();
	int command43();
public:
	ASound5(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound6 : public ASound {
private:
	typedef int (ASound6::*CommandPtr)();
	static const CommandPtr _commandList[30];

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command29();
public:
	ASound6(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound7 : public ASound {
private:
	typedef int (ASound7::*CommandPtr)();
	static const CommandPtr _commandList[38];

	int command9();
	int command15();
	int command16();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command30();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
public:
	ASound7(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound8 : public ASound {
private:
	typedef int (ASound8::*CommandPtr)();
	static const CommandPtr _commandList[38];

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();

	void method1(byte *pData);
	void adjustRange(byte *pData, byte v, int incr);
public:
	ASound8(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound9 : public ASound {
private:
	int _v1, _v2;
	byte *_soundPtr;

	typedef int (ASound9::*CommandPtr)();
	static const CommandPtr _commandList[52];

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();
	int command40();
	int command41();
	int command42();
	int command43();
	int command44_46();
	int command45();
	int command47();
	int command48();
	int command49();
	int command50();
	int command51();
	int command57();
	int command59();
	int command60();
public:
	ASound9(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_SOUND_NEBULAR_H */
