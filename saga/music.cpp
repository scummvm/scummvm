/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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
#include "saga.h"
#include "reinherit.h"

#include "yslib.h"

#include "music.h"
#include "rscfile_mod.h"
#include "game_mod.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "common/config-manager.h"
#include "common/file.h"

namespace Saga {

// Instrument mapping for MT32 tracks emulated under GM.
static const byte mt32_to_gm[128] = {
//    0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	  0,   1,   0,   2,   4,   4,   5,   3,  16,  17,  18,  16,  16,  19,  20,  21, // 0x
	  6,   6,   6,   7,   7,   7,   8, 112,  62,  62,  63,  63,  38,  38,  39,  39, // 1x
	 88,  95,  52,  98,  97,  99,  14,  54, 102,  96,  53, 102,  81, 100,  14,  80, // 2x
	 48,  48,  49,  45,  41,  40,  42,  42,  43,  46,  45,  24,  25,  28,  27, 104, // 3x
	 32,  32,  34,  33,  36,  37,  35,  35,  79,  73,  72,  72,  74,  75,  64,  65, // 4x
	 66,  67,  71,  71,  68,  69,  70,  22,  56,  59,  57,  57,  60,  60,  58,  61, // 5x
	 61,  11,  11,  98,  14,   9,  14,  13,  12, 107, 107,  77,  78,  78,  76,  76, // 6x
	 47, 117, 127, 118, 118, 116, 115, 119, 115, 112,  55, 124, 123,   0,  14, 117  // 7x
};

MusicPlayer::MusicPlayer(MidiDriver *driver) : _parser(0), _driver(driver), _looping(false), _isPlaying(false)  {
	memset(_channel, 0, sizeof(_channel));
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
	byte channel = (byte)(b & 0x0F);
	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume
		byte volume = (byte)((b >> 16) & 0x7F);
		_channelVolume[channel] = volume;
		volume = volume * _masterVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xF0) == 0xC0 && !_isGM && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | mt32_to_gm[(b >> 8) & 0xFF] << 8;
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
	//Only thing we care about is End of Track.
	if (type != 0x2F)
		return;

	if (_looping)
		_parser->jumpToTick(0);
	else
		stopMusic();
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
	}
}

Music::Music(MidiDriver *driver, int enabled) : _enabled(enabled) {
	_player = new MusicPlayer(driver);
	_musicInitialized = 1;
}

Music::~Music() {
	delete _player;
}

// The Wyrmkeep release of Inherit The Earth features external MIDI files, so
// we need a mapping from resource number to filename.
//
// reset.mid seems to be unused.

const MUSIC_MIDITABLE Music::_midiTableITECD[26] = {
	{"cave", R_MUSIC_LOOP},		// 9
	{"intro", R_MUSIC_LOOP},	// 10
	{"fvillage", R_MUSIC_LOOP},	// 11
	{"elkhall", R_MUSIC_LOOP},	// 12
	{"mouse", 0},				// 13
	{"darkclaw", R_MUSIC_LOOP},	// 14
	{"birdchrp", R_MUSIC_LOOP},	// 15
	{"orbtempl", R_MUSIC_LOOP},	// 16
	{"spooky", R_MUSIC_LOOP},	// 17
	{"catfest", R_MUSIC_LOOP},	// 18
	{"elkfanfare", 0},			// 19
	{"bcexpl", R_MUSIC_LOOP},	// 20
	{"boargtnt", R_MUSIC_LOOP},	// 21
	{"boarking", R_MUSIC_LOOP},	// 22
	{"explorea", R_MUSIC_LOOP},	// 23
	{"exploreb", R_MUSIC_LOOP},	// 24
	{"explorec", R_MUSIC_LOOP},	// 25
	{"sunstatm", R_MUSIC_LOOP},	// 26
	{"nitstrlm", R_MUSIC_LOOP},	// 27
	{"humruinm", R_MUSIC_LOOP},	// 28
	{"damexplm", R_MUSIC_LOOP},	// 29
	{"tychom", R_MUSIC_LOOP},	// 30
	{"kitten", R_MUSIC_LOOP},	// 31
	{"sweet", R_MUSIC_LOOP},	// 32
	{"brutalmt", R_MUSIC_LOOP},	// 33
	{"shiala", R_MUSIC_LOOP}	// 34
};

int Music::play(uint32 music_rn, uint16 flags) {
	R_RSCFILE_CONTEXT *rsc_ctxt = NULL;

	byte *resource_data;
	size_t resource_size;

	if (!_musicInitialized) {
		return R_FAILURE;
	}

	if (!_enabled) {
		return R_SUCCESS;
	}

	File f_midi;
	MidiParser *parser;

	if (GAME_GetGameType() == R_GAMETYPE_ITE) {
		if (music_rn >= 9 && music_rn <= 34) {
			char file_name[20];
			sprintf(file_name, "music/%s.mid", _midiTableITECD[music_rn - 9].filename);
			f_midi.open(file_name);
		}
	}

	_player->stopMusic();

	// FIXME: Is resource_data ever freed?

	if (f_midi.isOpen()) {
		debug(0, "Using external MIDI file: %s", f_midi.name());
		resource_size = f_midi.size();
		resource_data = (byte *) malloc(resource_size);
		f_midi.read(resource_data, resource_size);
		f_midi.close();

		_player->setGM(true);
		parser = MidiParser::createParser_SMF();

		// FIXME: Is this really the case or we receive correct parameter?
		flags = _midiTableITECD[music_rn - 9].flags;
	} else {
		/* Load XMI resource data */
		GAME_GetFileContext(&rsc_ctxt, R_GAME_RESOURCEFILE, 0);

		if (RSC_LoadResource(rsc_ctxt, music_rn, &resource_data, 
				&resource_size) != R_SUCCESS ) {
			warning("Music::play(): Resource load failed: %u", music_rn);
			return R_FAILURE;
		}

		_player->setGM(false);
		parser = MidiParser::createParser_XMIDI();
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
	_player->setVolume(ConfMan.getInt("music_volume") * ConfMan.getInt("master_volume") / 255);
	if (flags & R_MUSIC_LOOP)
		_player->setLoop(true);
	else
		_player->setLoop(false);

	_player->playMusic();
	return R_SUCCESS;
}

int Music::pause(void) {
	if (!_musicInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int Music::resume(void) {
	if (!_musicInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int Music::stop(void) {
	if (!_musicInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

} // End of namespace Saga

