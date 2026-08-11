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

#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "common/debug.h"
#include "common/file.h"
#include "ultima/ultima0/music.h"

namespace Ultima {
namespace Ultima0 {

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
	if (!musicFile.open(_filename))
		return;

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
		delete parser;
	}
}

} // namespace Ultima0
} // namespace Ultima
