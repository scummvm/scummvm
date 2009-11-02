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

// MIDI and digital music class

#include "saga/saga.h"

#include "saga/resource.h"
#include "saga/music.h"

#include "sound/audiostream.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "common/config-manager.h"
#include "common/file.h"

namespace Saga {

#define BUFFER_SIZE 4096
#define MUSIC_SUNSPOT 26

MusicPlayer::MusicPlayer(MidiDriver *driver) : _parser(0), _driver(driver), _looping(false), _isPlaying(false), _passThrough(false), _isGM(false) {
	memset(_channel, 0, sizeof(_channel));
	_masterVolume = 0;
	this->open();
}

MusicPlayer::~MusicPlayer() {
	_driver->setTimerCallback(NULL, NULL);
	stopMusic();
	this->close();
}

void MusicPlayer::setVolume(int volume) {
	volume = CLIP(volume, 0, 255);

	if (_masterVolume == volume)
		return;

	_masterVolume = volume;

	Common::StackLock lock(_mutex);

	for (int i = 0; i < 16; ++i) {
		if (_channel[i]) {
			_channel[i]->volume(_channelVolume[i] * _masterVolume / 255);
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
	stopMusic();
	if (_driver)
		_driver->close();
	_driver = 0;
}

void MusicPlayer::send(uint32 b) {
	if (_passThrough) {
		_driver->send(b);
		return;
	}

	byte channel = (byte)(b & 0x0F);
	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume
		byte volume = (byte)((b >> 16) & 0x7F);
		_channelVolume[channel] = volume;
		volume = volume * _masterVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xF0) == 0xC0 && !_isGM && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}
	else if ((b & 0xFFF0) == 0x007BB0) {
		//Only respond to All Notes Off if this channel
		//has currently been allocated
		if (_channel[b & 0x0F])
			return;
	}

	if (!_channel[channel])
		_channel[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();

	if (_channel[channel])
		_channel[channel]->send(b);
}

void MusicPlayer::metaEvent(byte type, byte *data, uint16 length) {
	// FIXME: The "elkfanfare" is played much too quickly. There are some
	//        meta events that we don't handle. Perhaps there is a
	//        connection...?

	switch (type) {
	case 0x2F:	// End of Track
		if (_looping)
			_parser->jumpToTick(0);
		else
			stopMusic();
		break;
	default:
		//warning("Unhandled meta event: %02x", type);
		break;
	}
}

void MusicPlayer::onTimer(void *refCon) {
	MusicPlayer *music = (MusicPlayer *)refCon;
	Common::StackLock lock(music->_mutex);

	if (music->_isPlaying)
		music->_parser->onTimer();
}

void MusicPlayer::playMusic() {
	_isPlaying = true;
}

void MusicPlayer::stopMusic() {
	Common::StackLock lock(_mutex);

	_isPlaying = false;
	if (_parser) {
		_parser->unloadMusic();
		_parser = NULL;
	}
}

Music::Music(SagaEngine *vm, Audio::Mixer *mixer, MidiDriver *driver) : _vm(vm), _mixer(mixer), _adlib(false) {
	_player = new MusicPlayer(driver);
	_currentVolume = 0;

	xmidiParser = MidiParser::createParser_XMIDI();
	smfParser = MidiParser::createParser_SMF();

	_digitalMusicContext = _vm->_resource->getContext(GAME_DIGITALMUSICFILE);

	_songTableLen = 0;
	_songTable = 0;

	_midiMusicData = NULL;
	_digitalMusic = false;
}

Music::~Music() {
	_vm->getTimerManager()->removeTimerProc(&musicVolumeGaugeCallback);
	_mixer->stopHandle(_musicHandle);
	delete _player;
	xmidiParser->setMidiDriver(NULL);
	smfParser->setMidiDriver(NULL);
	delete xmidiParser;
	delete smfParser;

	free(_songTable);
	free(_midiMusicData);
}

void Music::musicVolumeGaugeCallback(void *refCon) {
	((Music *)refCon)->musicVolumeGauge();
}

void Music::musicVolumeGauge() {
	int volume;

	_currentVolumePercent += 10;

	if (_currentVolume - _targetVolume > 0) { // Volume decrease
		volume = _targetVolume + (_currentVolume - _targetVolume) * (100 - _currentVolumePercent) / 100;
	} else {
		volume = _currentVolume + (_targetVolume - _currentVolume) * _currentVolumePercent / 100;
	}

	if (volume < 0)
		volume = 1;

	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);
	_player->setVolume(volume);

	if (_currentVolumePercent == 100) {
		_vm->getTimerManager()->removeTimerProc(&musicVolumeGaugeCallback);
		_currentVolume = _targetVolume;
	}
}

void Music::setVolume(int volume, int time) {
	_targetVolume = volume;
	_currentVolumePercent = 0;

	if (volume == -1) // Set Full volume
		volume = 255;

	if (time == 1) {
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);
		_player->setVolume(volume);
		_vm->getTimerManager()->removeTimerProc(&musicVolumeGaugeCallback);
		_currentVolume = volume;
		return;
	}

	_vm->getTimerManager()->installTimerProc(&musicVolumeGaugeCallback, time * 100L, this);
}

bool Music::isPlaying() {
	return _mixer->isSoundHandleActive(_musicHandle) || _player->isPlaying();
}

void Music::play(uint32 resourceId, MusicFlags flags) {
	Audio::AudioStream *audioStream = NULL;
	MidiParser *parser;
	ResourceContext *context = NULL;
	byte *resourceData;
	size_t resourceSize;
	uint32 loopStart;

	debug(2, "Music::play %d, %d", resourceId, flags);

	if (isPlaying() && _trackNumber == resourceId) {
		return;
	}

	_trackNumber = resourceId;
	_player->stopMusic();
	_mixer->stopHandle(_musicHandle);

	int realTrackNumber;

	if (_vm->getGameId() == GID_ITE) {
		if (flags == MUSIC_DEFAULT) {
			if (resourceId == 13 || resourceId == 19) {
				flags = MUSIC_NORMAL;
			} else {
				flags = MUSIC_LOOP;
			}
		}
		realTrackNumber = resourceId - 8;
	} else {
		realTrackNumber = resourceId + 1;
	}

	// Try to open standalone digital track
	char trackName[2][16];
	sprintf(trackName[0], "track%d", realTrackNumber);
	sprintf(trackName[1], "track%02d", realTrackNumber);
	Audio::AudioStream *stream = 0;
	for (int i = 0; i < 2; ++i) {
		stream = Audio::AudioStream::openStreamFile(trackName[i], 0, 0, (flags == MUSIC_LOOP) ? 0 : 1);
		if (stream) {
			_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, stream);
			_digitalMusic = true;
			return;
		}
	}

	if (_vm->getGameId() == GID_ITE) {
		if (resourceId >= 9 && resourceId <= 34) {
			if (_digitalMusicContext != NULL) {
				loopStart = 0;
				// Fix ITE sunstatm/sunspot score
				if (resourceId == MUSIC_SUNSPOT)
					loopStart = 4 * 18727;

				// Digital music
				ResourceData *resData = _digitalMusicContext->getResourceData(resourceId - 9);
				Common::File *musicFile = _digitalMusicContext->getFile(resData);
				int offs = (_digitalMusicContext->isCompressed) ? 9 : 0;

				Common::SeekableSubReadStream *musicStream = new Common::SeekableSubReadStream(musicFile, 
							(uint32)resData->offset + offs, (uint32)resData->offset + resData->size - offs);

				if (!_digitalMusicContext->isCompressed) {
					byte musicFlags = Audio::Mixer::FLAG_AUTOFREE | Audio::Mixer::FLAG_STEREO | 
										Audio::Mixer::FLAG_16BITS | Audio::Mixer::FLAG_LITTLE_ENDIAN;
					if (flags == MUSIC_LOOP)
						musicFlags |= Audio::Mixer::FLAG_LOOP;

					Audio::LinearDiskStreamAudioBlock audioBlocks[1];
					audioBlocks[0].pos = 0;
					audioBlocks[0].len = resData->size / 2;	// 16-bit sound
					audioStream = Audio::makeLinearDiskStream(musicStream, audioBlocks, 1, 11025, musicFlags, false, loopStart, 0);
				} else {
					// Read compressed header to determine compression type
					musicFile->seek((uint32)resData->offset, SEEK_SET);
					byte identifier = musicFile->readByte();

					if (identifier == 0) {		// MP3
#ifdef USE_MAD
						audioStream = Audio::makeMP3Stream(musicStream, false, 0, 0, (flags == MUSIC_LOOP ? 0 : 1));
#endif
					} else if (identifier == 1) {	// OGG
#ifdef USE_VORBIS
						audioStream = Audio::makeVorbisStream(musicStream, false, 0, 0, (flags == MUSIC_LOOP ? 0 : 1));
#endif
					} else if (identifier == 2) {	// FLAC
#ifdef USE_FLAC
						audioStream = Audio::makeFlacStream(musicStream, false, 0, 0, (flags == MUSIC_LOOP ? 0 : 1));
#endif
					}
				}
			}
		}
	}

	if (audioStream) {
		debug(2, "Playing digitized music");
		_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, audioStream);
		_digitalMusic = true;
		return;
	}

	if (flags == MUSIC_DEFAULT) {
		flags = MUSIC_NORMAL;
	}

	// Load MIDI/XMI resource data

	if (_vm->getGameId() == GID_ITE) {
		context = _vm->_resource->getContext(GAME_MUSICFILE_GM);
		if (context == NULL) {
			context = _vm->_resource->getContext(GAME_RESOURCEFILE);
		}
	} else if (_vm->getGameId() == GID_IHNM && _vm->isMacResources()) {
		// The music of the Mac version of IHNM is loaded from its
		// associated external file later on
	} else {
		// I've listened to music from both the FM and the GM
		// file, and I've tentatively reached the conclusion
		// that they are both General MIDI. My guess is that
		// the FM file has been reorchestrated to sound better
		// on Adlib and other FM synths.
		//
		// Sev says the Adlib music does not sound like in the
		// original, but I still think assuming General MIDI is
		// the right thing to do. Some music, like the End
		// Title (song 0) sound absolutely atrocious when piped
		// through our MT-32 to GM mapping.
		//
		// It is, however, quite possible that the original
		// used a different GM to FM mapping. If the original
		// sounded markedly better, perhaps we should add some
		// way of replacing our stock mapping in adlib.cpp?
		//
		// For the composer's own recording of the End Title,
		// see http://www.johnottman.com/

		// Oddly enough, the intro music (song 1) is very
		// different in the two files. I have no idea why.
		// Note that the IHNM demo has only got one music file
		// (music.rsc). It is assumed that it contains FM music

		if (hasAdlib() || _vm->getFeatures() & GF_IHNM_DEMO) {
			context = _vm->_resource->getContext(GAME_MUSICFILE_FM);
		} else {
			context = _vm->_resource->getContext(GAME_MUSICFILE_GM);
		}
	}

	_player->setGM(true);

	if (_vm->getGameId() == GID_IHNM && _vm->isMacResources()) {
		// Load the external music file for Mac IHNM
		Common::File musicFile;
		char musicFileName[40];
		sprintf(musicFileName, "Music/Music%02x", resourceId);
		musicFile.open(musicFileName);
		resourceSize = musicFile.size();
		resourceData = new byte[resourceSize];
		musicFile.read(resourceData, resourceSize);
		musicFile.close();

		// TODO: The Mac music format is unsupported (QuickTime MIDI)
		// so stop here
		return;
	} else {
		_vm->_resource->loadResource(context, resourceId, resourceData, resourceSize);
	}

	if (resourceSize < 4) {
		error("Music::play() wrong music resource size");
	}

	if (xmidiParser->loadMusic(resourceData, resourceSize)) {
		if (_vm->getGameId() == GID_ITE)
			_player->setGM(false);

		parser = xmidiParser;
	} else {
		if (smfParser->loadMusic(resourceData, resourceSize)) {
			parser = smfParser;
		} else {
			error("Music::play() wrong music resource");
		}
	}

	parser->setTrack(0);
	parser->setMidiDriver(_player);
	parser->setTimerRate(_player->getBaseTempo());
	parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

	_player->_parser = parser;
	setVolume(_vm->_musicVolume);

	if (flags & MUSIC_LOOP)
		_player->setLoop(true);
	else
		_player->setLoop(false);

	_player->playMusic();
	free(_midiMusicData);
	_midiMusicData = resourceData;
}

void Music::pause() {
	_player->setVolume(-1);
	_player->setPlaying(false);
}

void Music::resume() {
	_player->setVolume(_vm->_musicVolume);
	_player->setPlaying(true);
}

void Music::stop() {
	_player->stopMusic();
}

} // End of namespace Saga

