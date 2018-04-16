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

#ifdef ENABLE_HE

#include "scumm/players/player_he.h"
#include "scumm/scumm.h"
#include "scumm/file.h"
#include "audio/miles.h"
#include "audio/midiparser.h"
#include "audio/mixer.h"
#include "common/memstream.h"

namespace Scumm {
Player_HE::Player_HE(ScummEngine *scumm) :
	_vm(scumm),
	_currentMusic(-1),
	_bank(NULL),
	_parser(NULL),
	_midi(NULL),
	_masterVolume(256) {

	for (int chan = 0; chan < 16; chan++)
		_channelVolume[chan] = 127;

	loadAdLibBank();

	Common::MemoryReadStream *bankStream = new Common::MemoryReadStream(_bank, _bankSize);

	_midi = Audio::MidiDriver_Miles_AdLib_create("", "", bankStream);
	if (!_midi) {
		error("Player_HE::Player_HE: could not create midi driver");
	}
	if (_midi->open() != 0) {
		error("Player_HE::Player_HE: could not open midi driver");
	}
}

Player_HE::~Player_HE() {
	if (_parser) {
		_parser->stopPlaying();
		delete _parser;
		_parser = NULL;
	}
	if (_midi) {
		_midi->setTimerCallback(0, 0);
		_midi->close();
		delete _midi;
		_midi = NULL;
	}
	if (_bank) {
		free(_bank);
	}
}

void Player_HE::setMusicVolume(int vol) {
	_masterVolume = vol;
	for (int chan = 0; chan < 16; chan++) {
		byte volume = (_channelVolume[chan] * vol) / 256;
		if (_midi)
			_midi->send(0x07b0 | chan | (volume << 16));
	}
}

void Player_HE::onTimer(void *data) {
	Player_HE *player = (Player_HE *)data;
	Common::StackLock lock(player->_mutex);
	if (player->_parser)
		player->_parser->onTimer();
}

void Player_HE::startSoundWithTrackID(int sound, int track) {
	Common::StackLock lock(_mutex);
	byte *ptr = _vm->getResourceAddress(rtSound, sound);
	if (ptr == NULL)
		return;

	if (_parser) {
		_parser->stopPlaying();
		delete _parser;
	}
	_parser = MidiParser::createParser_XMIDI();
	_parser->setMidiDriver(this);
	_parser->loadMusic(ptr + 40, 0);
	_parser->setTrack(track);
	_parser->setTimerRate(_midi->getBaseTempo());
	_midi->setTimerCallback(this, &Player_HE::onTimer);

	_currentMusic = sound;
}

void Player_HE::stopSound(int sound) {
	Common::StackLock lock(_mutex);
	if (!_parser || _currentMusic != sound)
		return;
	_parser->stopPlaying();
	delete _parser;
	_parser = NULL;
}

void Player_HE::stopAllSounds() {
	Common::StackLock lock(_mutex);
	if (!_parser)
		return;
	_parser->stopPlaying();
	delete _parser;
	_parser = NULL;
}

int Player_HE::getSoundStatus(int sound) const {
	Common::StackLock lock(_mutex);
	return (_parser && _currentMusic == sound) ? _parser->isPlaying() : 0;
}

int Player_HE::getMusicTimer() {
	Common::StackLock lock(_mutex);
	return _parser ? _parser->getTick() : 0;
}

void Player_HE::loadAdLibBank() {
	ScummFile file;
	Common::String drvName;
	char entryName[14];
	uint32 tag, entrySize, fileSize;
	Common::String bankName;

	if (_vm->_game.id == GID_PUTTMOON) {
		// Use GM bank
		bankName = "FAT.AD";
	} else {
		// Use MT32-like bank
		bankName = "MIDPAK.AD";
	}

	const char *ptr = strchr(_vm->_filenamePattern.pattern, '.');
	if (ptr) {
		drvName = Common::String(_vm->_filenamePattern.pattern, ptr - _vm->_filenamePattern.pattern + 1);
	} else {
		drvName = _vm->_filenamePattern.pattern;
		drvName += '.';
	}

	drvName += "drv";

	if (!file.open(drvName))
		error("Player_HE::loadAdLibBank(): could not open %s", drvName.c_str());

	uint32 size = (uint32)file.size();

	for (uint32 offset = 0; offset < size;) {
		file.seek(offset, SEEK_SET);
		if (size - offset < 31)
			error("Player_HE::loadAdLibBank(): unexpected end of file");

		tag = file.readUint32BE();
		entrySize = file.readUint32BE();
		if (size - offset < entrySize)
			error("Player_HE::loadAdLibBank(): unexpected end of file");
		fileSize = entrySize - 31;
		file.read(entryName, 13);
		entryName[13] = 0;

		if (tag != MKTAG('F', 'I', 'L', 'E'))
			error("Player_HE::loadAdLibBank(): unknown entry format");

		if (entryName == bankName) {
			_bank = (byte*)malloc(fileSize);
			file.read(_bank, fileSize);
			_bankSize = fileSize;
			return;
		}

		offset += entrySize;
	}
	error("Player_HE::loadAdLibBank(): could not find %s entry", bankName.c_str());
}

int Player_HE::open() {
	if (_midi)
		return _midi->open();
	return 0;
}

bool Player_HE::isOpen() const {
	if (_midi)
		return _midi->isOpen();
	return false;
}

void Player_HE::close() {
	if (_midi)
		_midi->close();
}

void Player_HE::setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) {
	if (_midi)
		_midi->setTimerCallback(timerParam, timerProc);
}

uint32 Player_HE::getBaseTempo() {
	if (_midi)
		return _midi->getBaseTempo();
	return 0;
}

void Player_HE::send(uint32 b) {
	byte chan = b & 0x0f;
	byte cmd = b & 0xf0;
	byte op1 = (b >> 8) & 0x7f;
	byte op2 = (b >> 16) & 0x7f;
	if (cmd == 0xb0 && op1 == 0x07) {
		_channelVolume[chan] = op2;
		op2 = (op2 * _masterVolume) / 256;
		b = (b & 0xffff) | (op2 << 16);
	}
	if (_midi)
		_midi->send(b);
}

}

#endif
