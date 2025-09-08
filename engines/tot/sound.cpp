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
#include "audio/decoders/voc.h"
#include "audio/mixer.h"
#include "audio/softsynth/pcspk.h"
#include "common/config-manager.h"

#include "tot/sound.h"
#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

SoundManager::SoundManager(Audio::Mixer *mixer) : _mixer(mixer) {

	_midiPlayer = new MidiPlayer();
	_speaker = new Audio::PCSpeaker();
	_speaker->init();
	_midiPlayer->open();

	g_engine->syncSoundSettings();
	_midiPlayer->syncSoundSettings();
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, 100);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, 100);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, 100);
}

SoundManager::~SoundManager() {
	if (_midiPlayer)
		delete _midiPlayer;
	free(_lastSrcStream);
	free(_audioStream);
	delete(_speaker);
}

void SoundManager::init() {
	setMidiVolume(3, 3);
	playMidi("SILENT", false);
	setSfxVolume(6, 6);
	_rightSfxVol = 6;
	_leftSfxVol = 6;
	_musicVolRight = 3;
	_musicVolLeft = 3;
}

void SoundManager::loadVoc(Common::String vocFile, int32 startPos, int16 size) {
	Common::File vocResource;

	if (size == 0) {
		if (!vocResource.open(Common::Path(vocFile + ".VOC"))) {
			showError(266);
		}

		_lastSrcStream = vocResource.readStream((uint32)vocResource.size());

	} else {
		if (!vocResource.open("EFECTOS.DAT")) {
			showError(266);
		}
		vocResource.seek(startPos);
		_lastSrcStream = vocResource.readStream((uint32)size);
	}
	_audioStream = Audio::makeVOCStream(_lastSrcStream, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
}

void SoundManager::autoPlayVoc(Common::String vocFile, int32 startPos, int16 vocSize) {
	loadVoc(vocFile, startPos, vocSize);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, Audio::makeLoopingAudioStream(_audioStream, 0), kSfxId, 255U, 0, DisposeAfterUse::NO);
}

void SoundManager::playVoc() {
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _audioStream, kSfxId, 255U, 0, DisposeAfterUse::NO);
	_lastSrcStream->seek(0);
	_audioStream->rewind();
}

void SoundManager::playVoc(Common::String vocFile, int32 startPos, uint vocSize) {
	loadVoc(vocFile, startPos, vocSize);
	playVoc();
}

void SoundManager::stopVoc() {
	_mixer->stopHandle(_soundHandle);
}

void SoundManager::waitForSoundEnd() {
	Common::Event e;
	do {
		while (g_system->getEventManager()->pollEvent(e)) { }
		g_engine->_chrono->updateChrono();
		g_engine->_screen->update();
		g_system->delayMillis(10);
	} while (g_engine->_sound->isVocPlaying());
}

bool SoundManager::isVocPlaying() {
	return _mixer->isSoundIDActive(kSfxId);
}

void SoundManager::playMidi(Common::String fileName, bool loop) {
	Common::File musicFile;
	musicFile.open(Common::Path(fileName + ".MUS"));
	if (!musicFile.isOpen()) {
		showError(267);
		return;
	}
	byte *curMidi = (byte *)malloc(musicFile.size());
	musicFile.read(curMidi, musicFile.size());
	playMidi(curMidi, musicFile.size(), loop);
	musicFile.close();
	free(curMidi);
}

void SoundManager::playMidi(byte *data, int size, bool loop) {
	Common::MemoryReadStream stream = Common::MemoryReadStream(data, size);
	_midiPlayer->load(&stream, size);
	_midiPlayer->setLoop(loop);
	_midiPlayer->play(0);
}

void SoundManager::toggleMusic() {
}

void SoundManager::beep(int32 frequency, int32 ms) {
	_speaker->stop();
	_speaker->play(Audio::PCSpeaker::kWaveFormSquare, frequency, ms);
}

void SoundManager::setSfxVolume(byte voll, byte volr) {

	if (voll == volr) {
		int volume = (voll) / (float)7 * 255;
		setSfxVolume(volume);
		setSfxBalance(true, true);
	} else {
		if (voll == 0) {
			setSfxBalance(false, true);
		} else {
			setSfxBalance(true, false);
		}
	}
}

void SoundManager::setMidiVolume(byte voll, byte volr) {
	int volume = (voll) / (float)7 * 255;
	setMusicVolume(volume);
}

void SoundManager::fadeOutMusic() {
	byte stepVol = (_musicVolLeft + _musicVolRight) / 2;
	for (int i = stepVol; i >= 0; i--) {
		setMidiVolume(i, i);
		delay(10);
	}
}

void SoundManager::fadeInMusic() {

	byte stepVol = (_musicVolLeft + _musicVolRight) / 2;
	for (int i = 0; i <= stepVol; i++) {
		setMidiVolume(i, i);
		delay(10);
	}
}

void SoundManager::setMasterVolume(byte voll, byte volr) {
}

void SoundManager::setSfxVolume(byte volume) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, volume);
	ConfMan.setInt("sfx_volume", volume);
	ConfMan.flushToDisk();
}

void SoundManager::setSfxBalance(bool left, bool right) {
	int balance = left ? -127 : 127;
	_mixer->setChannelBalance(_soundHandle, balance);
}

void SoundManager::setMusicVolume(byte volume) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);
	ConfMan.setInt("music_volume", volume);
	ConfMan.flushToDisk();
	_midiPlayer->syncSoundSettings();
}

} // End of namespace Tot
