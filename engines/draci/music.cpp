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
 * $URL$
 * $Id$
 *
 */

// MIDI and digital music class

#include "audio/audiostream.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "common/config-manager.h"
#include "common/file.h"

#include "draci/draci.h"
#include "draci/music.h"

namespace Draci {

MusicPlayer::MusicPlayer(const char *pathMask) : _pathMask(pathMask), _isGM(false), _track(-1) {

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_nativeMT32 = ((MidiDriver::getMusicType(dev) == MT_MT32) || ConfMan.getBool("native_mt32"));
	//bool adlib = (MidiDriver::getMusicType(dev) == MT_ADLIB);

	_driver = MidiDriver::createMidi(dev);
	assert(_driver);
	if (_nativeMT32)
		_driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	_smfParser = MidiParser::createParser_SMF();
	_midiMusicData = NULL;

	int ret = _driver->open();
	if (ret == 0) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();

		// TODO: Load cmf.ins with the instrument table.  It seems that an
		// interface for such an operation is supported for AdLib.  Maybe for
		// this card, setting instruments is necessary.

		_driver->setTimerCallback(this, &onTimer);
	}
}

MusicPlayer::~MusicPlayer() {
	_driver->setTimerCallback(NULL, NULL);
	stop();
	if (_driver) {
		_driver->close();
		delete _driver;
		_driver = 0;
	}
	_smfParser->setMidiDriver(NULL);
	delete _smfParser;
	delete[] _midiMusicData;
}

void MusicPlayer::sendToChannel(byte channel, uint32 b) {
	if (!_channelsTable[channel]) {
		_channelsTable[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		// If a new channel is allocated during the playback, make sure
		// its volume is correctly initialized.
		if (_channelsTable[channel])
			_channelsTable[channel]->volume(_channelsVolume[channel] * _masterVolume / 255);
	}

	if (_channelsTable[channel])
		_channelsTable[channel]->send(b);
}

void MusicPlayer::onTimer(void *refCon) {
	MusicPlayer *music = (MusicPlayer *)refCon;
	Common::StackLock lock(music->_mutex);

	if (music->_parser)
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
	delete[] _midiMusicData;
	_midiMusicData = new byte[midiMusicSize];
	musicFile.read(_midiMusicData, midiMusicSize);
	musicFile.close();

	if (_smfParser->loadMusic(_midiMusicData, midiMusicSize)) {
		MidiParser *parser = _smfParser;
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

		_parser = parser;

		syncVolume();

		_isLooping = loop;
		_isPlaying = true;
		_track = track;
		debugC(2, kDraciSoundDebugLevel, "Playing track %d", track);
	} else {
		debugC(2, kDraciSoundDebugLevel, "Cannot play track %d", track);
	}
}

void MusicPlayer::stop() {
	Audio::MidiPlayer::stop();
	debugC(2, kDraciSoundDebugLevel, "Stopping track %d", _track);
	_track = -1;
}

} // End of namespace Draci
