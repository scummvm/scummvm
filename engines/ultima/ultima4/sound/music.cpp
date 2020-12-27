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

#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/sound/sound.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/shared/core/file.h"
#include "audio/decoders/mp3.h"
#include "audio/mods/mod_xm_s3m.h"
#include "audio/midiparser.h"

namespace Ultima {
namespace Ultima4 {

Music *g_music;

Music::Music(Audio::Mixer *mixer) :
		Audio::MidiPlayer(), _mixer(mixer), _introMid(TOWNS) {
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


	_filenames.reserve(MAX);
	_filenames.push_back("");    // filename for MUSIC_NONE;

	/*
	 * load music track filenames from xml config file
	 */
	const Config *config = Config::getInstance();

	Std::vector<ConfigElement> musicConfs = config->getElement("music").getChildren();
	Std::vector<ConfigElement>::const_iterator i = musicConfs.begin();
	Std::vector<ConfigElement>::const_iterator theEnd = musicConfs.end();
	for (; i != theEnd; ++i) {
		if (i->getName() != "track")
			continue;

		_filenames.push_back(i->getString("file"));
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

void Music::playMusic(Type music) {
	playMusic(_filenames[music]);
}

void Music::playMapMusic() {
	playMusic(g_context->_location->_map->_music);
}

void Music::playMusic(const Common::String &filename) {
	stop();

	// First try opening the file with whatever filename is provided
	if (startMusic(filename))
		return;

	// TODO: Since the player doesn't yet support xu4 .it files,
	// try starting the file with other extensions - which some have
	const char *const EXTENSIONS[2] = { ".mp3", ".mid" };
	for (int idx = 0; idx < 2; ++idx) {
		size_t dotIndex = filename.findLastOf('.');
		Common::String fname = (dotIndex != Common::String::npos) ?
			Common::String(filename.c_str(), dotIndex) + EXTENSIONS[idx] :
			filename + EXTENSIONS[idx];
		if (startMusic(fname))
			return;
	}

	// At this point, we couldn't open the given music file
	warning("No support for playing music file - %s", filename.c_str());
}

bool Music::startMusic(const Common::String &filename) {
	Common::File musicFile;
	if (!musicFile.open(Common::String::format("data/mid/%s", filename.c_str())))
		// No such file exists
		return false;

#ifdef USE_MAD
	if (filename.hasSuffixIgnoreCase(".mp3")) {
		Audio::SeekableAudioStream *audioStream = Audio::makeMP3Stream(
			musicFile.readStream(musicFile.size()), DisposeAfterUse::YES);
		_mixer->playStream(Audio::Mixer::kMusicSoundType,
			&_soundHandle, audioStream);
		return true;

	} else
#endif
	if (filename.hasSuffixIgnoreCase(".mid")) {
		// Load MIDI resource data
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

			_isLooping = false;
			_isPlaying = true;
			return true;
		} else {
			delete parser;
			return false;
		}
	} else {
		return false;
	}
}

void Music::stop() {
	_mixer->stopHandle(_soundHandle);
	Audio::MidiPlayer::stop();
}

void Music::introSwitch(int n) {
	if (n > NONE &&n < MAX) {
		_introMid = static_cast<Type>(n);
		intro();
	}
}

} // End of namespace Ultima4
} // End of namespace Ultima
