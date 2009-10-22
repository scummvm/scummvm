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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/made/music.cpp $
 * $Id: music.cpp 35843 2009-01-13 10:11:52Z thebluegr $
 *
 */

// MIDI and digital music class

#include "sound/audiostream.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "common/config-manager.h"
#include "common/file.h"

#include "draci/draci.h"
#include "draci/music.h"

namespace Draci {

MusicPlayer::MusicPlayer(MidiDriver *driver, const char *pathMask) : _parser(0), _driver(driver), _pathMask(pathMask), _looping(false), _isPlaying(false), _passThrough(false), _isGM(false), _track(0) {
	memset(_channel, 0, sizeof(_channel));
	memset(_channelVolume, 255, sizeof(_channelVolume));
	_masterVolume = 0;
	this->open();
	_smfParser = MidiParser::createParser_SMF();
	_midiMusicData = NULL;
}

MusicPlayer::~MusicPlayer() {
	_driver->setTimerCallback(NULL, NULL);
	stop();
	this->close();
	_smfParser->setMidiDriver(NULL);
	delete _smfParser;
	delete _midiMusicData;
}

void MusicPlayer::setVolume(int volume) {
	volume = CLIP(volume, 0, 255);

	if (_masterVolume == volume)
		return;

	_masterVolume = volume;

	Common::StackLock lock(_mutex);

	for (int i = 0; i < 16; ++i) {
		if (_channel[i]) {
			int newVolume = _channelVolume[i] * _masterVolume / 255;
			debugC(3, kDraciSoundDebugLevel, "Music channel %d: volume %d->%d",
				i, _channelVolume[i], newVolume);
			_channel[i]->volume(newVolume);
		}
	}
}

int MusicPlayer::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;

	_driver->setTimerCallback(this, &onTimer);
	return 0;
}

void MusicPlayer::close() {
	stop();
	if (_driver)
		_driver->close();
	_driver = 0;
}

void MusicPlayer::send(uint32 b) {
	if (_passThrough) {
		_driver->send(b);
		return;
	}

	byte channel = (byte)(b & 0x0F);
	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume
		byte volume = (byte)((b >> 16) & 0x7F);
		_channelVolume[channel] = volume;
		volume = volume * _masterVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xF0) == 0xC0 && !_isGM && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}
	else if ((b & 0xFFF0) == 0x007BB0) {
		//Only respond to All Notes Off if this channel
		//has currently been allocated
		if (_channel[b & 0x0F])
			return;
	}

	if (!_channel[channel])
		_channel[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();

	if (_channel[channel])
		_channel[channel]->send(b);
}

void MusicPlayer::metaEvent(byte type, byte *data, uint16 length) {

	switch (type) {
	case 0x2F:	// End of Track
		if (_looping)
			_parser->jumpToTick(0);
		else
			stop();
		break;
	default:
		//warning("Unhandled meta event: %02x", type);
		break;
	}
}

void MusicPlayer::onTimer(void *refCon) {
	MusicPlayer *music = (MusicPlayer *)refCon;
	Common::StackLock lock(music->_mutex);

	if (music->_isPlaying)
		music->_parser->onTimer();
}

void MusicPlayer::playSMF(int track, bool loop) {
	if (_isPlaying && track == _track) {
		debugC(2, kDraciSoundDebugLevel, "Already plaing track %d", track);
		return;
	}

	stop();

	_isGM = true;


	// Load MIDI resource data
	Common::File musicFile;
	char musicFileName[40];
	sprintf(musicFileName, _pathMask.c_str(), track);
	musicFile.open(musicFileName);
	if (!musicFile.isOpen()) {
		debugC(2, kDraciSoundDebugLevel, "Cannot open track %d", track);
		return;
	}
	int midiMusicSize = musicFile.size();
	delete _midiMusicData;
	_midiMusicData = new byte[midiMusicSize];
	musicFile.read(_midiMusicData, midiMusicSize);
	musicFile.close();

	if (_smfParser->loadMusic(_midiMusicData, midiMusicSize)) {
		MidiParser *parser = _smfParser;
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

		_parser = parser;

		syncVolume();

		_looping = loop;
		_isPlaying = true;
		_track = track;
		debugC(2, kDraciSoundDebugLevel, "Playing track %d", track);
	} else {
		debugC(2, kDraciSoundDebugLevel, "Cannot play track %d", track);
	}
}

void MusicPlayer::stop() {
	Common::StackLock lock(_mutex);

	if (!_isPlaying)
		return;

	debugC(2, kDraciSoundDebugLevel, "Stopping track %d", _track);
	_track = 0;
	_isPlaying = false;
	if (_parser) {
		_parser->unloadMusic();
		_parser = NULL;
	}
}

void MusicPlayer::pause() {
	debugC(2, kDraciSoundDebugLevel, "Pausing track %d", _track);
	setVolume(-1);
	_isPlaying = false;
}

void MusicPlayer::resume() {
	debugC(2, kDraciSoundDebugLevel, "Resuming track %d", _track);
	syncVolume();
	_isPlaying = true;
}

void MusicPlayer::syncVolume() {
	int volume = ConfMan.getInt("music_volume");
	debugC(2, kDraciSoundDebugLevel, "Syncing music volume to %d", volume);
	setVolume(volume);
}

// TODO:
// + volume support
// - bindings to GPL2 scripting
// - load cmf.ins
// - enable Adlib
// + resuming after configuration
// + error handling

} // End of namespace Draci
