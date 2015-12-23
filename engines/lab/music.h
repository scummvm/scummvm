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

	uint16 _lastMusicRoom ;

	uint32 _tLeftInFile;
	uint32 _leftInFile;

	Audio::SoundHandle _musicHandle;
	Audio::SoundHandle _sfxHandle;
	Audio::QueuingAudioStream *_queuingAudioStream;

private:
	void fillbuffer(byte *musicBuffer);
	uint16 getPlayingBufferCount();

	/**
	 * Pauses the background music.
	 */
	void pauseBackMusic();
	void readSound(bool waitTillFinished, Common::File *file);

	/**
	 * Starts up the music initially.
	 */
	void startMusic(bool restartFl);

public:
	bool _loopSoundEffect;

public:
	Music(LabEngine *vm);

	/**
	 * Changes the background music to something else.
	 */
	void changeMusic(const Common::String filename);

	/**
	 * Checks the music that should be playing in a particular room.
	 */
	void checkRoomMusic();

	/**
	 * Frees up the music buffers and closes the file.
	 */
	void freeMusic();

	/**
	 * Initializes the music buffers.
	 */
	bool initMusic(const Common::String filename);
	bool isSoundEffectActive() const;
	void playSoundEffect(uint16 sampleSpeed, uint32 length, Common::File *dataFile);

	/**
	 * Reads in a music file.  Ignores any graphics.
	 */
	bool readMusic(const Common::String filename, bool waitTillFinished);

	/**
	 * Changes the background music to the original piece playing.
	 */
	void resetMusic();

	/**
	 * Resumes the paused background music.
	 */
	void resumeBackMusic();

	/**
	 * Turns the music on and off.
	 */
	void setMusic(bool on);
	void setMusicReset(bool reset) { _doReset = reset; }
	void stopSoundEffect();

	/**
	 * Figures out which buffer is currently playing based on messages sent to
	 * it from the Audio device.
	 */
	void updateMusic();
};

} // End of namespace Lab

#endif // LAB_MUSIC_H
