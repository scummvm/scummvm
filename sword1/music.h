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
#include "sound/audiostream.h"
#include "sound/rate.h"

#define TOTAL_TUNES 270

#define WAVEHEADERSIZE 0x2C

class SoundMixer;

class SwordMusicHandle : public AudioInputStream {
private:
	File _file;
	bool _looping;
	int32 _fading;
	int _rate;
	bool _stereo;
public:
	SwordMusicHandle() : _looping(false), _fading(0) {}
	virtual int readBuffer(int16 *buffer, const int numSamples);
	int16 read();
	bool play(const char *filename, bool loop);
	void stop();
	void fadeUp();
	void fadeDown();
	bool streaming() const { return _file.isOpen(); }
	int32 fading() { return _fading; }
	bool endOfData() const;
	bool endOfStream() const { return false; }
	bool isStereo() const { return _stereo; }
	int getRate() const { return _rate; }
};

class SwordMusic {
public:
	SwordMusic(OSystem *system, SoundMixer *pMixer);
	~SwordMusic();
	void startMusic(int32 tuneId, int32 loopFlag);
	void fadeDown();
private:
	SwordMusicHandle _handles[2];
	RateConverter *_converter[2];
	OSystem *_system;
	SoundMixer *_mixer;
	OSystem::MutexRef _mutex;
	static void passMixerFunc(void *param, int16 *buf, uint len);
	void mixer(int16 *buf, uint32 len);
	static const char _tuneList[TOTAL_TUNES][8]; // in staticres.cpp
};

#endif // BSMUSIC_H
