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

// MIDI music class

#include "saga2/saga2.h"
#include "saga2/fta.h"

#include "saga2/music.h"
#include "saga2/hresmgr.h"

#include "audio/audiostream.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "audio/midiparser_qt.h"
#include "audio/miles.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/substream.h"

namespace Saga2 {

#define BUFFER_SIZE 4096
#define MUSIC_SUNSPOT 26

MusicDriver::MusicDriver() {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_driverType = MidiDriver::getMusicType(dev);

	switch (_driverType) {
	case MT_ADLIB:
		if (Common::File::exists("SAMPLE.AD") && Common::File::exists("SAMPLE.OPL")) {
			_milesAudioMode = true;
			_driver = Audio::MidiDriver_Miles_AdLib_create("SAMPLE.AD", "SAMPLE.OPL");
			warning("*** YES MILES FILES");
		} else {
			_milesAudioMode = false;
			MidiPlayer::createDriver();

			warning("*** NO MILES FILES");
		}
		break;
	case MT_MT32:
		_milesAudioMode = true;
		_driver = Audio::MidiDriver_Miles_MT32_create("");
		break;
	default:
		_milesAudioMode = false;
		MidiPlayer::createDriver();
		break;
	}

	int retValue = _driver->open();
	if (retValue == 0) {
		if (_driverType != MT_ADLIB) {
			if (_driverType == MT_MT32 || _nativeMT32)
				_driver->sendMT32Reset();
			else
				_driver->sendGMReset();
		}

		_driver->setTimerCallback(this, &timerCallback);
	}
}

void MusicDriver::send(uint32 b) {
	if (_milesAudioMode) {
		_driver->send(b);
		return;
	}

	if ((b & 0xF0) == 0xC0 && !_nativeMT32) {
		// Remap MT32 instruments to General Midi
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}
	Audio::MidiPlayer::send(b);
}

void MusicDriver::metaEvent(byte type, byte *data, uint16 length) {
	// TODO: Seems SAGA does not want / need to handle end-of-track events?
}

void MusicDriver::play(byte *data, uint32 size, bool loop) {
	if (size < 4) {
		error("Music::play() wrong music resource size");
	}

	// Check if the game is using XMIDI or SMF music
	if (!memcmp(data, "FORM", 4)) {
		_parser = MidiParser::createParser_XMIDI();
	} else {
		error("MusicDriver::play(): Unsupported music format");
	}

	if (!_parser->loadMusic(data, size))
		error("Music::play() wrong music resource");

	_parser->setTrack(0);
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());
	_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
	_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);

	// Handle music looping
	_parser->property(MidiParser::mpAutoLoop, loop);

	_isPlaying = true;
}

void MusicDriver::pause() {
	_isPlaying = false;
}

void MusicDriver::resume() {
	_isPlaying = true;
}


Music::Music(hResContext *musicRes, Audio::Mixer *mixer) : _mixer(mixer), _player(0), _musicContext(musicRes) {
	_currentVolume = 0;
	_currentMusicBuffer = nullptr;

	_player = new MusicDriver();

	_trackNumber = 0;
}

Music::~Music() {
	_mixer->stopHandle(_musicHandle);
	delete _player;
}

bool Music::isPlaying() {
	return _mixer->isSoundHandleActive(_musicHandle) || _player->isPlaying();
}

void Music::play(uint32 resourceId, MusicFlags flags) {
	debug(2, "Music::play %d, %d", resourceId, flags);

	if (isPlaying() && _trackNumber == resourceId)
		return;

	_trackNumber = resourceId;
	_mixer->stopHandle(_musicHandle);
	_player->stop();

	delete _currentMusicBuffer;

	_currentMusicBuffer = (byte *)LoadResource(_musicContext, resourceId, "music data");
	uint32 size = _musicContext->size(resourceId);

	_player->play(_currentMusicBuffer, size, (flags & MUSIC_LOOP));
}

void Music::pause() {
	_player->pause();
	_player->setVolume(0);
}

void Music::resume() {
	_player->resume();
}

void Music::stop() {
	_player->stop();
}

} // End of namespace Saga
