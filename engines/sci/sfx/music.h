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

#include "sound/mixer.h"
#include "sound/audiostream.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "common/mutex.h"
#include "common/savefile.h"

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/sfx/softseq/mididriver.h"

/*
 Sound drivers info: (from driver cmd0)
 Adlib/SB  : track 0 , voices 9 , patch 3	ah=1
 ProAudioSp: track 0 , voices 9 , patch 3	ah=17
 GenerlMIDI: track 7 , voices 32, patch 4	ah=1   SCI1.1
 Game Blast: track 9 , voices 12, patch 101	ah=1
 MT-32	  : track 12, voices 32, patch 1	ah=1
 PC Speaker: track 18, voices 1 , patch 0xFF ah=1
 Tandy	  : track 19, voices 3 , patch 101	ah=1
 IBM PS/1  : track 19, voices 3 , patch 101	ah=1

 */

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
	kStopped = 0, kPaused, kPlaying
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

	byte FadeTo;
	short FadeStep;
	uint32 FadeTicker;
	uint32 FadeTickerStep;

	MidiParser_SCI *pMidiParser;
	Audio::AudioStream* pStreamAud;
	Audio::SoundHandle hCurrentAud;
	kSndStatus status;
};

class SciMusic {
public:
	SciMusic();
	~SciMusic();

	void init();
#if 0
	void loadPatch();
#endif
	void onTimer();
	bool saveState(Common::OutSaveFile *pFile);
	bool restoreState(Common::InSaveFile *pFile);
	void stopAll();
	void clearPlaylist() {
		_playList.clear();
	}
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

	int findListSlot(reg_t obj) {
		for (uint32 i = 0; i < _playList.size(); i++) {
			if (_playList[i]->soundObj == obj)
				return i;
		}
		return -1;
	}

	uint16 _savelen;
	Common::Array<MusicEntry *> _playList;

protected:
	byte findAudEntry(uint16 nAud, byte&oVolume, uint32& oOffset, uint32&oSize);
	void sortPlayList();
#if 0
	void loadPatchMT32();
	void patchSysEx(byte * addr, byte *pdata, int len);
	void patchUpdateAddr(byte *addr, int len);
#endif

	void doFade(MusicEntry *pSnd);

	Audio::Mixer *_pMixer;
	MidiDriver *_pMidiDrv;
	MidiDriverType _midiType;
	Common::Mutex _mutex;

	uint32 _dwTempo;
	bool _bMultiMidi; // use adlib's digital track if midi track don't have one
private:
	static void miditimerCallback(void *p);
};

class MidiParser_SCI : public MidiParser {
public:
	MidiParser_SCI();
	~MidiParser_SCI();
	bool loadMusic(SoundResource::Track *track, MusicEntry *psnd);
	bool loadMusic(byte *, uint32) {
		return false;
	}
	void unloadMusic();
	void setVolume(byte bVolume);
	void stop() {
		_abort_parse = true;/*hangAllActiveNotes();*/
	}
	void pause() {
		_abort_parse = true; /*hangAllActiveNotes();*/
	}

protected:
	void parseNextEvent(EventInfo &info);
	byte *midiMixChannels();
	byte midiGetNextChannel(long ticker);
	byte *_mixedData;
	SoundResource::Track *_track;
	MusicEntry *_pSnd;
	uint32 _loopTick;
	byte _volume;
};

} // end of namespace

#endif
