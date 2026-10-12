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
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "backends/audiocd/audiocd.h"
#include "common/config-manager.h"

#include "igor/igor.h"

namespace Igor {

int IgorEngine::getTalkMode() const {
	if (!(_game.flags & kFlagTalkie) || ConfMan.getBool("mute") || ConfMan.getBool("speech_mute"))
		return kTalkModeTextOnly;
	return CLIP<int>(_gameState.talkMode, kTalkModeSpeechOnly, kTalkModeTextOnly);
}

void IgorEngine::syncSoundSettings() {
	Engine::syncSoundSettings();
	const bool mute = ConfMan.getBool("mute");
	const bool musicMute = mute || ConfMan.getBool("music_mute");
	const bool sfxMute = mute || ConfMan.getBool("sfx_mute");
	const bool speechMute = mute || ConfMan.getBool("speech_mute") || !(_game.flags & kFlagTalkie);
	_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, musicMute);
	_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, sfxMute);
	_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, speechMute);
	_system->getAudioCDManager()->setVolume(musicMute ? 0 : Audio::Mixer::kMaxChannelVolume);
	_gameState.configSoundEnabled = sfxMute ? 0 : 1;

	_gameState.talkMode = speechMute ? kTalkModeTextOnly
		: (ConfMan.getBool("subtitles") ? kTalkModeSpeechAndText : kTalkModeSpeechOnly);

	if (speechMute && (_gameState.talkSpeed < 1 || _gameState.talkSpeed > 5))
		_gameState.talkSpeed = 3;
	if (sfxMute)
		_mixer->stopHandle(_sfxHandle);

	if (speechMute) {
		stopDialogueSpeech();
		// Disabling speech during a sentence reveals its text and starts the
		// same reading timer used by sentences begun in text-only mode.
		if (_gameState.dialogueTextRunning && _talkDelay < 0) {
			_talkDelay = 2 * _dialogueTextsTable[_dialogueTextsStart].count
				* _talkDelays[CLIP<int>(_gameState.talkSpeed, 1, 5)];
			_talkDelayCounter = 0;
			memcpy(_screenVGA + _dialogueDirtyRectY, _screenTextLayer, _dialogueDirtyRectSize);
		}
	}
}

void IgorEngine::playMusic(int num, bool restart) {
	debugC(9, kDebugEngine, "playMusic() %d", num);
	if (_gameState.musicNum == num && !restart)
		return;
	if (_gameState.musicNum != num) {
		_gameState.musicNum = num;
		_gameState.musicSequenceIndex = 1;
	}
	if (_game.flags & kFlagFloppy) {
		// TODO: the track number -> CMF file mapping is not known yet
		// startMusic(...);
	} else {
		g_system->getAudioCDManager()->stop();
		// the first CD track is the data track: music number N is CD track N + 1
		g_system->getAudioCDManager()->play(num + 1, -1, 0, 0);
	}
}

void IgorEngine::playSound(int num, int type) {
	if (type == 1 && (ConfMan.getBool("mute") || ConfMan.getBool("sfx_mute")))
		return;
	if (type == 0 && getTalkMode() == kTalkModeTextOnly) {
		stopDialogueSpeech();
		return;
	}

	// A zero or "no speech" number means the line has no recorded speech
	if (type == 0 && (num == 0 || num == kNoSpeechSound)) {
		if (_mixer->isSoundHandleActive(_speechHandle)) {
			_mixer->stopHandle(_speechHandle);
		}
		return;
	}
	--num;
	int soundOffset = -1;
	Audio::Mixer::SoundType soundType;
	Audio::SoundHandle *soundHandle = 0;
	if (type == 1) {
		// debugC(9, kDebugEngine, "playSound() %d -> sfx offset %d", num + 1, num);
		if (_mixer->isSoundHandleActive(_sfxHandle)) {
			return;
		}
		assert(num >= 0 && num < _soundOffsetsCount);
		soundOffset = _soundOffsets[num];
		soundType = Audio::Mixer::kSFXSoundType;
		soundHandle = &_sfxHandle;
	} else if (type == 0) {
		if (_mixer->isSoundHandleActive(_speechHandle)) {
            debugC(9, kDebugEngine, "stopping previous handle");
			_mixer->stopHandle(_speechHandle);
		}
		if ((_game.flags & kFlagTalkie) == 0) {
			return;
		}
		num += 101;
		assert(num >= 0 && num < _soundOffsetsCount);
		soundOffset = _soundOffsets[num];
		soundType = Audio::Mixer::kSpeechSoundType;
		soundHandle = &_speechHandle;
	} else {
		return;
	}

	// two streams must never share one Common::File: the second stream's seek would
	// clobber the first one's cursor and the running sound would come out as
	// noise. Give each stream its own handle and let the stream dispose of it.
	Common::File *sndFile = new Common::File;
	if (!sndFile->open(_game.sfxFileName)) {
		delete sndFile;
		return;
	}
	sndFile->seek(soundOffset);
	Audio::AudioStream *stream = Audio::makeVOCStream(sndFile, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	if (stream) {
		_mixer->playStream(soundType, soundHandle, stream);
	}
}

void IgorEngine::stopSound() {
	_mixer->stopHandle(_sfxHandle);
	_mixer->stopHandle(_speechHandle);
}

} // End of namespace Igor
