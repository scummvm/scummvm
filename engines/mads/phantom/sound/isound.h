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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MADS_PHANTOM_SOUND_ISOUND_H
#define MADS_PHANTOM_SOUND_ISOUND_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "mads/core/pcspk_pit.h"
#include "mads/core/sound_manager.h"

namespace MADS {
namespace Phantom {
namespace Sound {

/** Native implementation of the Return of the Phantom IBM PC speaker VM. */
class ISound : public SoundDriver, public Audio::AudioStream {
public:
	enum {
		kPitClockHz = 1193182,
		kHostTimerDivisor = 0x07a8,
		kHostServiceDivider = 2,
		kSequenceServiceDivider = 5,
		kPCSpeakerSampleRate = 48000,
		kDefaultOutputVolume = 20,
		kMaxOperationsPerTick = 1024,
		kScriptVariableCount = 34
	};

	struct OverlaySpec {
		uint16 noteTableOffset;
		uint16 nullSequenceOffset;
		uint16 randomSeedOffset;
	};

protected:
	struct OverlayLayout {
		uint32 dataOffset;
		uint32 initializedDataSize;
		uint16 dataSegmentSize;
	};

	Audio::SoundHandle _speakerHandle;
	OverlaySpec _spec;
	bool _noiseEnabled;
	bool _updatesEnabled;
	bool _streamInvalid;
	int _masterVolume;
	int _outputRate;
	uint64 _hostTimerAccumulator;
	byte _sequenceServiceCountdown;
	PCSpeakerPITRenderer _pitRenderer;

	uint16 _frameCounter;
	uint16 _randomSeed;
	int16 _pollResult;
	int8 _resultState;

	byte _priority;
	uint16 _sequenceStart;
	uint16 _position;
	uint16 _innerLoopStart;
	uint16 _outerLoopStart;
	uint16 _restartOverride;
	uint16 _branchReturn;
	byte _innerLoopCount;
	byte _outerLoopCount;

	byte _note;
	byte _activeTicks;
	byte _releaseCounter;
	byte _releaseOverride;
	byte _gateOffset;
	byte _transpose;
	int8 _fineOffset;

	uint16 _noiseMask;
	uint16 _currentDivisor;
	uint16 _pitchStep;
	uint16 _directDivisor;

	byte _alternationReload;
	byte _alternationOffset;
	byte _alternationCounter;
	bool _alternationToggle;

	bool _sweepInitialized;
	uint16 _sweepUpper;
	uint16 _sweepLower;
	int16 _sweepDirection;

	byte _scriptVariables[kScriptVariableCount];
	uint16 _tempoShift;
	uint16 _tempoTarget;
	uint16 _tempoReload;
	uint16 _tempoScale;

	static bool readOverlayLayout(const Common::Path &filename,
		OverlayLayout &layout, Common::String *reason = nullptr);

	bool readByte(uint16 offset, byte &value);
	bool readWord(uint16 offset, uint16 &value);
	bool writeByte(uint16 offset, byte value);
	void invalidateStream(const char *reason, uint16 offset);
	bool readControlByte(uint16 delta, byte &value);
	bool readControlWord(uint16 delta, uint16 &value);
	bool isScriptVariableValid(byte index);
	bool transferControl(bool take, bool saveReturn);

	void resetDriver();
	void initializeDriver();
	int executeCommonCommand(int commandId);
	void playSequence(uint16 sequenceOffset, byte priority);

	void update();
	void timerTick();
	void noiseTick();
	void processSequenceTick();
	bool processControl(byte opcode);
	void processOrdinaryEvent();
	void processInnerLoop();
	void processOuterLoop();

	void updatePitch();
	void updateAlternation();
	bool calculateNoteDivisor(byte note, uint16 &divisor);
	uint16 nextRandom();
	void setResultState(int8 state);

	void outputDivisor(uint16 divisor);
	void startSpeaker();
	void stopSpeaker();
	byte outputVolume() const;

public:
	ISound(Audio::Mixer *mixer, const Common::Path &filename,
		const OverlaySpec &spec);
	~ISound() override;

	static bool isOverlaySupported(const Common::Path &filename,
		Common::String *reason = nullptr);

	int stop() override;
	int poll() override;
	void noise() override;
	void setVolume(int volume) override;

	int readBuffer(int16 *buffer, int numSamples) override;
	bool isStereo() const override { return false; }
	bool endOfData() const override { return false; }
	bool endOfStream() const override { return false; }
	int getRate() const override { return _outputRate; }
};

} // namespace Sound
} // namespace Phantom
} // namespace MADS

#endif
