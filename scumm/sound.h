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
#include "common/timer.h"

class Scumm;
class File;

class Sound {

private:

enum {
	SOUND_HEADER_SIZE = 26,
	SOUND_HEADER_BIG_SIZE = 26 + 8
};

	int16 _soundQuePos, _soundQue[0x100];
	int16 _soundQue2Pos, _soundQue2[10];
	bool _soundsPaused2;
	bool _soundVolumePreset;

	char * _nameBundleMusic;
	char * _newNameBundleMusic;
	int32 _currentSampleBundleMusic;
	int32 _numberSamplesBundleMusic;
	int32 _offsetSampleBundleMusic;
	int32 _offsetBufBundleMusic;
	byte * _musicBundleBufFinal;
	byte * _musicBundleBufOutput;
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

	#define CACHE_TRACKS 10

	/* used for mp3 CD music */

	int _cached_tracks[CACHE_TRACKS];
	int _dig_cd_index;
	int _dig_cd_track;
	int _dig_cd_start;
	int _dig_cd_delay;
	int _dig_cd_num_loops;
	bool _dig_cd_playing;

	class DigitalTrackInfo {
	public:
		virtual bool error() = 0;
		virtual int play(SoundMixer *mixer, int start, int delay) = 0;
		virtual ~DigitalTrackInfo() { }
	};

	DigitalTrackInfo *_track_info[CACHE_TRACKS];

#ifdef USE_MAD
	class MP3TrackInfo : public DigitalTrackInfo {
	private:
		struct mad_header _mad_header;
		long _size;
		File *_file;
		bool _error_flag;

	public:
		MP3TrackInfo(File *file);
		~MP3TrackInfo();
		bool error() { return _error_flag; }
		int play(SoundMixer *mixer, int start, int delay);
	};
#endif

#ifdef USE_VORBIS
	class VorbisTrackInfo : public DigitalTrackInfo {
	private:
		File *_file;
		OggVorbis_File _ov_file;
		bool _error_flag;

	public:
		VorbisTrackInfo(File *file);
		~VorbisTrackInfo();
		bool error() { return _error_flag; }
		int play(SoundMixer *mixer, int start, int delay);
	};
#endif

	Scumm * _scumm;

public:

	int _current_cache;

	int _talkChannel;	/* Mixer channel actor is talking on */
	int _cd_timer_value;
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
	bool isSoundActive(int sound);
	bool isSoundInQueue(int sound);
	void stopSound(int a);
	void stopAllSounds();
	void clearSoundQue();
	void soundKludge(int *list, int num);
	void talkSound(uint32 a, uint32 b, int mode, int frame);
	void setupSound();
	void pauseSounds(bool pause);
	int startSfxSound(File *file, int file_size);
	File * openSfxFile();
	void stopSfxSound();
	bool isSfxFinished();
	uint32 decode12BitsSample(byte * src, byte ** dst, uint32 size, bool stereo);
	void playBundleMusic(char * song);
	void pauseBundleMusic(bool state);
	void bundleMusicHandler(Scumm * scumm);
	void stopBundleMusic();
	int playBundleSound(char *sound);
	byte * readCreativeVocFile(byte * ptr, uint32 & size, uint32 & rate, uint32 & loops);
	int playSfxSound(void *sound, uint32 size, uint rate, bool isUnsigned);
	int playSfxSound_MP3(void *sound, uint32 size);
	int playSfxSound_Vorbis(void *sound, uint32 size);

	int readCDTimer();
	void startCDTimer();
	void stopCDTimer();

	void playCDTrack(int track, int num_loops, int start, int delay);
	void stopCD();
	int pollCD();
	void updateCD();

protected:
	int getCachedTrack(int track);
	int playMP3CDTrack(int track, int num_loops, int start, int delay);
	int stopMP3CD();
	int pollMP3CD();
	int updateMP3CD();
};

#endif

