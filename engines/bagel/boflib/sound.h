
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/".
 *
 */

#ifndef BAGEL_BOFLIB_SOUND_H
#define BAGEL_BOFLIB_SOUND_H

#include "audio/mixer.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/llist.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/queue.h"

namespace Bagel {

//
// Wavemix-related constants
//
#define VOLUME_INDEX_MIN 0
#define VOLUME_INDEX_MAX 12
#define VOLUME_INDEX_DEFAULT 10

// Convert to ScummVM scale
#define VOLUME_SVM(x) ((x) * Audio::Mixer::kMaxChannelVolume / VOLUME_INDEX_MAX)
#define SVM_VOLUME(x) (((x) >= 252) ? VOLUME_INDEX_MAX : (x) * VOLUME_INDEX_MAX / 252)

#define SOUND_BUFFERED 0x0001
#define SOUND_ASYNCH 0x0002
#define SOUND_NOTIFY 0x0004
#define SOUND_AUTODELETE 0x0008
#define SOUND_LOOP 0x0010
#define SOUND_WAVE 0x0020
#define SOUND_MIDI 0x0040
#define SOUND_DONT_LOOP_TO_END 0x0080
#define SOUND_QUEUE 0x0100   // With wavemix: play after previous sound finishes
#define SOUND_MIX 0x0200     // Use wavemix to really play asynchronously
#define SOUND_ASAP 0x0400    // Play as soon as a channel is free (maybe now)
#define SOUND_WAIT 0x0800    // For wavemix: Set sound up but don't play it yet
#define SOUND_LOCK 0x1000    // Reserve this channel until user stops it
#define SOUND_PRELOAD 0x2000 // Only works for Resource MAC snd files
#define SOUND_OVEROK 0x4000  // OK to play another sound over this file
#define SND_MEMORY 0x8000    // Raw in-memory sound passed rather than a filename

#define SND_NODEFAULT 0
#define SND_SYNC 0
#define SND_ASYNC SOUND_ASYNCH

#define SOUND_TYPE_WAV 1
#define SOUND_TYPE_XM 2
#define SOUND_TYPE_QT 3
#define SOUND_TYPE_SMF 4

#define FMT_MILLISEC 1 // Time format specifiers.
#define FMT_BPM 2

#define NUM_QUEUES 8

class CBofSound : public CBofError, public CBofObject, public CLList {
public:
	friend class MusicPlayer;

	CBofSound();
	CBofSound(void *pWnd, const char *pszPathName, uint16 wFlags, int nLoops = 1);
	virtual ~CBofSound();
	void addToSoundChain();

	void initialize(const char *pszPathName, uint16 wFlags) {
		initialize(nullptr, pszPathName, wFlags);
	}
	void initialize(void *pWnd, const char *pszPathName, uint16 wFlags);
	void setDrivePath(const char *path) {}
	bool midiLoopPlaySegment(uint32 LoopBegin, uint32 LoopEnd = 0L, uint32 FirstPassBegin = 0L, uint32 TimeFmt = FMT_MILLISEC);
	bool play(uint32 StartOfPlay = 0L, uint32 TimeFmtFlag = FMT_MILLISEC);
	bool pause();
	bool resume();
	void stop();

	CBofSound *getNext() {
		return (CBofSound *)_pNext;
	}
	CBofSound *getPrev() {
		return (CBofSound *)_pPrev;
	}

	char *getFileName() {
		return &_szFileName[0];
	}

	void setFlags(uint16 wFlags) {
		_wFlags = wFlags;
	}
	uint16 getFlags() {
		return _wFlags;
	}

	bool playing() {
		return isPlaying();
	}

	bool isPlaying() {
		return _bPlaying;
	}
	bool isQueued() {
		return _bInQueue;
	}

	bool paused() {
		return _bPaused;
	}

	void setQSlot(int nSlot) {
		_iQSlot = nSlot;
	}
	int getQSlot() {
		return _iQSlot;
	}

	void setVolume(int nVol);
	int getVolume() {
		return _nVol;
	}

	static void initialize();
	static void shutdown();
	static void setVolume(int MidiVolume, int WaveVolume);

	static bool soundPlaying() {
		return (_nCount > 0) ? true : false;
	}

	static bool waveSoundPlaying();
	static bool midiSoundPlaying();
	static bool soundsPlayingNotOver();

	static void setQVol(int nSlot, int nVol);

	ErrorCode playWAV();

	static ErrorCode flushQueue(int nSlot);

	static void resetQVolumes();

	static bool pauseSounds();
	static bool resumeSounds();
	static void stopSounds();
	static void stopWaveSounds();
	static void stopMidiSounds();
	static void clearSounds();
	static void clearWaveSounds();
	static void clearMidiSounds();
	static void waitSounds();
	static void waitWaveSounds();
	static void waitMidiSounds();
	static bool handleMessages();
	static bool bofSleep(uint32 dwTime);
	static void audioTask();
	static bool soundsPlaying();
	static bool MidiAvailable() {
		return true;
	}
	static bool SoundAvailable() {
		return true;
	}
	static bool SoundVolumeAvailable() {
		return true;
	}
	static bool MidiVolumeAvailable() {
		return true;
	}

	static CBofSound *OnMCIStopped(WPARAM wParam, LPARAM lParam) {
		return nullptr;
	}
	static CBofSound *OnMMIOStopped(WPARAM wParam, LPARAM lParam) {
		return nullptr;
	}

private:
	bool loadSound();
	bool releaseSound();

private:
	char _szFileName[MAX_FNAME]; // Path spec for sound file
	int8 _chType = 0;            // Type of sound commands used

	uint16 _wLoops = 0;     // Number of times to loop the sound (0xFFFF means infinite)
	uint16 _wFlags = 0;     // Flags for playing
	bool _bPaused = false;  // Whether its paused
	bool _bPlaying = false; // Whether its playing

	bool _bExtensionsUsed = false;
	uint32 _dwPlayStart = 0;
	uint32 _dwRePlayStart = 0;
	uint32 _dwRePlayEnd = 0;
	Audio::SoundHandle _handle;
	byte *_pFileBuf = nullptr;
	uint32 _iFileSize = 0;

	int _iQSlot = 0;
	bool _bInQueue = false;
	bool _bStarted = false;
	int _nVol = 0;

	void *_pWnd = nullptr; // Parent window for messages

	static char _szDrivePath[MAX_DIRPATH]; // Path spec to drive
	static CBofSound *_pSoundChain;        // First item in chain or nullptr
	static int _nCount;                    // Count of active sounds
	static int _nWavCount;                 // Number of wave sound devices
	static int _nMidiCount;                // Number of midi sound devices
	static void *_pMainWnd;                // Window for message processing
	static bool _bInit;

	static int _nSlotVol[NUM_QUEUES];
	static CQueue *_cQueue[NUM_QUEUES];
};

#define CSound CBofSound

extern bool BofPlaySound(const char *pszSoundFile, uint32 nFlags, int iQSlot = 0);
extern bool BofPlaySoundEx(const char *pszSoundFile, uint32 nFlags, int iQSlot = 0, bool bWait = false);
extern bool MessageBeep(int uType);

#define sndPlaySound BofPlaySound

} // namespace Bagel

#endif
