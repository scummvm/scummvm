/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

// todo: add fadeout, crossfading.
// this code always loops. make it depend on _loopFlag

#include "stdafx.h"
#include "music.h"
#include "sound/mixer.h"
#include "common/util.h"
#include "common/file.h"

SwordMusic::SwordMusic(OSystem *system, SoundMixer *pMixer) {
	_system = system;
	_mixer = pMixer;
	_mixer->setupPremix(passMixerFunc, this);
	_fading = 0;
	_playing = false;
	_loop = false;
	_mutex = _system->create_mutex();
	_waveSize = _wavePos = _bufPos = _smpInBuf = 0;
	assert(_mixer->getOutputRate() == 22050);
}

void SwordMusic::passMixerFunc(void *param, int16 *buf, uint len) {
	((SwordMusic*)param)->mixer(buf, len);
}

void SwordMusic::mixer(int16 *buf, uint len) {
	if (!_playing)
		memset(buf, 0, 2 * len * sizeof(int16));
	else {
		_system->lock_mutex(_mutex);
		len >>= 1;
		if (len > _smpInBuf)
			warning("SwordMusic::mixer: sample buffer underrun");
		else {
			uint32 maxLen = BUFSIZE - _bufPos;
			if (len >= maxLen) {
				for (uint32 cnt = 0; cnt < maxLen; cnt++)
					buf[(cnt << 2) | 0] = buf[(cnt << 2) | 1] =
					buf[(cnt << 2) | 2] = buf[(cnt << 2) | 3] = (int16)READ_LE_UINT16(_musicBuf + _bufPos + cnt);
				_smpInBuf -= maxLen;
				_bufPos = 0;
				len -= maxLen;
				buf += maxLen * 4;
			}
			if (len) {
				for (uint32 cnt = 0; cnt < len; cnt++)
					buf[(cnt << 2) | 0] = buf[(cnt << 2) | 1] =
					buf[(cnt << 2) | 2] = buf[(cnt << 2) | 3] = (int16)READ_LE_UINT16(_musicBuf + _bufPos + cnt);
				_smpInBuf -= len;
				_bufPos += len;
			}
		}
		_system->unlock_mutex(_mutex);
	}
}

void SwordMusic::stream(void) {
	// make sure we've got enough samples in buffer.
	if ((_smpInBuf < SAMPLERATE) && _playing) {
		_system->lock_mutex(_mutex);
		uint32 loadTotal = BUFSIZE - _smpInBuf;
		while (uint32 doLoad = loadTotal) {
			if (BUFSIZE - ((_bufPos + _smpInBuf) % BUFSIZE) < loadTotal)
				doLoad = BUFSIZE - (_bufPos + _smpInBuf) % BUFSIZE;
			if (_waveSize - _wavePos < doLoad)
				doLoad = _waveSize - _wavePos;
			int16 *dest = _musicBuf + ((_bufPos + _smpInBuf) % BUFSIZE);
			_musicFile.read(dest, doLoad * 2);
			_wavePos += doLoad;
			if (_wavePos == _waveSize) {
				_wavePos = 0;
				_musicFile.seek(WAVEHEADERSIZE);
			}
			loadTotal -= doLoad;
			_smpInBuf += doLoad;
		}
		_system->unlock_mutex(_mutex);
	}
}

void SwordMusic::startMusic(int32 tuneId, int32 loopFlag) {
	_system->lock_mutex(_mutex);
	_loop = (loopFlag > 0);
	if (tuneId) {
		if (_musicFile.isOpen())
			_musicFile.close();
		char fName[20];
		sprintf(fName, "music/%s.wav", _tuneList[tuneId]);
		_musicFile.open(fName);
		if (_musicFile.isOpen()) {
			_musicFile.seek(0x28);
			_waveSize = _musicFile.readUint32LE() / 2;
			_wavePos = 0;
			_smpInBuf = 0;
			_bufPos = 0;
			_playing = true;
		} else
			_playing = false;
	} else
		_playing = false;
	_system->unlock_mutex(_mutex);
	stream();
}

void SwordMusic::fadeDown(void) {
	warning("stub: SwordMusic::fadeDown");
}

