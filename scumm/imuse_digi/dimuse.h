/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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

#ifndef IMUSE_DIGI_H
#define IMUSE_DIGI_H

#include "common/scummsys.h"

#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"
#include "scumm/imuse_digi/dimuse_sndmgr.h"
#include "scumm/music.h"

#include "sound/mixer.h"
#include "sound/audiostream.h"

namespace Scumm {

#define MAX_DIGITAL_TRACKS 16

struct imuseComiTable;

class IMuseDigital : public MusicEngine {
private:

	struct Track {
		int8 pan;			// pan
		int32 vol;			// volume
		int32 volFadeDest;	//
		int32 volFadeStep;	//
		int32 volFadeDelay;	//
		bool volFadeUsed;	//

		int soundId;
		bool used;
		bool toBeRemoved;
		bool started;
		int32 regionOffset;
		int32 trackOffset;
		int32 dataOffset;
		bool sequence;
		int curRegion;
		int curHookId;
		int soundGroup;
		int iteration;
		int mod;
		bool locked;
		int32 pullSize;
		ImuseDigiSndMgr::soundStruct *soundHandle;
		PlayingSoundHandle handle;
		AppendableAudioStream *stream;
		AudioStream *stream2;

		Track();
	};

	Track _track[MAX_DIGITAL_TRACKS];

	ScummEngine *_vm;
	ImuseDigiSndMgr *_sound;
	bool _pause;

	int _attributesState[97];
	int _attributesSeq[91];
	int _curSeqAtribPos;

	int _curMusicState;
	int _curMusicSeq;
	int _curMusicCue;

	int _curMusicSoundId;

	static void timer_handler(void *refConf);
	void callback();
	void switchToNextRegion(int track);
	void startSound(int soundId, const char *soundName, int soundType, int soundGroup, AudioStream *input, bool sequence, int hookId);

	int32 getPosInMs(int soundId);
	void getLipSync(int soundId, int syncId, int32 msPos, int32 &width, int32 &height);

	void stopMusic();

	int getSoundIdByName(const char *soundName);
	void fadeOutMusic(int fadeDelay);
	void setFtMusicState(int stateId);
	void setFtMusicSequence(int seqId);
	void setFtMusicCuePoint(int cueId);
	void playFtMusic(const char *songName, int opcode, int volume, bool sequence);

	void setComiMusicState(int stateId);
	void setComiMusicSequence(int seqId);
	void playComiMusic(const char *songName, const imuseComiTable *table, int atribPos, bool sequence);

public:
	IMuseDigital(ScummEngine *scumm);
	virtual ~IMuseDigital();

	void startVoice(int soundId, AudioStream *input)
		{ debug(5, "startVoiceStream(%d)", soundId); startSound(soundId, NULL, 0, IMUSE_VOICE, input, false, 0); }
	void startVoice(int soundId)
		{ debug(5, "startVoiceBundle(%d)", soundId); startSound(soundId, NULL, IMUSE_BUNDLE, IMUSE_VOICE, NULL, false, 0); }
	void startVoice(int soundId, const char *soundName)
		{ debug(5, "startVoiceBundle(%s)", soundName); startSound(soundId, soundName, IMUSE_BUNDLE, IMUSE_VOICE, NULL, false, 0); }
	void startMusic(int soundId, bool sequence)
		{ debug(5, "startMusicResource(%d)", soundId); startSound(soundId, NULL, IMUSE_RESOURCE, IMUSE_MUSIC, NULL, sequence, 0); }
	void startMusic(const char *soundName, int soundId, bool sequence, int hookId)
		{ debug(5, "startMusicBundle(%s)", soundName); startSound(soundId, soundName, IMUSE_BUNDLE, IMUSE_MUSIC, NULL, sequence, hookId); }
	void startSfx(int soundId)
		{ debug(5, "startSfx(%d)", soundId); startSound(soundId, NULL, IMUSE_RESOURCE, IMUSE_SFX, NULL, false, 0); }
	void startSound(int soundId)
		{ error("MusicEngine::startSound() Should be never called"); }

	void setMasterVolume(int vol) {}
	void stopSound(int soundId);
	void stopAllSounds() { stopAllSounds(false); }
	void stopAllSounds(bool waitForStop);
	void pause(bool pause);
	void parseScriptCmds(int a, int b, int c, int d, int e, int f, int g, int h);
	int getSoundStatus(int sound) const;
	int32 getCurMusicPosInMs();
	int32 getCurVoiceLipSyncWidth();
	int32 getCurVoiceLipSyncHeight();
	int32 getCurMusicLipSyncWidth(int syncId);
	int32 getCurMusicLipSyncHeight(int syncId);
};

struct imuse_music_map {
	int room;
	int table_index;
	int unk1;
	int unk2;
	int unk3;
	int unk4;
};

struct imuseDigtable {
	int room;
	int id;
	int unk1;
	int unk2;
	int unk3;
	int unk4;
	char name[30];
	char title[30];
	char filename[13];
};

struct imuseComiTable {
	char title[30];
	int opcode;
	int soundId;
	char name[24];
	int param;
	int hookId;
	int fadeDelay;
	char filename[13];
};


struct imuseFtNames {
	char name[20];
};

struct imuseFtStateTable {
	char audioName[9];
	int8 opcode;
	int8 volume;
	char name[21];
};

struct imuseFtSeqTable {
	char audioName[9];
	int8 opcode;
	int8 volume;
};

#ifdef __PALM_OS__
extern imuse_music_map *_digStateMusicMap;
extern const imuseDigtable *_digStateMusicTable;
extern const imuseDigtable *_comiStateMusicTable;
extern const imuseComiTable *_comiSeqMusicTable;
extern const imuseComiTable *_digSeqMusicTable;
extern const imuseFtStateTable *_ftStateMusicTable;
extern const imuseFtSeqTable *_ftSeqMusicTable;
extern const imuseFtNames *_ftSeqNames;
#else
extern imuse_music_map _digStateMusicMap[];
extern const imuseDigtable _digStateMusicTable[];
extern const imuseDigtable _digSeqMusicTable[];
extern const imuseComiTable _comiStateMusicTable[];
extern const imuseComiTable _comiSeqMusicTable[];
extern const imuseFtStateTable _ftStateMusicTable[];
extern const imuseFtSeqTable _ftSeqMusicTable[];
extern const imuseFtNames _ftSeqNames[];
#endif

} // End of namespace Scumm

#endif
