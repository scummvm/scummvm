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

#include "audio/audiostream.h"
#include "audio/decoders/voc.h"
#include "audio/midiparser.h"
#include "audio/midiplayer.h"
#include "audio/mixer.h"
#include "audio/softsynth/pcspk.h"
#include "common/config-manager.h"
#include "common/substream.h"

#include "tot/soundman.h"
#include "tot/tot.h"
#include "soundman.h"

namespace Tot {

SoundManager::SoundManager(Audio::Mixer *mixer) : _mixer(mixer) {

	_musicPlayer = new MusicPlayer();

	g_engine->syncSoundSettings();

	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, 100);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, 100);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, 100);
}
SoundManager::~SoundManager() {
	delete _musicPlayer;
	free(_lastSrcStream);
	free(_audioStream);
}

void SoundManager::loadVoc(Common::String fileName, long offset, int16 size) {
	Common::File vocResource;

	if (size == 0) {
		if (!vocResource.open(Common::Path(fileName + ".VOC"))) {
			error("Error loading file %s", fileName.c_str());
		}

		_lastSrcStream = vocResource.readStream((uint32)vocResource.size());

	} else {
		Common::File vocResource;
		if (!vocResource.open("EFECTOS.DAT")) {
			error("loadVoc(): could not open SFX file! (266)");
		}
		vocResource.seek(offset);
		_lastSrcStream = vocResource.readStream((uint32)size);
	}
	_audioStream = Audio::makeVOCStream(_lastSrcStream, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);

}
void SoundManager::autoPlayVoc() {
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, Audio::makeLoopingAudioStream(_audioStream, 0), kSfxId, 255U, 0, DisposeAfterUse::NO);
}

void SoundManager::playVoc() {
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _audioStream, kSfxId, 255U, 0, DisposeAfterUse::NO);
	_lastSrcStream->seek(0);
	_audioStream->rewind();
}

void SoundManager::stopVoc() {
	_mixer->stopHandle(_soundHandle);
}

void SoundManager::waitForSoundEnd() {
	do {
		g_engine->_chrono->updateChrono();
		g_engine->_screen->update();
	} while (g_engine->_sound->isVocPlaying());
}

bool SoundManager::isVocPlaying() {
	return _mixer->isSoundIDActive(kSfxId);
}

void SoundManager::playMidi(const char *fileName, bool loop) {
	_musicPlayer->playMidi(fileName, loop);
}

void SoundManager::playMidi(byte *data, int size, bool loop) {
	_musicPlayer->playMidi(data, size, loop);
}

void SoundManager::toggleMusic() {
}

void SoundManager::beep(int32 frequency, int32 ms) {
	Audio::PCSpeaker *speaker = new Audio::PCSpeaker(_mixer->getOutputRate());
	speaker->setVolume(255);
	speaker->play(Audio::PCSpeaker::kWaveFormSine, frequency, ms);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
		speaker,
		-1,
		Audio::Mixer::kMaxChannelVolume,
		0,
	    DisposeAfterUse::NO,
		true
	);
}

void SoundManager::setSfxVolume(int volume) {
	debug("Setting sfx volume to =%d", volume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, volume);
	ConfMan.setInt("sfx_volume", volume);
	ConfMan.flushToDisk();
}

void SoundManager::setSfxBalance(bool left, bool right) {
	int balance = left? -127: 127;
	_mixer->setChannelBalance(_soundHandle, balance);
	// _mixer->setChannelBalance();
}

void SoundManager::setMusicVolume(int volume) {

	debug("Setting music volume to =%d", volume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);
	ConfMan.setInt("music_volume", volume);
	ConfMan.flushToDisk();

	_musicPlayer->syncVolume();
}

MusicPlayer::MusicPlayer() {
	_data = nullptr;

	MidiPlayer::createDriver(MDT_MIDI | MDT_ADLIB);

	int ret = _driver->open();
	if (ret == 0) {
		_driver->setTimerCallback(this, &timerCallback);
	}
	_dataSize = -1;
}

MusicPlayer::~MusicPlayer() {
	killMidi();
}

void MusicPlayer::sndMidiStart(bool loop) {

	MidiParser *parser = MidiParser::createParser_SMF();
	if (parser->loadMusic(_data, _dataSize)) {
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());

		_parser = parser;

		syncVolume();

		// Al the tracks are supposed to loop
		_isLooping = loop;
		_isPlaying = true;
	}
}
void MusicPlayer::playMidi(const char *fileName, bool loop) {
	Common::File musicFile;
	debug("Opening music file %s", fileName);
	musicFile.open(fileName);
	if (!musicFile.isOpen()) {
		error("Cannot open music file %s", fileName);
		return;
	}
	byte *data = (byte *)malloc(musicFile.size());
	musicFile.read(data, musicFile.size());
	musicFile.close();
	playMidi(data, musicFile.size(), loop);
}

void MusicPlayer::playMidi(byte *data, int size, bool loop) {
	_dataSize = size;
	_data = data;
	// Start playing the music
	sndMidiStart(loop);
}

void MusicPlayer::killMidi() {
	Audio::MidiPlayer::stop();

	free(_data);
	_data = nullptr;
}

} // End of namespace Tot
