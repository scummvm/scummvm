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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_MUSIC_H
#define SCI_MUSIC_H

#include "common/savefile.h"
#include "common/serializer.h"
#include "common/mutex.h"

#include "sound/mixer.h"
#include "sound/audiostream.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/sfx/softseq/mididriver.h"

namespace Sci {

enum TrackType {
	kTrackAdlib = 0,
	kTrackGameBlaster = 9,
	kTrackMT32 = 12,
	kTrackSpeaker = 18,
	kTrackTandy = 19
};

enum SoundStatus {
	kSoundStopped = 0,
	kSoundInitialized = 1,
	kSoundPaused = 2,
	kSoundPlaying = 3
};

class MidiParser_SCI;

struct MusicEntry {
	reg_t soundObj;

	SoundResource *soundRes;
	uint16 resnum;

	uint16 dataInc;
	uint16 ticker;
	byte prio;
	byte loop;
	int16 volume;

	byte fadeTo;
	short fadeStep;
	uint32 fadeTicker;
	uint32 fadeTickerStep;

	MidiParser_SCI *pMidiParser;
	Audio::AudioStream* pStreamAud;
	Audio::SoundHandle hCurrentAud;
	SoundStatus status;
};

typedef Common::Array<MusicEntry *> MusicList;

class SciMusic 
#ifndef USE_OLD_MUSIC_FUNCTIONS
	: public Common::Serializable
#endif
{

public:
	SciMusic(SciVersion soundVersion);
	~SciMusic();

	void init();
#if 0
	void loadPatch();
#endif
	void onTimer();
	bool saveState(Common::OutSaveFile *pFile);
	void clearPlayList();
	void stopAll();

	// sound and midi functions
	void soundInitSnd(MusicEntry *pSnd);
	void soundPlay(MusicEntry *pSnd);
	void soundStop(MusicEntry *pSnd);
	void soundKill(MusicEntry *pSnd);
	void soundPause(MusicEntry *pSnd);
	void soundSetVolume(MusicEntry *pSnd, byte volume);
	void soundSetPriority(MusicEntry *pSnd, byte prio);
	uint16 soundGetMasterVolume();
	void soundSetMasterVolume(uint16 vol);
	uint16 soundGetSoundOn() { return _soundOn; }
	void soundSetSoundOn(bool soundOnFlag) {
		_soundOn = soundOnFlag;
		_pMidiDrv->playSwitch(soundOnFlag);
	}
	uint16 soundGetVoices();
	uint32 soundGetTempo() { return _dwTempo; }

	MusicEntry *getSlot(reg_t obj) { 
		for (uint32 i = 0; i < _playList.size(); i++) {
			if (_playList[i]->soundObj == obj) {
				return _playList[i];
			}
		}

		return NULL;
	}

	void pushBackSlot(MusicEntry *slotEntry) {
		_playList.push_back(slotEntry);
	}

	void printPlayList(Console *con);

	void reconstructPlayList(int savegame_version);

	void enterCriticalSection() { _inCriticalSection = true; }
	void leaveCriticalSection() { _inCriticalSection = false; }

#ifndef USE_OLD_MUSIC_FUNCTIONS
	virtual void saveLoadWithSerializer(Common::Serializer &ser);
#endif

protected:
	byte findAudEntry(uint16 nAud, byte&oVolume, uint32& oOffset, uint32&oSize);
	void sortPlayList();
#if 0
	void loadPatchMT32();
	void patchSysEx(byte * addr, byte *pdata, int len);
	void patchUpdateAddr(byte *addr, int len);
#endif

	void doFade(MusicEntry *pSnd);

	SciVersion _soundVersion;

	Audio::Mixer *_pMixer;
	MidiPlayer *_pMidiDrv;
	MidiDriverType _midiType;
	Common::Mutex _mutex;

	uint32 _dwTempo;
	bool _bMultiMidi; // use adlib's digital track if midi track don't have one
private:
	static void miditimerCallback(void *p);

	MusicList _playList;
	bool _soundOn;
	bool _inCriticalSection;
};

} // end of namespace

#endif
