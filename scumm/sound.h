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

#include "scummsys.h"

class Bundle;
class DigitalTrackInfo;
class File;
class Scumm;

struct MP3OffsetTable {					/* Compressed Sound (.SO3) */
	int org_offset;
	int new_offset;
	int num_tags;
	int compressed_size;
};

class Sound {
protected:
	int16 _soundQuePos, _soundQue[0x100];
	int16 _soundQue2Pos, _soundQue2[10];
	bool _soundsPaused2;
	bool _soundVolumePreset;

public:
	const char *_nameBundleMusic;

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
	int32 _bundleMusicTrack;
	bool _musicBundleToBeChanged;
	bool _musicBundleToBeRemoved;
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

	int _current_cd_sound;

	int _cached_tracks[CACHE_TRACKS];
	int _dig_cd_index;
	int _dig_cd_track;
	int _dig_cd_start;
	int _dig_cd_delay;
	int _dig_cd_num_loops;
	bool _dig_cd_playing;

	DigitalTrackInfo *_track_info[CACHE_TRACKS];

	Scumm *_scumm;

public:
	int _current_cache;
	int32 _bundleMusicPosition;

	int _talkChannel;	/* Mixer channel actor is talking on */
	bool _soundsPaused;
	int16 _sound_volume_master, _sound_volume_music, _sound_volume_sfx;
	byte _sfxMode;

	Bundle *_bundle;	// FIXME: should be protected but is used by Scumm::askForDisk

public:
	Sound(Scumm *parent);
	~Sound();
	void addSoundToQueue(int sound);
	void addSoundToQueue2(int sound);
	void processSoundQues();
	void playSound(int sound);
	void processSfxQueues();
	int startTalkSound(uint32 offset, uint32 b, int mode);
	void stopTalkSound();
	bool isMouthSyncOff(uint pos);
	int isSoundRunning(int sound);
	bool isSoundActive(int sound);
	bool isSoundInQueue(int sound);
	void stopSound(int a);
	void stopAllSounds();
	void soundKludge(int *list, int num);
	void talkSound(uint32 a, uint32 b, int mode, int frame);
	void setupSound();
	void pauseSounds(bool pause);

	void playBundleMusic(const char *song);
	void pauseBundleMusic(bool state);
	void bundleMusicHandler(Scumm *scumm);
	void stopBundleMusic();
	int playBundleSound(char *sound);

	uint32 decode12BitsSample(byte *src, byte **dst, uint32 size, bool stereo);

	void startCDTimer();
	void stopCDTimer();

	void playCDTrack(int track, int num_loops, int start, int delay);
	void stopCD();
	int pollCD();
	void updateCD();

protected:
	void clearSoundQue();

	File *openSfxFile();
	int startSfxSound(File *file, int file_size);
	void stopSfxSound();
	bool isSfxFinished();
	int playSfxSound(void *sound, uint32 size, uint rate, bool isUnsigned);
	int playSfxSound_MP3(void *sound, uint32 size);
	int playSfxSound_Vorbis(void *sound, uint32 size);

	int getCachedTrack(int track);
	int playMP3CDTrack(int track, int num_loops, int start, int delay);
	int stopMP3CD();
	int pollMP3CD();
	int updateMP3CD();
};

#endif

