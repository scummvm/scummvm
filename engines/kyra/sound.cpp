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
#include "kyra/resource.h"
#include "kyra/sound.h"

#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiostream.h"

#include "sound/mp3.h"
#include "sound/vorbis.h"
#include "sound/flac.h"

namespace Kyra {

Sound::Sound(KyraEngine *vm, Audio::Mixer *mixer)
	: _vm(vm), _mixer(mixer), _soundChannels(), _musicEnabled(1),
	_sfxEnabled(true), _soundDataList(0) {
}

Sound::~Sound() {
}

bool Sound::voiceFileIsPresent(const char *file) {
	char filenamebuffer[25];
	for (int i = 0; _supportedCodes[i].fileext; ++i) {
		strcpy(filenamebuffer, file);
		strcat(filenamebuffer, _supportedCodes[i].fileext);
		if (_vm->resource()->getFileSize(filenamebuffer) > 0)
			return true;
	}

	strcpy(filenamebuffer, file);
	strcat(filenamebuffer, ".VOC");

	if (_vm->resource()->getFileSize(filenamebuffer) > 0)
		return true;

	return false;
}

bool Sound::voicePlay(const char *file, bool isSfx) {
	uint32 fileSize = 0;
	byte *fileData = 0;
	bool found = false;
	char filenamebuffer[25];

	int h = 0;
	while (_mixer->isSoundHandleActive(_soundChannels[h].channelHandle) && h < kNumChannelHandles)
		h++;
	if (h >= kNumChannelHandles)
		return false;

	Audio::AudioStream *audioStream = 0;

	for (int i = 0; _supportedCodes[i].fileext; ++i) {
		strcpy(filenamebuffer, file);
		strcat(filenamebuffer, _supportedCodes[i].fileext);

		Common::SeekableReadStream *stream = _vm->resource()->getFileStream(filenamebuffer);
		if (!stream)
			continue;
		audioStream = _supportedCodes[i].streamFunc(stream, true, 0, 0, 1);
		found = true;
		break;
	}

	if (!found) {
		strcpy(filenamebuffer, file);
		strcat(filenamebuffer, ".VOC");

		fileData = _vm->resource()->fileData(filenamebuffer, &fileSize);
		if (!fileData)
			return false;

		Common::MemoryReadStream vocStream(fileData, fileSize);
		audioStream = Audio::makeVOCStream(vocStream);
	}

	_soundChannels[h].file = file;
	_mixer->playInputStream(isSfx ? Audio::Mixer::kSFXSoundType : Audio::Mixer::kSpeechSoundType, &_soundChannels[h].channelHandle, audioStream);

	delete [] fileData;
	fileSize = 0;

	return true;
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
				res = true;
		}
	}
	return res;
}

#pragma mark -

SoundMidiPC::SoundMidiPC(KyraEngine *vm, Audio::Mixer *mixer, MidiDriver *driver) : Sound(vm, mixer) {
	_driver = driver;
	_passThrough = false;
	_eventFromMusic = false;
	_fadeMusicOut = _sfxIsPlaying = false;
	_fadeStartTime = 0;
	_isPlaying = _nativeMT32 = _useC55 = false;
	_soundEffect = _parser = 0;
	_soundEffectSource = _parserSource = 0;

	memset(_channel, 0, sizeof(MidiChannel*) * 32);
	memset(_channelVolume, 50, sizeof(uint8) * 16);
	_channelVolume[10] = 100;
	for (int i = 0; i < 16; ++i)
		_virChannel[i] = i;
	_volume = 0;

	int ret = open();
	if (ret != MERR_ALREADY_OPEN && ret != 0)
		error("couldn't open midi driver");
}

SoundMidiPC::~SoundMidiPC() {
	stopMusic();
	stopSoundEffect();

	Common::StackLock lock(_mutex);

	_driver->setTimerCallback(0, 0);
	close();
}

void SoundMidiPC::hasNativeMT32(bool nativeMT32) {
	_nativeMT32 = nativeMT32;

	// C55 appears to be XMIDI for General MIDI instruments
	if (!_nativeMT32 && _vm->game() == GI_KYRA2)
		_useC55 = true;
	else
		_useC55 = false;
}

void SoundMidiPC::setVolume(int volume) {
	if (volume < 0)
		volume = 0;
	else if (volume > 255)
		volume = 255;

	if (_volume == volume)
		return;

	_volume = volume;
	for (int i = 0; i < 32; ++i) {
		if (_channel[i]) {
			if (i >= 16) {
				_channel[i]->volume(_channelVolume[i - 16] * _volume / 255);
			} else {
				_channel[i]->volume(_channelVolume[i] * _volume / 255);
			}
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
	// HACK: For Kyrandia, we make the simplifying assumption that a song
	// either loops in its entirety, or not at all. So if we see a FOR_LOOP
	// controller event, we turn on looping even if there isn't any
	// corresponding NEXT_BREAK event.
	//
	// This is a gross over-simplification of how XMIDI handles loops. If
	// anyone feels like doing a proper implementation, please refer to
	// the Exult project, and do it in midiparser_xmidi.cpp

	if ((b & 0xFFF0) == 0x74B0) {
		debugC(9, kDebugLevelMain | kDebugLevelSound, "SoundMidiPC: Looping song");
		_parser->property(MidiParser::mpAutoLoop, true);
		return;
	}

	if (_passThrough) {
		if ((b & 0xFFF0) == 0x007BB0)
			return;
		_driver->send(b);
		return;
	}

	uint8 channel = (byte)(b & 0x0F);
	if (((b & 0xFFF0) == 0x6FB0 || (b & 0xFFF0) == 0x6EB0) && channel != 9) {
		if (_virChannel[channel] == channel) {
			_virChannel[channel] = channel + 16;
			if (!_channel[_virChannel[channel]])
				_channel[_virChannel[channel]] = _driver->allocateChannel();
			if (_channel[_virChannel[channel]])
				_channel[_virChannel[channel]]->volume(_channelVolume[channel] * _volume / 255);
		}
		return;
	}

	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume
		uint8 volume = (uint8)((b >> 16) & 0x7F);
		_channelVolume[channel] = volume;
		volume = volume * _volume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xF0) == 0xC0 && !_nativeMT32 && !_useC55) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	} else if ((b & 0xFFF0) == 0x007BB0) {
		//Only respond to All Notes Off if this channel
		//has currently been allocated
		if (!_channel[channel])
			return;
	}

	if (!_channel[_virChannel[channel]]) {
		_channel[_virChannel[channel]] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		if (_channel[_virChannel[channel]])
			_channel[_virChannel[channel]]->volume(_channelVolume[channel] * _volume / 255);
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
			_sfxIsPlaying = false;
		}
		break;
	default:
		_driver->metaEvent(type, data, length);
		break;
	}
}

void SoundMidiPC::loadSoundFile(uint file) {
	char filename[25];
	sprintf(filename, "%s.%s", fileListEntry(file), _useC55 ? "C55" : "XMI");

	uint32 size;
	uint8 *data = (_vm->resource())->fileData(filename, &size);

	if (!data) {
		warning("couldn't load '%s'", filename);
		return;
	}

	playMusic(data, size);
	loadSoundEffectFile(file);
}

void SoundMidiPC::playMusic(uint8 *data, uint32 size) {
	stopMusic();

	Common::StackLock lock(_mutex);

	_parserSource = data;
	_parser = MidiParser::createParser_XMIDI();
	assert(_parser);

	if (!_parser->loadMusic(data, size)) {
		warning("Error reading track");
		delete _parser;
		_parser = 0;
		return;
	}

	_parser->setMidiDriver(this);
	_parser->setTimerRate(getBaseTempo());
	_parser->setTempo(0);
}

void SoundMidiPC::loadSoundEffectFile(uint file) {
	char filename[25];
	sprintf(filename, "%s.%s", fileListEntry(file), _useC55 ? "C55" : "XMI");

	uint32 size;
	uint8 *data = (_vm->resource())->fileData(filename, &size);

	if (!data) {
		warning("couldn't load '%s'", filename);
		return;
	}

	loadSoundEffectFile(data, size);
}

void SoundMidiPC::loadSoundEffectFile(uint8 *data, uint32 size) {
	stopSoundEffect();

	Common::StackLock lock(_mutex);

	_soundEffectSource = data;
	_soundEffect = MidiParser::createParser_XMIDI();
	assert(_soundEffect);

	if (!_soundEffect->loadMusic(data, size)) {
		warning("Error reading track");
		delete _parser;
		_parser = 0;
		return;
	}

	_soundEffect->setMidiDriver(this);
	_soundEffect->setTimerRate(getBaseTempo());
	_soundEffect->setTempo(0);
	_soundEffect->property(MidiParser::mpAutoLoop, false);
}

void SoundMidiPC::stopMusic() {
	Common::StackLock lock(_mutex);

	_isPlaying = false;
	if (_parser) {
		_parser->unloadMusic();
		delete _parser;
		_parser = 0;
		delete [] _parserSource;
		_parserSource = 0;

		_fadeStartTime = 0;
		_fadeMusicOut = false;
		setVolume(255);
	}
}

void SoundMidiPC::stopSoundEffect() {
	Common::StackLock lock(_mutex);

	_sfxIsPlaying = false;
	if (_soundEffect) {
		_soundEffect->unloadMusic();
		delete _soundEffect;
		_soundEffect = 0;
		delete [] _soundEffectSource;
		_soundEffectSource = 0;
	}
}

void SoundMidiPC::onTimer(void *refCon) {
	SoundMidiPC *music = (SoundMidiPC *)refCon;
	Common::StackLock lock(music->_mutex);

	// this should be set to the fadeToBlack value
	static const uint32 musicFadeTime = 2 * 1000;

	if (music->_fadeMusicOut && music->_fadeStartTime + musicFadeTime > music->_vm->_system->getMillis()) {
		byte volume = (byte)((musicFadeTime - (music->_vm->_system->getMillis() - music->_fadeStartTime)) * 255 / musicFadeTime);
		music->setVolume(volume);
	} else if (music->_fadeStartTime) {
		music->_fadeStartTime = 0;
		music->_fadeMusicOut = false;
		music->_isPlaying = false;

		music->_eventFromMusic = true;
		// from sound/midiparser.cpp
		for (int i = 0; i < 128; ++i) {
			for (int j = 0; j < 16; ++j) {
				music->send(0x80 | j | i << 8);
			}
		}

		for (int i = 0; i < 16; ++i)
			music->send(0x007BB0 | i);
	}

	if (music->_isPlaying) {
		if (music->_parser) {
			music->_eventFromMusic = true;
			music->_parser->onTimer();
		}
	}

	if (music->_sfxIsPlaying) {
		if (music->_soundEffect) {
			music->_eventFromMusic = false;
			music->_soundEffect->onTimer();
		}
	}
}

void SoundMidiPC::playTrack(uint8 track) {
	if (_parser && (track != 0 || _nativeMT32) && _musicEnabled) {
		_isPlaying = true;
		_fadeMusicOut = false;
		_fadeStartTime = 0;
		setVolume(255);
		_parser->setTrack(track);
		_parser->jumpToTick(0);
		_parser->setTempo(1);
		_parser->property(MidiParser::mpAutoLoop, false);
	}
}

void SoundMidiPC::haltTrack() {
	if (_parser) {
		_isPlaying = false;
		_fadeMusicOut = false;
		_fadeStartTime = 0;
		setVolume(255);
		_parser->setTrack(0);
		_parser->jumpToTick(0);
		_parser->setTempo(0);
	}
}

void SoundMidiPC::playSoundEffect(uint8 track) {
	if (_soundEffect && _sfxEnabled) {
		_sfxIsPlaying = true;
		_soundEffect->setTrack(track);
		_soundEffect->jumpToTick(0);
		_soundEffect->setTempo(1);
		_soundEffect->property(MidiParser::mpAutoLoop, false);
	}
}

void SoundMidiPC::beginFadeOut() {
	_fadeMusicOut = true;
	_fadeStartTime = _vm->_system->getMillis();
}

void KyraEngine::snd_playTheme(int file, int track) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine::snd_playTheme(%d)", file);
	_curSfxFile = _curMusicTheme = file;
	_sound->loadSoundFile(_curMusicTheme);
	_sound->playTrack(track);
}

void KyraEngine::snd_playSoundEffect(int track) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine::snd_playSoundEffect(%d)", track);
	_sound->playSoundEffect(track);
}

void KyraEngine::snd_playWanderScoreViaMap(int command, int restart) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine::snd_playWanderScoreViaMap(%d, %d)", command, restart);
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
				snd_playTheme(_trackMap[command*2]);
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

void KyraEngine::snd_stopVoice() {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine::snd_stopVoice()");
	_sound->voiceStop(_speechFile.empty() ? 0 : _speechFile.c_str());
}

bool KyraEngine::snd_voiceIsPlaying() {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine::snd_voiceIsPlaying()");
	return _sound->voiceIsPlaying(_speechFile.empty() ? 0 : _speechFile.c_str());
}

// static res

const Sound::SpeechCodecs Sound::_supportedCodes[] = {
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


