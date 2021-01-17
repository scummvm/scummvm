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

#include "ags/music.h"
#include "ags/engine/main/main.h"
#include "ags/lib/audio/midi.h"
#include "audio/midiparser.h"

namespace AGS {

Music *g_music;

Music::Music(Audio::Mixer *mixer) :
		Audio::MidiPlayer(), _mixer(mixer), _isFirstTime(true) {
	g_music = this;
	Audio::MidiPlayer::createDriver();

	int ret = _driver->open();
	if (ret == 0) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();

		_driver->setTimerCallback(this, &timerCallback);
	}
}

Music::~Music() {
	stop();
	g_music = nullptr;
}

void Music::sendToChannel(byte channel, uint32 b) {
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

void Music::playMusic(Common::SeekableReadStream *midi, bool repeat) {
	if (_isFirstTime) {
		if (::AGS3::psp_midi_preload_patches)
			::AGS3::load_midi_patches();
		_isFirstTime = false;
	}

	stop();

	// Load MIDI resource data
	int midiMusicSize = midi->size();

	_midiData.resize(midi->size());
	midi->read(&_midiData[0], midi->size());

	MidiParser *parser = MidiParser::createParser_SMF();
	if (parser->loadMusic(&_midiData[0], midiMusicSize)) {
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

		_parser = parser;

		syncVolume();

		_isLooping = repeat;
		_isPlaying = true;
	} else {
		delete parser;
	}
}

void Music::seek(size_t offset) {
	warning("TODO: Music::seek to offset %u", (uint32)offset);
}

} // End of namespace AGS
