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

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/substream.h"

#include "backends/audiocd/audiocd.h"

#include "drascula/drascula.h"

namespace Drascula {

void DrasculaEngine::syncSoundSettings() {
	// Sync the engine with the config manager

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	// We need to handle the speech mute separately here. This is because the
	// engine code should be able to rely on all speech sounds muted when the
	// user specified subtitles only mode, which results in "speech_mute" to
	// be set to "true". The global mute setting has precedence over the
	// speech mute setting though.
	bool speechMute = mute;
	if (!speechMute)
		speechMute = ConfMan.getBool("speech_mute");

	_mixer->muteSoundType(Audio::Mixer::kPlainSoundType, mute);
	_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, mute);
	_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, speechMute);
	_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, mute);

	int voiceVolume = ConfMan.getInt("speech_volume");
	int musicVolume = ConfMan.getInt("music_volume");
	// If the music and voice volume are correct, don't change anything.
	// Otherwise compute the master volume using an approximation of sqrt(max) * 16 which would result in the master
	// volume being the same value as the max of music and voice.
	if (_mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType) != voiceVolume || _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) != musicVolume) {
		int masterVolume = MAX(musicVolume, voiceVolume) * 2 / 3 + 86;
		_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, masterVolume);
		_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, voiceVolume);
		_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, voiceVolume);
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, musicVolume);
	}
}

int DrasculaEngine::updateVolume(int prevVolume, int prevVolumeY) {
	prevVolumeY += 10;
	if (_mouseY < prevVolumeY && prevVolume < 15)
		prevVolume++;
	if (_mouseY > prevVolumeY && prevVolume > 0)
		prevVolume--;
	return prevVolume;
}

void DrasculaEngine::volumeControls() {
	if (_lang == kSpanish && currentChapter != 6)
		loadPic(95, tableSurface);

	copyRect(1, 56, 73, 63, 177, 97, tableSurface, screenSurface);
	updateScreen(73, 63, 73, 63, 177, 97, screenSurface);

	setCursor(kCursorCrosshair);
	showCursor();

	// The engine has three volume controls: master, SFx/Speech and Music.
	// ScummVM doesn't have a master volume, so we abuse the kPlainSoundType to store it.
	// In drascula, we only use the kMusicSoundType and kSpeechSoundType to play sounds.
	// For consistency with the ScummVM options dialog we also set the kSFXSoundType volume
	// to the same value as the kMusicSoundType value, but we don't actually use it.

	// The engines uses masterVolume, voiceVolume and musicVolume between 0 and 15.
	// We store in the mixer:
	//   - masterVolume * 16 in kPlainSoundType
	//   - (masterVolume + 1) * (voiceVolume + 1) - 1 in both kSpeechSoundType and kSFXSoundType
	//   - (masterVolume + 1) * (musicVolume + 1) - 1 in kMusicSoundType

	while (!shouldQuit()) {
		int masterVolume = CLIP((_mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) / 16), 0, 15);
		int voiceVolume = CLIP(((_mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType) + 1) / (masterVolume + 1) - 1), 0, 15);
		int musicVolume = CLIP(((_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) + 1) / (masterVolume + 1) - 1), 0, 15);

		int masterVolumeY = 72 + 61 - masterVolume * 4;
		int voiceVolumeY = 72 + 61 - voiceVolume * 4;
		int musicVolumeY = 72 + 61 - musicVolume * 4;

		updateRoom();

		copyRect(1, 56, 73, 63, 177, 97, tableSurface, screenSurface);

		copyBackground(183, 56, 82, masterVolumeY, 39, 2 + masterVolume * 4, tableSurface, screenSurface);
		copyBackground(183, 56, 138, voiceVolumeY, 39, 2 + voiceVolume * 4, tableSurface, screenSurface);
		copyBackground(183, 56, 194, musicVolumeY, 39, 2 + musicVolume * 4, tableSurface, screenSurface);

		updateScreen();

		updateEvents();

		// we're ignoring keypresses, so just empty the keyboard buffer
		while (getScan())
			;

		if (_rightMouseButton == 1) {
			// Clear this to avoid going straight to the inventory
			_rightMouseButton = 0;
			delay(100);
			break;
		}
		if (_leftMouseButton == 1) {
			delay(100);
			if (_mouseX > 80 && _mouseX < 121) {
				masterVolume = updateVolume(masterVolume, masterVolumeY);
				_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, masterVolume * 16);
			}

			if (_mouseX > 136 && _mouseX < 178)
				voiceVolume = updateVolume(voiceVolume, voiceVolumeY);

			if (_mouseX > 192 && _mouseX < 233)
				musicVolume = updateVolume(musicVolume, musicVolumeY);

			voiceVolume = (masterVolume + 1) * (voiceVolume + 1) - 1;
			_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, voiceVolume);
			_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, voiceVolume);
			ConfMan.setInt("speech_volume", voiceVolume);
			ConfMan.setInt("sfx_volume", voiceVolume);

			musicVolume = (masterVolume + 1) * (musicVolume + 1) - 1;
			_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, musicVolume);
			ConfMan.setInt("music_volume", musicVolume);
		}

	}

	if (_lang == kSpanish && currentChapter != 6)
		loadPic(974, tableSurface);

	selectVerb(kVerbNone);

	updateEvents();
}

void DrasculaEngine::playSound(int soundNum) {
	char file[20];
	sprintf(file, "s%i.als", soundNum);

	playFile(file);
}

void DrasculaEngine::finishSound() {
	delay(1);

	while (soundIsActive())
		_system->delayMillis(10);
}

void DrasculaEngine::playMusic(int p) {
	_system->getAudioCDManager()->stop();
	_system->getAudioCDManager()->play(p - 1, 1, 0, 0);
}

void DrasculaEngine::stopMusic() {
	_system->getAudioCDManager()->stop();
}

void DrasculaEngine::updateMusic() {
	_system->getAudioCDManager()->update();
}

int DrasculaEngine::musicStatus() {
	return _system->getAudioCDManager()->isPlaying();
}

void DrasculaEngine::stopSound() {
	_mixer->stopHandle(_soundHandle);
}

void DrasculaEngine::MusicFadeout() {
	int org_vol = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
	while (!shouldQuit()) {
		int vol = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
		vol -= 10;
			if (vol < 0)
				vol = 0;
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, vol);
		if (vol == 0)
			break;
		updateEvents();
		_system->updateScreen();
		_system->delayMillis(50);
	}
	_system->getAudioCDManager()->stop();
	_system->delayMillis(100);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, org_vol);
}

void DrasculaEngine::playFile(const char *fname) {
	Common::SeekableReadStream *stream = _archives.open(fname);
	if (stream) {
		int startOffset = 32;
		int soundSize = stream->size() - 64;

		if (!strcmp(fname, "3.als") && soundSize == 145166 && _lang != kSpanish) {
			// WORKAROUND: File 3.als with English speech files has a big silence at
			// its beginning and end. We seek past the silence at the beginning,
			// and ignore the silence at the end
			// Fixes bug #2111815 - "DRASCULA: Voice delayed"
			startOffset = 73959;
			soundSize = soundSize - startOffset - 26306;
		}

		Common::SeekableReadStream *subStream = new Common::SeekableSubReadStream(
		    stream, startOffset, startOffset + soundSize, DisposeAfterUse::YES);
		if (!subStream) {
			warning("playFile: Out of memory");
			delete stream;
			return;
		}

		Audio::AudioStream *sound = Audio::makeRawStream(subStream, 11025,
		                                                 Audio::FLAG_UNSIGNED);
		_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandle, sound);
	} else
		warning("playFile: Could not open %s", fname);
}

bool DrasculaEngine::soundIsActive() {
	return _mixer->isSoundHandleActive(_soundHandle);
}

} // End of namespace Drascula
