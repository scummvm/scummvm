// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef IMUSE_H
#define IMUSE_H

#include "../stdafx.h"
#include "../bits.h"
#include "../debug.h"
#include "imuse.h"
#include "imuse_sndmgr.h"

#include "../mixer/mixer.h"
#include "../mixer/audiostream.h"

#define MAX_DIGITAL_TRACKS 8
#define MAX_DIGITAL_FADETRACKS 8

class Imuse {
private:

	int _callbackFps;

	struct Track {
		int trackId;

		int8 pan;
		int32 vol;
		int32 volFadeDest;
		int32 volFadeStep;
		int32 volFadeDelay;
		bool volFadeUsed;

		int32 soundId;
		char soundName[15];
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
		int32 soundType;
		int32 iteration;
		int32 mixerFlags;
		int32 mixerVol;
		int32 mixerPan;

		ImuseSndMgr::soundStruct *soundHandle;
		PlayingSoundHandle handle;
		AppendableAudioStream *stream;

		Track();
	};

	Track *_track[MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS];

	MutexRef _mutex;
	ImuseSndMgr *_sound;

	int32 _volVoice;
	int32 _volSfx;
	int32 _volMusic;

	bool _pause;

	int32 _attributes[185];
	int32 _curMusicState;
	int32 _curMusicSeq;

	static void timerHandler(void *refConf);
	void callback();
	void switchToNextRegion(Track *track);
	int allocSlot(int priority);
	void startSound(int soundId, const char *soundName, int soundType, int volGroupId, AudioStream *input, int hookId, int volume, int priority);
	void selectVolumeGroup(int soundId, int volGroupId);

	int32 getPosInMs(int soundId);

	int getSoundIdByName(const char *soundName);
	void fadeOutMusic(int fadeDelay);
	Track *cloneToFadeOutTrack(Track *track, int fadeDelay);

	void setMusicState(int stateId);
	void setMusicSequence(int seqId);
	void playMusic(const char *songName, const imuseComiTable *table, int atribPos, bool sequence);

public:
	Imuse(int fps);
	~Imuse();

	void startVoice(const char *soundName, int soundId);
	void startMusic(const char *soundName, int soundId, int hookId, int volume, int pan);
	void startSfx(int soundId, int priority);

//	void saveOrLoad(Serializer *ser);
	void resetState();

	void setGroupVoiceVolume(int volume) { _volVoice = volume; }
	void setGroupSfxVolume(int volume) { _volSfx = volume; }
	void setGroupMusicVolume(int volume) { _volMusic = volume; }
	int getGroupVoiceVolume() { return _volVoice; }
	int getGroupSfxVolume() { return _volSfx; }
	int getGroupMusicVolume() { return _volMusic; }

	void setPriority(int soundId, int priority);
	void setVolume(int soundId, int volume);
	void setPan(int soundId, int pan);
	void setFade(int soundId, int destVolume, int delay60HzTicks);
	void setMusicVolume(int vol) {}
	void stopSound(int sound);
	void stopAllSounds();
	void pause(bool pause);
	void parseScriptCmds(int cmd, int soundId, int sub_cmd, int d, int e, int f, int g, int h);
	void refreshScripts();
	void flushTracks();
	int getSoundStatus(int sound) const;
	int32 getCurMusicPosInMs();
};

struct imuseTable {
	byte opcode;
	int16 soundId;
	byte atribPos;
	byte hookId;
	int16 fadeOut60TicksDelay;
	byte volume;
	byte pan;
	char filename[32];
};

#endif
