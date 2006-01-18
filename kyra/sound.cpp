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
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "common/system.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

namespace Kyra {

MusicPlayer::MusicPlayer(MidiDriver *driver, KyraEngine *engine) {
	_engine = engine;
	_driver = driver;
	_passThrough = false;
	_eventFromMusic = false;
	_fadeMusicOut = _sfxIsPlaying = false;
	_fadeStartTime = 0;
	_isPlaying = _isLooping = _nativeMT32 = false;
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

MusicPlayer::~MusicPlayer() {
	_driver->setTimerCallback(NULL, NULL);
	close();
}

void MusicPlayer::setVolume(int volume) {
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

int MusicPlayer::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;

	_driver->setTimerCallback(this, &onTimer);
	return 0;
}

void MusicPlayer::close() {
	if (_driver)
		_driver->close();
	_driver = 0;
}

void MusicPlayer::send(uint32 b) {
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
		_channel[_virChannel[channel]]->volume(_channelVolume[channel] * _volume / 255);
	}
	if (_channel[_virChannel[channel]])
		_channel[_virChannel[channel]]->send(b);
}

void MusicPlayer::metaEvent(byte type, byte *data, uint16 length) {
	switch (type) {
	case 0x2F:	// End of Track
		if (_eventFromMusic) {
			if (!_isLooping) {
				_isPlaying = false;
			}
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

void MusicPlayer::playMusic(const char *file) {
	uint32 size;
	uint8 *data = (_engine->resource())->fileData(file, &size);

	if (!data) {
		warning("couldn't load '%s'", file);
		return;
	}

	playMusic(data, size);
}

void MusicPlayer::playMusic(uint8 *data, uint32 size) {
	stopMusic();

	_parserSource = data;
	_parser = MidiParser::createParser_XMIDI();
	assert(_parser);

	if (!_parser->loadMusic(data, size)) {
		warning("Error reading track!");
		delete _parser;
		_parser = 0;
		return;
	}

	_parser->setTrack(0);
	_parser->setMidiDriver(this);
	_parser->setTimerRate(getBaseTempo());
	_parser->property(MidiParser::mpAutoLoop, false);
}

void MusicPlayer::loadSoundEffectFile(const char *file) {
	uint32 size;
	uint8 *data = (_engine->resource())->fileData(file, &size);

	if (!data) {
		warning("couldn't load '%s'", file);
		return;
	}

	loadSoundEffectFile(data, size);
}

void MusicPlayer::loadSoundEffectFile(uint8 *data, uint32 size) {
	stopSoundEffect();

	_soundEffectSource = data;
	_soundEffect = MidiParser::createParser_XMIDI();
	assert(_soundEffect);

	if (!_soundEffect->loadMusic(data, size)) {
		warning("Error reading track!");
		delete _parser;
		_parser = 0;
		return;
	}

	_soundEffect->setTrack(0);
	_soundEffect->setMidiDriver(this);
	_soundEffect->setTimerRate(getBaseTempo());
	_soundEffect->property(MidiParser::mpAutoLoop, false);
}

void MusicPlayer::stopMusic() {
	_isLooping = false;
	_isPlaying = false;
	if (_parser) {
		_parser->unloadMusic();
		delete _parser;
		_parser = 0;
		delete [] _parserSource;
		_parserSource = 0;
	}
}

void MusicPlayer::stopSoundEffect() {
	_sfxIsPlaying = false;
	if (_soundEffect) {
		_soundEffect->unloadMusic();
		delete _soundEffect;
		_soundEffect = 0;
		delete [] _soundEffectSource;
		_soundEffectSource = 0;
	}
}

void MusicPlayer::onTimer(void *refCon) {
	MusicPlayer *music = (MusicPlayer *)refCon;

	// this should be set to the fadeToBlack value
	const static uint32 musicFadeTime = 2 * 1000;
	if (music->_fadeMusicOut && music->_fadeStartTime + musicFadeTime > music->_engine->_system->getMillis()) {
		byte volume = (byte)((musicFadeTime - (music->_engine->_system->getMillis() - music->_fadeStartTime)) * 255 / musicFadeTime);
		music->setVolume(volume);
	} else if(music->_fadeStartTime) {
		music->setVolume(255);
		music->_fadeStartTime = 0;
		music->_fadeMusicOut = false;
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

void MusicPlayer::playTrack(uint8 track, bool loop) {
	if (_parser) {
		_isPlaying = true;
		_isLooping = loop;
		_parser->setTrack(track);
		_parser->jumpToTick(0);
		_parser->setTempo(1);
		_parser->property(MidiParser::mpAutoLoop, loop);
	}
}

void MusicPlayer::playSoundEffect(uint8 track) {
	if (_soundEffect) {
		_sfxIsPlaying = true;
		_soundEffect->setTrack(track);
		_soundEffect->jumpToTick(0);
		_soundEffect->property(MidiParser::mpAutoLoop, false);
	}
}

void MusicPlayer::beginFadeOut() {
	// this should be something like fade out...
	_fadeMusicOut = true;
	_fadeStartTime = _engine->_system->getMillis();
}

} // end of namespace Kyra
