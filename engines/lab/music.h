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

#include "common/events.h"

// For the Music class - TODO: Move to another header file
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Lab {

class LabEngine;

/*---------------------------*/
/*----- From LabMusic.c -----*/
/*---------------------------*/

#define MAXBUFFERS         5L

class Music {
public:
	Music(LabEngine *vm);

	bool initMusic();
	void freeMusic();
	void updateMusic();
	uint16 getPlayingBufferCount();
	void closeMusic();
	void setMusic(bool on);
	void resumeBackMusic();
	void pauseBackMusic();
	void changeMusic(const char *newmusic);
	void checkRoomMusic();
	void resetMusic();
	void setMusicReset(bool reset) { _doReset = reset; }
	void playSoundEffect(uint16 SampleSpeed, uint32 Length, void *Data);
	void stopSoundEffect();
	bool isSoundEffectActive() const;
	bool readMusic(const char *filename, bool waitTillFinished);

	bool _winmusic, _doNotFilestopSoundEffect;
	bool _musicOn;
	bool _loopSoundEffect;
	bool _waitTillFinished;
	uint16 _lastMusicRoom ;
	bool _doReset;

private:
	void fillbuffer(byte *musicBuffer);
	void startMusic(bool restartFl);
	void readSound(bool waitTillFinished, Common::File *file);

	LabEngine *_vm;

	Common::File *_file;
	Common::File *_tFile;
	bool _musicPaused;

	bool _tMusicOn;
	uint32 _tLeftInFile;
	uint32 _leftinfile;

	Audio::SoundHandle _musicHandle;
	Audio::SoundHandle _sfxHandle;

	Audio::QueuingAudioStream *_queuingAudioStream;
};

} // End of namespace Lab

#endif /* LAB_MUSIC_H */
