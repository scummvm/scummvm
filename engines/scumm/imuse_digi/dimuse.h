/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#if !defined(IMUSE_DIGI_H) && !defined(DISABLE_SCUMM_7_8)
#define IMUSE_DIGI_H

#include "common/scummsys.h"
#include "common/util.h"

#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"
#include "scumm/imuse_digi/dimuse_sndmgr.h"
#include "scumm/music.h"
#include "scumm/sound.h"

#include "sound/mixer.h"
#include "sound/audiostream.h"

namespace Scumm {

#define MAX_DIGITAL_TRACKS 8
#define MAX_DIGITAL_FADETRACKS 8

struct imuseDigTable;
struct imuseComiTable;
class Serializer;

class IMuseDigital : public MusicEngine {
private:

	int _callbackFps;

	struct Track {
		int trackId;

		int8 pan;			// pan
		int32 vol;			// volume
		int32 volFadeDest;	//
		int32 volFadeStep;	//
		int32 volFadeDelay;	//
		bool volFadeUsed;	//

		int32 soundId;
		char soundName[15];
		bool used;
		bool toBeRemoved;
		bool readyToRemove;
		bool started;
		bool souStream;
		bool compressed;
		int32 priority;
		int32 regionOffset;
		int32 dataOffset;
		int32 curRegion;
		int32 curHookId;
		int32 volGroupId;
		int32 soundType;
		int32 iteration;
		int32 mod;
		int32 mixerFlags;

		ImuseDigiSndMgr::soundStruct *soundHandle;
		Audio::SoundHandle handle;
		Audio::AppendableAudioStream *stream;
		Audio::AudioStream *stream2;

		Track();
	};

	Track *_track[MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS];

	Common::Mutex _mutex;
	ScummEngine *_vm;
	ImuseDigiSndMgr *_sound;

	char *_audioNames;
	int32 _numAudioNames;

	bool _pause;

	int32 _attributes[188];
	int32 _nextSeqToPlay;
	int32 _curMusicState;
	int32 _curMusicSeq;
	int32 _curMusicCue;

	static void timer_handler(void *refConf);
	void callback();
	void switchToNextRegion(Track *track);
	int allocSlot(int priority);
	void startSound(int soundId, const char *soundName, int soundType, int volGroupId, Audio::AudioStream *input, int hookId, int volume, int priority);
	void selectVolumeGroup(int soundId, int volGroupId);

	int32 getPosInMs(int soundId);
	void getLipSync(int soundId, int syncId, int32 msPos, int32 &width, int32 &height);

	int getSoundIdByName(const char *soundName);
	void fadeOutMusic(int fadeDelay);
	Track *cloneToFadeOutTrack(Track *track, int fadeDelay);

	void setFtMusicState(int stateId);
	void setFtMusicSequence(int seqId);
	void setFtMusicCuePoint(int cueId);
	void playFtMusic(const char *songName, int opcode, int volume);

	void setComiMusicState(int stateId);
	void setComiMusicSequence(int seqId);
	void playComiMusic(const char *songName, const imuseComiTable *table, int atribPos, bool sequence);

	void setDigMusicState(int stateId);
	void setDigMusicSequence(int seqId);
	void playDigMusic(const char *songName, const imuseDigTable *table, int atribPos, bool sequence);

public:
	IMuseDigital(ScummEngine *scumm, int fps);
	virtual ~IMuseDigital();

	void setAudioNames(int32 num, char *names);

	void startVoice(int soundId, Audio::AudioStream *input);
	void startVoice(int soundId, const char *soundName);
	void startMusic(int soundId, int volume);
	void startMusic(const char *soundName, int soundId, int hookId, int volume);
	void startSfx(int soundId, int priority);
	void startSound(int sound)
		{ error("MusicEngine::startSound() Should be never called"); }

	void saveOrLoad(Serializer *ser);
	void resetState();

	void setPriority(int soundId, int priority);
	void setVolume(int soundId, int volume);
	void setPan(int soundId, int pan);
	void setFade(int soundId, int destVolume, int delay60HzTicks);
	int getCurMusicSoundId();
	char *getCurMusicSoundName();
	void setHookId(int soundId, int hookId);
	void setMusicVolume(int vol) {}
	void stopSound(int sound);
	void stopAllSounds();
	void pause(bool pause);
	void parseScriptCmds(int cmd, int soundId, int sub_cmd, int d, int e, int f, int g, int h);
	void refreshScripts();
	void flushTracks();
	int getSoundStatus(int sound) const;
	int32 getCurMusicPosInMs();
	int32 getCurVoiceLipSyncWidth();
	int32 getCurVoiceLipSyncHeight();
	int32 getCurMusicLipSyncWidth(int syncId);
	int32 getCurMusicLipSyncHeight(int syncId);
};

struct imuseRoomMap {
	int8 roomId;
	byte stateIndex1;
	byte offset;
	byte stateIndex2;
	byte atribPos;
	byte stateIndex3;
};

struct imuseDigTable {
	byte opcode;
	int16 soundId;
	char name[20];
	byte atribPos;
	byte hookId;
	char filename[13];
};

struct imuseComiTable {
	byte opcode;
	int16 soundId;
	char name[20];
	byte atribPos;
	byte hookId;
	int16 fadeOut60TicksDelay;
	char filename[13];
};


struct imuseFtNames {
	char name[20];
};

struct imuseFtStateTable {
	char audioName[9];
	byte opcode;
	byte volume;
	char name[21];
};

struct imuseFtSeqTable {
	char audioName[9];
	byte opcode;
	byte volume;
};

#ifdef PALMOS_68K
extern const imuseRoomMap *_digStateMusicMap;
extern const imuseDigTable *_digStateMusicTable;
extern const imuseDigTable *_digSeqMusicTable;
extern const imuseComiTable *_comiStateMusicTable;
extern const imuseComiTable *_comiSeqMusicTable;
extern const imuseFtStateTable *_ftStateMusicTable;
extern const imuseFtSeqTable *_ftSeqMusicTable;
extern const imuseFtNames *_ftSeqNames;
#else
extern const imuseRoomMap _digStateMusicMap[];
extern const imuseDigTable _digStateMusicTable[];
extern const imuseDigTable _digSeqMusicTable[];
extern const imuseComiTable _comiStateMusicTable[];
extern const imuseComiTable _comiSeqMusicTable[];
extern const imuseFtStateTable _ftStateMusicTable[];
extern const imuseFtSeqTable _ftSeqMusicTable[];
extern const imuseFtNames _ftSeqNames[];
#endif

} // End of namespace Scumm

#endif
