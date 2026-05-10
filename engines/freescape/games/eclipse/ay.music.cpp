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

#include "engines/freescape/games/eclipse/ay.music.h"

#include "common/textconsole.h"
#include "freescape/wb.h"
#include "freescape/games/eclipse/eclipse.musicdata.h"

using namespace Freescape::EclipseMusicData;

namespace Freescape {

// ============================================================================
// Embedded music data (extracted from Total Eclipse C64)
// ============================================================================

// AY-3-8910 period table (95 entries, derived from SID frequency table)
// AY clock = 1MHz, period = clock / (16 * freq_hz)
const uint16 kAYPeriods[] = {
	    0,  3657,  3455,  3265,  3076,  2908,  2743,  2589,
	 2447,  2309,  2176,  2055,  1939,  1832,  1728,  1632,
	 1540,  1454,  1371,  1295,  1222,  1153,  1088,  1027,
	  970,   915,   864,   816,   770,   726,   686,   647,
	  611,   577,   544,   514,   485,   458,   432,   406,
	  385,   363,   343,   324,   305,   288,   272,   257,
	  242,   229,   216,   204,   192,   182,   171,   162,
	  153,   144,   136,   128,   121,   114,   108,   102,
	   96,    91,    86,    81,    76,    72,    68,    64,
	   61,    57,    54,    51,    48,    45,    43,    40,
	   38,    36,    34,    32,    30,    29,    27,    25,
	   24,    23,    21,    20,    19,    18,    17
};

// Instruments, order lists, pattern data, and arpeggio intervals
// are in eclipse.musicdata.h (shared with the OPL player).

// ============================================================================




// ============================================================================
// Software ADSR rate tables (8.8 fixed point, per-frame at 50Hz)
// ============================================================================

// Attack rate: 0x0F00 / (time_ms / 20) in 8.8 fixed point at 50Hz
// SID attack times: 2, 8, 16, 24, 38, 56, 68, 80, 100, 250, 500, 800, 1000, 3000, 5000, 8000 ms
const uint16 kAttackRate[16] = {
	0x0F00, 0x0F00, 0x0F00, 0x0C80, // 0-2: <1 frame (instant), 3: 1.2 frames
	0x07E5, 0x055B, 0x0469, 0x03C0, // 4-7: 1.9-4.0 frames
	0x0300, 0x0133, 0x009A, 0x0060, // 8-11: 5-40 frames
	0x004D, 0x001A, 0x000F, 0x000A  // 12-15: 50-400 frames
};

// Decay/Release rate: 0x0F00 / (time_ms / 20) in 8.8 fixed point at 50Hz
// SID decay/release times: 6, 24, 48, 72, 114, 168, 204, 240, 300, 750, 1500, 2400, 3000, 9000, 15000, 24000 ms
const uint16 kDecayReleaseRate[16] = {
	0x0F00, 0x0C80, 0x0640, 0x042B, // 0-3: instant to 3.6 frames
	0x02A2, 0x01C9, 0x0178, 0x0140, // 4-7: 5.7-12 frames
	0x0100, 0x0066, 0x0033, 0x0020, // 8-11: 15-120 frames
	0x001A, 0x0009, 0x0005, 0x0003  // 12-15: 150-1200 frames
};

// ============================================================================
// ChannelState
// ============================================================================

void EclipseAYMusicPlayer::ChannelState::reset() {
	orderList = nullptr;
	orderPos = 0;
	patternDataOffset = 0;
	patternOffset = 0;
	instrumentOffset = 0;
	currentNote = 0;
	transpose = 0;
	currentPeriod = 0;
	durationReload = 0;
	durationCounter = 0;
	effectMode = 0;
	effectParam = 0;
	arpeggioTarget = 0;
	arpeggioParam = 0;
	arpeggioSequencePos = 0;
	memset(arpeggioSequence, 0, sizeof(arpeggioSequence));
	arpeggioSequenceLen = 0;
	noteStepCommand = 0;
	stepDownCounter = 0;
	vibratoPhase = 0;
	vibratoCounter = 0;
	delayValue = 0;
	delayCounter = 0;
	waveform = 0;
	instrumentFlags = 0;
	gateOffDisabled = false;
	adsrPhase = kPhaseOff;
	adsrVolume = 0;
	attackRate = 0;
	decayRate = 0;
	sustainLevel = 0;
	releaseRate = 0;
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

EclipseAYMusicPlayer::EclipseAYMusicPlayer(Audio::Mixer *mixer)
	: AY8912Stream(44100, 1000000),
	  _mixer(mixer),
	  _musicActive(false),
	  _speedDivider(1),
	  _speedCounter(0),
	  _mixerRegister(0x38),
	  _tickSampleCount(0) {
	memcpy(_arpeggioIntervals, kArpeggioIntervals, 8);
}

EclipseAYMusicPlayer::~EclipseAYMusicPlayer() {
	stopMusic();
}

// ============================================================================
// Public interface
// ============================================================================

void EclipseAYMusicPlayer::startMusic() {
	stopMusic();
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_handle, toAudioStream(),
	                   -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
	setupSong();
}

void EclipseAYMusicPlayer::stopMusic() {
	_musicActive = false;
	silenceAll();
	if (_mixer)
		_mixer->stopHandle(_handle);
}

bool EclipseAYMusicPlayer::isPlaying() const {
	return _musicActive;
}

// ============================================================================
// AudioStream
// ============================================================================

int EclipseAYMusicPlayer::readBuffer(int16 *buffer, const int numSamples) {
	if (!_musicActive) {
		memset(buffer, 0, numSamples * sizeof(int16));
		return numSamples;
	}

	int samplesGenerated = 0;
	int samplesPerTick = (getRate() / 50) * 2; // stereo: 2 int16 per frame

	while (samplesGenerated < numSamples) {
		int remaining = samplesPerTick - _tickSampleCount;
		int toGenerate = MIN(numSamples - samplesGenerated, remaining);

		if (toGenerate > 0) {
			generateSamples(buffer + samplesGenerated, toGenerate);
			samplesGenerated += toGenerate;
			_tickSampleCount += toGenerate;
		}

		if (_tickSampleCount >= samplesPerTick) {
			_tickSampleCount -= samplesPerTick;
			onTimer();
		}
	}

	return samplesGenerated;
}

// ============================================================================
// Timer / sequencer core
// ============================================================================

void EclipseAYMusicPlayer::onTimer() {
	if (!_musicActive)
		return;

	bool newBeat = (_speedCounter == 0);

	for (int channel = kChannelCount - 1; channel >= 0; channel--)
		processChannel(channel, newBeat);

	if (!_musicActive)
		return;

	if (newBeat)
		_speedCounter = _speedDivider;
	else
		_speedCounter--;
}

void EclipseAYMusicPlayer::processChannel(int channel, bool newBeat) {
	if (newBeat) {
		_channels[channel].durationCounter--;
		if (_channels[channel].durationCounter == 0xFF) {
			parseCommands(channel);
			if (!_musicActive)
				return;
			finalizeChannel(channel);
			return;
		}

		if (_channels[channel].noteStepCommand != 0) {
			if (_channels[channel].noteStepCommand == 0xDE) {
				if (_channels[channel].currentNote > 0)
					_channels[channel].currentNote--;
			} else if (_channels[channel].currentNote < kMaxNote) {
				_channels[channel].currentNote++;
			}
			loadCurrentPeriod(channel);
			finalizeChannel(channel);
			return;
		}
	} else if (_channels[channel].stepDownCounter != 0) {
		_channels[channel].stepDownCounter--;
		if (_channels[channel].currentNote > 0)
			_channels[channel].currentNote--;
		loadCurrentPeriod(channel);
		finalizeChannel(channel);
		return;
	}

	applyFrameEffects(channel);
	finalizeChannel(channel);
}

void EclipseAYMusicPlayer::finalizeChannel(int channel) {
	// Gate off at half duration triggers release phase
	if (_channels[channel].durationReload != 0 &&
	    !_channels[channel].gateOffDisabled &&
	    ((_channels[channel].durationReload >> 1) == _channels[channel].durationCounter)) {
		releaseADSR(channel);
	}

	updateADSR(channel);
}

// ============================================================================
// Song setup
// ============================================================================

void EclipseAYMusicPlayer::setupSong() {
	silenceAll();

	// Initialize AY: all tone enabled, noise disabled, default noise period
	_mixerRegister = 0x38;
	setReg(7, _mixerRegister);
	setReg(6, 0x07);

	_speedDivider = 1;
	_speedCounter = 0;

	const byte *orderLists[3] = { kOrderList0, kOrderList1, kOrderList2 };

	for (int i = 0; i < kChannelCount; i++) {
		_channels[i].reset();
		_channels[i].orderList = orderLists[i];
		loadNextPattern(i);
	}

	_musicActive = true;
}

void EclipseAYMusicPlayer::silenceAll() {
	for (int r = 0; r < 14; r++)
		setReg(r, 0);
	_mixerRegister = 0x38;
	setReg(7, _mixerRegister);
}

// ============================================================================
// Order list / pattern navigation
// ============================================================================

void EclipseAYMusicPlayer::loadNextPattern(int channel) {
	int safety = 200;
	while (safety-- > 0) {
		byte value = _channels[channel].orderList[_channels[channel].orderPos];
		_channels[channel].orderPos++;

		if (value == 0xFF) {
			_channels[channel].orderPos = 0;
			continue;
		}

		if (value >= 0xC0) {
			_channels[channel].transpose = (byte)WBCommon::decodeOrderTranspose(value);
			continue;
		}

		if (value < ARRAYSIZE(kPatternOffsets)) {
			_channels[channel].patternDataOffset = kPatternOffsets[value];
			_channels[channel].patternOffset = 0;
		}
		break;
	}
}

byte EclipseAYMusicPlayer::readPatternByte(int channel) {
	byte value = kPatternData[_channels[channel].patternDataOffset + _channels[channel].patternOffset];
	_channels[channel].patternOffset++;
	return value;
}

byte EclipseAYMusicPlayer::clampNote(byte note) const {
	return note > kMaxNote ? kMaxNote : note;
}

// ============================================================================
// Pattern command parser
// ============================================================================

void EclipseAYMusicPlayer::parseCommands(int channel) {
	if (_channels[channel].effectMode != 2) {
		_channels[channel].effectParam = 0;
		_channels[channel].effectMode = 0;
		_channels[channel].arpeggioSequenceLen = 0;
		_channels[channel].arpeggioSequencePos = 0;
	}

	_channels[channel].arpeggioTarget = 0;
	_channels[channel].noteStepCommand = 0;

	int safety = 200;
	while (safety-- > 0) {
		byte cmd = readPatternByte(channel);

		if (cmd == 0xFF) {
			loadNextPattern(channel);
			continue;
		}

		if (cmd == 0xFE) {
			stopMusic();
			return;
		}

		// Filter command: consume the byte (no filters on AY), continue
		if (cmd == 0xFD) {
			readPatternByte(channel); // discard filter value
			cmd = readPatternByte(channel);
			if (cmd == 0xFF) {
				loadNextPattern(channel);
				continue;
			}
		}

		if (cmd >= 0xF0) {
			_speedDivider = cmd & 0x0F;
			continue;
		}

		if (cmd >= 0xC0) {
			byte instrument = cmd & 0x1F;
			if (instrument < kInstrumentCount)
				_channels[channel].instrumentOffset = instrument * kInstrumentSize;
			continue;
		}

		if (cmd >= 0x80) {
			_channels[channel].durationReload = cmd & 0x3F;
			continue;
		}

		if (cmd == 0x7F) {
			_channels[channel].noteStepCommand = 0xDE;
			_channels[channel].effectMode = 0xDE;
			continue;
		}

		if (cmd == 0x7E) {
			_channels[channel].effectMode = 0xFE;
			continue;
		}

		if (cmd == 0x7D) {
			_channels[channel].effectMode = 1;
			_channels[channel].effectParam = readPatternByte(channel);
			buildEffectArpeggio(channel);
			continue;
		}

		if (cmd == 0x7C) {
			_channels[channel].effectMode = 2;
			_channels[channel].effectParam = readPatternByte(channel);
			buildEffectArpeggio(channel);
			continue;
		}

		if (cmd == 0x7B) {
			_channels[channel].effectParam = 0;
			_channels[channel].effectMode = 1;
			_channels[channel].arpeggioTarget = readPatternByte(channel) + _channels[channel].transpose;
			_channels[channel].arpeggioParam = readPatternByte(channel);
			continue;
		}

		if (cmd == 0x7A) {
			_channels[channel].delayValue = readPatternByte(channel);
			cmd = readPatternByte(channel);
		}

		applyNote(channel, cmd);
		return;
	}
}

// ============================================================================
// Note application
// ============================================================================

void EclipseAYMusicPlayer::applyNote(int channel, byte note) {
	byte instrumentOffset = _channels[channel].instrumentOffset;
	byte ctrl = kInstruments[instrumentOffset + 0];
	byte attackDecay = kInstruments[instrumentOffset + 1];
	byte sustainRelease = kInstruments[instrumentOffset + 2];
	byte autoEffect = kInstruments[instrumentOffset + 4];
	byte flags = kInstruments[instrumentOffset + 5];
	byte actualNote = note;

	if (actualNote != 0)
		actualNote = clampNote(actualNote + _channels[channel].transpose);

	_channels[channel].currentNote = actualNote;
	_channels[channel].waveform = ctrl;
	_channels[channel].instrumentFlags = flags;
	_channels[channel].stepDownCounter = 0;

	if (actualNote != 0 && _channels[channel].effectParam == 0 && autoEffect != 0) {
		_channels[channel].effectParam = autoEffect;
		buildEffectArpeggio(channel);
	}

	if (actualNote != 0 && (flags & 0x02) != 0) {
		_channels[channel].stepDownCounter = 2;
		_channels[channel].currentNote = clampNote(_channels[channel].currentNote + 2);
	}

	loadCurrentPeriod(channel);

	// Configure AY mixer for this channel: tone or noise
	if (actualNote == 0) {
		// Rest: disable both tone and noise
		_mixerRegister |= (1 << channel);
		_mixerRegister |= (1 << (channel + 3));
	} else if (ctrl & 0x80) {
		// Noise waveform
		_mixerRegister |= (1 << channel);        // disable tone
		_mixerRegister &= ~(1 << (channel + 3)); // enable noise
	} else {
		// Tone waveform (pulse, triangle, sawtooth all map to AY square)
		_mixerRegister &= ~(1 << channel);       // enable tone
		_mixerRegister |= (1 << (channel + 3));  // disable noise
	}
	setReg(7, _mixerRegister);

	// Trigger ADSR envelope if gate bit is set
	_channels[channel].gateOffDisabled = (sustainRelease & 0x0F) == 0x0F;
	if (ctrl & 0x01) {
		triggerADSR(channel, attackDecay, sustainRelease);
	} else {
		_channels[channel].adsrPhase = kPhaseOff;
		_channels[channel].adsrVolume = 0;
		setReg(8 + channel, 0);
	}

	_channels[channel].durationCounter = _channels[channel].durationReload;
	_channels[channel].delayCounter = _channels[channel].delayValue;
	_channels[channel].arpeggioSequencePos = 0;
}

// ============================================================================
// Period / frequency helpers
// ============================================================================

void EclipseAYMusicPlayer::writeChannelPeriod(int channel, uint16 period) {
	_channels[channel].currentPeriod = period;
	setReg(channel * 2, period & 0xFF);
	setReg(channel * 2 + 1, (period >> 8) & 0x0F);
}

void EclipseAYMusicPlayer::loadCurrentPeriod(int channel) {
	byte note = clampNote(_channels[channel].currentNote);
	writeChannelPeriod(channel, kAYPeriods[note]);
}

// ============================================================================
// Effects
// ============================================================================

void EclipseAYMusicPlayer::buildEffectArpeggio(int channel) {
	_channels[channel].arpeggioSequenceLen = WBCommon::buildArpeggioTable(
		_arpeggioIntervals,
		_channels[channel].effectParam,
		_channels[channel].arpeggioSequence,
		sizeof(_channels[channel].arpeggioSequence),
		true);
	_channels[channel].arpeggioSequencePos = 0;
}

void EclipseAYMusicPlayer::applyFrameEffects(int channel) {
	if (_channels[channel].currentNote == 0)
		return;

	if (applyInstrumentVibrato(channel))
		return;

	applyEffectArpeggio(channel);
	applyTimedSlide(channel);
}

bool EclipseAYMusicPlayer::applyInstrumentVibrato(int channel) {
	byte vibrato = kInstruments[_channels[channel].instrumentOffset + 3];
	if (vibrato == 0 || _channels[channel].currentNote >= kMaxNote)
		return false;

	byte shift = vibrato & 0x0F;
	byte span = vibrato >> 4;
	if (span == 0)
		return false;

	uint16 notePeriod = kAYPeriods[_channels[channel].currentNote];
	uint16 nextPeriod = kAYPeriods[_channels[channel].currentNote + 1];

	if (notePeriod <= nextPeriod)
		return false;

	uint16 delta = notePeriod - nextPeriod;

	while (shift-- != 0)
		delta >>= 1;

	if ((_channels[channel].vibratoPhase & 0x80) != 0) {
		if (_channels[channel].vibratoCounter != 0)
			_channels[channel].vibratoCounter--;
		if (_channels[channel].vibratoCounter == 0)
			_channels[channel].vibratoPhase = 0;
	} else {
		_channels[channel].vibratoCounter++;
		if (_channels[channel].vibratoCounter >= span)
			_channels[channel].vibratoPhase = 0xFF;
	}

	if (_channels[channel].delayCounter != 0) {
		_channels[channel].delayCounter--;
		return false;
	}

	// Modulate period: higher period = lower pitch
	// Start low (high period), sweep toward high (low period)
	int32 period = _channels[channel].currentPeriod;
	for (byte i = 0; i < (span >> 1); i++)
		period += delta;  // go down (increase period)
	for (byte i = 0; i < _channels[channel].vibratoCounter; i++)
		period -= delta;  // go up (decrease period)

	if (period < 1)
		period = 1;
	if (period > 4095)
		period = 4095;

	setReg(channel * 2, period & 0xFF);
	setReg(channel * 2 + 1, (period >> 8) & 0x0F);
	return true;
}

void EclipseAYMusicPlayer::applyEffectArpeggio(int channel) {
	if (_channels[channel].effectParam == 0 || _channels[channel].arpeggioSequenceLen == 0)
		return;

	if (_channels[channel].arpeggioSequencePos >= _channels[channel].arpeggioSequenceLen)
		_channels[channel].arpeggioSequencePos = 0;

	byte note = clampNote(_channels[channel].currentNote +
	                      _channels[channel].arpeggioSequence[_channels[channel].arpeggioSequencePos]);
	_channels[channel].arpeggioSequencePos++;

	uint16 period = kAYPeriods[note];
	setReg(channel * 2, period & 0xFF);
	setReg(channel * 2 + 1, (period >> 8) & 0x0F);
}

void EclipseAYMusicPlayer::applyTimedSlide(int channel) {
	if (_channels[channel].arpeggioTarget == 0)
		return;

	byte total = _channels[channel].durationReload;
	byte remaining = _channels[channel].durationCounter;
	byte start = _channels[channel].arpeggioParam >> 4;
	byte span = _channels[channel].arpeggioParam & 0x0F;
	byte elapsed = total - remaining;

	if (elapsed <= start || elapsed > start + span || span == 0)
		return;

	byte currentNote = clampNote(_channels[channel].currentNote);
	byte targetNote = clampNote(_channels[channel].arpeggioTarget);
	if (currentNote == targetNote)
		return;

	uint16 currentPeriod = _channels[channel].currentPeriod;
	uint16 sourcePeriod = kAYPeriods[currentNote];
	uint16 targetPeriod = kAYPeriods[targetNote];
	uint16 difference = sourcePeriod > targetPeriod ?
	                    sourcePeriod - targetPeriod :
	                    targetPeriod - sourcePeriod;
	uint16 divisor = span * (_speedDivider + 1);
	if (divisor == 0)
		return;

	uint16 delta = difference / divisor;
	if (delta == 0)
		return;

	if (targetPeriod > sourcePeriod)
		currentPeriod += delta;
	else
		currentPeriod -= delta;

	writeChannelPeriod(channel, currentPeriod);
}

// ============================================================================
// Software ADSR envelope
// ============================================================================

void EclipseAYMusicPlayer::triggerADSR(int channel, byte ad, byte sr) {
	_channels[channel].adsrPhase = kPhaseAttack;
	// Don't reset volume: SID re-gate (gate-off + gate-on in same frame)
	// starts the attack from the current volume, not from zero.
	// This avoids audible clicks between consecutive notes.
	_channels[channel].attackRate = kAttackRate[ad >> 4];
	_channels[channel].decayRate = kDecayReleaseRate[ad & 0x0F];
	_channels[channel].sustainLevel = sr >> 4;
	_channels[channel].releaseRate = kDecayReleaseRate[sr & 0x0F];
}

void EclipseAYMusicPlayer::releaseADSR(int channel) {
	if (_channels[channel].adsrPhase != kPhaseRelease &&
	    _channels[channel].adsrPhase != kPhaseOff) {
		_channels[channel].adsrPhase = kPhaseRelease;
	}
}

void EclipseAYMusicPlayer::updateADSR(int channel) {
	switch (_channels[channel].adsrPhase) {
	case kPhaseAttack:
		_channels[channel].adsrVolume += _channels[channel].attackRate;
		if (_channels[channel].adsrVolume >= 0x0F00) {
			_channels[channel].adsrVolume = 0x0F00;
			_channels[channel].adsrPhase = kPhaseDecay;
		}
		break;

	case kPhaseDecay: {
		uint16 sustainTarget = (uint16)_channels[channel].sustainLevel << 8;
		if (_channels[channel].adsrVolume > _channels[channel].decayRate + sustainTarget) {
			_channels[channel].adsrVolume -= _channels[channel].decayRate;
		} else {
			_channels[channel].adsrVolume = sustainTarget;
			_channels[channel].adsrPhase = kPhaseSustain;
		}
		break;
	}

	case kPhaseSustain:
		// Volume stays at sustain level
		break;

	case kPhaseRelease:
		if (_channels[channel].adsrVolume > _channels[channel].releaseRate) {
			_channels[channel].adsrVolume -= _channels[channel].releaseRate;
		} else {
			_channels[channel].adsrVolume = 0;
			_channels[channel].adsrPhase = kPhaseOff;
		}
		break;

	case kPhaseOff:
		_channels[channel].adsrVolume = 0;
		break;
	}

	setReg(8 + channel, _channels[channel].adsrVolume >> 8);
}

} // namespace Freescape
