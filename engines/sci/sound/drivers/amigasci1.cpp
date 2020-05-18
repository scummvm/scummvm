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

#include "audio/softsynth/emumidi.h"
#include "sci/sound/drivers/mididriver.h"
#include "sci/resource.h"

#include "common/debug-channels.h"
#include "common/file.h"
#include "common/frac.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "audio/mods/paula.h"

namespace Sci {

static const byte envSpeedToStep[32] = {
	0x40, 0x32, 0x24, 0x18, 0x14, 0x0f, 0x0d, 0x0b, 0x09, 0x08, 0x07, 0x06, 0x05, 0x0a, 0x04, 0x03,
	0x05, 0x02, 0x03, 0x0b, 0x05, 0x09, 0x09, 0x01, 0x02, 0x03, 0x07, 0x05, 0x04, 0x03, 0x03, 0x02
};

static const byte envSpeedToSkip[32] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x01, 0x00, 0x01, 0x07, 0x02, 0x05, 0x07, 0x00, 0x01, 0x02, 0x08, 0x08, 0x08, 0x09, 0x0e, 0x0b
};

// This table has 257 elements in SCI1 EGA and 255 elements in SCI1
static const byte noteToOctave[257] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,
	0x0a, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
	0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d,
	0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e,
	0x0e, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
	0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
	0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
	0x15
};

static const int16 pitchToSemitone[97] = {
	-12, -12, -12, -12, -11, -11, -11, -11,
	-10, -10, -10, -10, -9, -9, -9, -9,
	-8, -8, -8, -8, -7, -7, -7, -7,
	-6, -6, -6, -6, -5, -5, -5, -5,
	-4, -4, -4, -4, -3, -3, -3, -3,
	-2, -2, -2, -2, -1, -1, -1, -1,
	0, 0, 0, 0, 1, 1, 1, 1,
	2, 2, 2, 2, 3, 3, 3, 3,
	4, 4, 4, 4, 5, 5, 5, 5,
	6, 6, 6, 6, 7, 7, 7, 7,
	8, 8, 8, 8, 9, 9, 9, 9,
	10, 10, 10, 10, 11, 11, 11, 11,
	12
};

static const uint16 pitchToSemiRem[97] = {
	0x0000, 0x0004, 0x0008, 0x000c, 0x0000, 0x0004, 0x0008, 0x000c,
	0x0000, 0x0004, 0x0008, 0x000c, 0x0000, 0x0004, 0x0008, 0x000c,
	0x0000, 0x0004, 0x0008, 0x000c, 0x0000, 0x0004, 0x0008, 0x000c,
	0x0000, 0x0004, 0x0008, 0x000c, 0x0000, 0x0004, 0x0008, 0x000c,
	0x0000, 0x0004, 0x0008, 0x000c, 0x0000, 0x0004, 0x0008, 0x000c,
	0x0000, 0x0004, 0x0008, 0x000c, 0x0000, 0x0004, 0x0008, 0x000c,
	0x0000, 0x0004, 0x0008, 0x000c, 0x0000, 0x0004, 0x0008, 0x000c,
	0x0000, 0x0004, 0x0008, 0x000c, 0x0000, 0x0004, 0x0008, 0x000c,
	0x0000, 0x0004, 0x0008, 0x000c, 0x0000, 0x0004, 0x0008, 0x000c,
	0x0000, 0x0004, 0x0008, 0x000c, 0x0000, 0x0004, 0x0008, 0x000c,
	0x0000, 0x0004, 0x0008, 0x000c, 0x0000, 0x0004, 0x0008, 0x000c,
	0x0000, 0x0004, 0x0008, 0x000c, 0x0000, 0x0004, 0x0008, 0x000c,
	0x0000
};

static const byte velocityMap[64] = {
	0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
	0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,
	0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2a,
	0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x34, 0x35, 0x37, 0x39, 0x3a, 0x3c, 0x3e, 0x40
};

static const byte velocityMapSci1EGA[64] = {
	0x01, 0x04, 0x07, 0x0a, 0x0c, 0x0f, 0x11, 0x15, 0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x21, 0x22, 0x23,
	0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x30, 0x31, 0x31,
	0x32, 0x32, 0x33, 0x33, 0x34, 0x34, 0x35, 0x35, 0x36, 0x36, 0x37, 0x37, 0x38, 0x38, 0x38, 0x39,
	0x39, 0x39, 0x3a, 0x3a, 0x3a, 0x3b, 0x3b, 0x3b, 0x3c, 0x3d, 0x3e, 0x3e, 0x3f, 0x3f, 0x40, 0x40
};

class MidiDriver_AmigaSci1 : public MidiDriver, public Audio::Paula {
public:
	enum kEnvState {
		kEnvStateAttack,
		kEnvStateDecay,
		kEnvStateSustain,
		kEnvStateRelease
	};

	MidiDriver_AmigaSci1(Audio::Mixer *mixer);

	// MidiDriver
	int open();
	void close();
	void send(uint32 b);
	MidiChannel *allocateChannel() { return NULL; }
	MidiChannel *getPercussionChannel() { return NULL; }
	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc);
	uint32 getBaseTempo() { return 1000000 / _baseFreq; }
	bool isOpen() const { return _isOpen; }
	uint32 property(int prop, uint32 param) { return 0; }

	// Audio::Paula
	void interrupt();

	void setVolume(byte volume) { _masterVolume = volume; }
	void playSwitch(bool play) { _playSwitch = play; }

private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _mixerSoundHandle;
	Common::TimerManager::TimerProc _timerProc;
	void *_timerParam;
	bool _playSwitch;
	bool _isOpen;
	int _baseFreq;
	uint _masterVolume;

	bool loadPatches(Common::SeekableReadStream &file);
	void convertSamples();
	void voiceOn(byte voice, int8 note, int8 velocity);
	void voiceOff(byte voice);
	int8 findVoice(int8 channel);
	void voiceMapping(int8 channel, byte voices);
	void assignVoices(int8 channel, byte voices);
	void releaseVoices(int8 channel, byte voices);
	void donateVoices();
	uint16 calcPeriod(int8 note, byte voice, byte *noteRange, byte *wave, byte *periodTable);
	void setVoicePeriod(byte voice, uint16 period);
	bool calcVoicePeriod(byte voice);
	void noteOn(int8 channel, int8 note, int8 velocity);
	void noteOff(int8 channel, int8 note);
	void changePatch(int8 channel, int8 patch);
	void holdPedal(int8 channel, int8 pedal);
	void setPitchWheel(int8 channel, uint16 pitch);
	void calcMixVelocity(int8 voice);
	void processEnvelope(int8 voice);
	void initVoice(byte voice);
	void deinitVoice(byte voice);

	bool _isSci1EGA;
	int8 _voiceChannel[NUM_VOICES];
	bool _voiceReleased[NUM_VOICES];
	bool _voiceSustained[NUM_VOICES];
	int8 _voiceEnvCurVel[NUM_VOICES];
	kEnvState _voiceEnvState[NUM_VOICES];
	byte _voiceEnvCntDown[NUM_VOICES];
	uint16 _voiceTicks[NUM_VOICES];
	uint16 _voiceReleaseTicks[NUM_VOICES];
	byte *_voicePatch[NUM_VOICES];
	byte *_voiceNoteRange[NUM_VOICES];
	byte *_voiceWave[NUM_VOICES];
	byte *_voicePeriodTable[NUM_VOICES];
	byte _voiceVelocity[NUM_VOICES];
	int8 _voiceNote[NUM_VOICES];
	byte _voiceMixVelocity[NUM_VOICES];

	int8 _chanPatch[MIDI_CHANNELS];
	uint16 _chanPitch[MIDI_CHANNELS];
	bool _chanHold[MIDI_CHANNELS];
	int8 _chanVolume[MIDI_CHANNELS];
	int8 _chanLastVoice[MIDI_CHANNELS];
	byte _chanExtraVoices[MIDI_CHANNELS];

	byte *_patch;
};

#define PATCH_NAME 0
#define PATCH_NOTE_RANGE 10

#define WAVE_NAME 0
#define WAVE_IS_SIGNED 8
#define WAVE_PHASE1_START (_isSci1EGA ? 8 : 10)
#define WAVE_PHASE1_END (_isSci1EGA ? 10 : 12)
#define WAVE_PHASE2_START (_isSci1EGA ? 12 : 14)
#define WAVE_PHASE2_END (_isSci1EGA ? 14 : 16)
#define WAVE_NATIVE_NOTE (_isSci1EGA ? 16 : 18)
#define WAVE_STEP_TABLE_OFFSET (_isSci1EGA ? 18 : 20)
#define WAVE_SIZEOF (_isSci1EGA ? 22 : 24)

#define NOTE_RANGE_SIZE 20
#define NOTE_RANGE_START_NOTE 0
#define NOTE_RANGE_END_NOTE 2
#define NOTE_RANGE_SAMPLE_OFFSET 4
#define NOTE_RANGE_TRANSPOSE 8
#define NOTE_RANGE_ATTACK_SPEED 10
#define NOTE_RANGE_ATTACK_TARGET 11
#define NOTE_RANGE_DECAY_SPEED 12
#define NOTE_RANGE_DECAY_TARGET 13
#define NOTE_RANGE_RELEASE_SPEED 14
#define NOTE_RANGE_FIXED_NOTE 16
#define NOTE_RANGE_LOOP 18

#define INTERRUPT_FREQ 60

MidiDriver_AmigaSci1::MidiDriver_AmigaSci1(Audio::Mixer *mixer) :
	Audio::Paula(true, mixer->getOutputRate(), mixer->getOutputRate() / INTERRUPT_FREQ),
	_mixer(mixer),
	_isSci1EGA(false),
	_timerProc(nullptr),
	_timerParam(nullptr),
	_playSwitch(true),
	_isOpen(false),
	_baseFreq(INTERRUPT_FREQ),
	_masterVolume(15),
	_patch(0) {

	for (uint i = 0; i < NUM_VOICES; ++i) {
		_voiceChannel[i] = -1;
		_voiceReleased[i] = false;
		_voiceSustained[i] = false;
		_voiceEnvCurVel[i] = 0;
		_voiceEnvState[i] = kEnvStateAttack;
		_voiceEnvCntDown[i] = 0;
		_voiceTicks[i] = 0;
		_voiceReleaseTicks[i] = 0;
		_voicePatch[i] = 0;
		_voiceNoteRange[i] = 0;
		_voiceWave[i] = 0;
		_voicePeriodTable[i] = 0;
		_voiceVelocity[i] = 0;
		_voiceNote[i] = -1;
		_voiceMixVelocity[i] = 0;
	}

	for (uint i = 0; i < MIDI_CHANNELS; ++i) {
		_chanPatch[i] = 0;
		_chanPitch[i] = 0x2000;
		_chanHold[i] = false;
		_chanVolume[i] = 63;
		_chanLastVoice[i] = 0;
		_chanExtraVoices[i] = 0;
	}
}

int MidiDriver_AmigaSci1::open() {
	Resource *patch = g_sci->getResMan()->findResource(ResourceId(kResourceTypePatch, 9), false);
	uint offset = 0;

	if (!patch) {
		patch = g_sci->getResMan()->findResource(ResourceId(kResourceTypePatch, 5), false);
		_isSci1EGA = true;
		// SCI1 early has an extra dword at the beginning of the patch, skip it
		offset = 4;
	}

	if (!patch || offset > patch->size()) {
		warning("Could not open patch for Amiga SCI1 sound driver");
		return Common::kUnknownError;
	}

	const size_t size = patch->size() - offset;

	Common::MemoryReadStream stream(patch->toStream());
	_patch = new byte[size];
	stream.seek(offset);
	if (stream.read(_patch, size) < size) {
		warning("Failed to read patch for Amiga SCI1 sound driver");
		delete _patch;
		return Common::kUnknownError;
	}

	if (!_isSci1EGA)
		convertSamples();

	startPaula();
	// Enable reverse stereo to counteract Audio::Paula's reverse stereo
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO, false, true);
	_isOpen = true;

	return Common::kNoError;
}

void MidiDriver_AmigaSci1::close() {
	_mixer->stopHandle(_mixerSoundHandle);
	stopPaula();
	_isOpen = false;
	if (_patch)
		delete[] _patch;
}

void MidiDriver_AmigaSci1::convertSamples() {
	// This will change the original data returned by the resman
	// It would probably be better to copy the patch
	for (uint patchId = 0; patchId < 128; ++patchId) {
		uint32 offset = READ_BE_UINT32(_patch + patchId * 4);

		if (offset == 0)
			continue;

		byte *patch = _patch + offset;
		byte *noteRange = patch + PATCH_NOTE_RANGE;

		while (1) {
			int16 startNote = READ_BE_UINT16(noteRange + NOTE_RANGE_START_NOTE);

			if (startNote == -1)
				break;

			byte *wave = _patch + READ_BE_UINT32(noteRange + NOTE_RANGE_SAMPLE_OFFSET);

			if (READ_BE_UINT16(wave + WAVE_IS_SIGNED) == 0) {
				WRITE_BE_UINT16(wave + WAVE_IS_SIGNED, -1);

				int endOffset = READ_BE_UINT16(wave + WAVE_PHASE1_END);
				endOffset += 224; // Additional samples are present to facilitate easy looping

				// The original code uses a signed 16-bit type here, while some samples
				// exceed INT_MAX in size. In this case, it will change one "random" byte
				// in memory and then stop converting. We simulate this behaviour here, minus
				// the memory corruption.
				// The "maincrnh" instrument in Castle of Dr. Brain has an incorrect signedness
				// flag, but is not actually converted because of its size.
				if (endOffset <= 0x8000) {
					byte *samples = wave + WAVE_SIZEOF;

					do {
						samples[endOffset] -= 0x80;
					} while (--endOffset >= 0);
				}
			}

			noteRange += NOTE_RANGE_SIZE;
		}
	}
}

void MidiDriver_AmigaSci1::processEnvelope(int8 voice) {
	byte attackTarget = _voiceNoteRange[voice][NOTE_RANGE_ATTACK_TARGET];
	byte decayTarget = _voiceNoteRange[voice][NOTE_RANGE_DECAY_TARGET];

	if (READ_BE_UINT16(_voiceNoteRange[voice] + NOTE_RANGE_LOOP) != 0) {
		_voiceEnvCurVel[voice] = attackTarget;
		return;
	}

	if (_voiceReleased[voice])
		_voiceEnvState[voice] = kEnvStateRelease;

	switch(_voiceEnvState[voice]) {
	case kEnvStateAttack: {
		if (_voiceEnvCntDown[voice] != 0) {
			--_voiceEnvCntDown[voice];
			return;
		}
		byte attackSpeed = _voiceNoteRange[voice][NOTE_RANGE_ATTACK_SPEED];
		_voiceEnvCntDown[voice] = envSpeedToSkip[attackSpeed];
		_voiceEnvCurVel[voice] += envSpeedToStep[attackSpeed];
		if (attackTarget <= _voiceEnvCurVel[voice]) {
			_voiceEnvCurVel[voice] = attackTarget;
			_voiceEnvState[voice] = kEnvStateDecay;
		}
		break;
	}
	case kEnvStateDecay: {
		if (_voiceEnvCntDown[voice] != 0) {
			--_voiceEnvCntDown[voice];
			return;
		}
		byte decaySpeed = _voiceNoteRange[voice][NOTE_RANGE_DECAY_SPEED];
		_voiceEnvCntDown[voice] = envSpeedToSkip[decaySpeed];
		_voiceEnvCurVel[voice] -= envSpeedToStep[decaySpeed];
		if (decayTarget >= _voiceEnvCurVel[voice]) {
			_voiceEnvCurVel[voice] = decayTarget;
			_voiceEnvState[voice] = kEnvStateSustain;
		}
		break;
	}
	case kEnvStateSustain:
		_voiceEnvCurVel[voice] = decayTarget;
		break;
	case kEnvStateRelease: {
		if (_voiceEnvCntDown[voice] != 0) {
			--_voiceEnvCntDown[voice];
			return;
		}
		byte releaseSpeed = _voiceNoteRange[voice][NOTE_RANGE_RELEASE_SPEED];
		_voiceEnvCntDown[voice] = envSpeedToSkip[releaseSpeed];
		_voiceEnvCurVel[voice] -= envSpeedToStep[releaseSpeed];
		if (_voiceEnvCurVel[voice] <= 0)
			voiceOff(voice);
	}
	}
}

void MidiDriver_AmigaSci1::calcMixVelocity(int8 voice) {
	byte chanVol = _chanVolume[_voiceChannel[voice]];
	byte voiceVelocity = _voiceVelocity[voice];

	if (chanVol != 0) {
		if (voiceVelocity != 0) {
			voiceVelocity = voiceVelocity * chanVol / 63;
			if (_voiceEnvCurVel[voice] != 0) {
				voiceVelocity = voiceVelocity * _voiceEnvCurVel[voice] / 63;
				if (_masterVolume != 0) {
					voiceVelocity = voiceVelocity * (_masterVolume << 2) / 63;
					if (voiceVelocity == 0)
						++voiceVelocity;
				} else {
					voiceVelocity = 0;
				}
			} else {
				voiceVelocity = 0;
			}
		}
	} else {
		voiceVelocity = 0;
	}

	if (!_playSwitch)
		voiceVelocity = 0;

	// _reverb = voiceVelocity (??)
	setChannelVolume(voice, voiceVelocity);
}

void MidiDriver_AmigaSci1::interrupt() {
	// In the original driver, the interrupt handlers for each voice
	// call voiceOff when non-looping samples are finished.
	for (uint i = 0; i < NUM_VOICES; ++i) {
		if (_voiceNote[i] != -1) {
			if (READ_BE_UINT16(_voiceNoteRange[i] + NOTE_RANGE_LOOP) != 0) {
				if (getChannelDmaCount(i) > 0) {
					voiceOff(i);
				}
			}
		}
	}

	for (uint i = 0; i < NUM_VOICES; ++i) {
		if (_voiceNote[i] != -1) {
			++_voiceTicks[i];
			if (_voiceReleased[i])
				++_voiceReleaseTicks[i];
			processEnvelope(i);
			calcMixVelocity(i);
		}
	}

	if (_timerProc)
		(*_timerProc)(_timerParam);
}

int8 MidiDriver_AmigaSci1::findVoice(int8 channel) {
	int8 voice = _chanLastVoice[channel];
	uint16 maxTicks = 0;
	int8 maxTicksVoice = -1;

	do {
		voice = (voice + 1) % NUM_VOICES;

		if (_voiceChannel[voice] == channel) {
			if (_voiceNote[voice] == -1) {
				_chanLastVoice[channel] = voice;
				return voice;
			}
			uint16 ticks;
			if (_voiceReleaseTicks[voice] != 0)
				ticks = _voiceReleaseTicks[voice] + 0x8000;
			else
				ticks = _voiceTicks[voice];

			if (ticks >= maxTicks) {
				maxTicks = ticks;
				maxTicksVoice = voice;
			}
		}
	} while (voice != _chanLastVoice[channel]);

	if (maxTicksVoice != -1) {
		_voiceSustained[voice] = false;
		voiceOff(maxTicksVoice);
		_chanLastVoice[channel] = maxTicksVoice;
		return maxTicksVoice;
	}

	return -1;
}

void MidiDriver_AmigaSci1::voiceMapping(int8 channel, byte voices) {
	int curVoices = 0;

	for (uint i = 0; i < NUM_VOICES; ++i)
		if (_voiceChannel[i] == channel)
			curVoices++;

	curVoices += _chanExtraVoices[channel];

	if (curVoices < voices)
		assignVoices(channel, voices - curVoices);
	else if (curVoices > voices) {
		releaseVoices(channel, curVoices - voices);
		donateVoices();
	}
}

void MidiDriver_AmigaSci1::assignVoices(int8 channel, byte voices) {
	for (int i = 0; i < NUM_VOICES; ++i)
		if (_voiceChannel[i] == -1) {
			_voiceChannel[i] = channel;

			if (_voiceNote[i] != -1)
				voiceOff(i);

			if (--voices == 0)
				break;
		}

	_chanExtraVoices[channel] += voices;
	// _chanPatch[channel] = _chanPatch[channel];
}

void MidiDriver_AmigaSci1::releaseVoices(int8 channel, byte voices) {
	// It would suffice to just have the 'else' clause
	if (voices == _chanExtraVoices[channel]) {
		_chanExtraVoices[channel] = 0;
		return;
	} else if (voices <= _chanExtraVoices[channel]) {
		_chanExtraVoices[channel] -= voices;
		return;
	}

	voices -= _chanExtraVoices[channel];
	_chanExtraVoices[channel] = 0;

	for (uint i = 0; i < NUM_VOICES; ++i) {
		if ((channel == _voiceChannel[i]) && (_voiceNote[i] == -1)) {
			_voiceChannel[i] = -1;
			if (--voices == 0)
				return;
		}
	}

	do {
		uint16 maxTicks = 0;
		int8 maxTicksVoice = 0;

		for (uint i = 0; i < NUM_VOICES; ++i) {
			if (channel == _voiceChannel[i]) {
				// The original code seems to be broken here. It reads a word value from
				// byte array _voiceSustained.
				uint16 ticks = _voiceReleaseTicks[i];

				if (ticks != 0)
					ticks += 0x8000;
				else
					ticks = _voiceTicks[i];

				if (ticks >= maxTicks) {
					maxTicks = ticks;
					maxTicksVoice = i;
				}
			}
		}

		// This could be removed, as voiceOff already does it
		_voiceSustained[maxTicksVoice] = false;

		voiceOff(maxTicksVoice);
		_voiceChannel[maxTicksVoice] = -1;
	} while (--voices != 0);
}

void MidiDriver_AmigaSci1::donateVoices() {
	int freeVoices = 0;

	for (uint i = 0; i < NUM_VOICES; ++i)
		if (_voiceChannel[i] == -1)
			freeVoices++;

	if (freeVoices == 0)
		return;

	for (uint i = 0; i < MIDI_CHANNELS; ++i) {
		if (_chanExtraVoices[i] != 0) {
			if (_chanExtraVoices[i] >= freeVoices) {
				_chanExtraVoices[i] -= freeVoices;
				assignVoices(i, freeVoices);
				return;
			} else {
				freeVoices -= _chanExtraVoices[i];
				byte extraVoices = _chanExtraVoices[i];
				_chanExtraVoices[i] = 0;
				assignVoices(i, extraVoices);
			}
		}
	}
}

void MidiDriver_AmigaSci1::initVoice(byte voice) {
	setChannelVolume(voice, 0);

	// The original driver uses double buffering to play the samples. We will instead
	// play the data directly. The original driver might be OB1 in end offsets and
	// loop sizes on occasion. Currently, this behavior isn't taken into account, and
	// the samples are played according to the meta data in the sound bank.
	int8 *samples = (int8 *)_voiceWave[voice] + WAVE_SIZEOF;
	uint16 phase1Start = READ_BE_UINT16(_voiceWave[voice] + WAVE_PHASE1_START);
	uint16 phase1End = READ_BE_UINT16(_voiceWave[voice] + WAVE_PHASE1_END);
	uint16 phase2Start = READ_BE_UINT16(_voiceWave[voice] + WAVE_PHASE2_START);
	uint16 phase2End = READ_BE_UINT16(_voiceWave[voice] + WAVE_PHASE2_END);
	uint16 loop = READ_BE_UINT16(_voiceNoteRange[voice] + NOTE_RANGE_LOOP);

	uint16 endOffset = phase2End;

	if (endOffset == 0)
		endOffset = phase1End;

	// Paula consumes one word at a time
	phase1Start &= 0xfffe;
	phase2Start &= 0xfffe;

	// If endOffset is odd, the sample byte at endOffset is played, otherwise it isn't
	endOffset = (endOffset + 1) & 0xfffe;

	int phase1Len = endOffset - phase1Start;
	int phase2Len = endOffset - phase2Start;

	// The original driver delays the voice start for two MIDI ticks, possibly
	// due to DMA requirements
	if (phase2End == 0 || loop != 0) {
		// Non-looping
		setChannelData(voice, samples + phase1Start, nullptr, phase1Len, 0);
	} else {
		// Looping
		setChannelData(voice, samples + phase1Start, samples + phase2Start, phase1Len, phase2Len);
	}
}

void MidiDriver_AmigaSci1::deinitVoice(byte voice) {
	clearVoice(voice);
}

void MidiDriver_AmigaSci1::voiceOn(byte voice, int8 note, int8 velocity) {
	_voiceReleased[voice] = false;
	_voiceEnvCurVel[voice] = 0;
	_voiceEnvState[voice] = kEnvStateAttack;
	_voiceEnvCntDown[voice] = 0;
	_voiceTicks[voice] = 0;
	_voiceReleaseTicks[voice] = 0;

	int8 patchId = _chanPatch[_voiceChannel[voice]];
	uint32 offset = READ_BE_UINT32(_patch + patchId * 4);

	if (offset == 0)
		return;

	byte *patch = _patch + offset;
	byte *noteRange = patch + PATCH_NOTE_RANGE;

	while (1) {
		int16 startNote = READ_BE_UINT16(noteRange + NOTE_RANGE_START_NOTE);

		if (startNote == -1)
			return;

		int16 endNote = READ_BE_UINT16(noteRange + NOTE_RANGE_END_NOTE);

		if (startNote <= note && note <= endNote)
			break;

		noteRange += NOTE_RANGE_SIZE;
	}

	byte *wave = _patch + READ_BE_UINT32(noteRange + NOTE_RANGE_SAMPLE_OFFSET);
	byte *periodTable = _patch + READ_BE_UINT32(wave + WAVE_STEP_TABLE_OFFSET) + 16;

	_voicePatch[voice] = patch;
	_voiceNoteRange[voice] = noteRange;
	_voiceWave[voice] = wave;
	_voicePeriodTable[voice] = periodTable;

	if (velocity != 0) {
		if (_isSci1EGA)
			velocity = velocityMapSci1EGA[velocity >> 1];
		else
			velocity = velocityMap[velocity >> 1];
	}

	_voiceVelocity[voice] = velocity;
	_voiceNote[voice] = note;

	if (!calcVoicePeriod(voice)) {
		_voiceNote[voice] = -1;
		return;
	}

	initVoice(voice);
}

void MidiDriver_AmigaSci1::voiceOff(byte voice) {
	deinitVoice(voice);
	_voiceVelocity[voice] = 0;
	_voiceNote[voice] = -1;
	_voiceSustained[voice] = false;
	_voiceReleased[voice] = false;
	_voiceEnvState[voice] = kEnvStateAttack;
	_voiceEnvCntDown[voice] = 0;
	_voiceTicks[voice] = 0;
	_voiceReleaseTicks[voice] = 0;
}

uint16 MidiDriver_AmigaSci1::calcPeriod(int8 note, byte voice, byte *noteRange, byte *wave, byte *periodTable) {
	uint16 noteAdj = note + 127 - READ_BE_UINT16(wave + WAVE_NATIVE_NOTE);
	byte channel = _voiceChannel[voice];
	uint16 pitch = _chanPitch[channel];
	pitch /= 170;
	noteAdj += pitchToSemitone[pitch];

	// SCI1 EGA is off by one note
	if (_isSci1EGA)
		++noteAdj;

	byte offset = pitchToSemiRem[pitch];
	int octave = noteToOctave[noteAdj];

	while (noteAdj >= 12)
		noteAdj -= 12;

	uint32 period = READ_BE_UINT32(periodTable + (noteAdj << 4) + offset);

	int16 transpose = READ_BE_UINT16(noteRange + NOTE_RANGE_TRANSPOSE);
	if (transpose > 0) {
		uint32 delta = period - READ_BE_UINT32(periodTable + (noteAdj << 4) + offset + 16);
		delta >>= 4;
		delta *= transpose;
		period -= delta;
	} else if (transpose < 0) {
		uint32 delta = READ_BE_UINT32(periodTable + (noteAdj << 4) + offset - 16) - period;
		delta >>= 4;
		delta *= -transpose;
		period += delta;
	}

	if (octave != 0)
		period >>= octave;

	if (period < 0x7c || period > 0xffff)
		return -1;

	return period;
}

void MidiDriver_AmigaSci1::setVoicePeriod(byte voice, uint16 period) {
	// Audio::Paula uses int16 instead of uint16?
	setChannelPeriod(voice, period);
}

bool MidiDriver_AmigaSci1::calcVoicePeriod(byte voice) {
	int8 note = _voiceNote[voice];
	// byte *patch = _voicePatch[voice];
	byte *noteRange = _voiceNoteRange[voice];
	byte *wave = _voiceWave[voice];
	byte *periodTable = _voicePeriodTable[voice];

	int16 fixedNote = READ_BE_UINT16(noteRange + NOTE_RANGE_FIXED_NOTE);
	if (fixedNote != -1)
		note = fixedNote;

	uint16 period = calcPeriod(note, voice, noteRange, wave, periodTable);
	if (period == 0xffff)
		return false;

	setVoicePeriod(voice, period);
	return true;
}

void MidiDriver_AmigaSci1::noteOn(int8 channel, int8 note, int8 velocity) {
	if (velocity == 0) {
		noteOff(channel, note);
		return;
	}

	for (uint i = 0; i < NUM_VOICES; ++i) {
		if (_voiceChannel[i] == channel && _voiceNote[i] == note) {
			_voiceSustained[i] = false;
			voiceOff(i);
			voiceOn(i, note, velocity);
			return;
		}
	}

	int8 voice = findVoice(channel);
	if (voice != -1)
		voiceOn(voice, note, velocity);
}

void MidiDriver_AmigaSci1::noteOff(int8 channel, int8 note) {
	for (uint i = 0; i < NUM_VOICES; ++i) {
		if (_voiceChannel[i] == channel && _voiceNote[i] == note) {
			if (_chanHold[channel])
				_voiceSustained[i] = true;
			else {
				_voiceReleased[i] = true;
				_voiceEnvCntDown[i] = 0;
			}
			return;
		}
	}
}

void MidiDriver_AmigaSci1::changePatch(int8 channel, int8 patch) {
	_chanPatch[channel] = patch;
}

void MidiDriver_AmigaSci1::holdPedal(int8 channel, int8 pedal) {
	_chanHold[channel] = pedal;

	if (pedal != 0)
		return;

	for (uint voice = 0; voice < NUM_VOICES; ++voice) {
		if (_voiceChannel[voice] == channel && _voiceSustained[voice]) {
			_voiceSustained[voice] = false;
			_voiceReleased[voice] = true;
		}
	}
}

void MidiDriver_AmigaSci1::setPitchWheel(int8 channel, uint16 pitch) {
	_chanPitch[channel] = pitch;

	for (int i = 0; i < NUM_VOICES; ++i)
		if (_voiceNote[i] != -1 && _voiceChannel[i] == channel)
			calcVoicePeriod(i);
}

void MidiDriver_AmigaSci1::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	_timerProc = timer_proc;
	_timerParam = timer_param;
}

void MidiDriver_AmigaSci1::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;
	byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;

	switch(command) {
	case 0x80:
		noteOff(channel, op1);
		break;
	case 0x90:
		noteOn(channel, op1, op2);
		break;
	case 0xb0:
		switch (op1) {
		// case 0x01 (mod wheel) is also kept track of in the original driver, but not used
		case 0x07:
			if (op2 != 0) {
				op2 >>= 1;
				if (op2 == 0)
					++op2;
			}
			_chanVolume[channel] = op2;
			break;
		case 0x40:
			holdPedal(channel, op2);
			break;
		case 0x4b:
			voiceMapping(channel, op2);
			break;
		case 0x7b:
			for (uint voice = 0; voice < NUM_VOICES; ++voice) {
				if (_voiceChannel[voice] == channel && _voiceNote[voice] != -1)
					voiceOff(voice);
			}
		}
		break;
	case 0xc0:
		changePatch(channel, op1);
		break;
	case 0xe0:
		setPitchWheel(channel, (op2 << 7) | op1);
		break;
	}
}

class MidiPlayer_AmigaSci1 : public MidiPlayer {
public:
	MidiPlayer_AmigaSci1(SciVersion version) : MidiPlayer(version) { _driver = new MidiDriver_AmigaSci1(g_system->getMixer()); }
	byte getPlayId() const;
	int getPolyphony() const { return MidiDriver_AmigaSci1::NUM_VOICES; }
	bool hasRhythmChannel() const { return false; }
	void setVolume(byte volume) { static_cast<MidiDriver_AmigaSci1 *>(_driver)->setVolume(volume); }
	void playSwitch(bool play) { static_cast<MidiDriver_AmigaSci1 *>(_driver)->playSwitch(play); }
};

MidiPlayer *MidiPlayer_AmigaSci1_create(SciVersion version) {
	return new MidiPlayer_AmigaSci1(version);
}

byte MidiPlayer_AmigaSci1::getPlayId() const {
	return 0x06;
}

} // End of namespace Sci
