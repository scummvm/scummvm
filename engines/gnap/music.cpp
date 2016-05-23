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

// MIDI and digital music class

#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "common/debug.h"
#include "common/file.h"

#include "gnap/music.h"
#include "gnap/gnap.h"

namespace Gnap {

MusicPlayer::MusicPlayer(const char *filename) : _filename(filename) {

	MidiPlayer::createDriver();

	int ret = _driver->open();
	if (ret == 0) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();

		_driver->setTimerCallback(this, &timerCallback);
	}
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

void MusicPlayer::playSMF(bool loop) {
	Common::StackLock lock(_mutex);

	stop();

	// Load MIDI resource data
	Common::File musicFile;
	musicFile.open(_filename);
	if (!musicFile.isOpen()) {
		debugC(2, kDebugMusic, "Cannot open music file %s", _filename.c_str());
		return;
	}
	int midiMusicSize = musicFile.size();
	free(_midiData);
	_midiData = (byte *)malloc(midiMusicSize);
	musicFile.read(_midiData, midiMusicSize);
	musicFile.close();

	MidiParser *parser = MidiParser::createParser_SMF();
	if (parser->loadMusic(_midiData, midiMusicSize)) {
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

		_parser = parser;

		syncVolume();

		_isLooping = loop;
		_isPlaying = true;
	} else {
		debugC(2, kDebugMusic, "Cannot play music file %s", _filename.c_str());
		delete parser;
	}
}

void MusicPlayer::stop() {
	Audio::MidiPlayer::stop();
}

} // End of namespace Gnap
