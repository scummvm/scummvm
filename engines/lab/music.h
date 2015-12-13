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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#ifndef LAB_MUSIC_H
#define LAB_MUSIC_H

#include "common/file.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"

namespace Lab {

class LabEngine;

//---------------------------
//----- From LabMusic.c -----
//---------------------------

#define MAXBUFFERS 5

class Music {
private:
	LabEngine *_vm;

	Common::File *_file;
	Common::File *_tFile;

	bool _doReset;
	bool _musicOn;
	bool _musicPaused;
	bool _oldMusicOn;
	bool _winMusic;

	uint16 _lastMusicRoom ;

	uint32 _tLeftInFile;
	uint32 _leftInFile;

	Audio::SoundHandle _musicHandle;
	Audio::SoundHandle _sfxHandle;
	Audio::QueuingAudioStream *_queuingAudioStream;

private:
	void fillbuffer(byte *musicBuffer);
	uint16 getPlayingBufferCount();
	void pauseBackMusic();
	void readSound(bool waitTillFinished, Common::File *file);
	void startMusic(bool restartFl);

public:
	bool _unstoppableSoundEffect;
	bool _loopSoundEffect;
	bool _waitTillFinished;

public:
	Music(LabEngine *vm);

	void changeMusic(const char *newmusic);
	void checkRoomMusic();
	void freeMusic();
	bool initMusic();
	bool isSoundEffectActive() const;
	void playSoundEffect(uint16 sampleSpeed, uint32 length, void *data);
	bool readMusic(const char *filename, bool waitTillFinished);
	void resetMusic();
	void resumeBackMusic();
	void setMusic(bool on);
	void setMusicReset(bool reset) { _doReset = reset; }
	void setWinMusic() { _winMusic = true; }
	void stopSoundEffect();
	void updateMusic();
};

} // End of namespace Lab

#endif // LAB_MUSIC_H
