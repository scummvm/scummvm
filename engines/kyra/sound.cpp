/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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

#include "common/stdafx.h"
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

Sound::Sound(KyraEngine *engine, Audio::Mixer *mixer)
	: _engine(engine), _mixer(mixer), _currentVocFile(0), _vocHandle(), _compressHandle() {
}

Sound::~Sound() {
}

void Sound::voicePlay(const char *file) {
	uint32 fileSize = 0;
	byte *fileData = 0;
	bool found = false;
	char filenamebuffer[25];

	for (int i = 0; _supportedCodes[i].fileext; ++i) {
		strcpy(filenamebuffer, file);
		strcat(filenamebuffer, _supportedCodes[i].fileext);

		_engine->resource()->fileHandle(filenamebuffer, &fileSize, _compressHandle);
		if (!_compressHandle.isOpen())
			continue;
		
		_currentVocFile = _supportedCodes[i].streamFunc(&_compressHandle, fileSize);
		found = true;
		break;
	}

	if (!found) {
		strcpy(filenamebuffer, file);
		strcat(filenamebuffer, ".VOC");
		
		fileData = _engine->resource()->fileData(filenamebuffer, &fileSize);
		if (!fileData)
			return;

		Common::MemoryReadStream vocStream(fileData, fileSize);
		_mixer->stopHandle(_vocHandle);
		_currentVocFile = Audio::makeVOCStream(vocStream);
	}

	if (_currentVocFile)
		_mixer->playInputStream(Audio::Mixer::kSpeechSoundType, &_vocHandle, _currentVocFile);
	delete [] fileData;
	fileSize = 0;
}

bool Sound::voiceIsPlaying() {
	return _mixer->isSoundHandleActive(_vocHandle);
}

#pragma mark -

SoundMidiPC::SoundMidiPC(MidiDriver *driver, Audio::Mixer *mixer, KyraEngine *engine) : Sound(engine, mixer) {
	_driver = driver;
	_passThrough = false;
	_eventFromMusic = false;
	_fadeMusicOut = _sfxIsPlaying = false;
	_fadeStartTime = 0;
	_isPlaying = _nativeMT32 = false;
	_soundEffect = _parser = 0;
	_soundEffectSource = _parserSource = 0;

	memset(_channel, 0, sizeof(MidiChannel*) * 32);
	memset(_channelVolume, 50, sizeof(uint8) * 16);
	_channelVolume[10] = 100;
	for (int i = 0; i < 16; ++i) {
		_virChannel[i] = i;
	}
	_volume = 0;

	int ret = open();
	if (ret != MERR_ALREADY_OPEN && ret != 0) {
		error("couldn't open midi driver");
	}
}

SoundMidiPC::~SoundMidiPC() {
	_driver->setTimerCallback(NULL, NULL);
	close();
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
	} else if ((b & 0xF0) == 0xC0 && !_nativeMT32) {
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
			for (int i = 0; i < 16; ++i) {
				_virChannel[i] = i;
			}
		} else {
			_sfxIsPlaying = false;
		}
		break;
	default:
		_driver->metaEvent(type, data, length);
		break;
	}
}

void SoundMidiPC::loadMusicFile(const char *file) {
	char filename[25];
	sprintf(filename, "%s.XMI", file);

	uint32 size;
	uint8 *data = (_engine->resource())->fileData(filename, &size);

	if (!data) {
		warning("couldn't load '%s'", filename);
		return;
	}

	playMusic(data, size);
	loadSoundEffectFile(file);
}

void SoundMidiPC::playMusic(uint8 *data, uint32 size) {
	stopMusic();

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
	_parser->property(MidiParser::mpAutoLoop, true);
}

void SoundMidiPC::loadSoundEffectFile(const char *file) {
	char filename[25];
	sprintf(filename, "%s.XMI", file);

	uint32 size;
	uint8 *data = (_engine->resource())->fileData(filename, &size);

	if (!data) {
		warning("couldn't load '%s'", filename);
		return;
	}

	loadSoundEffectFile(data, size);
}

void SoundMidiPC::loadSoundEffectFile(uint8 *data, uint32 size) {
	stopSoundEffect();

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

	// this should be set to the fadeToBlack value
	static const uint32 musicFadeTime = 2 * 1000;

	if (music->_fadeMusicOut && music->_fadeStartTime + musicFadeTime > music->_engine->_system->getMillis()) {
		byte volume = (byte)((musicFadeTime - (music->_engine->_system->getMillis() - music->_fadeStartTime)) * 255 / musicFadeTime);
		music->setVolume(volume);
	} else if (music->_fadeStartTime) {
		music->setVolume(255);
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
		for (int i = 0; i < 16; ++i) {
			music->send(0x007BB0 | i);
		}
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
	if (_parser && (track != 0 || _nativeMT32)) {
		_isPlaying = true;
		_fadeMusicOut = false;
		_fadeStartTime = 0;
		setVolume(255);
		_parser->setTrack(track);
		_parser->jumpToTick(0);
		_parser->setTempo(1);
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
	if (_soundEffect) {
		_sfxIsPlaying = true;
		_soundEffect->setTrack(track);
		_soundEffect->jumpToTick(0);
		_soundEffect->setTempo(1);
		_soundEffect->property(MidiParser::mpAutoLoop, false);
	}
}

void SoundMidiPC::beginFadeOut() {
	_fadeMusicOut = true;
	_fadeStartTime = _engine->_system->getMillis();
}

#pragma mark -

void KyraEngine::snd_playTheme(int file, int track) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine::snd_playTheme(%d)", file);
	assert(file < _musicFilesCount);
	_curMusicTheme = _newMusicTheme = file;
	_sound->loadMusicFile(_musicFiles[file]);
	_sound->playTrack(track);
}

void KyraEngine::snd_playSoundEffect(int track) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine::snd_playSoundEffect(%d)", track);
	_sound->playSoundEffect(track);
}

void KyraEngine::snd_playWanderScoreViaMap(int command, int restart) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine::snd_playWanderScoreViaMap(%d, %d)", command, restart);
	static const int8 soundTable[] = {
		-1,   0,  -1,   1,   0,   3,   0,   2,
		 0,   4,   1,   2,   1,   3,   1,   4,
		 1,  92,   1,   6,   1,   7,   2,   2,
		 2,   3,   2,   4,   2,   5,   2,   6,
		 2,   7,   3,   3,   3,   4,   1,   8,
		 1,   9,   4,   2,   4,   3,   4,   4,
		 4,   5,   4,   6,   4,   7,   4,   8,
		 1,  11,   1,  12,   1,  14,   1,  13,
		 4,   9,   5,  12,   6,   2,   6,   6,
		 6,   7,   6,   8,   6,   9,   6,   3,
		 6,   4,   6,   5,   7,   2,   7,   3,
		 7,   4,   7,   5,   7,   6,   7,   7,
		 7,   8,   7,   9,   8,   2,   8,   3,
		 8,   4,   8,   5,   6,  11,   5,  11
	};
	//if (!_disableSound) {
	//	XXX
	//}
	assert(command*2+1 < ARRAYSIZE(soundTable));
	if (_curMusicTheme != soundTable[command*2]+1) {
		if (soundTable[command*2] != -1) {
			snd_playTheme(soundTable[command*2]+1);
		}
	}
	
	if (restart)
		_lastMusicCommand = -1;
	
	if (command != 1) {
		if (_lastMusicCommand != command) {
			_lastMusicCommand = command;
			_sound->haltTrack();
			_sound->playTrack(soundTable[command*2+1]);
		}
	} else {
		_lastMusicCommand = 1;
		_sound->beginFadeOut();
	}
}

void KyraEngine::snd_playVoiceFile(int id) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine::snd_playVoiceFile(%d)", id);
	char vocFile[9];
	assert(id >= 0 && id < 9999);
	sprintf(vocFile, "%03d", id);
	_sound->voicePlay(vocFile);
}

void KyraEngine::snd_voiceWaitForFinish(bool ingame) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine::snd_voiceWaitForFinish(%d)", ingame);
	while (_sound->voiceIsPlaying() && !_skipFlag) {
		if (ingame) {
			delay(10, true);
		} else {
			_system->delayMillis(10);
		}
	}
}

// static res

const Sound::SpeechCodecs Sound::_supportedCodes[] = {
#ifdef USE_MAD
	{ ".VO3", Audio::makeMP3Stream },
#endif // USE_MAD
#ifdef USE_VORBIS
	{ ".VOG", Audio::makeVorbisStream },
#endif // USE_VORBIS
#ifdef USE_FLAC
	{ ".VOF", Audio::makeFlacStream },
#endif // USE_FLAC
	{ 0, 0 }
};

} // end of namespace Kyra
