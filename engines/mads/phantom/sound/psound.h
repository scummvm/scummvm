/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT file.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef MADS_PHANTOM_SOUND_PSOUND_H
#define MADS_PHANTOM_SOUND_PSOUND_H

#include "mads/core/native_sound_timer.h"
#include "mads/core/sound_manager.h"

namespace OPL {
class OPL;
}

namespace MADS {
namespace Phantom {
namespace Sound {

struct PSoundTableLayout {
	uint16 panning;
	uint16 frequency;
	uint16 bank;
	uint16 channel;
	uint16 operators;
};

struct PSoundDriverData {
	const char *filename;
	int dataOffset;
	int initializedDataSize;
	int totalDataSize;
	uint16 randomSeedOffset;
	uint16 nullSequenceOffset;
	uint16 patchTableOffset;
	byte patchCount;
	PSoundTableLayout tables;
};

/** Interpreter for Return of the Phantom's PSOUND overlay family. */
class PSound : public SoundDriver {
public:
	enum {
		kChannelCount = 9,
		kMusicChannelCount = 6,
		kPatchSize = 0x40,
		kScriptVarCount = 32,
		kOpcodeBudgetPerTick = 256
	};

protected:
	enum RegisterBank {
		kFirstBank = 1,
		kSecondBank = 2,
		kBothBanks = kFirstBank | kSecondBank
	};

	/** Logical representation of the native 0x32-byte channel record. */
	struct Channel {
		byte activeCount;           // +00
		int8 pitchBend;             // +01
		int8 volumeFadeStep;        // +02
		int8 panningFadeStep;       // +03
		byte note;                  // +04
		byte patch;                 // +05
		byte volume;                // +06
		byte noteOffset;            // +07
		byte keyOnDelay;            // +08
		byte volumeFadeCounter;     // +09
		byte volumeFadeReload;      // +0a
		byte panningFadeCounter;    // +0b
		byte panningFadeReload;     // +0c
		byte panning;               // +0d
		int8 volumeOffset;          // +0e
		byte mode;                  // +0f
		byte operatorTotalLevel[4]; // +10..+13
		uint16 loopStart;           // +14
		uint16 position;            // +16
		uint16 innerLoopStart;      // +18
		uint16 outerLoopStart;      // +1a
		uint16 innerLoopCount;      // +1c
		uint16 outerLoopCount;      // +1e
		uint16 originalSequence;    // +20
		uint16 branchReturn;        // +22
		uint16 noiseMask;           // +24
		uint16 noiseBase;           // +26
		int16 noiseStep;            // +28
		byte noiseTicks;            // +2a
		byte savedNoiseTicks;       // +2b
		int8 transpose;             // +2c
		int8 noteTranspose;         // +2d
		byte pendingStop;           // +2e
		int8 patchAttenuation;      // +2f
		byte durationOverride;      // +30

		void reset();
		void load(uint16 sequenceOffset);
	};

	OPL::OPL *_opl;
	byte _registerCache[2][256];
	NativeSoundTimer _hostTimer;
	Channel _channels[kChannelCount];
	byte _scriptVars[kScriptVarCount];
	int _masterVolume;
	uint16 _randomSeed;
	uint16 _frameCounter;
	int16 _pollResult;
	int8 _resultFlag;
	uint16 _nullSequenceOffset;
	uint16 _patchTableOffset;
	PSoundTableLayout _tableLayout;
	byte _patchCount;
	bool _updatesEnabled;
	bool _noiseServiceEnabled;
	// The native BE-C1 opcodes update these fields, but the audited overlays'
	// per-tick tempo hook is a no-op. Preserve the state without inventing a
	// duration transform.
	uint16 _tickEnabled;
	uint16 _tickCounter;
	uint16 _tempoReload;
	uint16 _tempoTarget;
	int16 _tempoShift;
	uint16 _tempoBase;
	uint16 _tempoCurrent;
	uint16 _tempoScale;
	int _frameNumber2;

	PSound(Audio::Mixer *mixer, const PSoundDriverData &driverData);
	~PSound() override;

	bool isDataRangeValid(uint32 offset, uint32 length) const;
	const byte *getDataPointer(uint32 offset, uint32 length,
							   const char *operation) const;
	byte *getDataPointer(uint32 offset, uint32 length, const char *operation);
	byte readDataByte(uint32 offset) const;
	uint16 readDataUint16(uint32 offset) const;
	void writeDataByte(uint32 offset, byte value);
	void writeDataUint16(uint32 offset, uint16 value);

	byte getBankMask(uint channel) const;
	byte getOplChannel(uint channel) const;
	byte getOperatorOffset(uint channel, uint operatorIndex) const;
	const byte *getPatch(uint patchIndex) const;
	byte getPanningAttenuation(byte panning) const;
	uint16 getFrequencyNumber(byte semitone) const;
	void writeRegister(byte banks, byte reg, byte value);
	byte getCachedRegister(byte banks, byte reg) const;

	void initializePas16();
	void shutdownPas16();
	void resetDriver();
	int command0();
	int command1();
	int command2();
	int command3();
	int command4();
	int command5();
	int command6();
	int command7();
	int command8();

	void requestStop(uint firstChannel, uint endChannel);
	void setCurrentSequence(uint firstChannel, uint endChannel,
							uint16 sequenceOffset);
	void loadChannel(uint channel, uint16 sequenceOffset);
	void playSound(uint16 sequenceOffset);
	void playMusicAny(uint16 sequenceOffset);
	bool isSoundActive(uint16 sequenceOffset) const;

	void onTimer();
	int serviceUpdate();
	void serviceNoise();
	void update();
	void updateChannel(uint channelIndex);
	bool isOpcodeDataValid(uint16 position, uint32 length) const;
	byte readOpcodeByte(uint16 position, uint16 delta) const;
	uint16 readOpcodeWord(uint16 position, uint16 delta) const;
	bool isScriptVariableValid(byte index) const;
	bool transferOpcode(Channel &channel, uint16 position, bool take,
		bool isCall);
	bool executeOpcode(uint channelIndex, byte opcode, bool &levelsDirty);
	void finishChannel(uint channelIndex);
	void checkPendingStops();

	void loadPatch(uint channelIndex, byte patchIndex);
	void programOperator(byte banks, uint channelIndex, uint operatorIndex,
						 const byte *operatorData);
	void updatePanning(uint channelIndex);
	void updateChannelLevels(uint channelIndex);
	void updateChannelFrequency(uint channelIndex, bool keyOn);
	void updatePitchBend(uint channelIndex);
	void keyOff(uint channelIndex);
	void setNoiseFrequency(uint channelIndex, int frequency);
	void resultCheck();

	uint16 nextRandom();

	/** Implement only callbacks proved reachable from this overlay's streams. */
	virtual bool callFunction(uint16 targetOffset, Channel &channel) = 0;

public:
	static bool validateFile(const PSoundDriverData &driverData,
							 const char *first8192Md5, Common::String *reason = nullptr);
	bool isReady() const { return _opl != nullptr; }

	int stop() override;
	int poll() override;
	void noise() override;
	void setVolume(int volume) override;
};

} // namespace Sound
} // namespace Phantom
} // namespace MADS

#endif // MADS_PHANTOM_SOUND_PSOUND_H
