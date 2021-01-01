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

#include "xeen/sound_driver_adlib.h"
#include "xeen/xeen.h"

namespace Xeen {

#define CALLBACKS_PER_SECOND 73

const byte SoundDriverAdlib::OPERATOR1_INDEXES[CHANNEL_COUNT] = {
	0, 1, 2, 8, 9, 0xA, 0x10, 0x11, 0x12
};

const byte SoundDriverAdlib::OPERATOR2_INDEXES[CHANNEL_COUNT] = {
	3, 4, 5, 0xB, 0xC, 0xD, 0x13, 0x14, 0x15
};

const uint SoundDriverAdlib::WAVEFORMS[24] = {
	0, 347, 388, 436, 462, 519, 582, 646,
	0, 362, 406, 455, 484, 542, 607, 680,
	0, 327, 367, 412, 436, 489, 549, 618
};

/*------------------------------------------------------------------------*/

SoundDriverAdlib::SoundDriverAdlib() : _field180(0), _field181(0), _field182(0),
		_musicVolume(0), _sfxVolume(0) {
	Common::fill(&_musInstrumentPtrs[0], &_musInstrumentPtrs[16], (const byte *)nullptr);
	Common::fill(&_fxInstrumentPtrs[0], &_fxInstrumentPtrs[16], (const byte *)nullptr);

	_opl = OPL::Config::create();
	_opl->init();
	_opl->start(new Common::Functor0Mem<void, SoundDriverAdlib>(this, &SoundDriverAdlib::onTimer), CALLBACKS_PER_SECOND);
	initialize();
}

SoundDriverAdlib::~SoundDriverAdlib() {
	_opl->stop();
	delete _opl;
}

void SoundDriverAdlib::onTimer() {
	Common::StackLock slock(_driverMutex);
	execute();
	flush();
}

void SoundDriverAdlib::initialize() {
	write(1, 0x20);
	write(8, 0);
	write(0xBD, 0);

	resetFrequencies();
	SoundDriverAdlib::resetFX();
}

void SoundDriverAdlib::playFX(uint effectId, const byte *data) {
	Common::StackLock slock(_driverMutex);
	SoundDriver::playFX(effectId, data);
}

void SoundDriverAdlib::playSong(const byte *data) {
	Common::StackLock slock(_driverMutex);
	SoundDriver::playSong(data);
	_field180 = 0;
	resetFrequencies();
}

int SoundDriverAdlib::songCommand(uint commandId, byte musicVolume, byte sfxVolume) {
	Common::StackLock slock(_driverMutex);
	SoundDriver::songCommand(commandId, musicVolume, sfxVolume);

	if (commandId == STOP_SONG) {
		_field180 = 0;
		resetFrequencies();
	} else if (commandId == RESTART_SONG) {
		_field180 = 0;
		_streams[stMUSIC]._playing = true;
	} else if (commandId < 0x100) {
		if (_streams[stMUSIC]._playing) {
			_field180 = commandId;
			_field182 = 63;
		}
	} else if (commandId == SET_VOLUME) {
		_musicVolume = musicVolume;
		_sfxVolume = sfxVolume;
	} else if (commandId == GET_STATUS) {
		return _field180;
	}

	return 0;
}

void SoundDriverAdlib::write(int reg, int val) {
	_queue.push(RegisterValue(reg, val));
	debugC(9, kDebugSound, "%.2x %.2x", reg, val);
}

void SoundDriverAdlib::flush() {
	Common::StackLock slock(_driverMutex);

	while (!_queue.empty()) {
		RegisterValue v = _queue.pop();
		_opl->writeReg(v._regNum, v._value);
	}
}

void SoundDriverAdlib::pausePostProcess() {
	if (_field180 && ((_field181 += _field180) < 0)) {
		if (--_field182 < 0) {
			_streams[stMUSIC]._playing = false;
			_field180 = 0;
			resetFrequencies();
		} else {
			for (int channelNum = 6; channelNum >= 0; --channelNum) {
				if (_channels[channelNum]._volume < 63)
					setOutputLevel(channelNum, ++_channels[channelNum]._volume);
			}
		}
	}

	for (int channelNum = 8; channelNum > 6; --channelNum) {
		Channel &chan = _channels[channelNum];
		if (!chan._changeFrequency || (chan._freqCtr += chan._freqCtrChange) >= 0)
			continue;

		uint freq = chan._frequency & 0x3FF;
		uint val = chan._frequency >> 8;
		byte val1 = val & 0x20;
		byte val2 = val & 0x1C;

		freq += chan._freqChange;
		if (chan._freqChange < 0) {
			if (freq <= 388) {
				freq <<= 1;
				if (!(freq & 0x3FF))
					--freq;
			}

			val2 = (val2 - 4) & 0x1C;
		} else {
			if (freq >= 734) {
				freq >>= 1;
				if (!(freq & 0x3FF))
					++freq;
			}

			val2 = (val2 + 4) & 0x1C;
		}

		freq &= 0x3FF;
		freq |= (val2 << 8);
		freq |= val1;
		chan._frequency = freq;
		setFrequency(channelNum, freq);
	}
}

void SoundDriverAdlib::resetFX() {
	_channels[7]._frequency = 0;
	setFrequency(7, 0);
	_channels[7]._volume = 63;
	setOutputLevel(7, 63);

	_channels[8]._frequency = 0;
	setFrequency(8, 0);
	_channels[8]._volume = 63;
	setOutputLevel(8, 63);
}

void SoundDriverAdlib::resetFrequencies() {
	for (int opNum = 6; opNum >= 0; --opNum) {
		_channels[opNum]._frequency = 0;
		setFrequency(opNum, 0);
	}
}

void SoundDriverAdlib::setFrequency(byte operatorNum, uint frequency) {
	write(0xA0 + operatorNum, frequency & 0xff);
	write(0xB0 + operatorNum, (frequency >> 8));
}

uint SoundDriverAdlib::calcFrequency(byte note) {
	return WAVEFORMS[note & 0x1F] + ((note & 0xE0) << 5);
}

void SoundDriverAdlib::setOutputLevel(byte channelNum, uint level) {
	Channel &c = _channels[channelNum];
	write(0x40 + OPERATOR2_INDEXES[channelNum], calculateLevel(level, c._isFx) | (c._totalLevel & 0xC0));
}

void SoundDriverAdlib::playInstrument(byte channelNum, const byte *data, bool isFx) {
	byte op1 = OPERATOR1_INDEXES[channelNum];
	byte op2 = OPERATOR2_INDEXES[channelNum];
	int totalLevel;

	debugC(2, kDebugSound, "---START-playInstrument - %d", channelNum);
	_channels[channelNum]._isFx = isFx;
	write(0x20 + op1, *data++);

	totalLevel = *data++;
	write(0x40 + op1, calculateLevel(totalLevel, isFx));

	write(0x60 + op1, *data++);
	write(0x80 + op1, *data++);
	write(0xE0 + op1, *data++);
	write(0x20 + op2, *data++);

	totalLevel = *data++;
	_channels[channelNum]._totalLevel = totalLevel;

	if (totalLevel > 63) {
		totalLevel = 63;
		if (_field180)
			totalLevel = (totalLevel & 0xC0) | _field182;
	}

	write(0x40 + op2, calculateLevel(totalLevel, isFx));

	write(0x60 + op2, *data++);
	write(0x80 + op2, *data++);
	write(0xE0 + op2, *data++);
	write(0xC0 + channelNum, *data++);

	debugC(2, kDebugSound, "---END-playInstrument");
}

bool SoundDriverAdlib::musSetInstrument(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "musSetInstrument %d", param);
	_musInstrumentPtrs[param] = srcP;
	srcP += 26;

	return false;
}

bool SoundDriverAdlib::musSetPitchWheel(const byte *&srcP, byte param) {
	// Adlib does not support this
	debugC(3, kDebugSound, "musSetPitchWheel");
	srcP += 2;
	return false;
}

bool SoundDriverAdlib::musSetPanning(const byte *&srcP, byte param) {
	// Adlib does not support this
	debugC(3, kDebugSound, "musSetPanning");
	++srcP;
	return false;
}

bool SoundDriverAdlib::musFade(const byte *&srcP, byte param) {
	++srcP;
	if (param < 7)
		setFrequency(param, _channels[param]._frequency);
	debugC(3, kDebugSound, "musFade");

	return false;
}

bool SoundDriverAdlib::musStartNote(const byte *&srcP, byte param) {
	if (param < 7) {
		byte note = *srcP++;
		++srcP;		// Second byte is fade, which is unused by Adlib
		uint freq = calcFrequency(note);
		debugC(3, kDebugSound, "musStartNote %x -> %x", note, freq);

		setFrequency(param, freq);
		freq |= 0x2000;
		_channels[param]._frequency = freq;
		setFrequency(param, freq);
	} else {
		srcP += 2;
		debugC(3, kDebugSound, "musStartNote skipped");
	}

	return false;
}

bool SoundDriverAdlib::musSetVolume(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "musSetVolume %d", (int)*srcP);

	if (*srcP++ == 5 && !_field180) {
		_channels[param]._volume = *srcP;
		setOutputLevel(param, *srcP);
	}

	++srcP;
	return false;
}

bool SoundDriverAdlib::musInjectMidi(const byte *&srcP, byte param) {
	// Adlib does not support MIDI. So simply keep skipping over bytes
	// until an 'F7' byte is found that flags the end of the MIDI data
	debugC(3, kDebugSound, "musInjectMidi");
	while (*srcP++ != 0xF7)
		;

	return false;
}

bool SoundDriverAdlib::musPlayInstrument(const byte *&srcP, byte param) {
	byte instrument = *srcP++;
	debugC(3, kDebugSound, "musPlayInstrument %d, %d", param, instrument);

	if (param < 7)
		playInstrument(param, _musInstrumentPtrs[instrument], false);

	return false;
}

bool SoundDriverAdlib::fxSetInstrument(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxSetInstrument %d", param);
	_fxInstrumentPtrs[param] = srcP;
	srcP += 11;

	return false;
}

bool SoundDriverAdlib::fxSetVolume(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxSetVolume %d", (int)*srcP);

	if (!_field180) {
		_channels[param]._volume = *srcP;
		setOutputLevel(param, *srcP);
	}

	++srcP;
	return false;
}

bool SoundDriverAdlib::fxMidiReset(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxMidiReset");
	return false;
}

bool SoundDriverAdlib::fxMidiDword(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxMidiDword");
	return false;
}

bool SoundDriverAdlib::fxSetPanning(const byte *&srcP, byte param) {
	byte note = *srcP++;
	debugC(3, kDebugSound, "fxSetPanning - %x", note);

	uint freq = calcFrequency(note);
	setFrequency(param, freq);
	_channels[param]._frequency = freq;

	return false;
}

bool SoundDriverAdlib::fxChannelOff(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxChannelOff %d", param);
	_channels[param]._frequency &= ~0x2000;
	write(0xB0 + param, _channels[param]._frequency);
	return false;
}

bool SoundDriverAdlib::fxFade(const byte *&srcP, byte param) {
	uint freq = calcFrequency(*srcP++);
	debugC(3, kDebugSound, "fxFade %d %x", param, freq);

	_channels[param]._frequency = freq;
	setFrequency(param, freq);

	return false;
}

bool SoundDriverAdlib::fxStartNote(const byte *&srcP, byte param) {
	byte note = *srcP++;
	uint freq = calcFrequency(note);
	debugC(3, kDebugSound, "fxStartNote %x -> %x", note, freq);

	setFrequency(param, freq);
	freq |= 0x2000;
	_channels[param]._frequency = freq;
	setFrequency(param, freq);

	return false;
}

bool SoundDriverAdlib::fxInjectMidi(const byte *&srcP, byte param) {
	// Surpringly, unlike the musInjectMidi, this version doesn't have
	// any logic to skip over following MIDI data. Which must mean the opcode
	// and/or it's data aren't present in the admus driver file
	debugC(3, kDebugSound, "fxInjectMidi");
	return false;
}

bool SoundDriverAdlib::fxPlayInstrument(const byte *&srcP, byte param) {
	byte instrument = *srcP++;
	debugC(3, kDebugSound, "fxPlayInstrument %d, %d", param, instrument);

	playInstrument(param, _fxInstrumentPtrs[instrument], true);

	return false;
}

byte SoundDriverAdlib::calculateLevel(byte level, bool isFx) {
	uint volume = isFx ? _sfxVolume : _musicVolume;
	uint scaling = level & 0xc0;
	uint totalLevel = 0x3f - (level & 0x3f);
	totalLevel = totalLevel * volume / 255;

	return scaling | (0x3f - totalLevel);
}

} // End of namespace Xeen
