/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * $Header$
 */

#ifndef SOUND_H
#define SOUND_H

#include "common/scummsys.h"
#include "sound/mixer.h"

class File;

namespace Scumm {

class ScummEngine;

struct MP3OffsetTable;

enum {
	kTalkSoundID = 10000
};

class Sound {
protected:
	int16 _soundQuePos, _soundQue[0x100];
	int16 _soundQue2Pos, _soundQue2[10];

	File *_sfxFile;
	uint32 _talk_sound_a1, _talk_sound_a2, _talk_sound_b1, _talk_sound_b2;
	byte _talk_sound_mode;
	int _talk_sound_frame;
	bool _mouthSyncMode;
	bool _endOfMouthSync;
	uint16 _mouthSyncTimes[64];
	uint _curSoundPos;

	MP3OffsetTable *offset_table;	// SO3 MP3 compressed audio
	int num_sound_effects;		// SO3 MP3 compressed audio
	bool _vorbis_mode;	// true if using SOG, false if using SO3

	int _currentCDSound;

	ScummEngine *_scumm;

public:
	PlayingSoundHandle _talkChannelHandle;	// Handle of mixer channel actor is talking on
	bool _soundsPaused;
	byte _sfxMode;

public:
	Sound(ScummEngine *parent);
	~Sound();
	void addSoundToQueue(int sound);
	void addSoundToQueue2(int sound);
	void processSoundQues();
	void playSound(int sound);
	void startTalkSound(uint32 offset, uint32 b, int mode, PlayingSoundHandle *handle = NULL);
	void stopTalkSound();
	bool isMouthSyncOff(uint pos);
	int isSoundRunning(int sound) const;
	bool isSoundInUse(int sound) const;
	void stopSound(int a);
	void stopAllSounds();
	void soundKludge(int *list, int num);
	void talkSound(uint32 a, uint32 b, int mode, int frame);
	void setupSound();
	void pauseSounds(bool pause);

	void startCDTimer();
	void stopCDTimer();

	void playCDTrack(int track, int numLoops, int startFrame, int duration);
	void stopCD();
	int pollCD() const;
	void updateCD();
	int getCurrentCDSound() const { return _currentCDSound; }

protected:
	File *openSfxFile();
	void startSfxSound(File *file, int file_size, PlayingSoundHandle *handle, int id = -1);
	bool isSfxFinished() const;
	void processSfxQueues();

	bool isSoundInQueue(int sound) const;
};

} // End of namespace Scumm

#endif

