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

#ifndef MADS_NEBULAR_SOUND_PSOUND_H
#define MADS_NEBULAR_SOUND_PSOUND_H

#include "mads/core/native_sound_timer.h"
#include "mads/core/sound_manager.h"

namespace OPL {
class OPL;
}

namespace MADS {
namespace RexNebular {
namespace Sound {

/** Offsets of the lookup tables embedded in one PSOUND data image. */
struct PSoundTableLayout {
	uint16 panning;
	uint16 frequency;
	uint16 bank;
	uint16 channel;
	uint16 operators;
};

/** Layout information for one exact PSOUND driver variant. */
struct PSoundDriverData {
	const char *filename;
	int dataOffset;
	int initializedDataSize;
	int totalDataSize;
	uint16 nullSequenceOffset;
	uint16 patchTableOffset;
	byte patchCount;
	byte musicChannelCount;
	PSoundTableLayout tables;
};

/** Common PSOUND driver implementation. */
class PSound : public SoundDriver {
public:
	enum {
		kChannelCount = 9,
		kMusicChannelCount = 6,
		kPatchSize = 64,
		kOpcodeBudgetPerTick = 256
	};

	enum RegisterBank {
		kFirstBank = 1,
		kSecondBank = 2,
		kBothBanks = kFirstBank | kSecondBank
	};

protected:
	/** Logical layout of the original 0x26-byte channel record. */
	struct Channel {
		byte activeCount;              // +00
		int8 pitchBend;                // +01
		int8 volumeFadeStep;           // +02
		int8 panningFadeStep;           // +03
		byte note;                     // +04
		byte patch;                    // +05
		byte volume;                   // +06
		byte noteOffset;               // +07
		byte keyOnDelay;               // +08
		byte volumeFadeCounter;        // +09
		byte volumeFadeReload;         // +0a
		byte panningFadeCounter;        // +0b
		byte panningFadeReload;         // +0c
		byte panning;                  // +0d
		int8 volumeOffset;             // +0e
		byte mode;                     // +0f
		byte operatorTotalLevel[4];    // +10..+13
		uint16 sequenceStart;          // +14
		uint16 position;               // +16
		uint16 innerLoopStart;         // +18
		uint16 outerLoopStart;         // +1a
		uint16 innerLoopCount;         // +1c
		uint16 outerLoopCount;         // +1e
		uint16 originalSequence;       // +20
		int8 transpose;                // +22
		int8 noteTranspose;            // +23
		byte pendingStop;              // +24
		int8 patchAttenuation;         // +25

		void reset();
		void load(uint16 sequenceOffset);
	};

	struct ChannelData {
		byte noiseMode;
		uint16 frequencyMask;
		uint16 frequencyBase;
		int16 frequencyStep;

		void reset();
	};

	OPL::OPL *_opl;
	byte _registerCache[2][256];
	NativeSoundTimer _hostTimer;
	Channel _channels[kChannelCount];
	ChannelData _channelData[kChannelCount];

	int _masterVolume;
	uint16 _randomSeed;
	uint16 _frameCounter;
	int16 _pollResult;
	int8 _resultFlag;
	uint16 _nullSequenceOffset;
	uint16 _patchTableOffset;
	PSoundTableLayout _tableLayout;
	byte _patchCount;
	byte _musicChannelCount;
	int _commandParam;
	bool _updatesEnabled;
	// Host-owned gate for native export 4; changed only by export-3 results.
	bool _noiseServiceEnabled;

	byte _noiseTicks[2];
	byte _savedNoiseTicks[2];
	byte _noiseChannel[2];
	uint16 _noiseMask[2];
	int32 _noiseBase[2];
	int16 _noiseStep[2];
	bool _noiseState;

	PSound(Audio::Mixer *mixer, const PSoundDriverData &driverData);
	~PSound() override;

	bool isDataRangeValid(uint32 offset, uint32 length) const;
	void requireDataRange(uint32 offset, uint32 length, const char *operation) const;
	const byte *getDataPointer(uint32 offset, uint32 length,
			const char *operation) const;
	byte *getDataPointer(uint32 offset, uint32 length, const char *operation);
	void validateDataLayout(const PSoundDriverData &driverData) const;

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
	void resetDriver();
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

	void loadChannel(uint channel, uint16 sequenceOffset);
	void playSound(uint16 sequenceOffset);
	void playSoundAny(uint16 sequenceOffset);
	bool isSoundActive(uint16 sequenceOffset) const;
	void requestStop(uint firstChannel, uint endChannel);
	void setCurrentSequence(uint firstChannel, uint endChannel,
			uint16 sequenceOffset);

	void onTimer();
	int serviceUpdate();
	void serviceNoise();
	void update();
	virtual void tickCallback() {
	}
	virtual byte getStopFadeReload() const {
		return 4;
	}
	void updateChannel(uint channelIndex);
	bool executeOpcode(uint channelIndex, byte opcode, bool &levelsDirty);
	void checkPendingStops();
	void finishChannel(uint channelIndex);

	void loadPatch(uint channelIndex, byte patchIndex);
	void programOperator(byte banks, uint channelIndex, uint operatorIndex,
			const byte *operatorData);
	void updatePanning(uint channelIndex);
	void updateChannelLevels(uint channelIndex);
	void updateChannelFrequency(uint channelIndex, bool keyOn);
	void updatePitchBend(uint channelIndex);
	void keyOff(uint channelIndex);
	void startNoise(uint channelIndex);
	void updateNoise();
	void setNoiseFrequency(uint channelIndex, int frequency);
	void resultCheck();

	uint16 nextRandom();
	byte scaledCommandParameter(int param) const;

public:
	static void validate(bool isDemo);
	bool isReady() const { return _opl != nullptr; }

	int stop() override;
	int poll() override;
	void noise() override;
	void setVolume(int volume) override;
};

} // namespace Sound
} // namespace RexNebular
} // namespace MADS

#endif // MADS_NEBULAR_SOUND_PSOUND_H
