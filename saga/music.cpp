/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "saga/saga.h"

#include "saga/music.h"
#include "saga/rscfile_mod.h"
#include "saga/stream.h"
#include "sound/audiostream.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "common/config-manager.h"
#include "common/file.h"

namespace Saga {

#define BUFFER_SIZE 4096

// I haven't decided yet if it's a good idea to make looping part of the audio
// stream class, or if I should use a "wrapper" class, like I did for Broken
// Sword 2, to make it easier to add support for compressed music... but I'll
// worry about that later.

class RAWInputStream : public AudioStream {
private:
	Common::File *_file;
	uint32 _file_pos;
	uint32 _start_pos;
	uint32 _end_pos;
	bool _finished;
	bool _looping;
	int16 _buf[BUFFER_SIZE];
	const int16 *_bufferEnd;
	const int16 *_pos;

	void refill();
	inline bool eosIntern() const;

public:
	RAWInputStream(Common::File *file, int size, bool looping);
	~RAWInputStream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const	{ return eosIntern(); }
	bool isStereo() const	{ return true; }
	int getRate() const	{ return 11025; }
};

RAWInputStream::RAWInputStream(Common::File *file, int size, bool looping)
	: _file(file), _finished(false), _looping(looping),
	  _bufferEnd(_buf + BUFFER_SIZE) {

	_file->incRef();

	// Determine the end position
	_file_pos = _file->pos();
	_start_pos = _file_pos;
	_end_pos = _file_pos + size;

	// Read in initial data
	refill();
}

RAWInputStream::~RAWInputStream() {
	_file->decRef();
}

inline bool RAWInputStream::eosIntern() const {
	return _pos >= _bufferEnd;
}

int RAWInputStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		const int len = MIN(numSamples - samples, (int) (_bufferEnd - _pos));
		memcpy(buffer, _pos, len * 2);
		buffer += len;
		_pos += len;
		samples += len;
		if (_pos >= _bufferEnd) {
			refill();
		}
	}
	return samples;
}

void RAWInputStream::refill() {
	if (_finished)
		return;

	uint32 len_left;
	byte *ptr = (byte *) _buf;
	

	_file->seek(_file_pos, SEEK_SET);

	if (_looping)
		len_left = 2 * BUFFER_SIZE;
	else
		len_left = MIN((uint32) (2 * BUFFER_SIZE), _end_pos - _file_pos);

	while (len_left > 0) {
		uint32 len = _file->read(ptr, MIN(len_left, _end_pos - _file->pos()));

		if (len & 1)
			len--;

		if (_vm->getFeatures() & GF_BIG_ENDIAN_DATA) {			
			uint16 *ptr16 = (uint16 *)ptr;
			for (uint32 i = 0; i < (len / 2); i++)
				ptr16[i] = TO_BE_16(ptr16[i]);
		}
			
		len_left -= len;
		ptr += len;

		if (len_left > 0)
			_file->seek(_start_pos);
	}

	_file_pos = _file->pos();
	_pos = _buf;
	_bufferEnd = (int16 *)ptr;

	if (!_looping && _file_pos >= _end_pos)
		_finished = true;
}

AudioStream *makeRAWStream(const char *filename, uint32 pos, int size, bool looping) {
	Common::File *file = new Common::File();

	if (!file->open(filename)) {
		delete file;
		return NULL;
	}

	file->seek(pos);

	AudioStream *audioStream = new RAWInputStream(file, size, looping);

	file->decRef();
	return audioStream;
}

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
	if (volume < 0)
		volume = 0;
	else if (volume > 255)
		volume = 255;

	if (_masterVolume == volume)
		return;

	_masterVolume = volume;

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
	if (music->_isPlaying)
		music->_parser->onTimer();
}

void MusicPlayer::playMusic() {
	_isPlaying = true;
}

void MusicPlayer::stopMusic() {
	_isPlaying = false;
	if (_parser) {
		_parser->unloadMusic();
		delete _parser;
		_parser = NULL;
	}
}

Music::Music(SoundMixer *mixer, MidiDriver *driver, int enabled) : _mixer(mixer), _enabled(enabled), _adlib(false) {
	_player = new MusicPlayer(driver);
	_musicInitialized = 1;
	_mixer->setVolumeForSoundType(SoundMixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	if (_vm->getGameType() == GType_ITE) {
		Common::File file;
		byte footerBuf[ARRAYSIZE(_digiTableITECD) * 8];

		// The lookup table is stored at the end of music.rsc. I don't
		// know why it has 27 elements, but the last one represents a
		// very short tune. Perhaps it's a dummy?
		//
		// FIXME: Well, it's a hack which I don't like. Logically it should
		// call LoadResource() et al, but I don't want to load those
		// huge files into memory. So I use FileContext just for getting file
		// name and reuse its opened file a bit.
		//
		// Proper approach would be to extend resource manager so it could
		// return File object.

		_musicContext = _vm->getFileContext(GAME_MUSICFILE, 0);
		if (_musicContext != NULL) {
			_hasDigiMusic = true;

			_musicFname = RSC_FileName(_musicContext);

			file.open(_musicFname);
			assert(file.size() > sizeof(footerBuf));

			file.seek(-ARRAYSIZE(_digiTableITECD) * 8, SEEK_END);
			file.read(footerBuf, sizeof(footerBuf));

			MemoryReadStreamEndian readS(footerBuf, sizeof(footerBuf), IS_BIG_ENDIAN);


			for (int i = 0; i < ARRAYSIZE(_digiTableITECD); i++) {
				_digiTableITECD[i].start = readS.readUint32();
				_digiTableITECD[i].length = readS.readUint32();
			}

			file.close();

			// The "birdchrp" is just a short, high-pitched
			// whining. Use the MIDI/XMIDI version instead.
			_digiTableITECD[6].length = 0;
		} else {
			_hasDigiMusic = false;
			memset(_digiTableITECD, 0, sizeof(_digiTableITECD));
		}
	}
}

Music::~Music() {
	delete _player;
}

bool Music::isPlaying() {
	return _mixer->isSoundHandleActive(_musicHandle) || _player->isPlaying();
}

// The Wyrmkeep release of Inherit The Earth features external MIDI files, so
// we need a mapping from resource number to filename.
//
// reset.mid seems to be unused.

const MUSIC_MIDITABLE Music::_midiTableITECD[26] = {
	{ "cave.mid",       MUSIC_LOOP },	// 9
	{ "intro.mid",      MUSIC_LOOP },	// 10
	{ "fvillage.mid",   MUSIC_LOOP },	// 11
	{ "elkhall.mid",    MUSIC_LOOP },	// 12
	{ "mouse.mid",      0          },	// 13
	{ "darkclaw.mid",   MUSIC_LOOP },	// 14
	{ "birdchrp.mid",   MUSIC_LOOP },	// 15
	{ "orbtempl.mid",   MUSIC_LOOP },	// 16
	{ "spooky.mid",     MUSIC_LOOP },	// 17
	{ "catfest.mid",    MUSIC_LOOP },	// 18
	{ "elkfanfare.mid", 0          },	// 19
	{ "bcexpl.mid",     MUSIC_LOOP },	// 20
	{ "boargtnt.mid",   MUSIC_LOOP },	// 21
	{ "boarking.mid",   MUSIC_LOOP },	// 22
	{ "explorea.mid",   MUSIC_LOOP },	// 23
	{ "exploreb.mid",   MUSIC_LOOP },	// 24
	{ "explorec.mid",   MUSIC_LOOP },	// 25
	{ "sunstatm.mid",   MUSIC_LOOP },	// 26
	{ "nitstrlm.mid",   MUSIC_LOOP },	// 27
	{ "humruinm.mid",   MUSIC_LOOP },	// 28
	{ "damexplm.mid",   MUSIC_LOOP },	// 29
	{ "tychom.mid",     MUSIC_LOOP },	// 30
	{ "kitten.mid",     MUSIC_LOOP },	// 31
	{ "sweet.mid",      MUSIC_LOOP },	// 32
	{ "brutalmt.mid",   MUSIC_LOOP },	// 33
	{ "shiala.mid",     MUSIC_LOOP }	// 34
};

int Music::play(uint32 music_rn, uint16 flags) {
	RSCFILE_CONTEXT *rsc_ctxt = NULL;

	byte *resource_data;
	size_t resource_size;

	if (!_musicInitialized) {
		return FAILURE;
	}

	if (!_enabled) {
		return SUCCESS;
	}

	if (isPlaying() && _trackNumber == music_rn) {
		return SUCCESS;
	}

	_trackNumber = music_rn;

	_player->stopMusic();

	_mixer->stopHandle(_musicHandle);

	AudioStream *audioStream = NULL;
	MidiParser *parser;
	Common::File midiFile;

	if (_vm->getGameType() == GType_ITE) {
		if (music_rn >= 9 && music_rn <= 34) {
			if (flags == MUSIC_DEFAULT) {
				flags = _midiTableITECD[music_rn - 9].flags;
			}

			if (_hasDigiMusic) {
				uint32 start = _digiTableITECD[music_rn - 9].start;
				uint32 length = _digiTableITECD[music_rn - 9].length;

				if (length > 0) {
					audioStream = makeRAWStream(_musicFname, start, length, flags == MUSIC_LOOP);
				}
			}

			// No digitized music - try standalone MIDI.
			if (!audioStream) {
				midiFile.open(_midiTableITECD[music_rn - 9].filename);

				if (!midiFile.isOpen()) {
					warning("Cannot open music file %s", _midiTableITECD[music_rn - 9].filename);
				}
			}
		}
	}

	if (flags == MUSIC_DEFAULT) {
		flags = 0;
	}

	if (audioStream) {
		debug(0, "Playing digitized music");
		_mixer->playInputStream(SoundMixer::kMusicSoundType, &_musicHandle, audioStream);
		return SUCCESS;
	}

	// FIXME: Is resource_data ever freed?

	if (midiFile.isOpen()) {
		debug(0, "Using external MIDI file: %s", midiFile.name());
		resource_size = midiFile.size();
		resource_data = (byte *) malloc(resource_size);
		midiFile.read(resource_data, resource_size);
		midiFile.close();

		_player->setGM(true);
		parser = MidiParser::createParser_SMF();
	} else {
		// Load MIDI/XMI resource data

		if (_vm->getGameType() == GType_ITE) {
			rsc_ctxt = _vm->getFileContext(GAME_RESOURCEFILE, 0);
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

			if (hasAdlib()) {
				rsc_ctxt = _vm->getFileContext(GAME_MUSICFILE_FM, 0);
			} else {
				rsc_ctxt = _vm->getFileContext(GAME_MUSICFILE_GM, 0);
			}

			_player->setGM(true);
		}

		if (RSC_LoadResource(rsc_ctxt, music_rn, &resource_data, 
				&resource_size) != SUCCESS) {
			warning("Music::play(): Resource load failed: %u", music_rn);
			return FAILURE;
		}

		parser = MidiParser::createParser_XMIDI();
	}

	if (resource_size <= 0) {
		warning("Music::play(): Resource load failed: %u", music_rn);
		return FAILURE;
	}

	if (!parser->loadMusic(resource_data, resource_size)) {
		warning("Error reading track!");
		delete parser;
		parser = 0;
	}

	debug(0, "Music::play(%d, %d)", music_rn, flags);

	parser->setTrack(0);
	parser->setMidiDriver(_player);
	parser->setTimerRate(_player->getBaseTempo());

	_player->_parser = parser;
	_player->setVolume(ConfMan.getInt("music_volume"));
	if (flags & MUSIC_LOOP)
		_player->setLoop(true);
	else
		_player->setLoop(false);

	_player->playMusic();
	return SUCCESS;
}

int Music::pause(void) {
	if (!_musicInitialized) {
		return FAILURE;
	}

	return SUCCESS;
}

int Music::resume(void) {
	if (!_musicInitialized) {
		return FAILURE;
	}

	return SUCCESS;
}

int Music::stop(void) {
	if (!_musicInitialized) {
		return FAILURE;
	}

	return SUCCESS;
}

} // End of namespace Saga

