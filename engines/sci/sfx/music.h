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

typedef uint16 SCIHANDLE;
typedef uint16 HEAPHANDLE;

enum kTrackType {
	kTrackAdlib = 0,
	kTrackGameBlaster = 9,
	kTrackMT32 = 12,
	kTrackSpeaker = 18,
	kTrackTandy = 19
};

enum kSndStatus {
	kSndStatusStopped = 0,
	kSndStatusInitialized = 1,
	kSndStatusPaused = 2,
	kSndStatusPlaying = 3
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
	byte volume;

	byte fadeTo;
	short fadeStep;
	uint32 fadeTicker;
	uint32 fadeTickerStep;

	MidiParser_SCI *pMidiParser;
	Audio::AudioStream* pStreamAud;
	Audio::SoundHandle hCurrentAud;
	kSndStatus status;
};

typedef Common::Array<MusicEntry *> MusicList;

class SciMusic : public Common::Serializable {
public:
	SciMusic(SciVersion soundVersion);
	~SciMusic();

	void init();
#if 0
	void loadPatch();
#endif
	void onTimer();
	bool saveState(Common::OutSaveFile *pFile);
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
	uint16 soundGetVoices();
	uint32 soundGetTempo() {
		return _dwTempo;
	}

	MusicEntry *getSlot(reg_t obj) { 
		_mutex.lock();

		for (uint32 i = 0; i < _playList.size(); i++) {
			if (_playList[i]->soundObj == obj) {
				_mutex.unlock();
				return _playList[i];
			}
		}

		_mutex.unlock();
		return NULL;
	}

	void pushBackSlot(MusicEntry *slotEntry) {
		_mutex.lock();
		_playList.push_back(slotEntry);
		_mutex.unlock();
	}

	void reconstructSounds(int savegame_version);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

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
};

} // end of namespace

#endif
