/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header: /cvsroot/scummvm/scummvm/scumm/player_v3a.cpp
 */

#include "stdafx.h"
#include "base/engine.h"
#include "player_v3a.h"
#include "scumm.h"
#include "sound/mixer.h"
#include "common/timer.h"

static const uint16 note_freqs[4][12] = {
	{0x06B0,0x0650,0x05F4,0x05A0,0x054C,0x0500,0x04B8,0x0474,0x0434,0x03F8,0x03C0,0x0388},
	{0x0358,0x0328,0x02FA,0x02D0,0x02A6,0x0280,0x025C,0x023A,0x021A,0x01FC,0x01E0,0x01C4},
	{0x01AC,0x0194,0x017D,0x0168,0x0153,0x0140,0x012E,0x011D,0x010D,0x00FE,0x00F0,0x00E2},
	{0x00D6,0x00CA,0x00BE,0x00B4,0x00A9,0x00A0,0x0097,0x008E,0x0086,0x007F,0x00F0,0x00E2}
};

////////////////////////////////////////
//
// V3 Amiga sound/music driver
//
////////////////////////////////////////

Player_V3A::Player_V3A(Scumm *scumm) {
	int i;
	_scumm = scumm;
	_system = scumm->_system;
	_mixer = scumm->_mixer;

	for (i = 0; i < V3A_MAXSFX; i++)
		_sfx[i].id = _sfx[i].dur = 0;

	for (i = 0; i < V3A_MAXMUS; i++)
		_mus[i].id = _mus[i].dur = 0;

	_curSong = 0;
	_songData = NULL;
	_songPtr = 0;
	_songDelay = 0;

	_music_timer = 0;

	_maxvol = 255;

	scumm->_timer->installProcedure(timerHandler, 16666, this);

	_isinit = false;
}

Player_V3A::~Player_V3A() {
	_scumm->_timer->releaseProcedure(timerHandler);
	if (!_isinit)
		return;
	for (int i = 0; _wavetable[i] != NULL; i++) {
		for (int j = 0; j < 6; j++) {
			free(_wavetable[i]->_idat[j]);
			free(_wavetable[i]->_ldat[j]);
		}
		free(_wavetable[i]);
	}
	free(_wavetable);
}

void Player_V3A::setMasterVolume (int vol) {
	_maxvol = vol;
}

void Player_V3A::stopAllSounds() {
	int i;
	for (i = 0; i < V3A_MAXMUS; i++) {
		if (_mus[i].id)
			_mixer->stopID(V3A_MUS_BASEID + i);
		_mus[i].id = 0;
		_mus[i].dur = 0;
	}
	_curSong = 0;
	_songPtr = 0;
	_songDelay = 0;
	_songData = NULL;
	for (i = 0; i < V3A_MAXSFX; i++) {
		if (_sfx[i].id)
			_mixer->stopID(V3A_SFX_BASEID + i);
		_sfx[i].id = 0;
		_sfx[i].dur = 0;
	}
}

void Player_V3A::stopSound(int nr) {
	int i;
	if (nr == _curSong) {
		for (i = 0; i < V3A_MAXMUS; i++) {
			if (_mus[i].id)
				_mixer->stopID(V3A_MUS_BASEID + i);
			_mus[i].id = 0;
			_mus[i].dur = 0;
		}
		_curSong = 0;
		_songPtr = 0;
		_songDelay = 0;
		_songData = NULL;
	} else {
		for (i = 0; i < V3A_MAXSFX; i++) {
			if (_sfx[i].id == nr) {
				_mixer->stopID(V3A_SFX_BASEID + i);
				_sfx[i].id = 0;
				_sfx[i].dur = 0;
				break;
			}
		}
	}
}

void Player_V3A::playSoundSFX (int nr, char *data, int size, int rate, int vol, int tl, bool looped, int loopStart, int loopEnd) {
	int i;
	for (i = 0; i < V3A_MAXSFX; i++) {
		if (!_sfx[i].id)
			break;
	}
	if (i == V3A_MAXSFX) {
		warning("player_v3a - too many sound effects playing (%i max)",V3A_MAXSFX);
		return;
	}
	_sfx[i].id = nr;
	_sfx[i].dur = tl;

	vol = (vol * _maxvol) / 255;
	_mixer->playRaw(NULL, data, size, rate, SoundMixer::FLAG_AUTOFREE | (looped ? SoundMixer::FLAG_LOOP : 0),
		V3A_SFX_BASEID + i, vol, 0, loopStart, loopEnd);
}

void Player_V3A::playSoundMUS (char *data, int size, int rate, int vol, int tl, bool looped, int loopStart, int loopEnd) {
	int i;
	for (i = 0; i < V3A_MAXMUS; i++) {
		if (!_mus[i].id)
			break;
	}
	if (i == V3A_MAXMUS) {
		warning("player_v3a - too many music channels playing (%i max)",V3A_MAXMUS);
		return;
	}
	_mus[i].id = i + 1;
	_mus[i].dur = tl;

	vol = (vol * _maxvol) / 255;
	_mixer->playRaw(NULL, data, size, rate, SoundMixer::FLAG_AUTOFREE | (looped ? SoundMixer::FLAG_LOOP : 0),
		V3A_MUS_BASEID + i, vol, 0, loopStart, loopEnd);
}

void Player_V3A::startSound(int nr) {
	assert(_scumm);
	byte *data = _scumm->getResourceAddress(rtSound, nr);
	assert(data);

	if (_scumm->_gameId != GID_INDY3 && _scumm->_gameId != GID_LOOM)
		error("player_v3a - unknown game!");

	if (!_isinit) {
		int i;
		unsigned char *ptr;
		int offset = 4;
		int numInstruments;

		if (_scumm->_gameId == GID_INDY3) {
			ptr = _scumm->getResourceAddress(rtSound, 83);
			numInstruments = 12;
		} else {
			ptr = _scumm->getResourceAddress(rtSound, 79);
			numInstruments = 9;
		}
		assert(ptr);
		_wavetable = (instData **)malloc((numInstruments + 1) * sizeof(void *));
		for (i = 0; i < numInstruments; i++) {
			_wavetable[i] = (instData *)malloc(sizeof(instData));
			for (int j = 0; j < 6; j++) {
				int off, len;
				off = READ_BE_UINT16(ptr + offset + 0);
				_wavetable[i]->_ilen[j] = len = READ_BE_UINT16(ptr + offset + 2);
				if (len) {
					_wavetable[i]->_idat[j] = (char *)malloc(len);
					memcpy(_wavetable[i]->_idat[j],ptr + off,len);
				} else	_wavetable[i]->_idat[j] = NULL;
				off = READ_BE_UINT16(ptr + offset + 4);
				_wavetable[i]->_llen[j] = len = READ_BE_UINT16(ptr + offset + 6);
				if (len) {
					_wavetable[i]->_ldat[j] = (char *)malloc(len);
					memcpy(_wavetable[i]->_ldat[j],ptr + off,len);
				} else	_wavetable[i]->_ldat[j] = NULL;
				_wavetable[i]->_oct[j] = READ_BE_UINT16(ptr + offset + 8);
				offset += 10;
			}
			if (_scumm->_gameId == GID_INDY3) {
				_wavetable[i]->_pitadjust = 0;
				offset += 2;
			} else {
				_wavetable[i]->_pitadjust = READ_BE_UINT16(ptr + offset + 2);
				offset += 4;
			}
		}
		_wavetable[i] = NULL;
		_isinit = true;
	}
	
	if (getSoundStatus(nr))
		stopSound(nr);	// if a sound is playing, restart it
	
	if (data[26]) {
		_curSong = nr;
		_songData = data;
		_songPtr = 0x1C;
		_songDelay = 1;
		_music_timer = 0;
	} else {
		int size = READ_BE_UINT16(data + 12);
		int rate = 3579545 / READ_BE_UINT16(data + 20);
		char *sound = (char *)malloc(size);
		int vol = (data[24] << 2) | (data[24] >> 4);
		memcpy(sound,data + READ_BE_UINT16(data + 8),size);
		if ((READ_BE_UINT16(data + 16) || READ_BE_UINT16(data + 6))) {
			// the first check is for complex (pitch-bending) looped sounds
			// the second check is for simple looped sounds
			int loopStart = READ_BE_UINT16(data + 10) - READ_BE_UINT16(data + 8);
			int loopEnd = READ_BE_UINT16(data + 14);
			int tl = -1;
			if ((_scumm->_gameId == GID_INDY3) && (nr == 60))
				tl = 240;	// the "airplane dive" sound needs to end on its own - the game won't stop it
			playSoundSFX(nr, sound, size, rate, vol, tl, true, loopStart, loopEnd);
		} else {
			int tl = 1 + 60 * size / rate;
			playSoundSFX(nr, sound, size, rate, vol, tl, false);
		}
	}
}

void Player_V3A::timerHandler(void *refCon) {
	Player_V3A *player = (Player_V3A *)refCon;
	assert(player);
	player->playMusic();
}

void Player_V3A::playMusic() {
	int i;
	for (i = 0; i < V3A_MAXSFX; i++) {
		if ((_sfx[i].dur) && (!--_sfx[i].dur))
			stopSound(_sfx[i].id);
	}
	for (i = 0; i < V3A_MAXMUS; i++) {
		if ((_mus[i].dur) && (!--_mus[i].dur)) {
			_scumm->_mixer->stopID(V3A_MUS_BASEID + i);
			_mus[i].id = 0;
			_mus[i].dur = 0;
		}
	}
	_music_timer++;
	if (!_curSong)
		return;
	if (_songDelay && --_songDelay)
		return;
	if (((_songData[_songPtr] & 0xF0) != 0x80) && (_songData[_songPtr] != 0xFB)) {
		// at the end of the song, and it wasn't looped - kill it
		_curSong = 0;
		return;
	}
	while (1) {
		int inst, pit, vol, dur, oct;
		inst = _songData[_songPtr++];
		if ((inst & 0xF0) != 0x80) {
			// tune is at the end - figure out what's still playing
			// and see how long we have to wait until we stop/restart
			for (i = 0; i < V3A_MAXMUS; i++) {
				if (_songDelay < _mus[i].dur)
					_songDelay = _mus[i].dur;
			}
			if (inst == 0xFB)	// it's a looped song, restart it afterwards
				_songPtr = 0x1C;
			break;
		}
		inst &= 0xF;
		pit = _songData[_songPtr++];
		vol = _songData[_songPtr++] & 0x7F;
		vol = (vol << 1) | (vol >> 7);	// 7-bit volume (Amiga drops the bottom bit), convert to 8-bit
		dur = _songData[_songPtr++];
		if (pit == 0) {
			_songDelay = dur;
			break;
		}
		pit += _wavetable[inst]->_pitadjust;
		oct = (pit / 12) - 2;
		pit = pit % 12;
		if (oct < 0)
			oct = 0;
		if (oct > 5)
			oct = 5;
		char *data = (char *)malloc(_wavetable[inst]->_ilen[oct] + _wavetable[inst]->_llen[oct]);
		if (_wavetable[inst]->_idat[oct])
			memcpy(data, _wavetable[inst]->_idat[oct], _wavetable[inst]->_ilen[oct]);
		if (_wavetable[inst]->_ldat[oct])
			memcpy(data + _wavetable[inst]->_ilen[oct], _wavetable[inst]->_ldat[oct], _wavetable[inst]->_llen[oct]);
		playSoundMUS(data, _wavetable[inst]->_ilen[oct] + _wavetable[inst]->_llen[oct], 3579545 / note_freqs[_wavetable[inst]->_oct[oct]][pit], vol, dur,
			(_wavetable[inst]->_ldat[oct] != NULL), _wavetable[inst]->_ilen[oct], _wavetable[inst]->_ilen[oct] + _wavetable[inst]->_llen[oct]);
	}
}

int Player_V3A::getMusicTimer() const {
	return _music_timer / 30;
}

int Player_V3A::getSoundStatus(int nr) const {
	if (nr == _curSong)
		return 1;
	for (int i = 0; i < V3A_MAXSFX; i++)
		if (_sfx[i].id == nr)
			return 1;
	return 0;
}
