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

SoundDriver::SoundDriver() : _frameCtr(0) {
	_channels.resize(CHANNEL_COUNT);
	_streams[stMUSIC] = Stream(MUSIC_COMMANDS);
	_streams[stFX] = Stream(FX_COMMANDS);
}

SoundDriver::~SoundDriver() {
}

SoundDriver::Stream *SoundDriver::tickStream() {
	for (size_t i = 0; i < stLAST; ++i) {
		Stream& stream = _streams[i];
		if (stream._playing && (stream._countdownTimer == 0 || --stream._countdownTimer == 0))
			return &stream;
	}

	return nullptr;
}

void SoundDriver::execute() {
	Stream *stream = tickStream();
	if (!stream) {
		pausePostProcess();
		return;
	}

	++_frameCtr;
	debugC(3, kDebugSound, "\nSoundDriver frame - #%x", _frameCtr);

	// Main loop
	bool breakFlag = false;
	while (!breakFlag) {
		debugCN(3, kDebugSound, "MUSCODE %.4x - %.2x  ", (uint)(stream->_dataPtr - stream->_startPtr), (uint)*stream->_dataPtr);
		byte nextByte = *stream->_dataPtr++;
		int cmd = (nextByte >> 4) & 15;
		int param = (nextByte & 15);

		CommandFn fn = stream->_commands[cmd];
		breakFlag = (this->*fn)(stream->_dataPtr, param);
	}
}


bool SoundDriver::musCallSubroutine(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "musCallSubroutine");
	if (_musSubroutines.size() < 16) {
		const byte *returnP = srcP + 2;
		srcP = _streams[stMUSIC]._startPtr + READ_LE_UINT16(srcP);

		_musSubroutines.push(Subroutine(returnP, srcP));
	}

	return false;
}

bool SoundDriver::musSetCountdown(const byte *&srcP, byte param) {
	// Set the countdown timer
	if (!param)
		param = *srcP++;
	_streams[stMUSIC]._countdownTimer = param;
	_streams[stMUSIC]._dataPtr = srcP;
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
	_channels[param]._changeFrequency = false;
	return false;
}

bool SoundDriver::cmdChangeFrequency(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "cmdChangeFrequency %d", param);

	_channels[param]._freqCtrChange = (int8)*srcP++;
	_channels[param]._freqCtr = 0xFF;
	_channels[param]._changeFrequency = true;
	_channels[param]._freqChange = (int16)READ_BE_UINT16(srcP);
	srcP += 2;

	return false;
}

bool SoundDriver::musEndSubroutine(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "musEndSubroutine %d", param);

	if (param != 15) {
		// Music has ended, so flag it stopped
		_streams[stMUSIC]._playing = false;
		return true;
	}

	// Returning from subroutine, or looping back to start of music
	srcP = _musSubroutines.empty() ? _streams[stMUSIC]._startPtr : _musSubroutines.pop()._returnP;
	return false;
}

bool SoundDriver::fxCallSubroutine(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxCallSubroutine");

	if (_fxSubroutines.size() < 16) {
		const byte *startP = srcP + 2;
		srcP = _streams[stMUSIC]._startPtr + READ_LE_UINT16(srcP);

		_fxSubroutines.push(Subroutine(startP, srcP));
	}

	return false;
}

bool SoundDriver::fxSetCountdown(const byte *&srcP, byte param) {
	// Set the countdown timer
	if (!param)
		param = *srcP++;
	_streams[stFX]._countdownTimer = param;
	_streams[stFX]._dataPtr = srcP;
	debugC(3, kDebugSound, "fxSetCountdown %d", param);

	// Do paused handling and break out of processing loop
	pausePostProcess();
	return true;
}

bool SoundDriver::fxEndSubroutine(const byte *&srcP, byte param) {
	debugC(3, kDebugSound, "fxEndSubroutine %d", param);

	if (param != 15) {
		// FX has ended, so flag it stopped
		_streams[stFX]._playing = false;
		return true;
	}

	srcP = _fxSubroutines.empty() ? _streams[stFX]._startPtr : _fxSubroutines.pop()._returnP;
	return false;
}

void SoundDriver::playFX(uint effectId, const byte *data) {
	if (!_streams[stFX]._playing || effectId < 7 || effectId >= 11) {
		_streams[stFX]._dataPtr = _streams[stFX]._startPtr = data;
		_streams[stFX]._countdownTimer = 0;
		_channels[7]._changeFrequency = _channels[8]._changeFrequency = false;
		resetFX();
		_streams[stFX]._playing = true;
	}

	debugC(1, kDebugSound, "Starting FX %d", effectId);
}

void SoundDriver::stopFX() {
	resetFX();
	_streams[stFX]._playing = false;
	_streams[stFX]._startPtr = _streams[stFX]._dataPtr = nullptr;
}

void SoundDriver::playSong(const byte *data) {
	_streams[stMUSIC]._dataPtr = _streams[stMUSIC]._startPtr = data;
	_musSubroutines.clear();
	_streams[stMUSIC]._countdownTimer = 0;
	_streams[stMUSIC]._playing = true;
	debugC(1, kDebugSound, "Starting song");
}

int SoundDriver::songCommand(uint commandId, byte musicVolume, byte sfxVolume) {
	if (commandId == STOP_SONG) {
		_streams[stMUSIC]._playing = false;
	} else if (commandId == RESTART_SONG) {
		_streams[stMUSIC]._playing = true;
		_streams[stMUSIC]._dataPtr = nullptr;
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

} // End of namespace Xeen
