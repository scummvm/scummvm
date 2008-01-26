/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef IMUSE_H
#define IMUSE_H

#include "stdafx.h"
#include "bits.h"
#include "debug.h"
#include "lua.h"

#include "mixer/mixer.h"
#include "mixer/audiostream.h"

#include "imuse/imuse_sndmgr.h"
#include "imuse/imuse_mcmp_mgr.h"

#define MAX_IMUSE_TRACKS 16
#define MAX_IMUSE_FADETRACKS 16

struct ImuseTable {
	byte opcode;
	int16 soundId;
	byte atribPos;
	byte hookId;
	int16 fadeOut60TicksDelay;
	byte volume;
	byte pan;
	char filename[32];
};

class SaveGame;

class Imuse {
private:

	int _callbackFps;

	struct Track {
		int trackId;

		int32 pan;
		int32 panFadeDest;
		int32 panFadeStep;
		int32 panFadeDelay;
		bool panFadeUsed;
		int32 vol;
		int32 volFadeDest;
		int32 volFadeStep;
		int32 volFadeDelay;
		bool volFadeUsed;

		char soundName[32];
		bool used;
		bool toBeRemoved;
		bool readyToRemove;
		bool started;
		int32 priority;
		int32 regionOffset;
		int32 dataOffset;
		int32 curRegion;
		int32 curHookId;
		int32 volGroupId;
		int32 iteration;
		int32 mixerFlags;
		int32 mixerVol;
		int32 mixerPan;

		ImuseSndMgr::SoundStruct *soundHandle;
		PlayingSoundHandle handle;
		AppendableAudioStream *stream;

		Track();
	};

	Track *_track[MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS];

	MutexRef _mutex;
	ImuseSndMgr *_sound;

	int32 _volVoice;
	int32 _volSfx;
	int32 _volMusic;

	bool _pause;

	int32 _attributes[185];
	int32 _curMusicState;
	int32 _curMusicSeq;

	const ImuseTable *_stateMusicTable;
	const ImuseTable *_seqMusicTable;

	static void timerHandler(void *refConf);
	void callback();
	void switchToNextRegion(Track *track);
	int allocSlot(int priority);
	void selectVolumeGroup(const char *soundName, int volGroupId);

	void fadeOutMusic(int fadeDelay);
	Track *cloneToFadeOutTrack(Track *track, int fadeDelay);

	void playMusic(const ImuseTable *table, int atribPos, bool sequence);

public:
	Imuse(int fps);
	~Imuse();

	bool startSound(const char *soundName, int volGroupId, int hookId, int volume, int pan, int priority);
	void startVoice(const char *soundName, int volume = 127, int pan = 64);
	void startMusic(const char *soundName, int hookId, int volume, int pan);
	void startSfx(const char *soundName, int priority = 127);

	void restoreState(SaveGame *savedState);
	void saveState(SaveGame *savedState);
	void resetState();

	void setGroupVoiceVolume(int volume) { _volVoice = volume; }
	void setGroupSfxVolume(int volume) { _volSfx = volume; }
	void setGroupMusicVolume(int volume) { _volMusic = volume; }
	int getGroupVoiceVolume() { return _volVoice; }
	int getGroupSfxVolume() { return _volSfx; }
	int getGroupMusicVolume() { return _volMusic; }

	Track *findTrack(const char *soundName);
	void setPriority(const char *soundName, int priority);
	void setVolume(const char *soundName, int volume);
	int getVolume(const char *soundName);
	void setPan(const char *soundName, int pan);
	void setFadePan(const char *soundName, int destPan, int duration);
	void setFadeVolume(const char *soundName, int destVolume, int duration);
	void setHookId(const char *soundName, int hookId);
	int getCountPlayedTracks(const char *soundName);
	void stopSound(const char *soundName);
	void stopAllSounds();
	void pause(bool pause);
	void setMusicState(int stateId);
	int setMusicSequence(int seqId);
	void refreshScripts();
	void flushTracks();
	bool isVoicePlaying();
	char *getCurMusicSoundName();
	bool getSoundStatus(const char *soundName);
	int32 getPosIn60HzTicks(const char *soundName);
};

extern Imuse *g_imuse;

#endif
