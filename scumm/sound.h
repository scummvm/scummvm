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

class Bundle;
class DigitalTrackInfo;
class ScummEngine;

struct MP3OffsetTable;

class Sound {
protected:
	int16 _soundQuePos, _soundQue[0x100];
	int16 _soundQue2Pos, _soundQue2[10];

public:
	const char *_nameBundleMusic;
	int32 _bundleMusicPosition;

protected:
	const char *_newNameBundleMusic;
	byte _musicDisk;
	byte _voiceDisk;
	int32 _currentSampleBundleMusic;
	int32 _numberSamplesBundleMusic;
	int32 _offsetSampleBundleMusic;
	int32 _offsetBufBundleMusic;
	byte *_musicBundleBufFinal;
	byte *_musicBundleBufOutput;
	bool _pauseBundleMusic;
	PlayingSoundHandle _bundleMusicTrack;
	bool _musicBundleToBeChanged;
	int32 _bundleMusicSampleBits;
	int32 _outputMixerSize;
	int32 _bundleSampleChannels;

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

	enum {
		CACHE_TRACKS = 10
	};

	/* used for mp3 CD music */

	int _currentCDSound;

	int _cached_tracks[CACHE_TRACKS];
	struct {
		PlayingSoundHandle handle;
		int track;
		int start;
		int duration;
		int numLoops;
		bool playing;
	} _dig_cd;

	DigitalTrackInfo *_track_info[CACHE_TRACKS];
	int _current_cache;

	ScummEngine *_scumm;

public:
	PlayingSoundHandle _talkChannelHandle;	// Handle of mixer channel actor is talking on
	bool _soundsPaused;
	byte _sfxMode;
	
	Bundle *_bundle;	// FIXME: should be protected but is used by ScummEngine::askForDisk

public:
	Sound(ScummEngine *parent);
	~Sound();
	void addSoundToQueue(int sound);
	void addSoundToQueue2(int sound);
	void processSoundQues();
	void playSound(int sound);
	void processSfxQueues();
	void startTalkSound(uint32 offset, uint32 b, int mode, PlayingSoundHandle *handle = NULL);
	void stopTalkSound();
	bool isMouthSyncOff(uint pos);
	int isSoundRunning(int sound) const;
	bool isSoundInUse(int sound) const;
	bool isSoundInQueue(int sound) const;
	void stopSound(int a);
	void stopAllSounds();
	void soundKludge(int *list, int num);
	void talkSound(uint32 a, uint32 b, int mode, int frame);
	void setupSound();
	void pauseSounds(bool pause);

	void playBundleMusic(const char *song);
	void pauseBundleMusic(bool state);
	void bundleMusicHandler(ScummEngine *scumm);
	void stopBundleMusic();
	void playBundleSound(char *sound, PlayingSoundHandle *handle);

	uint32 decode12BitsSample(byte *src, byte **dst, uint32 size, bool stereo);

	void startCDTimer();
	void stopCDTimer();

	void playCDTrack(int track, int numLoops, int startFrame, int duration);
	void stopCD();
	int pollCD() const;
	void updateCD();
	int getCurrentCDSound() const { return _currentCDSound; }

protected:
	File *openSfxFile();
	void startSfxSound(File *file, int file_size, PlayingSoundHandle *handle);
	bool isSfxFinished() const;
	void playSfxSound(void *sound, uint32 size, uint rate, bool isUnsigned, PlayingSoundHandle *handle);
	void playSfxSound_Vorbis(File *file, uint32 size, PlayingSoundHandle *handle);

	int getCachedTrack(int track);
};

} // End of namespace Scumm

#endif

