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

#include "common/file.h"
#include "common/frac.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "audio/mods/paula.h"

namespace Sci {

// FIXME: SQ3, LSL2 and HOYLE1 for Amiga don't seem to load any
// patches, even though patches are present. Later games do load
// patches, but include disabled patches with a 'd' appended to the
// filename, e.g. sound.010d. For SQ3, LSL2 and HOYLE1, we should
// probably disable patch loading. Maybe the original interpreter
// loads these disabled patches under some specific condition?

static const uint16 periodTable[525] = {
	0x3bb9, 0x3ade, 0x3a05, 0x3930, 0x385e, 0x378f, 0x36c3, 0x35fa,
	0x3534, 0x3471, 0x33b0, 0x32f3, 0x3238, 0x3180, 0x30ca, 0x3017,
	0x2f66, 0x2eb8, 0x2e0d, 0x2d64, 0x2cbd, 0x2c19, 0x2b77, 0x2ad7,
	0x2a3a, 0x299f, 0x2907, 0x2870, 0x27dc, 0x274a, 0x26b9, 0x262b,
	0x259f, 0x2515, 0x248d, 0x2406, 0x2382, 0x2300, 0x227f, 0x2201,
	0x2184, 0x2109, 0x2090, 0x2019, 0x1fa3, 0x1f2f, 0x1ebc, 0x1e4b,
	0x3bb9, 0x3ade, 0x3a05, 0x3930, 0x385e, 0x378f, 0x36c3, 0x35fa,
	0x3534, 0x3471, 0x33b0, 0x32f3, 0x3238, 0x3180, 0x30ca, 0x3017,
	0x2f66, 0x2eb8, 0x2e0d, 0x2d64, 0x2cbd, 0x2c19, 0x2b77, 0x2ad7,
	0x2a3a, 0x299f, 0x2907, 0x2870, 0x27dc, 0x274a, 0x26b9, 0x262b,
	0x259f, 0x2515, 0x248d, 0x2406, 0x2382, 0x2300, 0x227f, 0x2201,
	0x2184, 0x2109, 0x2090, 0x2019, 0x1fa3, 0x1f2f, 0x1ebc, 0x1e4b,
	0x3bb9, 0x3ade, 0x3a05, 0x3930, 0x385e, 0x378f, 0x36c3, 0x35fa,
	0x3534, 0x3471, 0x33b0, 0x32f3, 0x3238, 0x3180, 0x30ca, 0x3017,
	0x2f66, 0x2eb8, 0x2e0d, 0x2d64, 0x2cbd, 0x2c19, 0x2b77, 0x2ad7,
	0x2a3a, 0x299f, 0x2907, 0x2870, 0x27dc, 0x274a, 0x26b9, 0x262b,
	0x259f, 0x2515, 0x248d, 0x2406, 0x2382, 0x2300, 0x227f, 0x2201,
	0x2184, 0x2109, 0x2090, 0x2019, 0x1fa3, 0x1f2f, 0x1ebc, 0x1e4b,
	0x1ddc, 0x1d6e, 0x1d02, 0x1c98, 0x1c2f, 0x1bc8, 0x1b62, 0x1afd,
	0x1a9a, 0x1a38, 0x19d8, 0x1979, 0x191c, 0x18c0, 0x1865, 0x180b,
	0x17b3, 0x175c, 0x1706, 0x16b1, 0x165e, 0x160c, 0x15bb, 0x156c,
	0x151d, 0x14d0, 0x1483, 0x1438, 0x13ee, 0x13a5, 0x135c, 0x1315,
	0x12cf, 0x128a, 0x1246, 0x1203, 0x11c1, 0x1180, 0x1140, 0x1100,
	0x10c2, 0x1084, 0x1048, 0x100c, 0x0fd1, 0x0f97, 0x0f5e, 0x0f26,
	0x0eee, 0x0eb7, 0x0e81, 0x0e4c, 0x0e17, 0x0de3, 0x0db1, 0x0d7e,
	0x0d4d, 0x0d1c, 0x0cec, 0x0cbd, 0x0c8e, 0x0c60, 0x0c32, 0x0c05,
	0x0bd9, 0x0bae, 0x0b83, 0x0b59, 0x0b2f, 0x0b06, 0x0add, 0x0ab5,
	0x0a8e, 0x0a67, 0x0a41, 0x0a1c, 0x09f7, 0x09d2, 0x09ae, 0x098a,
	0x0967, 0x0945, 0x0923, 0x0901, 0x08e0, 0x08c0, 0x08a0, 0x0880,
	0x0861, 0x0842, 0x0824, 0x0806, 0x07e8, 0x07cb, 0x07af, 0x0793,
	0x0777, 0x075b, 0x0740, 0x0725, 0x070b, 0x06f1, 0x06d8, 0x06bf,
	0x06a6, 0x068e, 0x0676, 0x065e, 0x0647, 0x0630, 0x0619, 0x0602,
	0x05ec, 0x05d6, 0x05c1, 0x05ac, 0x0597, 0x0583, 0x056e, 0x055b,
	0x0547, 0x0534, 0x0520, 0x050e, 0x04fb, 0x04e9, 0x04d6, 0x04c5,
	0x04b3, 0x04a2, 0x0491, 0x0480, 0x0470, 0x0460, 0x0450, 0x0440,
	0x0430, 0x0421, 0x0412, 0x0403, 0x03f4, 0x03e5, 0x03d7, 0x03c9,
	0x03bb, 0x03ad, 0x03a0, 0x0392, 0x0385, 0x0378, 0x036c, 0x035f,
	0x0353, 0x0347, 0x033b, 0x032f, 0x0323, 0x0318, 0x030c, 0x0301,
	0x02f6, 0x02eb, 0x02e0, 0x02d6, 0x02cb, 0x02c1, 0x02b7, 0x02ad,
	0x02a3, 0x0299, 0x0290, 0x0286, 0x027d, 0x0274, 0x026b, 0x0262,
	0x0259, 0x0251, 0x0248, 0x0240, 0x0238, 0x0230, 0x0228, 0x0220,
	0x0218, 0x0210, 0x0209, 0x0201, 0x01fa, 0x01f3, 0x01eb, 0x01e4,
	0x01dd, 0x01d6, 0x01cf, 0x01c9, 0x01c2, 0x01bc, 0x01b5, 0x01af,
	0x01a9, 0x01a3, 0x019d, 0x0197, 0x0191, 0x018b, 0x0186, 0x0180,
	0x017b, 0x0175, 0x0170, 0x016a, 0x0165, 0x0160, 0x015b, 0x0156,
	0x0151, 0x014c, 0x0147, 0x0143, 0x013e, 0x0139, 0x0135, 0x0130,
	0x012c, 0x0128, 0x0124, 0x0120, 0x011c, 0x0118, 0x0114, 0x0110,
	0x010c, 0x0108, 0x0104, 0x0101, 0x00fd, 0x00f9, 0x00f5, 0x00f2,
	0x00ee, 0x00eb, 0x00e7, 0x00e4, 0x00e1, 0x00de, 0x00da, 0x00d7,
	0x00d4, 0x00d1, 0x00ce, 0x00cb, 0x00c8, 0x00c5, 0x00c2, 0x00c0,
	0x00bd, 0x00ba, 0x00b7, 0x00b5, 0x00b2, 0x00af, 0x00ad, 0x00aa,
	0x00a8, 0x00a6, 0x00a3, 0x00a1, 0x009f, 0x009d, 0x009a, 0x0098,
	0x0096, 0x0094, 0x0092, 0x0090, 0x008e, 0x008c, 0x008a, 0x0088,
	0x0086, 0x0084, 0x0082, 0x0080, 0x007e, 0x0000, 0x0000, 0x0000,
	// Last elements are 0x0000 to indicate out of Paula frequency range

	// Early drivers contain two more octaves, transposed. The
	// 0x0000 above will never be accessed in "early driver mode",
	// due to missing pitch bend support.
	0x01dd, 0x01d6, 0x01cf, 0x01c9, 0x01c2, 0x01bc, 0x01b5, 0x01af,
	0x01a9, 0x01a3, 0x019d, 0x0197, 0x0191, 0x018b, 0x0186, 0x0180,
	0x017b, 0x0175, 0x0170, 0x016a, 0x0165, 0x0160, 0x015b, 0x0156,
	0x0151, 0x014c, 0x0147, 0x0143, 0x013e, 0x0139, 0x0135, 0x0130,
	0x012c, 0x0128, 0x0124, 0x0120, 0x011c, 0x0118, 0x0114, 0x0110,
	0x010c, 0x0108, 0x0104, 0x0101, 0x00fd, 0x00f9, 0x00f5, 0x00f2,
	0x00ee, 0x00eb, 0x00e7, 0x00e4, 0x00e1, 0x00de, 0x00da, 0x00d7,
	0x00d4, 0x00d1, 0x00ce, 0x00cb, 0x00c8, 0x00c5, 0x00c2, 0x00c0,
	0x00bd, 0x00ba, 0x00b7, 0x00b5, 0x00b2, 0x00af, 0x00ad, 0x00aa,
	0x00a8, 0x00a6, 0x00a3, 0x00a1, 0x009f, 0x009d, 0x009a, 0x0098,
	0x0096, 0x0094, 0x0092, 0x0090, 0x008e, 0x008c, 0x008a, 0x0088,
	0x0086, 0x0084, 0x0082, 0x0080, 0x007e
};

static const int8 silence[2] = { 0, 0 };

class MidiDriver_AmigaSci0 : public MidiDriver, public Audio::Paula {
public:
	MidiDriver_AmigaSci0(Audio::Mixer *mixer);
	virtual ~MidiDriver_AmigaSci0();

	// MidiDriver
	int open();
	void close();
	void initTrack(SciSpan<const byte> &);
	void send(uint32 b);
	MidiChannel *allocateChannel() { return NULL; }
	MidiChannel *getPercussionChannel() { return NULL; }
	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc);
	uint32 getBaseTempo() { return 1000000 / _baseFreq; }
	bool isOpen() const { return _isOpen; }

	// Audio::Paula
	void interrupt();

	void setVolume(byte volume);
	void playSwitch(bool play) { }

private:
	struct Instrument {
		uint16 flags;
		int16 seg1Size;
		uint32 seg2Offset;
		int16 seg2Size;
		uint32 seg3Offset;
		int16 seg3Size;
		int8 *samples;
		int8 transpose;
		byte envelope[12];
		byte name[31];
	};

	struct {
		char name[30];
		uint16 instrumentCount;
		Instrument *instrument[128];
		uint16 patchNr[128];
	} _bank;

	struct VoiceState {
		const Instrument *instrument;
		uint16 period;
		byte velocity;
		byte loop;
	};

	struct Voice {
		byte loop;
		const int8 *seg1, *seg2;
		int16 seg1Size, seg2Size;
		uint16 period, volume;
	};

	struct {
		byte state[NUM_VOICES];
		byte countDown[NUM_VOICES];
		byte length[4][NUM_VOICES];
		int8 velocity[4][NUM_VOICES];
		int8 delta[4][NUM_VOICES];
	} _envelopeState;

	byte _startVoice;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _mixerSoundHandle;
	Common::TimerManager::TimerProc _timerProc;
	void *_timerParam;
	bool _isOpen;
	int _baseFreq;
	byte _masterVolume;
	bool _playSwitch;
	bool _isEarlyDriver;

	VoiceState _voiceState[NUM_VOICES];
	Voice _voice[NUM_VOICES];
	byte _voicePatch[NUM_VOICES];
	byte _voiceVelocity[NUM_VOICES]; // MIDI velocity 0-127
	byte _voiceVolume[NUM_VOICES]; // Amiga volume 0-63
	int8 _voicePitchWheelSign[NUM_VOICES];
	byte _voicePitchWheelOffset[NUM_VOICES];
	int8 _chanVoice[MIDI_CHANNELS];
	int8 _voiceNote[NUM_VOICES];

	bool readInstruments();
	void startVoice(int8 voice);
	void stopVoice(int8 voice);
	void setupVoice(int8 voice);
	void startVoices();
	void stopVoices();
	void doEnvelopes();
	void noteOn(int8 voice, int8 note, int8 velocity);
	void noteOff(int8 voice, int8 note);
	bool voiceOn(int8 voice, int8 note, bool newNote);
	void pitchWheel(int8 voice, int16 pitch);
};

#define MODE_LOOPING (1 << 0)
#define MODE_PITCH_CHANGES (1 << 1)

MidiDriver_AmigaSci0::MidiDriver_AmigaSci0(Audio::Mixer *mixer) :
	Audio::Paula(true, mixer->getOutputRate(), mixer->getOutputRate() / 60),
	_startVoice(0),
	_mixer(mixer),
	_timerProc(nullptr),
	_timerParam(nullptr),
	_isOpen(false),
	_baseFreq(60),
	_masterVolume(0),
	_playSwitch(true) {

	memset(_voiceState, 0, sizeof(_voiceState));
	memset(_voice, 0, sizeof(_voice));
	memset(_voicePatch, 0, sizeof(_voicePatch));
	memset(_voiceVelocity, 0, sizeof(_voiceVelocity));
	memset(_voiceVolume, 0, sizeof(_voiceVolume));
	memset(_voicePitchWheelSign, 0, sizeof(_voicePitchWheelSign));
	memset(_voicePitchWheelOffset, 0, sizeof(_voicePitchWheelOffset));
	memset(_chanVoice, 0, sizeof(_chanVoice));
	memset(_voiceNote, 0, sizeof(_voiceNote));
	memset(&_envelopeState, 0, sizeof(_envelopeState));
	memset(&_bank, 0, sizeof(_bank));

	switch (g_sci->getGameId()) {
	case GID_HOYLE1:
	case GID_LSL2:
	case GID_LSL3:
	case GID_SQ3:
	case GID_QFG1:
		_isEarlyDriver = true;
		break;
	default:
		_isEarlyDriver = false;
	}
}

MidiDriver_AmigaSci0::~MidiDriver_AmigaSci0() {
	for (uint i = 0; i < ARRAYSIZE(_bank.instrument); i++) {
		if (_bank.instrument[i]) {
			delete[] _bank.instrument[i]->samples;
			delete _bank.instrument[i];
		}
	}
}

int MidiDriver_AmigaSci0::open() {
	if (!readInstruments()) {
		warning("Could not read patch data from bank.001");
		return Common::kUnknownError;
	}

	startPaula();
	// Enable reverse stereo to counteract Audio::Paula's reverse stereo
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO, false, true);
	_isOpen = true;

	return Common::kNoError;
}

void MidiDriver_AmigaSci0::close() {
	_mixer->stopHandle(_mixerSoundHandle);
	stopPaula();
	_isOpen = false;
}

void MidiDriver_AmigaSci0::initTrack(SciSpan<const byte>& header) {
	if (!_isOpen)
		return;

	uint8 readPos = 0;
	const uint8 caps = header.getInt8At(readPos++);

	// We only implement the MIDI functionality here, samples are
	// handled by the generic sample code
	if (caps != 0)
		return;

	uint voices = 0;

	for (uint i = 0; i < 15; ++i) {
		readPos++;
		const uint8 flags = header.getInt8At(readPos++);

		if ((flags & 0x40) && (voices < NUM_VOICES))
			_chanVoice[i] = voices++;
		else
			_chanVoice[i] = -1;
	}

	_chanVoice[15] = -1;

	for (uint i = 0; i < NUM_VOICES; ++i) {
		_voiceVelocity[i] = 0;
		_voiceNote[i] = -1;
		_voicePitchWheelSign[i] = 0;
		_voicePitchWheelOffset[i] = 0;
	}
}

void MidiDriver_AmigaSci0::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	_timerProc = timer_proc;
	_timerParam = timer_param;
}

void MidiDriver_AmigaSci0::interrupt() {
	if (_timerProc)
		(*_timerProc)(_timerParam);

	startVoices();
	doEnvelopes();
}

void MidiDriver_AmigaSci0::doEnvelopes() {
	for (uint voice = 0; voice < NUM_VOICES; ++voice) {
		byte state = _envelopeState.state[voice];

		if (state == 0 || state == 3)
			continue;

		if (state == 6) {
			stopVoice(voice);
			_envelopeState.state[voice] = 0;
			continue;
		}

		if (state > 3)
			state -= 2;
		else
			--state;

		if (_envelopeState.countDown[voice] == 0) {
			_envelopeState.countDown[voice] = _envelopeState.length[state][voice];
			int8 velocity = _envelopeState.velocity[state][voice];

			if (velocity <= 0) {
				stopVoice(voice);
				_envelopeState.state[voice] = 0;
				continue;
			}

			if (velocity > 63)
				velocity = 63;

			if (!_playSwitch)
				velocity = 0;

			// Early games ignore note velocity for envelope-enabled notes
			if (_isEarlyDriver)
				setChannelVolume(voice, velocity * _masterVolume >> 6);
			else
				setChannelVolume(voice, (velocity * _masterVolume >> 6) * _voiceVolume[voice] >> 6);

			int8 delta = _envelopeState.delta[state][voice];
			if (delta < 0) {
				_envelopeState.velocity[state][voice] -= delta;
				if (_envelopeState.velocity[state][voice] > _envelopeState.velocity[state + 1][voice])
					++_envelopeState.state[voice];
			} else {
				_envelopeState.velocity[state][voice] -= delta;
				if (_envelopeState.velocity[state][voice] < _envelopeState.velocity[state + 1][voice])
					++_envelopeState.state[voice];
			}
		}

		--_envelopeState.countDown[voice];
	}
}

void MidiDriver_AmigaSci0::startVoice(int8 voice) {
	setChannelData(voice, _voice[voice].seg1, _voice[voice].seg2, _voice[voice].seg1Size * 2, _voice[voice].seg2Size * 2);
	if (_playSwitch)
		setChannelVolume(voice, _masterVolume * _voice[voice].volume >> 6);
	setChannelPeriod(voice, _voice[voice].period);
}

void MidiDriver_AmigaSci0::stopVoice(int8 voice) {
	clearVoice(voice);
}

void MidiDriver_AmigaSci0::setupVoice(int8 voice) {
	// NOTE: PCJr mode not implemented
	const Instrument *ins = _voiceState[voice].instrument;

	_voice[voice].loop = _voiceState[voice].loop;
	_voice[voice].seg1 = _voice[voice].seg2 = ins->samples;
	_voice[voice].seg1Size = ins->seg1Size; 
	_voice[voice].seg2 += ins->seg2Offset & 0xfffe;
	_voice[voice].seg1Size = ins->seg1Size; 
	_voice[voice].seg2Size = ins->seg2Size;

	if (_voiceState[voice].loop == 0) {
		_voice[voice].seg1Size = _voice[voice].seg1Size + _voice[voice].seg2Size + ins->seg3Size;
		_voice[voice].seg2 = silence;
		_voice[voice].seg2Size = 1;
	}

	_voice[voice].period = _voiceState[voice].period;
	_voice[voice].volume = _voiceState[voice].velocity >> 1;
	_envelopeState.state[voice] = 0;
	_envelopeState.length[0][voice] = ins->envelope[0];
	if (_envelopeState.length[0][voice] != 0 && _voiceState[voice].loop) {
		_envelopeState.length[1][voice] = ins->envelope[1];
		_envelopeState.length[2][voice] = ins->envelope[2];
		_envelopeState.length[3][voice] = ins->envelope[3];
		_envelopeState.delta[0][voice] = ins->envelope[4];
		_envelopeState.delta[1][voice] = ins->envelope[5];
		_envelopeState.delta[2][voice] = ins->envelope[6];
		_envelopeState.delta[3][voice] = ins->envelope[7];
		_envelopeState.velocity[0][voice] = _voice[voice].volume;
		_envelopeState.velocity[1][voice] = ins->envelope[8];
		_envelopeState.velocity[2][voice] = ins->envelope[9];
		_envelopeState.velocity[3][voice] = ins->envelope[10];
		_envelopeState.countDown[voice] = 0;
		_envelopeState.state[voice] = 1;
	}
}

void MidiDriver_AmigaSci0::startVoices() {
	for (uint i = 0; i < NUM_VOICES; ++i)
		if (_startVoice & (1 << i)) {
			setupVoice(i);
			startVoice(i);
		}

	_startVoice = 0;
}

void MidiDriver_AmigaSci0::stopVoices() {
	// FIXME: Why is master volume temporarily set to 0 here?
	byte masterVolume = _masterVolume;
	_masterVolume = 0;

	for (uint i = 0; i < NUM_VOICES; ++i)
		stopVoice(i);

	_masterVolume = masterVolume;
}

bool MidiDriver_AmigaSci0::voiceOn(int8 voice, int8 note, bool newNote) {
	_voiceState[voice].instrument = _bank.instrument[_voicePatch[voice]];

	// Default to the first instrument in the bank
	if (!_voiceState[voice].instrument)
		_voiceState[voice].instrument = _bank.instrument[_bank.patchNr[0]];

	_voiceState[voice].velocity = _voiceVelocity[voice];
	_voiceVolume[voice] = _voiceVelocity[voice] >> 1;

	if (newNote) {
		if (_voiceState[voice].instrument->flags & MODE_LOOPING)
			_voiceState[voice].loop = 3;
		else
			_voiceState[voice].loop = 0;
	}

	// In the original the test here is flags <= 1
	if (!(_voiceState[voice].instrument->flags & MODE_PITCH_CHANGES))
		note = 101;

	int16 index = (note + _voiceState[voice].instrument->transpose) * 4;
	if (_voicePitchWheelSign[voice] != 0) {
		if (_voicePitchWheelSign[voice] > 0)
			index += _voicePitchWheelOffset[voice];
		else
			index -= _voicePitchWheelOffset[voice];

		if (index < 0 || index > 430)
			return false;
	}

	if (periodTable[index] == 0)
		return false;

	_voiceState[voice].period = periodTable[index];
	_voiceNote[voice] = note;
	return true;
}

void MidiDriver_AmigaSci0::noteOn(int8 voice, int8 note, int8 velocity) {
	_voiceVelocity[voice] = velocity;

	if (velocity == 0) {
		noteOff(voice, note);
		return;
	}

	if (voiceOn(voice, note, true)) {
		_startVoice |= (1 << voice);
		stopVoice(voice);
	}
}

void MidiDriver_AmigaSci0::noteOff(int8 voice, int8 note) {
	if (_voiceNote[voice] == note) {
		if (_envelopeState.state[voice] != 0)
			_envelopeState.state[voice] = 4;
	}
}

void MidiDriver_AmigaSci0::pitchWheel(int8 voice, int16 pitch) {
	if (pitch == 0x2000) {
		_voicePitchWheelSign[voice] = 0;
		_voicePitchWheelOffset[voice] = 0;
	} else {
		if (pitch >= 0x2000) {
			pitch -= 0x2000;
			_voicePitchWheelSign[voice] = 1;
		} else {
			pitch = 0x2000 - pitch;
			_voicePitchWheelSign[voice] = -1;
		}

		_voicePitchWheelOffset[voice] = pitch / 171;
	}

	if (_voiceNote[voice] != -1) {
		voiceOn(voice, _voiceNote[voice], false);
		_voice[voice].period = _voiceState[voice].period;
		setChannelPeriod(voice, _voice[voice].period);
	}
}

void MidiDriver_AmigaSci0::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;
	byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;

	int8 voice = _chanVoice[channel];

	if (voice == -1)
		return;

	switch(command) {
	case 0x80:
		noteOff(voice, op1);
		break;
	case 0x90:
		noteOn(voice, op1, op2);
		break;
	case 0xb0:
		// Not in original driver
		if (op1 == 0x7b)
			stopVoice(voice);
		break;
	case 0xc0:
		_voicePatch[voice] = op1;
		break;
	case 0xe0:
		if (!_isEarlyDriver)
			pitchWheel(voice, (op2 << 7) | op1);
		break;
	}
}

void MidiDriver_AmigaSci0::setVolume(byte volume) {
	// FIXME This doesn't seem to make sense, as volume should be 0..15
	if (volume > 64)
		volume = 64;
	_masterVolume = volume << 2;
}

bool MidiDriver_AmigaSci0::readInstruments() {
	Common::File file;

	if (!file.open("bank.001"))
		return false;

	file.read(_bank.name, 8);
	if (strcmp(_bank.name, "X0iUo123") != 0)
		return false;

	file.read(_bank.name, 30);
	_bank.instrumentCount = file.readUint16BE();

	for (uint i = 0; i < _bank.instrumentCount; ++i) {
		Instrument *ins = new Instrument();
		_bank.patchNr[i] = file.readUint16BE();
		_bank.instrument[_bank.patchNr[i]] = ins;

		file.read(ins->name, 30);
		ins->flags = file.readUint16BE();
		ins->transpose = file.readByte();
		ins->seg1Size = file.readSint16BE();
		ins->seg2Offset = file.readUint32BE();
		ins->seg2Size = file.readSint16BE();
		ins->seg3Offset = file.readUint32BE();
		ins->seg3Size = file.readSint16BE();
		file.read(ins->envelope, 12);

		int32 sampleSize = ins->seg1Size + ins->seg2Size + ins->seg3Size;
		sampleSize <<= 1;
		ins->samples = new int8[sampleSize];
		file.read(ins->samples, sampleSize);
	}

	return true;
}

class MidiPlayer_AmigaSci0 : public MidiPlayer {
public:
	MidiPlayer_AmigaSci0(SciVersion version) : MidiPlayer(version) { _driver = new MidiDriver_AmigaSci0(g_system->getMixer()); }
	~MidiPlayer_AmigaSci0() {
		delete _driver; 
	}

	byte getPlayId() const { return 0x40; }
	int getPolyphony() const { return MidiDriver_AmigaSci0::NUM_VOICES; }
	bool hasRhythmChannel() const { return false; }
	void setVolume(byte volume) { static_cast<MidiDriver_AmigaSci0 *>(_driver)->setVolume(volume); }
	void playSwitch(bool play) { static_cast<MidiDriver_AmigaSci0 *>(_driver)->playSwitch(play); }
	void initTrack(SciSpan<const byte> &trackData) { static_cast<MidiDriver_AmigaSci0 *>(_driver)->initTrack(trackData); }
};

MidiPlayer *MidiPlayer_AmigaSci0_create(SciVersion version) {
	return new MidiPlayer_AmigaSci0(version);
}

} // End of namespace Sci
