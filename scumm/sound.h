/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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
#include "sound/mixer.h"

class Scumm;
class File;

class Sound {

private:

enum {
	SOUND_HEADER_SIZE = 26,
	SOUND_HEADER_BIG_SIZE = 26 + 8,

};

	int16 _soundQuePos, _soundQue[0x100];
	byte _soundQue2Pos, _soundQue2[10];
	bool _soundsPaused2;
	bool _soundVolumePreset;

	int32 _numberBundleMusic;
	int32 _currentSampleBundleMusic;
	int32 _numberSamplesBundleMusic;
	int32 _offsetSampleBundleMusic;
	int32 _offsetBufBundleMusic;
	byte * _musicBundleBufFinal;
	byte * _musicBundleBufOutput;
	bool _pauseBundleMusic;


	int _talkChannel;	/* Mixer channel actor is talking on */
	File *_sfxFile;
	uint32 _talk_sound_a, _talk_sound_b;
	byte _talk_sound_mode;
	bool _mouthSyncMode;
	bool _endOfMouthSync;
	uint16 _mouthSyncTimes[52];
	uint _curSoundPos;

	MP3OffsetTable *offset_table;	// SO3 MP3 compressed audio
	int num_sound_effects;		// SO3 MP3 compressed audio
#ifdef COMPRESSED_SOUND_FILE

	#define CACHE_TRACKS 10

	/* used for mp3 CD music */

	int _cached_tracks[CACHE_TRACKS];
	struct mad_header _mad_header[CACHE_TRACKS];
	long _mp3_size[CACHE_TRACKS];
	File *_mp3_tracks[CACHE_TRACKS];
	int _mp3_index;
	bool _mp3_cd_playing;
#endif

	Scumm * _scumm;

public:

#ifdef COMPRESSED_SOUND_FILE

	int _current_cache;

#endif

	bool _soundsPaused;
	int16 _sound_volume_master, _sound_volume_music, _sound_volume_sfx;
	byte _sfxMode;

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
	bool isSoundInQueue(int sound);
	void stopSound(int a);
	void stopAllSounds();
	void clearSoundQue();
	void soundKludge(int16 * list);
	void talkSound(uint32 a, uint32 b, int mode);
	void setupSound();
	void pauseSounds(bool pause);
	int startSfxSound(File *file, int file_size);
	File * openSfxFile();
	void stopSfxSound();
	bool isSfxFinished();
	uint32 decode12BitsSample(byte * src, byte ** dst, uint32 size);
	void playBundleMusic(int32 song);
	void pauseBundleMusic(bool state);
	void bundleMusicHandler(Scumm * scumm);
	void stopBundleMusic();
	void playBundleSound(char *sound);
	int playSfxSound(void *sound, uint32 size, uint rate, bool isUnsigned);
	int playSfxSound_MP3(void *sound, uint32 size);

	void playCDTrack(int track, int num_loops, int start, int delay);
	void stopCD();
	int pollCD();
	void updateCD();

protected:
#ifdef COMPRESSED_SOUND_FILE
	int getCachedTrack(int track);
	int playMP3CDTrack(int track, int num_loops, int start, int delay);
	int stopMP3CD();
	int pollMP3CD();
	int updateMP3CD();
#endif
};

#endif

