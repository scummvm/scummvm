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

class SoundRes : Resource {
public:
	enum kTrackType {
		kTrackAdlib = 0,
		kTrackGameBlaster = 9,
		kTrackMT32 = 12,
		kTrackSpeaker = 18,
		kTrackTandy = 19
	};

	struct tagChannel {
		byte number;
		byte poly;
		uint16 unk;
		uint16 size;
		byte *ptr;
		long time;
		byte prev;
	};

	struct tagTrack {
		kTrackType type;
		byte nDigital;
		byte nChannels;
		tagChannel *aChannels;
		uint sz;
	};
public:
	SoundRes(SCIHANDLE handle, uint32 ResId);
	~SoundRes();
	tagTrack *getTrackByNumber(uint16 number);
	tagTrack *getTrackByType(kTrackType type);

protected:
	byte nTracks;
	tagTrack *aTracks;
};

enum kSndStatus {
	kStopped = 0, kPaused, kPlaying
};

// script-used struct to manipulate sound (358 bytes)
class MidiParser_SCI;

struct sciSound {
	//sciNode node; // [0-5]	// we use a Common::List
	uint16 resnum;// [6-7]
	//  byte * pMidiData;
	Audio::AudioStream* pStreamAud;
	MidiParser_SCI *pMidiParser;
	Audio::SoundHandle hCurrentAud;
	kSndStatus sndStatus;

	uint16 dataInc; //[338-339]
	uint16 ticker;
	uint16 signal; //[344]
	byte prio; // 348
	byte loop; // 349
	byte volume; // 350

	byte FadeTo;
	short FadeStep;
	uint32 FadeTicker;
	uint32 FadeTickerStep;

	//  byte unk7[7];
};

class SciMusic {
public:
	SciMusic();
	~SciMusic();

	void init();
	void loadPatch();
	void onTimer();
	bool saveState(Common::OutSaveFile *pFile);
	bool restoreState(Common::InSaveFile *pFile);
	void stopAll();
	void clearPlaylist() {
		_playList.clear();
	}
	// sound and midi functions
	void soundInitSnd(SoundRes *res, sciSound *pSnd);
	void soundPlay(sciSound *pSnd);
	void soundStop(sciSound *pSnd);
	void soundKill(sciSound *pSnd);
	void soundPause(sciSound *pSnd);
	void soundSetVolume(sciSound *pSnd, byte volume);
	void soundSetPriority(sciSound *pSnd, byte prio);
	uint16 soundGetMasterVolume();
	void soundSetMasterVolume(uint16 vol);
	uint16 soundGetVoices();
	uint32 soundGetTempo() {
		return _dwTempo;
	}

	uint16 _savelen;
protected:
	byte findAudEntry(uint16 nAud, byte&oVolume, uint32& oOffset, uint32&oSize);
	void sortPlayList();
	void loadPatchMT32();
	void patchSysEx(byte * addr, byte *pdata, int len);
	void patchUpdateAddr(byte *addr, int len);
	void doFade(sciSound *pSnd);

	Audio::Mixer *_pMixer;
	MidiDriver *_pMidiDrv;
	int _midiType;
	Common::Mutex _mutex;

	Common::Array<sciSound *> _playList;
	uint32 _dwTempo;
	bool _bMultiMidi; // use adlib's digital track if midi track don't have one

private:
	static void miditimerCallback(void *p);

};

class MidiParser_SCI : public MidiParser {
public:
	MidiParser_SCI();
	~MidiParser_SCI();
	bool loadMusic(SoundRes::tagTrack *ptrack, sciSound *psnd);
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
	byte *_pMidiData;
	SoundRes::tagTrack *_pTrack;
	sciSound *_pSnd;
	uint32 _loopTick;
	byte _volume;
};

} // end of namespace

#endif
