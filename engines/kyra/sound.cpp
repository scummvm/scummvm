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


#include "common/system.h"
#include "common/config-manager.h"

#include "kyra/resource.h"
#include "kyra/sound.h"

#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiostream.h"

#include "sound/mp3.h"
#include "sound/vorbis.h"
#include "sound/flac.h"

namespace Kyra {

Sound::Sound(KyraEngine_v1 *vm, Audio::Mixer *mixer)
	: _vm(vm), _mixer(mixer), _soundChannels(), _musicEnabled(1),
	_sfxEnabled(true), _soundDataList(0) {
}

Sound::~Sound() {
}

bool Sound::voiceFileIsPresent(const char *file) {
	char filenamebuffer[25];
	for (int i = 0; _supportedCodecs[i].fileext; ++i) {
		strcpy(filenamebuffer, file);
		strcat(filenamebuffer, _supportedCodecs[i].fileext);
		if (_vm->resource()->getFileSize(filenamebuffer) > 0)
			return true;
	}

	strcpy(filenamebuffer, file);
	strcat(filenamebuffer, ".VOC");

	if (_vm->resource()->getFileSize(filenamebuffer) > 0)
		return true;

	return false;
}

int32 Sound::voicePlay(const char *file, bool isSfx) {
	char filenamebuffer[25];

	int h = 0;
	while (_mixer->isSoundHandleActive(_soundChannels[h].channelHandle) && h < kNumChannelHandles)
		h++;
	if (h >= kNumChannelHandles)
		return 0;

	Audio::AudioStream *audioStream = 0;

	for (int i = 0; _supportedCodecs[i].fileext; ++i) {
		strcpy(filenamebuffer, file);
		strcat(filenamebuffer, _supportedCodecs[i].fileext);

		Common::SeekableReadStream *stream = _vm->resource()->getFileStream(filenamebuffer);
		if (!stream)
			continue;
		audioStream = _supportedCodecs[i].streamFunc(stream, true, 0, 0, 1);
		break;
	}

	if (!audioStream) {
		strcpy(filenamebuffer, file);
		strcat(filenamebuffer, ".VOC");

		uint32 fileSize = 0;
		byte *fileData = _vm->resource()->fileData(filenamebuffer, &fileSize);
		if (!fileData)
			return 0;

		Common::MemoryReadStream vocStream(fileData, fileSize);
		audioStream = Audio::makeVOCStream(vocStream);

		delete[] fileData;
		fileSize = 0;
	}

	_soundChannels[h].file = file;
	_mixer->playInputStream(isSfx ? Audio::Mixer::kSFXSoundType : Audio::Mixer::kSpeechSoundType, &_soundChannels[h].channelHandle, audioStream);

	return audioStream->getTotalPlayTime();
}

void Sound::voiceStop(const char *file) {
	if (!file) {
		for (int h = 0; h < kNumChannelHandles; h++) {
			if (_mixer->isSoundHandleActive(_soundChannels[h].channelHandle))
				_mixer->stopHandle(_soundChannels[h].channelHandle);
		}
	} else {
		for (int i = 0; i < kNumChannelHandles; ++i) {
			if (_soundChannels[i].file == file)
				_mixer->stopHandle(_soundChannels[i].channelHandle);
		}
	}
}

bool Sound::voiceIsPlaying(const char *file) {
	bool res = false;
	if (!file) {
		for (int h = 0; h < kNumChannelHandles; h++) {
			if (_mixer->isSoundHandleActive(_soundChannels[h].channelHandle))
				res = true;
		}
	} else {
		for (int i = 0; i < kNumChannelHandles; ++i) {
			if (_soundChannels[i].file == file)
				res = _mixer->isSoundHandleActive(_soundChannels[i].channelHandle);
		}
	}
	return res;
}

uint32 Sound::voicePlayedTime(const char *file) {
	if (!file)
		return 0;

	for (int i = 0; i < kNumChannelHandles; ++i) {
		if (_soundChannels[i].file == file)
			return _mixer->getSoundElapsedTime(_soundChannels[i].channelHandle);
	}

	return 0;
}

#pragma mark -

SoundMidiPC::SoundMidiPC(KyraEngine_v1 *vm, Audio::Mixer *mixer, MidiDriver *driver) : Sound(vm, mixer) {
	_driver = driver;
	_passThrough = false;

	_musicParser = _sfxParser = 0;
	_isMusicPlaying = _isSfxPlaying = false;
	_eventFromMusic = false;
	
	_nativeMT32 = _useC55 = false;

	_fadeStartTime = 0;
	_fadeMusicOut = false;

	memset(_channel, 0, sizeof(_channel));
	memset(_channelVolume, 50, sizeof(_channelVolume));
	_channelVolume[10] = 100;
	for (int i = 0; i < 16; ++i)
		_virChannel[i] = i;

	int ret = open();
	if (ret != MERR_ALREADY_OPEN && ret != 0)
		error("Couldn't open midi driver");
}

SoundMidiPC::~SoundMidiPC() {
	Common::StackLock lock(_mutex);

	delete _musicParser;
	delete _sfxParser;

	_driver->setTimerCallback(0, 0);
	close();
}

bool SoundMidiPC::init() {
	_musicParser = MidiParser::createParser_XMIDI();
	_sfxParser = MidiParser::createParser_XMIDI();

	if (!_musicParser || !_sfxParser)
		return false;

	_musicParser->setMidiDriver(this);
	_sfxParser->setMidiDriver(this);

	return true;
}

void SoundMidiPC::updateVolumeSettings() {
	_musicVolume = ConfMan.getInt("music_volume");
	_sfxVolume = ConfMan.getInt("sfx_volume");

	updateChannelVolume(_musicVolume);
}

void SoundMidiPC::hasNativeMT32(bool nativeMT32) {
	_nativeMT32 = nativeMT32;

	// C55 appears to be XMIDI for General MIDI instruments
	if (!_nativeMT32 && _vm->game() == GI_KYRA2)
		_useC55 = true;
	else
		_useC55 = false;
}

void SoundMidiPC::updateChannelVolume(uint8 volume) {
	for (int i = 0; i < 32; ++i) {
		if (_channel[i]) {
			if (i >= 16)
				_channel[i]->volume(_channelVolume[i - 16] * volume / 255);
			else
				_channel[i]->volume(_channelVolume[i] * volume / 255);
		}
	}
}

int SoundMidiPC::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;

	_driver->setTimerCallback(this, &onTimer);
	return 0;
}

void SoundMidiPC::close() {
	if (_driver)
		_driver->close();
	_driver = 0;
}

void SoundMidiPC::send(uint32 b) {
	if (_passThrough) {
		if ((b & 0xFFF0) == 0x007BB0)
			return;
		_driver->send(b);
		return;
	}

	const int volume = /*_eventFromMusic ? */_musicVolume/* : _sfxVolume*/;

	uint8 channel = (byte)(b & 0x0F);
	if (((b & 0xFFF0) == 0x6FB0 || (b & 0xFFF0) == 0x6EB0) && channel != 9) {
		if (_virChannel[channel] == channel) {
			_virChannel[channel] = channel + 16;
			if (!_channel[_virChannel[channel]])
				_channel[_virChannel[channel]] = _driver->allocateChannel();
			if (_channel[_virChannel[channel]])
				_channel[_virChannel[channel]]->volume(_channelVolume[channel] * volume / 255);
		}
		return;
	}

	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume
		uint8 vol = (uint8)((b >> 16) & 0x7F);
		_channelVolume[channel] = vol;
		vol = vol * volume / 255;
		b = (b & 0xFF00FFFF) | (vol << 16);
	} else if ((b & 0xF0) == 0xC0 && !_nativeMT32 && !_useC55) {
		b = (b & 0xFFFF00FF) | (MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8);
	} else if ((b & 0xFFF0) == 0x007BB0) {
		//Only respond to All Notes Off if this channel
		//has currently been allocated
		if (!_channel[/*_virChannel[channel]*/channel])
			return;
	}

	if (!_channel[_virChannel[channel]]) {
		_channel[_virChannel[channel]] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		if (_channel[_virChannel[channel]])
			_channel[_virChannel[channel]]->volume(_channelVolume[channel] * volume / 255);
	}
	if (_channel[_virChannel[channel]])
		_channel[_virChannel[channel]]->send(b);
}

void SoundMidiPC::metaEvent(byte type, byte *data, uint16 length) {
	switch (type) {
	case 0x2F:	// End of Track
		if (_eventFromMusic) {
			// remap all channels
			for (int i = 0; i < 16; ++i)
				_virChannel[i] = i;
		} else {
			_isSfxPlaying = false;
		}
		break;
	default:
		_driver->metaEvent(type, data, length);
		break;
	}
}


struct DeleterArray {
	void operator ()(byte *ptr) {
		delete[] ptr;
	}
};

void SoundMidiPC::loadSoundFile(uint file) {
	Common::StackLock lock(_mutex);

	Common::String filename = fileListEntry(file);
	filename += ".";
	filename += _useC55 ? "C55" : "XMI";

	if (filename == _currentTrack) {
		_isMusicPlaying = _isSfxPlaying = false;
		_fadeStartTime = 0;
		_fadeMusicOut = false;
		updateChannelVolume(_musicVolume);
		_musicParser->setTempo(0);
		_sfxParser->setTempo(0);
		_musicParser->property(MidiParser::mpAutoLoop, false);
		_sfxParser->property(MidiParser::mpAutoLoop, false);
		return;
	}

	uint32 size;
	uint8 *data = (_vm->resource())->fileData(filename.c_str(), &size);

	if (!data) {
		warning("Couldn't load soundfile '%s'", filename.c_str());
		return;
	}

	_currentTrack = filename;

	_musicParser->unloadMusic();
	_sfxParser->unloadMusic();
	_midiFile = Common::SharedPtr<byte>(data, DeleterArray());

	_isMusicPlaying = _isSfxPlaying = false;
	_fadeStartTime = 0;
	_fadeMusicOut = false;
	updateChannelVolume(_musicVolume);

	if (_musicParser->loadMusic(_midiFile.get(), size)) {
		_musicParser->setTimerRate(getBaseTempo());
		_musicParser->setTempo(0);
		_musicParser->property(MidiParser::mpAutoLoop, false);
	} else {
		warning("Error parsing music track '%s'", filename.c_str());
	}

	if (_sfxParser->loadMusic(_midiFile.get(), size)) {
		_sfxParser->setTimerRate(getBaseTempo());
		_sfxParser->setTempo(0);
		_sfxParser->property(MidiParser::mpAutoLoop, false);
	} else {
		warning("Error parsing sfx track '%s'", filename.c_str());
	}
}

void SoundMidiPC::onTimer(void *refCon) {
	SoundMidiPC *sound = (SoundMidiPC *)refCon;
	Common::StackLock lock(sound->_mutex);

	// this should be set to the fadeToBlack value
	static const uint32 musicFadeTime = 1 * 1000;

	if (sound->_fadeMusicOut) {
		if (sound->_fadeStartTime + musicFadeTime > sound->_vm->_system->getMillis()) {
			byte volume = (byte)((musicFadeTime - (sound->_vm->_system->getMillis() - sound->_fadeStartTime)) * sound->_musicVolume / musicFadeTime);
			sound->updateChannelVolume(volume);
		} else {
			sound->_fadeStartTime = 0;
			sound->_fadeMusicOut = false;
			sound->_isMusicPlaying = false;

			sound->_eventFromMusic = true;
			// from sound/midiparser.cpp
			for (int i = 0; i < 128; ++i) {
				for (int j = 0; j < 16; ++j) {
					sound->send(0x80 | j | i << 8);
				}
			}

			for (int i = 0; i < 16; ++i)
				sound->send(0x007BB0 | i);
		}
	}

	if (sound->_isMusicPlaying) {
		sound->_eventFromMusic = true;
		sound->_musicParser->onTimer();
	}

	if (sound->_isSfxPlaying) {
		sound->_eventFromMusic = false;
		sound->_sfxParser->onTimer();
	}
}

void SoundMidiPC::playTrack(uint8 track) {
	Common::StackLock lock(_mutex);

	if (_musicParser && (track != 0 || _nativeMT32) && _musicEnabled) {
		_isMusicPlaying = true;
		_fadeMusicOut = false;
		_fadeStartTime = 0;
		updateChannelVolume(_musicVolume);
		_musicParser->setTrack(track);
		_musicParser->jumpToTick(0);
		_musicParser->setTempo(1);
		_musicParser->property(MidiParser::mpAutoLoop, false);
	}
}

void SoundMidiPC::haltTrack() {
	Common::StackLock lock(_mutex);

	if (_musicParser) {
		_isMusicPlaying = false;
		_fadeMusicOut = false;
		_fadeStartTime = 0;
		updateChannelVolume(_musicVolume);
		_musicParser->setTrack(0);
		_musicParser->jumpToTick(0);
		_musicParser->setTempo(0);
	}
}

void SoundMidiPC::playSoundEffect(uint8 track) {
	Common::StackLock lock(_mutex);

	if (_sfxParser && _sfxEnabled) {
		_isSfxPlaying = true;
		_sfxParser->setTrack(track);
		_sfxParser->jumpToTick(0);
		_sfxParser->setTempo(1);
		_sfxParser->property(MidiParser::mpAutoLoop, false);
	}
}

void SoundMidiPC::beginFadeOut() {
	_fadeMusicOut = true;
	_fadeStartTime = _vm->_system->getMillis();
}

#pragma mark -

void KyraEngine_v1::snd_playTheme(int file, int track) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_playTheme(%d, %d)", file, track);
	if (_curMusicTheme == file)
		return;

	_curSfxFile = _curMusicTheme = file;
	_sound->loadSoundFile(_curMusicTheme);
	if (track != -1)
		_sound->playTrack(track);
}

void KyraEngine_v1::snd_playSoundEffect(int track, int volume) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_playSoundEffect(%d, %d)", track, volume);
	_sound->playSoundEffect(track);
}

void KyraEngine_v1::snd_playWanderScoreViaMap(int command, int restart) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_playWanderScoreViaMap(%d, %d)", command, restart);
	if (restart)
		_lastMusicCommand = -1;

	// no track mapping given
	// so don't do anything here
	if (!_trackMap || !_trackMapSize)
		return;

	//if (!_disableSound) {
	//	XXX
	//}

	if (_flags.platform == Common::kPlatformPC) {
		assert(command*2+1 < _trackMapSize);
		if (_curMusicTheme != _trackMap[command*2]) {
			if (_trackMap[command*2] != -1 && _trackMap[command*2] != -2)
				snd_playTheme(_trackMap[command*2], -1);
		}

		if (command != 1) {
			if (_lastMusicCommand != command) {
				_sound->haltTrack();
				_sound->playTrack(_trackMap[command*2+1]);
			}
		} else {
			_sound->beginFadeOut();
		}
	} else if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98) {
		if (command == -1) {
			_sound->haltTrack();
		} else {
			assert(command*2+1 < _trackMapSize);
			if (_trackMap[command*2] != -2 && command != _lastMusicCommand) {
				_sound->haltTrack();
				_sound->playTrack(command);
			}
		}
	}

	_lastMusicCommand = command;
}

void KyraEngine_v1::snd_stopVoice() {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_stopVoice()");
	if (!_speechFile.empty()) {
		_sound->voiceStop(_speechFile.c_str());
		_speechFile.clear();
	}
}

bool KyraEngine_v1::snd_voiceIsPlaying() {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_voiceIsPlaying()");
	return _speechFile.empty() ? false : _sound->voiceIsPlaying(_speechFile.c_str());
}

// static res

const Sound::SpeechCodecs Sound::_supportedCodecs[] = {
#ifdef USE_FLAC
	{ ".VOF", Audio::makeFlacStream },
#endif // USE_FLAC
#ifdef USE_VORBIS
	{ ".VOG", Audio::makeVorbisStream },
#endif // USE_VORBIS
#ifdef USE_MAD
	{ ".VO3", Audio::makeMP3Stream },
#endif // USE_MAD
	{ 0, 0 }
};

} // end of namespace Kyra


