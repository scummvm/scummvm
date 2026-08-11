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

#include "common/debug.h"
#include "common/endian.h"
#include "common/textconsole.h"
#include "freescape/games/dark/c64.music.h"
#include "freescape/games/dark/dark.musicdata.h"

using namespace Freescape::DarkMusicData;

namespace Freescape {

// ============================================================
// Data tables extracted from darkside.prg (load address $0400)
// ============================================================

const int kSIDOffset[3] = { 0, 7, 14 };


// ============================================================
// Implementation
// ============================================================

void DarkSideC64MusicPlayer::ChannelState::reset() {
	orderData = nullptr;
	orderPos = 0;
	patData = nullptr;
	patOffset = 0;
	instIdx = 0;
	curNote = 0;
	transpose = 0;
	freqLo = 0;
	freqHi = 0;
	pwLo = 0;
	pwHi = 0;
	durReload = 1;
	durCounter = 1;
	effectMode = 0;
	effectParam = 0;
	arpPattern = 0;
	arpParam2 = 0;
	arpSeqPos = 0;
	arpSeqLen = 0;
	memset(arpSeqData, 0, sizeof(arpSeqData));
	noteStepCommand = 0;
	stepDownCounter = 0;
	vibPhase = 0;
	vibCounter = 0;
	pwDirection = 0;
	delayValue = 0;
	delayCounter = 0;
	envCounter = 0;
	gateOffDisabled = false;
	gateModeControl = false;
	specialAttack = false;
	attackDone = false;
	waveform = 0;
	instFlags = 0;
}

DarkSideC64MusicPlayer::DarkSideC64MusicPlayer() {
	_sid = nullptr;
	_musicActive = false;
	_speedDiv = 1;
	_speedCounter = 0;
	for (int i = 0; i < 3; i++)
		_ch[i].reset();
	initSID();
}

DarkSideC64MusicPlayer::~DarkSideC64MusicPlayer() {
	destroySID();
}

void DarkSideC64MusicPlayer::destroySID() {
	if (_sid) {
		_sid->stop();
		delete _sid;
		_sid = nullptr;
	}
}

void DarkSideC64MusicPlayer::initSID() {
	if (_sid) {
		_sid->stop();
		delete _sid;
	}

	_sid = SID::Config::create(SID::Config::kSidPAL);
	if (!_sid || !_sid->init()) {
		warning("DarkSideC64MusicPlayer: Failed to create SID emulator");
		return;
	}
	_sid->start(new Common::Functor0Mem<void, DarkSideC64MusicPlayer>(this, &DarkSideC64MusicPlayer::onTimer), 60);
}

void DarkSideC64MusicPlayer::sidWrite(int reg, uint8 data) {
	if (_sid)
		_sid->writeReg(reg, data);
}

void DarkSideC64MusicPlayer::silenceAll() {
	for (int i = 0; i <= 0x18; i++)
		sidWrite(i, 0);
}

bool DarkSideC64MusicPlayer::isPlaying() const {
	return _musicActive;
}

void DarkSideC64MusicPlayer::startMusic() {
	setupSong();
}

void DarkSideC64MusicPlayer::stopMusic() {
	_musicActive = false;
	silenceAll();
}

void DarkSideC64MusicPlayer::setupSong() {
	silenceAll();

	// Init routine at $0FF6.
	sidWrite(0x18, 0x5F);
	sidWrite(0x17, 0x00);

	_speedDiv = 1;
	_speedCounter = 0;

	for (int ch = 0; ch < 3; ch++) {
		_ch[ch].reset();
		_ch[ch].orderData = kOrderLists[ch];
		loadNextPattern(ch);
	}

	_musicActive = true;
}

void DarkSideC64MusicPlayer::loadNextPattern(int ch) {
	int safety = 200;
	while (safety-- > 0) {
		uint8 byte = _ch[ch].orderData[_ch[ch].orderPos];

		if (byte == 0xFF) {
			// Loop to start
			_ch[ch].orderPos = 0;
			continue;
		}

		_ch[ch].orderPos++;

		if (byte >= 0x80) {
			// Transpose: (byte + $40) & $FF
			_ch[ch].transpose = (byte + 0x40) & 0xFF;
			continue;
		}

		// Pattern number
		if (byte < 29) {
			_ch[ch].patData = kPatterns[byte];
			_ch[ch].patOffset = 0;
		}
		break;
	}
}

uint8 DarkSideC64MusicPlayer::readPatByte(int ch) {
	uint8 b = _ch[ch].patData[_ch[ch].patOffset];
	_ch[ch].patOffset++;
	return b;
}

void DarkSideC64MusicPlayer::buildEffectArpeggio(int ch) {
	uint8 bits = _ch[ch].effectParam;
	uint8 len = 1;

	_ch[ch].arpSeqData[0] = 0;
	for (int i = 0; i < 8 && len < sizeof(_ch[ch].arpSeqData); i++) {
		if (bits & (1 << i))
			_ch[ch].arpSeqData[len++] = kArpIntervals[i];
	}

	_ch[ch].arpSeqLen = len;
	_ch[ch].arpSeqPos = 0;
	_ch[ch].arpPattern = 0;
}

void DarkSideC64MusicPlayer::loadCurrentFrequency(int ch) {
	int off = kSIDOffset[ch];
	uint8 note = (_ch[ch].curNote > 94) ? 94 : _ch[ch].curNote;

	_ch[ch].freqLo = kFreqLo[note];
	_ch[ch].freqHi = kFreqHi[note];
	sidWrite(off + 0, _ch[ch].freqLo);
	sidWrite(off + 1, _ch[ch].freqHi);
}

void DarkSideC64MusicPlayer::finalizeChannel(int ch) {
	int off = kSIDOffset[ch];

	if (_ch[ch].durReload != 0 && !_ch[ch].gateOffDisabled) {
		if ((_ch[ch].durReload >> 1) == _ch[ch].durCounter)
			sidWrite(off + 4, _ch[ch].waveform & 0xFE);
	}

	applyPWModulation(ch);
	sidWrite(off + 2, _ch[ch].pwLo);
	sidWrite(off + 3, _ch[ch].pwHi & 0x0F);
}

// ---- Main timer callback (50 Hz) ----

void DarkSideC64MusicPlayer::onTimer() {
	if (!_musicActive)
		return;

	for (int ch = 0; ch < 3; ch++)
		_ch[ch].envCounter++;

	bool newBeat = (_speedCounter == 0);

	for (int ch = 2; ch >= 0; ch--)
		processChannel(ch, newBeat);

	if (!_musicActive)
		return;

	if (newBeat)
		_speedCounter = _speedDiv;
	else
		_speedCounter--;
}

void DarkSideC64MusicPlayer::processChannel(int ch, bool newBeat) {
	if (newBeat) {
		_ch[ch].durCounter--;
		if (_ch[ch].durCounter == 0xFF) {
			parseCommands(ch);
			if (!_musicActive)
				return;
			finalizeChannel(ch);
			return;
		}

		if (_ch[ch].noteStepCommand != 0) {
			if (_ch[ch].noteStepCommand == 0xDE) {
				if (_ch[ch].curNote > 0)
					_ch[ch].curNote--;
			} else if (_ch[ch].curNote < 94) {
				_ch[ch].curNote++;
			}
			loadCurrentFrequency(ch);
			finalizeChannel(ch);
			return;
		}
	} else if (_ch[ch].stepDownCounter != 0) {
		_ch[ch].stepDownCounter--;
		if (_ch[ch].curNote > 0)
			_ch[ch].curNote--;
		loadCurrentFrequency(ch);
		finalizeChannel(ch);
		return;
	}

	applyFrameEffects(ch);
	finalizeChannel(ch);
}

// ---- Command parser ----

void DarkSideC64MusicPlayer::parseCommands(int ch) {
	if (_ch[ch].effectMode != 2) {
		_ch[ch].effectParam = 0;
		_ch[ch].effectMode = 0;
		_ch[ch].arpSeqLen = 0;
		_ch[ch].arpSeqPos = 0;
	}
	_ch[ch].arpPattern = 0;
	_ch[ch].noteStepCommand = 0;

	int safety = 200;
	while (safety-- > 0) {
		uint8 cmd = readPatByte(ch);

		if (cmd == 0xFF) {
			loadNextPattern(ch);
			continue;
		}

		if (cmd == 0xFE) {
			stopMusic();
			return;
		}

		if (cmd == 0xFD) {
			uint8 filterVal = readPatByte(ch);
			sidWrite(0x18, filterVal);
			sidWrite(0x17, (filterVal >> 4) | 0x07);
			cmd = readPatByte(ch);
			if (cmd == 0xFF) {
				loadNextPattern(ch);
				continue;
			}
		}

		if (cmd >= 0xF0) {
			_speedDiv = cmd & 0x0F;
			continue;
		}

		if (cmd >= 0xC0) {
			uint8 instNum = cmd & 0x1F;
			if (instNum < 18)
				_ch[ch].instIdx = instNum * 8;
			continue;
		}

		if (cmd >= 0x80) {
			_ch[ch].durReload = cmd & 0x3F;
			continue;
		}

		if (cmd == 0x7F) {
			_ch[ch].noteStepCommand = 0xDE;
			_ch[ch].effectMode = 0xDE;
			continue;
		}

		if (cmd == 0x7E) {
			_ch[ch].noteStepCommand = 0xFE;
			_ch[ch].effectMode = 0xFE;
			continue;
		}

		if (cmd == 0x7D) {
			_ch[ch].effectMode = 1;
			_ch[ch].effectParam = readPatByte(ch);
			buildEffectArpeggio(ch);
			continue;
		}

		if (cmd == 0x7C) {
			_ch[ch].effectMode = 2;
			_ch[ch].effectParam = readPatByte(ch);
			buildEffectArpeggio(ch);
			continue;
		}

		if (cmd == 0x7B) {
			_ch[ch].effectParam = 0;
			_ch[ch].effectMode = 1;
			_ch[ch].arpPattern = (readPatByte(ch) + _ch[ch].transpose) & 0xFF;
			_ch[ch].arpParam2 = readPatByte(ch);
			continue;
		}

		if (cmd == 0x7A) {
			_ch[ch].delayValue = readPatByte(ch);
			cmd = readPatByte(ch);
		}

		applyNote(ch, cmd);
		return;
	}
}

// ---- Note handling ----

void DarkSideC64MusicPlayer::applyNote(int ch, uint8 note) {
	int off = kSIDOffset[ch];
	uint8 instBase = _ch[ch].instIdx;
	uint8 ctrl = kInstruments[instBase + 0];
	uint8 ad = kInstruments[instBase + 1];
	uint8 sr = kInstruments[instBase + 2];
	uint8 initialPW = kInstruments[instBase + 3];
	uint8 autoFx = kInstruments[instBase + 6];
	uint8 flags = kInstruments[instBase + 7];
	uint8 actualNote = note;

	if (actualNote != 0)
		actualNote = (actualNote + _ch[ch].transpose) & 0xFF;
	if (actualNote > 94)
		actualNote = 94;

	_ch[ch].curNote = actualNote;
	_ch[ch].waveform = ctrl;
	_ch[ch].instFlags = flags;
	_ch[ch].attackDone = false;
	_ch[ch].envCounter = 0xFF;
	_ch[ch].gateModeControl = (flags & 0x80) != 0;
	_ch[ch].specialAttack = (flags & 0x08) != 0;
	_ch[ch].stepDownCounter = 0;

	if (actualNote != 0 && _ch[ch].effectParam == 0 && autoFx != 0) {
		_ch[ch].effectParam = autoFx;
		buildEffectArpeggio(ch);
	}

	if (actualNote != 0 && (flags & 0x02)) {
		_ch[ch].stepDownCounter = 2;
		_ch[ch].curNote = (_ch[ch].curNote > 92) ? 94 : _ch[ch].curNote + 2;
	}

	loadCurrentFrequency(ch);

	if (!_ch[ch].gateModeControl) {
		_ch[ch].pwLo = initialPW & 0xF0;
		_ch[ch].pwHi = initialPW & 0x0F;
		sidWrite(off + 2, _ch[ch].pwLo);
		sidWrite(off + 3, _ch[ch].pwHi & 0x0F);
	}

	sidWrite(off + 5, ad);
	sidWrite(off + 6, sr);
	_ch[ch].gateOffDisabled = (sr & 0x0F) == 0x0F;
	sidWrite(off + 4, 0);
	sidWrite(off + 4, ctrl);
	_ch[ch].durCounter = _ch[ch].durReload;
	_ch[ch].delayCounter = _ch[ch].delayValue;
	_ch[ch].arpSeqPos = 0;
}

void DarkSideC64MusicPlayer::applyFrameEffects(int ch) {
	if (_ch[ch].curNote == 0)
		return;

	if (applySpecialAttack(ch))
		return;

	if (applyEnvelopeSequence(ch))
		return;

	if (applyInstrumentVibrato(ch))
		return;

	applyEffectArpeggio(ch);
	applyTimedSlide(ch);
}

bool DarkSideC64MusicPlayer::applySpecialAttack(int ch) {
	if (!_ch[ch].specialAttack || _ch[ch].attackDone)
		return false;

	int off = kSIDOffset[ch];
	if (_ch[ch].envCounter < 1) {
		sidWrite(off + 0, 0x00);
		sidWrite(off + 1, 0x48);
		sidWrite(off + 4, 0x81);
	} else {
		loadCurrentFrequency(ch);
		_ch[ch].attackDone = true;
		sidWrite(off + 4, _ch[ch].waveform);
	}
	return true;
}

bool DarkSideC64MusicPlayer::applyEnvelopeSequence(int ch) {
	if ((_ch[ch].instFlags & 0x01) == 0 || _ch[ch].envCounter >= 15)
		return false;

	int off = kSIDOffset[ch];
	uint8 instBase = _ch[ch].instIdx;
	uint8 envMode = kInstruments[instBase + 4];
	uint8 table = (envMode & 0x0F) ? 1 : 0;
	uint8 data = kEnvData[table][_ch[ch].envCounter];

	sidWrite(off + 4, kEnvControl[table][_ch[ch].envCounter]);
	if (envMode & 0x10) {
		uint8 note = (_ch[ch].curNote + data > 94) ? 94 : _ch[ch].curNote + data;
		sidWrite(off + 0, kFreqLo[note]);
		sidWrite(off + 1, kFreqHi[note]);
	} else {
		sidWrite(off + 0, 0x00);
		sidWrite(off + 1, data + 0x0D);
	}
	return true;
}

bool DarkSideC64MusicPlayer::applyInstrumentVibrato(int ch) {
	uint8 instBase = _ch[ch].instIdx;
	uint8 vib = kInstruments[instBase + 4];
	if (vib == 0 || _ch[ch].curNote >= 94)
		return false;

	uint8 shift = vib & 0x0F;
	uint8 span = vib >> 4;
	if (span == 0)
		return false;

	uint16 noteFreq = ((uint16)kFreqHi[_ch[ch].curNote] << 8) | kFreqLo[_ch[ch].curNote];
	uint16 nextFreq = ((uint16)kFreqHi[_ch[ch].curNote + 1] << 8) | kFreqLo[_ch[ch].curNote + 1];
	uint16 delta = nextFreq - noteFreq;

	while (shift-- != 0)
		delta >>= 1;

	if (_ch[ch].vibPhase & 0x80) {
		if (_ch[ch].vibCounter != 0)
			_ch[ch].vibCounter--;
		if (_ch[ch].vibCounter == 0)
			_ch[ch].vibPhase = 0;
	} else {
		_ch[ch].vibCounter++;
		if (_ch[ch].vibCounter >= span)
			_ch[ch].vibPhase = 0xFF;
	}

	if (_ch[ch].delayCounter != 0) {
		_ch[ch].delayCounter--;
		return false;
	}

	int32 freq = ((uint16)_ch[ch].freqHi << 8) | _ch[ch].freqLo;
	for (uint8 i = 0; i < (span >> 1); i++)
		freq -= delta;
	for (uint8 i = 0; i < _ch[ch].vibCounter; i++)
		freq += delta;

	if (freq < 0)
		freq = 0;
	if (freq > 0xFFFF)
		freq = 0xFFFF;

	int off = kSIDOffset[ch];
	sidWrite(off + 0, freq & 0xFF);
	sidWrite(off + 1, (freq >> 8) & 0xFF);
	return true;
}

void DarkSideC64MusicPlayer::applyEffectArpeggio(int ch) {
	if (_ch[ch].effectParam == 0 || _ch[ch].arpSeqLen == 0)
		return;

	if (_ch[ch].arpSeqPos >= _ch[ch].arpSeqLen)
		_ch[ch].arpSeqPos = 0;

	uint8 note = (_ch[ch].curNote + _ch[ch].arpSeqData[_ch[ch].arpSeqPos] > 94) ? 94 : _ch[ch].curNote + _ch[ch].arpSeqData[_ch[ch].arpSeqPos];
	_ch[ch].arpSeqPos++;

	int off = kSIDOffset[ch];
	sidWrite(off + 0, kFreqLo[note]);
	sidWrite(off + 1, kFreqHi[note]);
}

void DarkSideC64MusicPlayer::applyTimedSlide(int ch) {
	if (_ch[ch].arpPattern == 0)
		return;

	uint8 total = _ch[ch].durReload;
	uint8 remaining = _ch[ch].durCounter;
	uint8 start = _ch[ch].arpParam2 >> 4;
	uint8 span = _ch[ch].arpParam2 & 0x0F;
	uint8 elapsed = total - remaining;

	if (elapsed <= start || elapsed > start + span || span == 0)
		return;

	uint8 currentNote = (_ch[ch].curNote > 94) ? 94 : _ch[ch].curNote;
	uint8 targetNote = (_ch[ch].arpPattern > 94) ? 94 : _ch[ch].arpPattern;
	if (currentNote == targetNote)
		return;

	uint16 currentFreq = ((uint16)_ch[ch].freqHi << 8) | _ch[ch].freqLo;
	uint16 sourceFreq = ((uint16)kFreqHi[currentNote] << 8) | kFreqLo[currentNote];
	uint16 targetFreq = ((uint16)kFreqHi[targetNote] << 8) | kFreqLo[targetNote];
	uint16 diff = (sourceFreq > targetFreq) ? (sourceFreq - targetFreq) : (targetFreq - sourceFreq);
	uint16 divisor = span * (_speedDiv + 1);
	if (divisor == 0)
		return;

	uint16 delta = diff / divisor;
	if (delta == 0)
		return;

	if (targetFreq > sourceFreq)
		currentFreq += delta;
	else
		currentFreq -= delta;

	_ch[ch].freqLo = currentFreq & 0xFF;
	_ch[ch].freqHi = (currentFreq >> 8) & 0xFF;

	int off = kSIDOffset[ch];
	sidWrite(off + 0, _ch[ch].freqLo);
	sidWrite(off + 1, _ch[ch].freqHi);
}

void DarkSideC64MusicPlayer::applyPWModulation(int ch) {
	uint8 instBase = _ch[ch].instIdx;
	uint8 pwMod = kInstruments[instBase + 5];
	if (pwMod == 0)
		return;

	uint8 flags = kInstruments[instBase + 7];

	if (flags & 0x04) {
		_ch[ch].pwLo += pwMod;
	} else {
		uint16 pw = ((uint16)_ch[ch].pwHi << 8) | _ch[ch].pwLo;
		if (_ch[ch].pwDirection == 0) {
			pw += pwMod;
			if ((pw >> 8) >= 0x0F)
				_ch[ch].pwDirection = 1;
		} else {
			pw -= pwMod;
			if ((pw >> 8) < 0x08)
				_ch[ch].pwDirection = 0;
		}

		_ch[ch].pwLo = pw & 0xFF;
		_ch[ch].pwHi = (pw >> 8) & 0xFF;
	}
}

} // namespace Freescape
