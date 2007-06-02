/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#if !defined(SCUMM_IMUSE_DIGI_H) && !defined(DISABLE_SCUMM_7_8)
#define SCUMM_IMUSE_DIGI_H

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
class ScummEngine_v7;

// These flag bits correspond exactly to the sound mixer flags of March 2007.
// We don't want to use the mixer flags directly, because then our saved games
// will break in interesting ways if the mixer flags are ever assigned new
// values. Now they should keep working, as long as these flags don't change.

enum {
	kFlagUnsigned = 1 << 0,
	kFlag16Bits = 1 << 1,
	kFlagLittleEndian = 1 << 2,
	kFlagStereo = 1 << 3,
	kFlagReverseStereo = 1 << 4

	// Not used by Digital iMUSE
	// kFlagAutoFree = 1 << 5,
	// kFlagLoop = 1 << 6
};

class IMuseDigital : public MusicEngine {
private:

	int _callbackFps;		// value how many times callback needs to be called per second

	struct Track {
		int trackId;		// used to identify track by value (0-15)

		int8 pan;			// panning value of sound
		int32 vol;			// volume level (values 0-127 * 1000)
		int32 volFadeDest;	// volume level which fading target (values 0-127 * 1000)
		int32 volFadeStep;	// delta of step while changing volume at each imuse callback
		int32 volFadeDelay;	// time in ms how long fading volume must be
		bool volFadeUsed;	// flag if fading is in progress

		int32 soundId;		// sound id used by scumm script
		char soundName[15]; // sound name but also filename of sound in bundle data
		bool used;			// flag mean that track is used
		bool toBeRemoved;   // flag mean that track need to be free
		bool readyToRemove; // flag mean that track is ready to stop
		bool mixerStreamRunning;	// flag mean sound mixer's stream is running
		bool souStreamUsed;	// flag mean that track use stream from sou file
		bool sndDataExtComp;// flag mean that sound data is compressed by scummvm tools
		int32 soundPriority;// priority level of played sound (0-127)
		int32 regionOffset; // offset to sound data relative to begining of current region
		int32 dataOffset;	// offset to sound data relative to begining of 'DATA' chunk
		int32 curRegion;	// id of current used region
		int32 curHookId;	// id of current used hook id
		int32 volGroupId;	// id of volume group (IMUSE_VOLGRP_VOICE, IMUSE_VOLGRP_SFX, IMUSE_VOLGRP_MUSIC)
		int32 soundType;	// type of sound data (kSpeechSoundType, kSFXSoundType, kMusicSoundType)
		int32 feedSize;		// size of sound data needed to be filled at each callback iteration
		int32 dataMod12Bit;	// value used between all callback to align 12 bit source of data
		int32 mixerFlags;	// flags for sound mixer's channel (kFlagStereo, kFlag16Bits, kFlagReverseStereo, kFlagUnsigned, kFlagLittleEndian)

		ImuseDigiSndMgr::soundStruct *soundHandle;	// sound handle used by iMuse sound manager
		Audio::SoundHandle mixChanHandle;					// sound mixer's channel handle
		Audio::AppendableAudioStream *stream;		// sound mixer's audio stream handle for *.la1 and *.bun
		Audio::AudioStream *streamSou;				// sound mixer's audio stream handle for *.sou

		Track();
	};

	Track *_track[MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS];

	Common::Mutex _mutex;
	ScummEngine_v7 *_vm;
	Audio::Mixer *_mixer;
	ImuseDigiSndMgr *_sound;

	char *_audioNames;		// filenames of sound SFX used in FT
	int32 _numAudioNames;	// number of above filenames

	bool _pause;			// flag mean that iMuse callback should be idle

	int32 _attributes[188];	// internal atributes for each music file to store and check later
	int32 _nextSeqToPlay;	// id of sequence type of music needed played
	int32 _curMusicState;	// current or previous id of music
	int32 _curMusicSeq;		// current or previous id of sequence music
	int32 _curMusicCue;		// current cue for current music. used in FT

	int32 makeMixerFlags(int32 flags);
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
	Track *cloneToFadeOutTrack(const Track *track, int fadeDelay);

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
	IMuseDigital(ScummEngine_v7 *scumm, Audio::Mixer *mixer, int fps);
	virtual ~IMuseDigital();

	void setAudioNames(int32 num, char *names);

	void startVoice(int soundId, Audio::AudioStream *input);
	void startVoice(int soundId, const char *soundName);
	void startMusic(int soundId, int volume);
	void startMusic(const char *soundName, int soundId, int hookId, int volume);
	void startSfx(int soundId, int priority);
	void startSound(int sound)
		{ error("IMuseDigital::startSound(int) should be never called"); }

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
	byte transitionType;
	int16 soundId;
	char name[20];
	byte atribPos;
	byte hookId;
	char filename[13];
};

struct imuseComiTable {
	byte transitionType;
	int16 soundId;
	char name[20];
	byte atribPos;
	byte hookId;
	int16 fadeOutDelay;
	char filename[13];
};


struct imuseFtNames {
	char name[20];
};

struct imuseFtStateTable {
	char audioName[9];
	byte transitionType;
	byte volume;
	char name[21];
};

struct imuseFtSeqTable {
	char audioName[9];
	byte transitionType;
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
