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
	_fading = false;
	_playing = false;
	_loop = false;
	_mutex = _system->create_mutex();
	_fadeSmpInBuf = _fadeBufPos = _waveSize = _wavePos = _bufPos = _smpInBuf = 0;
	assert(_mixer->getOutputRate() == 22050);
	_fadeBuf = NULL;
	_musicBuf = NULL;
}

SwordMusic::~SwordMusic() {
	_mixer->setupPremix(0, 0);
}

void SwordMusic::passMixerFunc(void *param, int16 *buf, uint len) {
	((SwordMusic*)param)->mixer(buf, len);
}

void SwordMusic::mixTo(int16 *src, int16 *dst, uint32 len) {
	if (!_playing)
		memset(dst, 0, len * 8);
	if (!_fading) { // no fading, simply copy it over
		for (uint32 cnt = 0; cnt < len; cnt++)
			dst[(cnt << 2) | 0] = dst[(cnt << 2) | 1] =
			dst[(cnt << 2) | 2] = dst[(cnt << 2) | 3] = (int16)READ_LE_UINT16(src + cnt);
	} else {
		if (_fadeBuf) { // do a cross fade
			for (uint32 cnt = 0; cnt < len; cnt++) {
				int16 resVal = ((int16)READ_LE_UINT16(_fadeBuf + _fadeBufPos) * _fadeVal) >> 15;
				resVal += ((int16)READ_LE_UINT16(src + cnt) * (32768 - _fadeVal)) >> 15;
				dst[(cnt << 2) | 0] = dst[(cnt << 2) | 1] =
				dst[(cnt << 2) | 2] = dst[(cnt << 2) | 3] = resVal;
				_fadeVal--;
				_fadeBufPos++;
				_fadeSmpInBuf--;
			}
			if ((!_fadeVal) || (!_fadeSmpInBuf)) {
				free(_fadeBuf);
				_fadeBuf = NULL;
				_fading = false;
			}
			if (_fadeBufPos == BUFSIZE)
				_fadeBufPos = 0;
		} else { // simple fadeout
			for (uint32 cnt = 0; cnt < len; cnt++) {
				dst[(cnt << 2) | 0] = dst[(cnt << 2) | 1] =
				dst[(cnt << 2) | 2] = dst[(cnt << 2) | 3] = 
				((int16)READ_LE_UINT16(src + cnt) * _fadeVal) >> 15;
				_fadeVal--;
			}
			if (!_fadeVal) {
				_fading = _playing = false;
				free(_musicBuf);
				_musicBuf = NULL;
			}
		}
	}
}

void SwordMusic::mixer(int16 *buf, uint32 len) {
	if (!_playing) {
		memset(buf, 0, 2 * len * sizeof(int16));
		return;
	}
	uint32 remain = 0;	
	if (_smpInBuf < (len >> 1)) {
		if (_loop)
			return;
		remain = (len >> 1) - _smpInBuf;
		len = _smpInBuf << 1;
	}
	_system->lock_mutex(_mutex);
	len >>= 1;
	while (len) {
		uint32 length = len;
		length = MIN(length, BUFSIZE - _bufPos);
		if (_fading) {
			length = MIN(length, (uint32)_fadeVal);
			length = MIN(length, _fadeSmpInBuf);
			length = MIN(length, BUFSIZE - _fadeBufPos);
		}
		mixTo(_musicBuf + _bufPos, buf, length);
		len -= length;
		buf += 4 * length;
		_bufPos += length;
		_smpInBuf -= length;
		if (_bufPos == BUFSIZE)
			_bufPos = 0;
	}
	if (remain) {
		memset(buf, 0, remain * 8);
		_playing = false;
	}
	_system->unlock_mutex(_mutex);
}

void SwordMusic::stream(void) {
	// make sure we've got enough samples in buffer.
	if ((_smpInBuf < 4 * SAMPLERATE) && _playing) {
		_system->lock_mutex(_mutex);
		uint32 loadTotal = BUFSIZE - _smpInBuf;
		while (uint32 doLoad = loadTotal) {
			if (BUFSIZE - ((_bufPos + _smpInBuf) % BUFSIZE) < loadTotal)
				doLoad = BUFSIZE - (_bufPos + _smpInBuf) % BUFSIZE;
			doLoad = MIN(doLoad, _waveSize - _wavePos);
			
			int16 *dest = _musicBuf + ((_bufPos + _smpInBuf) % BUFSIZE);
			_musicFile.read(dest, doLoad * 2);
			_wavePos += doLoad;
			if (_wavePos == _waveSize) {
				if (_loop) {
					_wavePos = 0;
					_musicFile.seek(WAVEHEADERSIZE);
				} else
					loadTotal = doLoad;
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
			if (_playing) { // do a cross fade
				_fadeBuf = _musicBuf;
				_fadeBufPos = _bufPos;
				_fadeSmpInBuf = _smpInBuf;
				_fading = true;
				_fadeVal = 32768;
			} else
				_fading = false;
			_musicBuf = (int16*)malloc(BUFSIZE * 2);

			_musicFile.seek(0x28);
			_waveSize = _musicFile.readUint32LE() / 2;
			_wavePos = 0;
			_smpInBuf = 0;
			_bufPos = 0;
			_playing = true;
		} else {
			_fading = true;
			_fadeVal = 32768;
			if (_fadeBuf) {
				free(_fadeBuf);
				_fadeBuf = NULL;
			}
		}
	} else if (_playing)
		fadeDown();
	_system->unlock_mutex(_mutex);
	stream();
}

void SwordMusic::fadeDown(void) {
	_fadeVal = 32768;
	_fading = true;
	if (_fadeBuf) {
		free(_fadeBuf);
		_fadeBuf = NULL;
	}
}

