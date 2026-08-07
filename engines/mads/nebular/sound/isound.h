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
 *
 */

#ifndef MADS_NEBULAR_SOUND_ISOUND_H
#define MADS_NEBULAR_SOUND_ISOUND_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "mads/core/pcspk_pit.h"
#include "mads/core/sound_manager.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

/**
 * Shared native implementation of the Rex Nebular IBM PC Speaker runtime.
 *
 * The original ISOUND files are relocatable DOS MZ driver overlays. They
 * contain an 11-entry driver descriptor, a data segment with a monophonic
 * sequence interpreter's state and streams, and PIT channel 2 divisor data.
 * ScummVM implements the interpreter natively and reads the original data
 * segment; it does not execute the 16-bit overlay code.
 */
class ISound : public SoundDriver, public Audio::AudioStream {
public:
	enum {
		kPitClockHz = 1193182,
		kHostTimerDivisor = 0x07a8,
		kHostServiceDivider = 2,
		kSequenceServiceDivider = 5,
		// Match the fixed 48-kHz rate used by DOSBox Staging's post-0.82.2
		// impulse model (b53ac15). Audio::Mixer handles device conversion.
		kPCSpeakerSampleRate = 48000,
		kDefaultOutputVolume = 20,
		kFrequencyTableOffset = 0x0114,
		kInitialNullSequenceOffset = 0x00f0,
		kMaxOperationsPerTick = 1024
	};

protected:
	struct OverlayLayout {
		uint32 dataOffset;
		uint32 initializedDataSize;
		uint16 dataSegmentSize;
	};

	Audio::SoundHandle _speakerHandle;
	bool _noiseEnabled;
	bool _updatesEnabled;
	int _masterVolume;
	int _outputRate;
	uint64 _hostTimerAccumulator;
	byte _sequenceServiceCountdown;
	PCSpeakerPITRenderer _pitRenderer;

	uint16 _frameCounter;
	uint16 _randomSeed;
	uint16 _commandParam;
	int16 _pollResult;
	int8 _resultState;

	byte _priority;
	uint16 _sequenceStart;
	uint16 _position;
	uint16 _innerLoopStart;
	uint16 _outerLoopStart;
	uint16 _restartOverride;
	byte _innerLoopCount;
	byte _outerLoopCount;

	byte _note;
	byte _activeTicks;
	byte _releaseCounter;
	byte _gateOffset;
	byte _transpose;
	int8 _fineOffset;

	uint16 _noiseMask;
	uint16 _currentDivisor;
	uint16 _pitchStep;
	uint16 _directDivisor;

	// F6 alternates between the current note and an offset note.
	byte _alternationReload;
	byte _alternationOffset;
	byte _alternationCounter;
	bool _alternationToggle;

	// Pitch-step values with high nibble 8 implement a bounded triangle sweep.
	bool _sweepInitialized;
	uint16 _sweepUpper;
	uint16 _sweepLower;
	int16 _sweepDirection;

	static OverlayLayout readOverlayLayout(const Common::Path &filename);

	ISound(Audio::Mixer *mixer, const Common::Path &filename, const OverlayLayout &layout);

	byte readSequenceByte(uint16 offset) const;
	uint16 readSequenceUint16(uint16 offset) const;
	void writeSequenceByte(uint16 offset, byte value);

	void resetDriver();
	void initializeDriver();
	void beginCommand(int param);
	int executeCommonCommand(int commandId);
	void playSequence(uint16 sequenceOffset, byte priority);

	void update();
	void timerTick();
	void noiseTick();
	void processSequenceTick();
	void processInnerLoop();
	void processOuterLoop();
	void processRestart();
	void processRandomMutation();
	void processOrdinaryEvent();

	void updatePitch();
	void updateAlternation();
	uint16 calculateNoteDivisor(byte note) const;

	uint16 nextRandom();
	void setResultState(int8 state);

	void outputDivisor(uint16 divisor);
	void startSpeaker();
	void stopSpeaker();
	byte outputVolume() const;
	int16 generateSample();

public:
	ISound(Audio::Mixer *mixer, const Common::Path &filename);
	~ISound() override;

	int stop() override;
	int poll() override;
	void noise() override;
	void setVolume(int volume) override;

	int readBuffer(int16 *buffer, int numSamples) override;
	bool isStereo() const override {
		return false;
	}
	bool endOfData() const override {
		return false;
	}
	bool endOfStream() const override {
		return false;
	}
	int getRate() const override {
		return _outputRate;
	}

	uint16 frameCounter() const {
		return _frameCounter;
	}
};

} // namespace Sound
} // namespace RexNebular
} // namespace MADS

#endif
