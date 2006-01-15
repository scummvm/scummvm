/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * $Header$
 */

#ifndef SOUND_H
#define SOUND_H

#include "common/scummsys.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "scumm/saveload.h"

namespace Common {
	class File;
}

namespace Scumm {

class ScummEngine;
class ScummFile;

struct MP3OffsetTable;
struct SaveLoadEntry;

enum {
	kTalkSoundID = 10000
};

class Sound : public Serializable {
#ifdef PALMOS_MODE
public:
#else
protected:
#endif
	enum SoundMode {
		kVOCMode,
		kMP3Mode,
		kVorbisMode,
		kFlacMode
	};

#ifdef PALMOS_MODE
protected:
#endif
	ScummEngine *_vm;

	int16 _soundQuePos, _soundQue[0x100];
	int16 _soundQue2Pos;

	struct {
		int16 sound;
		int32 offset;
		int16 channel;
		int16 flags;
	} _soundQue2[10];

	ScummFile *_sfxFile;
	SoundMode _soundMode;
	MP3OffsetTable *_offsetTable;	// For compressed audio
	int _numSoundEffects;		// For compressed audio

	uint32 _talk_sound_a1, _talk_sound_a2, _talk_sound_b1, _talk_sound_b2;
	byte _talk_sound_mode, _talk_sound_channel;
	bool _mouthSyncMode;
	bool _endOfMouthSync;
	uint16 _mouthSyncTimes[64];
	uint _curSoundPos;

	int _overrideFreq;

	int16 _currentCDSound;
	int16 _currentMusic;

	struct HEMusic{
		int32 id;
		int32 offset;
		int32 size;
	};
	HEMusic *_heMusic;
	int16 _heMusicTracks;

public: // Used by createSound()
	struct {
		int sound;
		int codeOffs;
		int priority;
		int sbngBlock;
		int soundVars[27];
	} _heChannel[8];

public:
	Audio::SoundHandle _talkChannelHandle;	// Handle of mixer channel actor is talking on
	Audio::SoundHandle _heSoundChannels[8];

	bool _soundsPaused;
	byte _sfxMode;

public:
	Sound(ScummEngine *parent);
	~Sound();
	void addSoundToQueue(int sound, int heOffset = 0, int heChannel = 0, int heFlags = 0);
	void addSoundToQueue2(int sound, int heOffset = 0, int heChannel = 0, int heFlags = 0);
	void processSound();
	void processSoundQueues();

	void playSound(int soundID);
	void startTalkSound(uint32 offset, uint32 b, int mode, Audio::SoundHandle *handle = NULL);
	void stopTalkSound();
	bool isMouthSyncOff(uint pos);
	int isSoundRunning(int sound) const;
	bool isSoundInUse(int sound) const;
	void stopSound(int sound);
	void stopAllSounds();
	void soundKludge(int *list, int num);
	void talkSound(uint32 a, uint32 b, int mode, int channel = 0);
	void setupSound();
	void pauseSounds(bool pause);

	void startCDTimer();
	void stopCDTimer();

	void playCDTrack(int track, int numLoops, int startFrame, int duration);
	void stopCD();
	int pollCD() const;
	void updateCD();
	int getCurrentCDSound() const { return _currentCDSound; }

	// HE specific
	bool getHEMusicDetails(int id, int &musicOffs, int &musicSize);
	int findFreeSoundChannel();
	int isSoundCodeUsed(int sound);
	int getSoundPos(int sound);
	int getSoundVar(int sound, int var);
	void setSoundVar(int sound, int var, int val);
	void playHESound(int soundID, int heOffset, int heChannel, int heFlags);
	void processSoundCode();
	void processSoundOpcodes(int sound, byte *codePtr, int *soundVars);
	void setOverrideFreq(int freq);
	void setupHEMusicFile();
	void startHETalkSound(uint32 offset);

	// Used by the save/load system:
	void saveLoadWithSerializer(Serializer *ser);

protected:
	ScummFile *openSfxFile();
	bool isSfxFinished() const;
	void processSfxQueues();

	bool isSoundInQueue(int sound) const;
};

/**
 * An audio stream to which additional data can be appended on-the-fly.
 * Used by SMUSH and iMuseDigital.
 */
class AppendableAudioStream : public AudioStream {
public:
	virtual void append(const byte *data, uint32 len) = 0;
	virtual void finish() = 0;
};

AppendableAudioStream *makeAppendableAudioStream(int rate, byte _flags, uint32 len);


} // End of namespace Scumm

#endif
