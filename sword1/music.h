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

#ifndef BSMUSIC_H
#define BSMUSIC_H

#include "scummsys.h"
#include "common/system.h"
#include "common/file.h"
#define TOTAL_TUNES 270

#define SAMPLERATE 11025
#define BUFSIZE (6 * SAMPLERATE)
#define WAVEHEADERSIZE 0x2C

class SoundMixer;

class SwordMusic {
public:
	SwordMusic(OSystem *system, SoundMixer *pMixer);
	~SwordMusic();
	void stream(void);
	void startMusic(int32 tuneId, int32 loopFlag);
	void fadeDown(void);
private:
	File _musicFile;
	OSystem *_system;
	SoundMixer *_mixer;
	bool _fading;
	uint16 _fadeVal;
	bool _playing;
	bool _loop;
	OSystem::MutexRef _mutex;
	static void passMixerFunc(void *param, int16 *buf, uint len);
	void mixTo(int16 *src, int16 *dst, uint32 len);
	void mixer(int16 *buf, uint32 len);
	static const char _tuneList[TOTAL_TUNES][8]; // in staticres.cpp
	uint32 _waveSize, _wavePos;
	int16 *_musicBuf; // samples for 6 seconds
	uint32 _bufPos, _smpInBuf;
	int16 *_fadeBuf;
	uint32 _fadeBufPos, _fadeSmpInBuf;
};

#endif // BSMUSIC_H
