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

#include "common/md5.h"
#include "common/config-manager.h"
#include "xeen/sound_driver.h"
#include "xeen/xeen.h"
#include "xeen/files.h"

namespace Xeen {

#define CALLBACKS_PER_SECOND 73

/*------------------------------------------------------------------------*/

SoundDriver::SoundDriver() : _musicPlaying(false), _fxPlaying(false),
		_musCountdownTimer(0), _fxCountdownTimer(0), _musDataPtr(nullptr),
		_fxDataPtr(nullptr), _fxStartPtr(nullptr), _musStartPtr(nullptr),
		_exclude7(false), _frameCtr(0) {
	_channels.resize(CHANNEL_COUNT);
}

SoundDriver::~SoundDriver() {
	_musicPlaying = _fxPlaying = false;
	_musCountdownTimer = _fxCountdownTimer = 0;
}

void SoundDriver::execute() {
	bool isFX = false;
	const byte *srcP = nullptr;
	const byte *startP = nullptr;

	// Single iteration loop to avoid use of GOTO
	do {
		if (_musicPlaying) {
			startP = _musStartPtr;
			srcP = _musDataPtr;
			isFX = false;
			if (_musCountdownTimer == 0 || --_musCountdownTimer == 0)
				break;
		}

		if (_fxPlaying) {
			startP = _fxStartPtr;
			srcP = _fxDataPtr;
			isFX = true;
			if (_fxCountdownTimer == 0 || --_fxCountdownTimer == 0)
				break;
		}

		pausePostProcess();
		return;
	} while (0);

	++_frameCtr;
	debugC(3, kDebugSound, "\nSoundDriver frame - #%x", _frameCtr);

	// Main loop
	bool breakFlag = false;
	while (!breakFlag) {
		debugCN(3, kDebugSound, "MUSCODE %.4x - %.2x  ", (uint)(srcP - startP), (uint)*srcP);
		byte nextByte = *srcP++;
		int cmd = (nextByte >> 4) & 15;
		int param = (nextByte & 15);

		CommandFn fn = isFX ? FX_COMMANDS[cmd] : MUSIC_COMMANDS[cmd];
		breakFlag = (this->*fn)(srcP, param);
	}
}


bool SoundDriver::musCallSubroutine(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "musCallSubroutine");
	if (_musSubroutines.size() < 16) {
		const byte *returnP = srcP + 2;
		srcP = _musStartPtr + READ_LE_UINT16(srcP);

		_musSubroutines.push(Subroutine(returnP, srcP));
	}

	return false;
}

bool SoundDriver::musSetCountdown(const byte *&srcP, byte param) {
	// Set the countdown timer
	if (!param)
		param = *srcP++;
	_musCountdownTimer = param;
	_musDataPtr = srcP;
	debugC(3, kDebugSound, "musSetCountdown %d", param);

	// Do paused handling and break out of processing loop
	pausePostProcess();
	return true;
}

bool SoundDriver::cmdNoOperation(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "cmdNoOperation");
	return false;
}

bool SoundDriver::musSkipWord(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "musSkipWord");
	srcP += 2;
	return false;
}

bool SoundDriver::cmdFreezeFrequency(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "cmdFreezeFrequency %d", param);
	if (param >= _channels.size())
		warning("Invalid channel %d in cmdFreezeFrequency call", param);
	else
		_channels[param]._changeFrequency = false;
	return false;
}

bool SoundDriver::cmdChangeFrequency(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "cmdChangeFrequency %d", param);

	if (param != 7 || !_exclude7) {
		_channels[param]._freqCtrChange = (int8)*srcP++;
		_channels[param]._freqCtr = 0xFF;
		_channels[param]._changeFrequency = true;
		_channels[param]._freqChange = (int16)READ_BE_UINT16(srcP);
		srcP += 2;
	} else {
		srcP += 3;
	}

	return false;
}

bool SoundDriver::musEndSubroutine(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "musEndSubroutine %d", param);

	if (param != 15) {
		// Music has ended, so flag it stopped
		_musicPlaying = false;
		return true;
	}

	// Returning from subroutine, or looping back to start of music
	srcP = _musSubroutines.empty() ? _musStartPtr : _musSubroutines.pop()._returnP;
	return false;
}

bool SoundDriver::fxCallSubroutine(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxCallSubroutine");

	if (_fxSubroutines.size() < 16) {
		const byte *startP = srcP + 2;
		srcP = _musStartPtr + READ_LE_UINT16(srcP);

		_fxSubroutines.push(Subroutine(startP, srcP));
	}

	return false;
}

bool SoundDriver::fxSetCountdown(const byte *&srcP, byte param) {
	// Set the countdown timer
	if (!param)
		param = *srcP++;
	_fxCountdownTimer = param;
	_fxDataPtr = srcP;
	debugC(3, kDebugSound, "fxSetCountdown %d", param);

	// Do paused handling and break out of processing loop
	pausePostProcess();
	return true;
}

bool SoundDriver::fxEndSubroutine(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxEndSubroutine %d", param);

	if (param != 15) {
		// FX has ended, so flag it stopped
		_fxPlaying = false;
		return true;
	}

	srcP = _fxSubroutines.empty() ? _fxStartPtr : _fxSubroutines.pop()._returnP;
	return false;
}

void SoundDriver::playFX(uint effectId, const byte *data) {
	if (!_fxPlaying || effectId < 7 || effectId >= 11) {
		_fxDataPtr = _fxStartPtr = data;
		_fxCountdownTimer = 0;
		_channels[7]._changeFrequency = _channels[8]._changeFrequency = false;
		resetFX();
		_fxPlaying = true;
	}

	debugC(1, kDebugSound, "Starting FX %d", effectId);
}

void SoundDriver::stopFX() {
	resetFX();
	_fxPlaying = false;
	_fxStartPtr = _fxDataPtr = nullptr;
}

void SoundDriver::playSong(const byte *data) {
	_musDataPtr = _musStartPtr = data;
	_musSubroutines.clear();
	_musCountdownTimer = 0;
	_musicPlaying = true;
	debugC(1, kDebugSound, "Starting song");
}

int SoundDriver::songCommand(uint commandId, byte volume) {
	if (commandId == STOP_SONG) {
		_musicPlaying = false;
	} else if (commandId == RESTART_SONG) {
		_musicPlaying = true;
		_musDataPtr = nullptr;
		_musSubroutines.clear();
	}

	return 0;
}

const CommandFn SoundDriver::MUSIC_COMMANDS[16] = {
	&SoundDriver::musCallSubroutine,   &SoundDriver::musSetCountdown,
	&SoundDriver::musSetInstrument,    &SoundDriver::cmdNoOperation,
	&SoundDriver::musSetPitchWheel,    &SoundDriver::musSkipWord,
	&SoundDriver::musSetPanning,       &SoundDriver::cmdNoOperation,
	&SoundDriver::musFade,             &SoundDriver::musStartNote,
	&SoundDriver::musSetVolume,        &SoundDriver::musInjectMidi,
	&SoundDriver::musPlayInstrument,   &SoundDriver::cmdFreezeFrequency,
	&SoundDriver::cmdChangeFrequency,  &SoundDriver::musEndSubroutine
};

const CommandFn SoundDriver::FX_COMMANDS[16] = {
	&SoundDriver::fxCallSubroutine,    &SoundDriver::fxSetCountdown,
	&SoundDriver::fxSetInstrument,     &SoundDriver::fxSetVolume,
	&SoundDriver::fxMidiReset,         &SoundDriver::fxMidiDword,
	&SoundDriver::fxSetPanning,        &SoundDriver::fxChannelOff,
	&SoundDriver::fxFade,              &SoundDriver::fxStartNote,
	&SoundDriver::cmdNoOperation,      &SoundDriver::fxInjectMidi,
	&SoundDriver::fxPlayInstrument,    &SoundDriver::cmdFreezeFrequency,
	&SoundDriver::cmdChangeFrequency,  &SoundDriver::fxEndSubroutine
};

/*------------------------------------------------------------------------*/

AdlibSoundDriver::AdlibSoundDriver() : _field180(0), _field181(0), _field182(0),
		_volume(127) {
	Common::fill(&_musInstrumentPtrs[0], &_musInstrumentPtrs[16], (const byte *)nullptr);
	Common::fill(&_fxInstrumentPtrs[0], &_fxInstrumentPtrs[16], (const byte *)nullptr);

	_opl = OPL::Config::create();
	_opl->init();
	_opl->start(new Common::Functor0Mem<void, AdlibSoundDriver>(this, &AdlibSoundDriver::onTimer), CALLBACKS_PER_SECOND);
	initialize();
}

AdlibSoundDriver::~AdlibSoundDriver() {
	_opl->stop();
	delete _opl;
}

void AdlibSoundDriver::onTimer() {
	Common::StackLock slock(_driverMutex);
	execute();
	flush();
}

void AdlibSoundDriver::initialize() {
	write(1, 0x20);
	write(8, 0);
	write(0xBD, 0);

	resetFrequencies();
	AdlibSoundDriver::resetFX();
}

void AdlibSoundDriver::playFX(uint effectId, const byte *data) {
	Common::StackLock slock(_driverMutex);
	SoundDriver::playFX(effectId, data);
}

void AdlibSoundDriver::playSong(const byte *data) {
	Common::StackLock slock(_driverMutex);
	SoundDriver::playSong(data);
	_field180 = 0;
	resetFrequencies();
}

int AdlibSoundDriver::songCommand(uint commandId, byte volume) {
	Common::StackLock slock(_driverMutex);
	SoundDriver::songCommand(commandId, volume);

	if (commandId == STOP_SONG) {
		_field180 = 0;
		resetFrequencies();
	} else if (commandId == RESTART_SONG) {
		_field180 = 0;
		_musicPlaying = true;
	} else if (commandId < 0x100) {
		if (_musicPlaying) {
			_field180 = commandId;
			_field182 = 63;
		}
	} else if (commandId == SET_VOLUME) {
		_volume = volume;
	} else if (commandId == GET_STATUS) {
		return _field180;
	}

	return 0;
}

void AdlibSoundDriver::write(int reg, int val) {
	_queue.push(RegisterValue(reg, val));
	debugC(9, kDebugSound, "%.2x %.2x", reg, val);
}

void AdlibSoundDriver::flush() {
	Common::StackLock slock(_driverMutex);

	while (!_queue.empty()) {
		RegisterValue v = _queue.pop();
		_opl->writeReg(v._regNum, v._value);
	}
}

void AdlibSoundDriver::pausePostProcess() {
	if (_field180 && ((_field181 += _field180) < 0)) {
		if (--_field182 < 0) {
			_musicPlaying = false;
			_field180 = 0;
			resetFrequencies();
		} else {
			for (int channelNum = 6; channelNum >= 0; --channelNum) {
				if (_channels[channelNum]._volume < 63)
					setOutputLevel(channelNum, ++_channels[channelNum]._volume);
			}
		}
	}

	for (int channelNum = 8; channelNum > (_exclude7 ? 7 : 6); --channelNum) {
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

void AdlibSoundDriver::resetFX() {
	if (!_exclude7) {
		_channels[7]._frequency = 0;
		setFrequency(7, 0);
		_channels[7]._volume = 63;
		setOutputLevel(7, 63);
	}

	_channels[8]._frequency = 0;
	setFrequency(8, 0);
	_channels[8]._volume = 63;
	setOutputLevel(8, 63);
}

void AdlibSoundDriver::resetFrequencies() {
	for (int opNum = 6; opNum >= 0; --opNum) {
		_channels[opNum]._frequency = 0;
		setFrequency(opNum, 0);
	}
}

void AdlibSoundDriver::setFrequency(byte operatorNum, uint frequency) {
	write(0xA0 + operatorNum, frequency & 0xff);
	write(0xB0 + operatorNum, (frequency >> 8));
}

uint AdlibSoundDriver::calcFrequency(byte note) {
	return WAVEFORMS[note & 0x1F] + ((note & 0xE0) << 5);
}

void AdlibSoundDriver::setOutputLevel(byte channelNum, uint level) {
	write(0x40 + OPERATOR2_INDEXES[channelNum], level |
		(_channels[channelNum]._scalingValue & 0xC0));
}

void AdlibSoundDriver::playInstrument(byte channelNum, const byte *data) {
	byte op1 = OPERATOR1_INDEXES[channelNum];
	byte op2 = OPERATOR2_INDEXES[channelNum];
	debugC(2, kDebugSound, "---START-playInstrument - %d", channelNum);
	write(0x20 + op1, *data++);
	write(0x40 + op1, *data++);
	write(0x60 + op1, *data++);
	write(0x80 + op1, *data++);
	write(0xE0 + op1, *data++);
	write(0x20 + op2, *data++);

	int scalingVal = *data++;
	_channels[channelNum]._scalingValue = scalingVal;
	scalingVal += (127 - _volume) / 2;

	if (scalingVal > 63) {
		scalingVal = 63;
		if (_field180)
			scalingVal = (scalingVal & 0xC0) | _field182;
	}
	write(0x40 + op2, scalingVal);

	write(0x60 + op2, *data++);
	write(0x80 + op2, *data++);
	write(0xE0 + op2, *data++);
	write(0xC0 + channelNum, *data++);

	debugC(2, kDebugSound, "---END-playInstrument");
}

bool AdlibSoundDriver::musSetInstrument(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "musSetInstrument %d", param);
	_musInstrumentPtrs[param] = srcP;
	srcP += 26;

	return false;
}

bool AdlibSoundDriver::musSetPitchWheel(const byte *&srcP, byte param) {
	// Adlib does not support this
	debugC(3, kDebugSound, "musSetPitchWheel");
	srcP += 2;
	return false;
}

bool AdlibSoundDriver::musSetPanning(const byte *&srcP, byte param) {
	// Adlib does not support this
	debugC(3, kDebugSound, "musSetPanning");
	++srcP;
	return false;
}

bool AdlibSoundDriver::musFade(const byte *&srcP, byte param) {
	++srcP;
	if (param < 7)
		setFrequency(param, _channels[param]._frequency);
	debugC(3, kDebugSound, "musFade");

	return false;
}

bool AdlibSoundDriver::musStartNote(const byte *&srcP, byte param) {
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

bool AdlibSoundDriver::musSetVolume(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "musSetVolume %d", (int)*srcP);

	if (*srcP++ == 5 && !_field180) {
		_channels[param]._volume = *srcP;
		setOutputLevel(param, *srcP);
	}

	++srcP;
	return false;
}

bool AdlibSoundDriver::musInjectMidi(const byte *&srcP, byte param) {
	// Adlib does not support MIDI. So simply keep skipping over bytes
	// until an 'F7' byte is found that flags the end of the MIDI data
	debugC(3, kDebugSound, "musInjectMidi");
	while (*srcP++ != 0xF7)
		;

	return false;
}

bool AdlibSoundDriver::musPlayInstrument(const byte *&srcP, byte param) {
	byte instrument = *srcP++;
	debugC(3, kDebugSound, "musPlayInstrument %d, %d", param, instrument);

	if (param < 7)
		playInstrument(param, _musInstrumentPtrs[instrument]);

	return false;
}

bool AdlibSoundDriver::fxSetInstrument(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxSetInstrument %d", param);
	_fxInstrumentPtrs[param] = srcP;
	srcP += 11;

	return false;
}

bool AdlibSoundDriver::fxSetVolume(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxSetVolume %d", (int)*srcP);

	if (!_field180 && (!_exclude7 || param != 7)) {
		_channels[param]._volume = *srcP;
		setOutputLevel(param, *srcP);
	}

	++srcP;
	return false;
}

bool AdlibSoundDriver::fxMidiReset(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxMidiReset");
	return false;
}

bool AdlibSoundDriver::fxMidiDword(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxMidiDword");
	return false;
}

bool AdlibSoundDriver::fxSetPanning(const byte *&srcP, byte param) {
	byte note = *srcP++;
	debugC(3, kDebugSound, "fxSetPanning - %x", note);

	if (!_exclude7 || param != 7) {
		uint freq = calcFrequency(note);
		setFrequency(param, freq);
		_channels[param]._frequency = freq;
	}

	return false;
}

bool AdlibSoundDriver::fxChannelOff(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxChannelOff %d", param);
	_channels[param]._frequency &= ~0x2000;
	write(0xB0 + param, _channels[param]._frequency);
	return false;
}

bool AdlibSoundDriver::fxFade(const byte *&srcP, byte param) {
	uint freq = calcFrequency(*srcP++);
	debugC(3, kDebugSound, "fxFade %d %x", param, freq);

	if (!_exclude7 || param != 7) {
		_channels[param]._frequency = freq;
		setFrequency(param, freq);
	}

	return false;
}

bool AdlibSoundDriver::fxStartNote(const byte *&srcP, byte param) {
	if (!_exclude7 || param != 7) {
		byte note = *srcP++;
		uint freq = calcFrequency(note);
		debugC(3, kDebugSound, "fxStartNote %x -> %x", note, freq);

		setFrequency(param, freq);
		freq |= 0x2000;
		_channels[param]._frequency = freq;
		setFrequency(param, freq);
	} else {
		++srcP;
		debugC(3, kDebugSound, "fxStartNote skipped");
	}

	return false;
}

bool AdlibSoundDriver::fxInjectMidi(const byte *&srcP, byte param) {
	// Surpringly, unlike the musInjectMidi, this version doesn't have
	// any logic to skip over following MIDI data. Which must mean the opcode
	// and/or it's data aren't present in the admus driver file
	debugC(3, kDebugSound, "fxInjectMidi");
	return false;
}

bool AdlibSoundDriver::fxPlayInstrument(const byte *&srcP, byte param) {
	byte instrument = *srcP++;
	debugC(3, kDebugSound, "fxPlayInstrument %d, %d", param, instrument);

	if (!_exclude7 || param != 7)
		playInstrument(param, _fxInstrumentPtrs[instrument]);

	return false;
}

/*------------------------------------------------------------------------*/

const byte AdlibSoundDriver::OPERATOR1_INDEXES[CHANNEL_COUNT] = {
	0, 1, 2, 8, 9, 0xA, 0x10, 0x11, 0x12
};

const byte AdlibSoundDriver::OPERATOR2_INDEXES[CHANNEL_COUNT] = {
	3, 4, 5, 0xB, 0xC, 0xD, 0x13, 0x14, 0x15
};

const uint AdlibSoundDriver::WAVEFORMS[24] = {
	0, 347, 388, 436, 462, 519, 582, 646,
	0, 362, 406, 455, 484, 542, 607, 680,
	0, 327, 367, 412, 436, 489, 549, 618
};

} // End of namespace Xeen
