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

#include "audio/mixer.h"

namespace Common {
class File;
}

namespace Lab {

class LabEngine;

//---------------------------
//----- From LabMusic.c -----
//---------------------------

#define MAXBUFFERS 5

class Music {
private:
	LabEngine *_vm;

	Common::File *_musicFile;
	uint32 _storedPos;

	Audio::SoundHandle _musicHandle;
	Audio::SoundHandle _sfxHandle;

private:
	byte getSoundFlags();

public:
	Music(LabEngine *vm);

	/**
	 * Changes the background music to something else.
	 */
	void changeMusic(const Common::String filename, bool storeCurPos, bool seektoStoredPos);

	void resetMusic(bool seekToStoredPos);

	/**
	 * Checks the music that should be playing in a particular room.
	 */
	void checkRoomMusic(uint16 prevRoom, uint16 newRoom);

	/**
	 * Frees up the music buffers and closes the file.
	 */
	void freeMusic();

	bool isSoundEffectActive() const;
	void playSoundEffect(uint16 sampleSpeed, uint32 length, bool loop, Common::File *dataFile);

	/**
	 * Reads in a sound effect file.  Ignores any graphics.
	 */
	void loadSoundEffect(const Common::String filename, bool loop, bool waitTillFinished);

	void stopSoundEffect();
};

} // End of namespace Lab

#endif // LAB_MUSIC_H
