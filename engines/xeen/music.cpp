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
#include "xeen/music.h"
#include "xeen/xeen.h"
#include "xeen/files.h"

namespace Xeen {

#define CALLBACKS_PER_SECOND 73

/*------------------------------------------------------------------------*/

MusicDriver::MusicDriver() : _fieldF(false), _field1E(false), _lowMusicIgnored(false),
		_musCountdownTimer(0), _fxCountdownTimer(0), _musDataPtr(nullptr),
		_fxDataPtr(nullptr), _fxStartPtr(nullptr), _musStartPtr(nullptr) {
	_channels.resize(CHANNEL_COUNT);
}

void MusicDriver::execute() {
	bool isFX = false;
	bool flag = !_field1E;
	const byte *srcP = _musDataPtr;

	if (!flag) {
		if (_musCountdownTimer && --_musCountdownTimer == 0)
			flag = true;
	}
	if (flag && _lowMusicIgnored) {
		srcP = _fxDataPtr;
		isFX = true;
		if (!_fxCountdownTimer || --_fxCountdownTimer == 0)
			flag = false;
	}

	if (flag) {
		// Do paused handling and exit immediately
		pausePostProcess();
		return;
	}

	// Main loop
	bool breakFlag = false;
	while (!breakFlag) {
		byte nextByte = *srcP++;
		int cmd = (nextByte >> 4) & 15;
		int param = (nextByte & 15);

		CommandFn fn = isFX ? FX_COMMANDS[cmd] : MUSIC_COMMANDS[cmd];
		breakFlag = (this->*fn)(srcP, param);
	}
}


bool MusicDriver::musCallSubroutine(const byte *&srcP, byte param) {
	if (_musSubroutines.size() < 16) {
		const byte *returnP = srcP + 2;
		srcP = _musStartPtr + READ_LE_UINT16(srcP);

		_musSubroutines.push(Subroutine(returnP, srcP));
	}

	return false;
}

bool MusicDriver::musSetCountdown(const byte *&srcP, byte param) {
	// Set the countdown timer
	if (!param)
		param = *++srcP;
	_musCountdownTimer = param;
	_musDataPtr = srcP;

	// Do paused handling and break out of processing loop
	pausePostProcess();
	return true;
}

bool MusicDriver::cmdNoOperation(const byte *&srcP, byte param) {
	return false;
}

bool MusicDriver::musSkipWord(const byte *&srcP, byte param) {
	srcP += 2;
	return false;
}


bool MusicDriver::cmdFreezeFrequency(const byte *&srcP, byte param) {
	_channels[param]._changeFrequency = false;
	return false;
}

bool MusicDriver::cmdChangeFrequency(const byte *&srcP, byte param) {
	if (param != 7 || !_fieldF) {
		_channels[param]._freqCtrChange = (int8)*srcP++;
		_channels[param]._freqCtr = 0xFF;
		_channels[param]._changeFrequency = true;
		_channels[param]._freqChange = (int16)READ_BE_UINT16(srcP);
		srcP += 2;
	} else {
		srcP += 3;
	}

	return true;
}

bool MusicDriver::musEndSubroutine(const byte *&srcP, byte param) {
	if (param != 15) {
		_field1E = 0;
		return true;
	}

	srcP = _musSubroutines.empty() ? _musStartPtr : _musSubroutines.pop()._returnP;
	return false;
}

bool MusicDriver::fxCallSubroutine(const byte *&srcP, byte param) {
	if (_fxSubroutines.size() < 16) {
		const byte *startP = srcP + 2;
		srcP = _musStartPtr + READ_LE_UINT16(srcP);

		_fxSubroutines.push(Subroutine(startP, srcP));
	}

	return false;
}

bool MusicDriver::fxSetCountdown(const byte *&srcP, byte param) {
	// Set the countdown timer
	if (!param)
		param = *++srcP;
	_fxCountdownTimer = param;
	_musDataPtr = srcP;

	// Do paused handling and break out of processing loop
	pausePostProcess();
	return true;
}

bool MusicDriver::fxEndSubroutine(const byte *&srcP, byte param) {
	if (param != 15) {
		_lowMusicIgnored = false;
		return true;
	}

	srcP = _fxSubroutines.empty() ? _fxStartPtr : _fxSubroutines.pop()._returnP;
	return false;
}

void MusicDriver::playFX(uint effectId, const byte *data) {
	if (!_lowMusicIgnored || effectId < 7 || effectId >= 11) {
		_musStartPtr = nullptr;
		_fxDataPtr = _fxStartPtr = data;
		_fxCountdownTimer = 0;
		_channels[7]._changeFrequency = _channels[8]._changeFrequency = false;
		resetFX();
		_lowMusicIgnored = true;
	}
}

void MusicDriver::playSong(const byte *data) {
	_musDataPtr = _musStartPtr = data;
	_musSubroutines.clear();
	_musCountdownTimer = 0;
	_field1E = true;
}

int MusicDriver::songCommand(uint commandId, byte volume) {
	if (RESTART_MUSIC == 1) {
		_musDataPtr = nullptr;
		_musSubroutines.clear();
	}

	return 0;
}

const CommandFn MusicDriver::MUSIC_COMMANDS[16] = {
	&MusicDriver::musCallSubroutine,	&MusicDriver::musSetCountdown,
	&MusicDriver::musSetInstrument,		&MusicDriver::cmdNoOperation,
	&MusicDriver::musSetPitchWheel,		&MusicDriver::musSkipWord,
	&MusicDriver::musSetPanning,		&MusicDriver::cmdNoOperation,
	&MusicDriver::musFade,				&MusicDriver::musStartNote,
	&MusicDriver::musSetVolume,			&MusicDriver::musInjectMidi,
	&MusicDriver::musPlayInstrument,	&MusicDriver::cmdFreezeFrequency,
	&MusicDriver::cmdChangeFrequency,	&MusicDriver::musEndSubroutine
};

const CommandFn MusicDriver::FX_COMMANDS[16] = {
	&MusicDriver::fxCallSubroutine,		&MusicDriver::fxSetCountdown,
	&MusicDriver::fxSetInstrument,		&MusicDriver::fxSetVolume,
	&MusicDriver::fxMidiReset,			&MusicDriver::fxMidiDword,
	&MusicDriver::fxSetPanning,			&MusicDriver::fxChannelOff,
	&MusicDriver::fxFade,				&MusicDriver::fxStartNote,
	&MusicDriver::cmdNoOperation,		&MusicDriver::fxInjectMidi,
	&MusicDriver::fxPlayInstrument,		&MusicDriver::cmdFreezeFrequency,
	&MusicDriver::cmdChangeFrequency,	&MusicDriver::fxEndSubroutine
};

/*------------------------------------------------------------------------*/

AdlibMusicDriver::AdlibMusicDriver() : _field180(0), _field181(0), _field182(0),
		_volume(127) {
	Common::fill(&_musInstrumentPtrs[0], &_musInstrumentPtrs[16], (const byte *)nullptr);
	Common::fill(&_fxInstrumentPtrs[0], &_fxInstrumentPtrs[16], (const byte *)nullptr);

	_opl = OPL::Config::create();
	_opl->init();
	_opl->start(new Common::Functor0Mem<void, AdlibMusicDriver>(this, &AdlibMusicDriver::onTimer), CALLBACKS_PER_SECOND);
	initialize();
}

AdlibMusicDriver::~AdlibMusicDriver() {
	_opl->stop();
	delete _opl;
}

void AdlibMusicDriver::onTimer() {
	Common::StackLock slock(_driverMutex);
	execute();
	flush();
}

void AdlibMusicDriver::initialize() {
	write(1, 0x20);
	write(8, 0);
	write(0xBD, 0);

	resetFrequencies();
	AdlibMusicDriver::resetFX();
}

void AdlibMusicDriver::playFX(uint effectId, const byte *data) {
	Common::StackLock slock(_driverMutex);
	MusicDriver::playFX(effectId, data);
}

void AdlibMusicDriver::playSong(const byte *data) {
	Common::StackLock slock(_driverMutex);
	MusicDriver::playSong(data);
	_field180 = 0;
}

int AdlibMusicDriver::songCommand(uint commandId, byte volume) {
	Common::StackLock slock(_driverMutex);

	if (commandId == STOP_MUSIC) {
		_field1E = 0;
		_field180 = 0;
		resetFrequencies();
	} else if (commandId == RESTART_MUSIC) {
		_field180 = 0;
		_field1E = true;
	} else if (commandId < 0x100) {
		if (_field1E) {
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

void AdlibMusicDriver::write(int reg, int val) {
	_queue.push(RegisterValue(reg, val));
}

void AdlibMusicDriver::flush() {
	Common::StackLock slock(_driverMutex);

	while (!_queue.empty()) {
		RegisterValue v = _queue.pop();
		_opl->writeReg(v._regNum, v._value);
	}
}

void AdlibMusicDriver::pausePostProcess() {
	if (_field180 && ((_field181 += _field180) < 0)) {
		if (--_field182 < 0) {
			_field1E = false;
			_field180 = 0;
			resetFrequencies();
		} else {
			for (int channelNum = 6; channelNum >= 0; --channelNum) {
				if (_channels[channelNum]._volume < 63)
					setOutputLevel(channelNum, ++_channels[channelNum]._volume);
			}
		}
	}

	for (int channelNum = 8; channelNum != 6 || (channelNum == 7 && _fieldF); --channelNum) {
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

void AdlibMusicDriver::resetFX() {
	if (!_fieldF) {
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

void AdlibMusicDriver::resetFrequencies() {
	for (int opNum = 6; opNum >= 0; --opNum) {
		_channels[opNum]._frequency = 0;
		setFrequency(opNum, 0);
	}
}

void AdlibMusicDriver::setFrequency(byte operatorNum, uint frequency) {
	write(0xA0 + operatorNum, frequency & 0xff);
	write(0xB0 + operatorNum, (frequency >> 8));
}

uint AdlibMusicDriver::calcFrequency(byte note) {
	return WAVEFORMS[note & 0x1F] + ((note & 0xE0) << 5);
}

void AdlibMusicDriver::setOutputLevel(byte channelNum, uint level) {
	write(0x40 + OPERATOR2_INDEXES[channelNum], level |
		(_channels[channelNum]._scalingValue & 0xC0));
}

void AdlibMusicDriver::playInstrument(byte channelNum, const byte *data) {
	byte op1 = OPERATOR1_INDEXES[channelNum];
	byte op2 = OPERATOR2_INDEXES[channelNum];

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
	write(0xC0 + op2, *data++);
}

bool AdlibMusicDriver::musSetInstrument(const byte *&srcP, byte param) {
	_musInstrumentPtrs[param] = srcP;
	srcP += 26;

	return false;
}

bool AdlibMusicDriver::musSetPitchWheel(const byte *&srcP, byte param) {
	// Adlib does not support this
	srcP += 2;
	return false;
}

bool AdlibMusicDriver::musSetPanning(const byte *&srcP, byte param) {
	// Adlib does not support this
	++srcP;
	return false;
}

bool AdlibMusicDriver::musFade(const byte *&srcP, byte param) {
	++srcP;
	if (param < 7)
		setFrequency(param, _channels[param]._frequency);

	return false;
}

bool AdlibMusicDriver::musStartNote(const byte *&srcP, byte param) {
	if (param < 7) {
		byte note = *srcP++;
		++srcP;		// Second byte is fade, which is unused by Adlib
		uint freq = calcFrequency(note);
		setFrequency(param, freq);
		_channels[param]._frequency = freq | 0x2000;
		setFrequency(param, freq);
	} else {
		srcP += 2;
	}

	return false;
}

bool AdlibMusicDriver::musSetVolume(const byte *&srcP, byte param) {
	if (*srcP++ == 2 && !_field180) {
		_channels[param]._volume = *srcP;
		setOutputLevel(param, *srcP);
	}

	++srcP;
	return false;
}

bool AdlibMusicDriver::musInjectMidi(const byte *&srcP, byte param) {
	// Adlib does not support MIDI. So simply keep skipping over bytes
	// until an 'F7' byte is found that flags the end of the MIDI data
	while (*srcP++ != 0xF7)
		;

	return false;
}

bool AdlibMusicDriver::musPlayInstrument(const byte *&srcP, byte param) {
	byte instrument = *srcP++;
	if (param < 7)
		playInstrument(param, _musInstrumentPtrs[instrument]);

	return false;
}

bool AdlibMusicDriver::fxSetInstrument(const byte *&srcP, byte param) {
	_fxInstrumentPtrs[param] = srcP;
	srcP += 11;

	return false;
}

bool AdlibMusicDriver::fxSetVolume(const byte *&srcP, byte param) {
	if (!_field180 && (!_fieldF || param != 7)) {
		_channels[param]._volume = *srcP;
		setOutputLevel(param, *srcP);
	}

	++srcP;
	return false;
}

bool AdlibMusicDriver::fxMidiReset(const byte *&srcP, byte param) {
	return false;
}

bool AdlibMusicDriver::fxMidiDword(const byte *&srcP, byte param) {
	return false;
}

bool AdlibMusicDriver::fxSetPanning(const byte *&srcP, byte param) {
	byte note = *srcP++;
	if (!_fieldF || param != 7) {
		uint freq = calcFrequency(note);
		setFrequency(param, freq);
		_channels[param]._frequency = freq;
	}

	return false;
}

bool AdlibMusicDriver::fxChannelOff(const byte *&srcP, byte param) {
	_channels[param]._frequency &= ~0x2000;
	write(0xB0 + param, _channels[param]._frequency);
	return false;
}

bool AdlibMusicDriver::fxFade(const byte *&srcP, byte param) {
	uint freq = calcFrequency(*srcP++);
	if (!_fieldF || param != 7) {
		_channels[param]._frequency = freq;
		setFrequency(param, freq);
	}

	return false;
}

bool AdlibMusicDriver::fxStartNote(const byte *&srcP, byte param) {
	if (!_fieldF || param != 7) {
		byte note = *srcP++;
		uint freq = calcFrequency(note);
		setFrequency(param, freq);
		_channels[param]._frequency = freq | 0x2000;
		setFrequency(param, freq);
	} else {
		++srcP;
	}

	return false;
}

bool AdlibMusicDriver::fxInjectMidi(const byte *&srcP, byte param) {
	// Surpringly, unlike the musInjectMidi, this version doesn't have
	// any logic to skip over following MIDI data. Which must mean the opcode
	// and/or it's data aren't present in the admus driver file
	return false;
}

bool AdlibMusicDriver::fxPlayInstrument(const byte *&srcP, byte param) {
	if (!_fieldF || param != 7)
		playInstrument(param, _fxInstrumentPtrs[param]);

	return false;
}


const byte AdlibMusicDriver::OPERATOR1_INDEXES[CHANNEL_COUNT] = {
	0, 1, 2, 8, 9, 0xA, 0x10, 0x11, 0x12
};

const byte AdlibMusicDriver::OPERATOR2_INDEXES[CHANNEL_COUNT] = {
	3, 4, 5, 0xB, 0xC, 0xD, 0x13, 0x14, 0x15
};

const uint AdlibMusicDriver::WAVEFORMS[24] = {
	0, 347, 388, 436, 462, 519, 582, 646,
	0, 362, 406, 455, 484, 542, 607, 680,
	0, 327, 367, 412, 436, 489, 549, 618
};

/*------------------------------------------------------------------------*/

Music::Music() : _musicDriver(nullptr), _songData(nullptr) {
	_musicDriver = new AdlibMusicDriver();
	loadEffectsData();
}

Music::~Music() {
	delete _musicDriver;
	delete[] _effectsData;
	delete[] _songData;
}

void Music::loadEffectsData() {
	File file("admus");
	Common::String md5str = Common::computeStreamMD5AsString(file, 8192);

	if (md5str != "be8989a5e868913f0e53963046e3ea13")
		error("Unknown music driver encountered");

	// Load in the driver data
	byte *effectsData = new byte[file.size()];
	file.seek(0);
	file.read(effectsData, file.size());
	file.close();
	_effectsData = effectsData;

	// Extract the effects offsets
	_effectsOffsets.resize(180);
	const int EFFECTS_OFFSET = 0x91D;
	for (int idx = 0; idx < 180; ++idx)
		_effectsOffsets[idx] = READ_LE_UINT16(&effectsData[EFFECTS_OFFSET + idx * 2]);
}

void Music::playFX(uint effectId) {
	if (effectId < _effectsOffsets.size()) {
		const byte *dataP = &_effectsData[_effectsOffsets[effectId]];
		_musicDriver->playFX(effectId, dataP);
	}
}

int Music::songCommand(uint commandId, byte volume) {
	int result = _musicDriver->songCommand(commandId, volume);
	if (commandId == STOP_MUSIC) {
		delete[] _songData;
		_songData = nullptr;
	}

	return result;
}

void Music::playSong(Common::SeekableReadStream &stream) {
	if (_songData)
		stopMusic();

	byte *songData = new byte[stream.size()];
	stream.seek(0);
	stream.read(songData, stream.size());
	_songData = songData;

	_musicDriver->playSong(_songData);
}

void Music::playSong(const Common::String &name) {
	File f(name);
	playSong(f);
}

} // End of namespace Xeen
