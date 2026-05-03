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

#include "engines/freescape/games/eclipse/c64.music.h"

#include "common/textconsole.h"
#include "freescape/wb.h"

namespace Freescape {

static const int kSIDOffset[] = {0, 7, 14};

void EclipseC64MusicPlayer::ChannelState::reset() {
	orderAddr = 0;
	orderPos = 0;
	patternAddr = 0;
	patternOffset = 0;
	instrumentOffset = 0;
	currentNote = 0;
	transpose = 0;
	frequencyLow = 0;
	frequencyHigh = 0;
	pulseWidthLow = 0;
	pulseWidthHigh = 0;
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
	pulseWidthDirection = 0;
	delayValue = 0;
	delayCounter = 0;
	waveform = 0;
	instrumentFlags = 0;
	gateOffDisabled = false;
}

EclipseC64MusicPlayer::EclipseC64MusicPlayer(const Common::Array<byte> &musicData)
	: _sid(nullptr),
	  _musicData(musicData),
	  _musicActive(false),
	  _speedDivider(1),
	  _speedCounter(0) {
	memset(_arpeggioIntervals, 0, sizeof(_arpeggioIntervals));
	for (int i = 0; i < 8; i++)
		_arpeggioIntervals[i] = readByte(kArpeggioIntervalTable + i);
	initSID();
}

EclipseC64MusicPlayer::~EclipseC64MusicPlayer() {
	destroySID();
}

void EclipseC64MusicPlayer::destroySID() {
	if (_sid) {
		_sid->stop();
		delete _sid;
		_sid = nullptr;
	}
}

void EclipseC64MusicPlayer::initSID() {
	if (_sid) {
		_sid->stop();
		delete _sid;
	}

	_sid = SID::Config::create(SID::Config::kSidPAL);
	if (!_sid || !_sid->init()) {
		warning("EclipseC64MusicPlayer: Failed to create SID emulator");
		return;
	}

	_sid->start(new Common::Functor0Mem<void, EclipseC64MusicPlayer>(this, &EclipseC64MusicPlayer::onTimer), 50);
}

bool EclipseC64MusicPlayer::isPlaying() const {
	return _musicActive;
}

void EclipseC64MusicPlayer::sidWrite(int reg, byte data) {
	if (_sid)
		_sid->writeReg(reg, data);
}

void EclipseC64MusicPlayer::silenceAll() {
	for (int i = 0; i <= 0x18; i++)
		sidWrite(i, 0);
}

byte EclipseC64MusicPlayer::readByte(uint16 address) const {
	if (address < kLoadAddress)
		return 0;
	uint32 offset = address - kLoadAddress;
	if (offset >= _musicData.size())
		return 0;
	return _musicData[offset];
}

uint16 EclipseC64MusicPlayer::readWordLE(uint16 address) const {
	return readByte(address) | (readByte(address + 1) << 8);
}

uint16 EclipseC64MusicPlayer::readPatternPointer(byte index) const {
	if (index >= kPatternCount)
		return 0;
	return readByte(kPatternPointerLowTable + index) | (readByte(kPatternPointerHighTable + index) << 8);
}

byte EclipseC64MusicPlayer::readInstrumentByte(byte instrumentOffset, byte field) const {
	return readByte(kInstrumentTable + instrumentOffset + field);
}

byte EclipseC64MusicPlayer::readPatternByte(int channel) {
	byte value = readByte(_channels[channel].patternAddr + _channels[channel].patternOffset);
	_channels[channel].patternOffset++;
	return value;
}

byte EclipseC64MusicPlayer::clampNote(byte note) const {
	return note > kMaxNote ? kMaxNote : note;
}

void EclipseC64MusicPlayer::startMusic() {
	if (_musicData.empty())
		return;
	initSID();
	setupSong();
}

void EclipseC64MusicPlayer::stopMusic() {
	_musicActive = false;
	silenceAll();
	destroySID();
}

void EclipseC64MusicPlayer::setupSong() {
	silenceAll();

	sidWrite(0x15, 0x00);
	sidWrite(0x16, 0x00);
	sidWrite(0x17, 0x77);
	sidWrite(0x18, 0x5F);

	_speedDivider = 1;
	_speedCounter = 0;

	for (int i = 0; i < kChannelCount; i++) {
		_channels[i].reset();
		_channels[i].orderAddr = readWordLE(kOrderPointerTable + i * 2);
		loadNextPattern(i);
	}

	_musicActive = true;
}

void EclipseC64MusicPlayer::loadNextPattern(int channel) {
	int safety = 200;
	while (safety-- > 0) {
		byte value = readByte(_channels[channel].orderAddr + _channels[channel].orderPos);
		_channels[channel].orderPos++;

		if (value == 0xFF) {
			_channels[channel].orderPos = 0;
			continue;
		}

		if (value >= 0xC0) {
			_channels[channel].transpose = (byte)WBCommon::decodeOrderTranspose(value);
			continue;
		}

		_channels[channel].patternAddr = readPatternPointer(value);
		_channels[channel].patternOffset = 0;
		break;
	}
}

void EclipseC64MusicPlayer::buildEffectArpeggio(int channel) {
	_channels[channel].arpeggioSequenceLen = WBCommon::buildArpeggioTable(
		_arpeggioIntervals,
		_channels[channel].effectParam,
		_channels[channel].arpeggioSequence,
		sizeof(_channels[channel].arpeggioSequence),
		true);
	_channels[channel].arpeggioSequencePos = 0;
}

void EclipseC64MusicPlayer::loadCurrentFrequency(int channel) {
	byte note = clampNote(_channels[channel].currentNote);

	_channels[channel].frequencyLow = readByte(kFrequencyLowTable + note);
	_channels[channel].frequencyHigh = readByte(kFrequencyHighTable + note);
	sidWrite(kSIDOffset[channel] + 0, _channels[channel].frequencyLow);
	sidWrite(kSIDOffset[channel] + 1, _channels[channel].frequencyHigh);
}

void EclipseC64MusicPlayer::finalizeChannel(int channel) {
	if (_channels[channel].durationReload != 0 &&
		!_channels[channel].gateOffDisabled &&
		((_channels[channel].durationReload >> 1) == _channels[channel].durationCounter)) {
		sidWrite(kSIDOffset[channel] + 4, _channels[channel].waveform & 0xFE);
	}

	applyPulseWidthModulation(channel);
	sidWrite(kSIDOffset[channel] + 2, _channels[channel].pulseWidthLow);
	sidWrite(kSIDOffset[channel] + 3, _channels[channel].pulseWidthHigh & 0x0F);
}

void EclipseC64MusicPlayer::onTimer() {
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

void EclipseC64MusicPlayer::processChannel(int channel, bool newBeat) {
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
			loadCurrentFrequency(channel);
			finalizeChannel(channel);
			return;
		}
	} else if (_channels[channel].stepDownCounter != 0) {
		_channels[channel].stepDownCounter--;
		if (_channels[channel].currentNote > 0)
			_channels[channel].currentNote--;
		loadCurrentFrequency(channel);
		finalizeChannel(channel);
		return;
	}

	applyFrameEffects(channel);
	finalizeChannel(channel);
}

void EclipseC64MusicPlayer::parseCommands(int channel) {
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

		if (cmd == 0xFD) {
			sidWrite(0x18, readPatternByte(channel));
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
				_channels[channel].instrumentOffset = instrument * 8;
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

void EclipseC64MusicPlayer::applyNote(int channel, byte note) {
	byte instrumentOffset = _channels[channel].instrumentOffset;
	byte ctrl = readInstrumentByte(instrumentOffset, 0);
	byte attackDecay = readInstrumentByte(instrumentOffset, 1);
	byte sustainRelease = readInstrumentByte(instrumentOffset, 2);
	byte initialPulseWidth = readInstrumentByte(instrumentOffset, 3);
	byte autoEffect = readInstrumentByte(instrumentOffset, 6);
	byte flags = readInstrumentByte(instrumentOffset, 7);
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

	loadCurrentFrequency(channel);

	_channels[channel].pulseWidthLow = initialPulseWidth & 0xF0;
	_channels[channel].pulseWidthHigh = initialPulseWidth & 0x0F;
	sidWrite(kSIDOffset[channel] + 2, _channels[channel].pulseWidthLow);
	sidWrite(kSIDOffset[channel] + 3, _channels[channel].pulseWidthHigh & 0x0F);

	sidWrite(kSIDOffset[channel] + 5, attackDecay);
	sidWrite(kSIDOffset[channel] + 6, sustainRelease);
	_channels[channel].gateOffDisabled = (sustainRelease & 0x0F) == 0x0F;
	sidWrite(kSIDOffset[channel] + 4, 0x00);
	sidWrite(kSIDOffset[channel] + 4, ctrl);

	_channels[channel].durationCounter = _channels[channel].durationReload;
	_channels[channel].delayCounter = _channels[channel].delayValue;
	_channels[channel].arpeggioSequencePos = 0;
}

void EclipseC64MusicPlayer::applyFrameEffects(int channel) {
	if (_channels[channel].currentNote == 0)
		return;

	if (applyInstrumentVibrato(channel))
		return;

	applyEffectArpeggio(channel);
	applyTimedSlide(channel);
}

bool EclipseC64MusicPlayer::applyInstrumentVibrato(int channel) {
	byte vibrato = readInstrumentByte(_channels[channel].instrumentOffset, 4);
	if (vibrato == 0 || _channels[channel].currentNote >= kMaxNote)
		return false;

	byte shift = vibrato & 0x0F;
	byte span = vibrato >> 4;
	if (span == 0)
		return false;

	uint16 noteFrequency = (readByte(kFrequencyHighTable + _channels[channel].currentNote) << 8) |
		readByte(kFrequencyLowTable + _channels[channel].currentNote);
	uint16 nextFrequency = (readByte(kFrequencyHighTable + _channels[channel].currentNote + 1) << 8) |
		readByte(kFrequencyLowTable + _channels[channel].currentNote + 1);
	uint16 delta = nextFrequency - noteFrequency;

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

	int32 frequency = (_channels[channel].frequencyHigh << 8) | _channels[channel].frequencyLow;
	for (byte i = 0; i < (span >> 1); i++)
		frequency -= delta;
	for (byte i = 0; i < _channels[channel].vibratoCounter; i++)
		frequency += delta;

	if (frequency < 0)
		frequency = 0;
	if (frequency > 0xFFFF)
		frequency = 0xFFFF;

	sidWrite(kSIDOffset[channel] + 0, frequency & 0xFF);
	sidWrite(kSIDOffset[channel] + 1, (frequency >> 8) & 0xFF);
	return true;
}

void EclipseC64MusicPlayer::applyEffectArpeggio(int channel) {
	if (_channels[channel].effectParam == 0 || _channels[channel].arpeggioSequenceLen == 0)
		return;

	if (_channels[channel].arpeggioSequencePos >= _channels[channel].arpeggioSequenceLen)
		_channels[channel].arpeggioSequencePos = 0;

	byte note = clampNote(_channels[channel].currentNote + _channels[channel].arpeggioSequence[_channels[channel].arpeggioSequencePos]);
	_channels[channel].arpeggioSequencePos++;

	sidWrite(kSIDOffset[channel] + 0, readByte(kFrequencyLowTable + note));
	sidWrite(kSIDOffset[channel] + 1, readByte(kFrequencyHighTable + note));
}

void EclipseC64MusicPlayer::applyTimedSlide(int channel) {
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

	uint16 currentFrequency = (_channels[channel].frequencyHigh << 8) | _channels[channel].frequencyLow;
	uint16 sourceFrequency = (readByte(kFrequencyHighTable + currentNote) << 8) | readByte(kFrequencyLowTable + currentNote);
	uint16 targetFrequency = (readByte(kFrequencyHighTable + targetNote) << 8) | readByte(kFrequencyLowTable + targetNote);
	uint16 difference = sourceFrequency > targetFrequency ? sourceFrequency - targetFrequency : targetFrequency - sourceFrequency;
	uint16 divisor = span * (_speedDivider + 1);
	if (divisor == 0)
		return;

	uint16 delta = difference / divisor;
	if (delta == 0)
		return;

	if (targetFrequency > sourceFrequency)
		currentFrequency += delta;
	else
		currentFrequency -= delta;

	_channels[channel].frequencyLow = currentFrequency & 0xFF;
	_channels[channel].frequencyHigh = (currentFrequency >> 8) & 0xFF;
	sidWrite(kSIDOffset[channel] + 0, _channels[channel].frequencyLow);
	sidWrite(kSIDOffset[channel] + 1, _channels[channel].frequencyHigh);
}

void EclipseC64MusicPlayer::applyPulseWidthModulation(int channel) {
	byte pulseWidthMod = readInstrumentByte(_channels[channel].instrumentOffset, 5);
	if (pulseWidthMod == 0)
		return;

	if ((_channels[channel].instrumentFlags & 0x04) != 0) {
		_channels[channel].pulseWidthLow += pulseWidthMod;
		return;
	}

	uint16 pulseWidth = (_channels[channel].pulseWidthHigh << 8) | _channels[channel].pulseWidthLow;
	if (_channels[channel].pulseWidthDirection == 0) {
		pulseWidth += pulseWidthMod;
		if ((pulseWidth >> 8) >= 0x0F)
			_channels[channel].pulseWidthDirection = 1;
	} else {
		pulseWidth -= pulseWidthMod;
		if ((pulseWidth >> 8) < 0x08)
			_channels[channel].pulseWidthDirection = 0;
	}

	_channels[channel].pulseWidthLow = pulseWidth & 0xFF;
	_channels[channel].pulseWidthHigh = (pulseWidth >> 8) & 0xFF;
}

} // namespace Freescape
