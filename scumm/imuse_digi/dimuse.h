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

class IMuseDigital : public MusicEngine {
private:

	struct Track {
		int8 pan;			// pan
		int32 vol;			// volume
		int32 volFadeDest;	//
		int32 volFadeStep;	//
		int32 volFadeDelay;	//
		bool volFadeUsed;	//

		int idSound;
		bool used;
		bool toBeRemoved;
		bool started;
		int32 regionOffset;
		int32 trackOffset;
		int curRegion;
		int curHookId;
		int soundGroup;
		int iteration;
		void *soundHandle;
		int32 pullSize;
		int mod;
		PlayingSoundHandle handle;
		AppendableAudioStream *stream;
		AudioStream *stream2;

		Track();
	};

	Track _track[MAX_DIGITAL_TRACKS];

	ScummEngine *_scumm;
	ImuseDigiSndMgr *_sound;
	bool _pause;
	int _curMusicId;

	static void timer_handler(void *refConf);
	void callback();
	void switchToNextRegion(int track);
	void startSound(int soundId, const char *soundName, int soundType, int soundGroup, AudioStream *input);

public:
	IMuseDigital(ScummEngine *scumm);
	~IMuseDigital();

	void startVoice(int soundId, AudioStream *input)
		{ debug(5, "startVoiceStream(%d)", soundId); startSound(soundId, NULL, 0, IMUSE_VOICE, input); }
	void startVoice(int soundId)
		{ debug(5, "startVoiceBundle(%d)", soundId); startSound(soundId, NULL, IMUSE_BUNDLE, IMUSE_VOICE, NULL); }
	void startVoice(int soundId, const char *soundName)
		{ debug(5, "startVoiceBundle(%s)", soundName); startSound(soundId, soundName, IMUSE_BUNDLE, IMUSE_VOICE, NULL); }
	void startMusic(int soundId)
		{ debug(5, "startMusicResource(%d)", soundId); startSound(soundId, NULL, IMUSE_RESOURCE, IMUSE_MUSIC, NULL); }
	void startMusic(const char *soundName, int soundId)
		{ debug(5, "startMusicBundle(%s)", soundName); startSound(soundId, soundName, IMUSE_BUNDLE, IMUSE_MUSIC, NULL); }
	void startSfx(int soundId)
		{ debug(5, "startSfx(%d)", soundId); startSound(soundId, NULL, IMUSE_RESOURCE, IMUSE_SFX, NULL); }
	void startSound(int soundId)
		{ error("MusicEngine::startSound() Should be never called"); }

	void setMasterVolume(int vol) {}
	void stopMusic();
	void stopSound(int soundId);
	void stopAllSounds();
	void pause(bool pause);
	void parseScriptCmds(int a, int b, int c, int d, int e, int f, int g, int h);
	int getSoundStatus(int sound) const;
	int32 getPosInMs(int soundId);
	int32 getCurMusicPosInMs();
	int32 getCurVoiceLipSyncWidth();
	int32 getCurVoiceLipSyncHeight();
	int32 getCurMusicLipSyncWidth();
	int32 getCurMusicLipSyncHeight();

	void closeBundleFiles();
};

struct imuse_music_table {
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

struct imuse_music_map {
	int room;
	int table_index;
	int unk1;
	int unk2;
	int unk3;
	int unk4;
};

struct imuse_ft_music_table {
	int index;
	char audioname[15];
	int unk1;
	int volume;
	char name[30];
};

extern imuse_music_map _digStateMusicMap[];
extern const imuse_music_table _digStateMusicTable[];
extern const imuse_music_table _digSeqMusicTable[];
extern const imuse_music_table _comiStateMusicTable[];
extern const imuse_music_table _comiSeqMusicTable[];
extern const imuse_ft_music_table _ftStateMusicTable[];
extern const imuse_ft_music_table _ftSeqMusicTable[];

} // End of namespace Scumm

#endif
