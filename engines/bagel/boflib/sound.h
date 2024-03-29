
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
#include "bagel/boflib/boffo.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/llist.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/debug.h"
#include "bagel/boflib/queue.h"
#include "bagel/boflib/gui/window.h"

namespace Bagel {


//
// Wavemix-related constants
//
#define VOLUME_INDEX_MIN 0
#define VOLUME_INDEX_MAX 12
#define VOLUME_INDEX_DEFAULT 10

// Convert to ScummVM scale
#define VOLUME_SVM(x) (x * Audio::Mixer::kMaxChannelVolume / VOLUME_INDEX_MAX)

#define SOUND_BUFFERED 0x0001
#define SOUND_ASYNCH 0x0002
#define SOUND_NOTIFY 0x0004
#define SOUND_AUTODELETE 0x0008
#define SOUND_LOOP 0x0010
#define SOUND_WAVE 0x0020
#define SOUND_MIDI 0x0040
#define SOUND_DONT_LOOP_TO_END 0x0080
#define SOUND_QUEUE 0x0100   // with wavemix: play after previous sound finishes
#define SOUND_MIX 0x0200     // Use wavemix to really play asynchronously
#define SOUND_ASAP 0x0400    // Play as soon as a channel is free (maybe now)
#define SOUND_WAIT 0x0800    // for wavemix: Set sound up but don't play it yet
#define SOUND_LOCK 0x1000    // Reserve this channel until user stops it
#define SOUND_PRELOAD 0x2000 // Only works for Resource MAC snd files
#define SOUND_OVEROK 0x4000  // OK to play another sound over this file

#define SOUND_TYPE_WAV 1
#define SOUND_TYPE_XM 2
#define SOUND_TYPE_QT 3

#define FMT_MILLISEC 1 // time format specifiers.
#define FMT_BPM 2

#define NUM_QUEUES 8

class CBofSound : public CBofError, public CBofObject, public CLList {
public:
	friend class MusicPlayer;

	CBofSound(CBofWindow *pWnd, const CHAR *pszPathName, WORD wFlags, const INT nLoops = 1);
	virtual ~CBofSound();

	// Implementation
	static VOID SetDrivePath(const CHAR *pszDrivePath);
	static VOID GetDrivePath(CHAR *pszDrivePath);

	BOOL MidiLoopPlaySegment(DWORD LoopBegin, DWORD LoopEnd = 0L, DWORD FirstPassBegin = 0L, DWORD TimeFmt = FMT_MILLISEC);
	BOOL Play(DWORD StartOfPlay = 0L, DWORD TimeFmtFlag = FMT_MILLISEC);
	BOOL Pause();
	BOOL Resume();
	BOOL Stop();

	CBofSound *GetNext() {
		return (CBofSound *)m_pNext;
	}
	CBofSound *GetPrev() {
		return (CBofSound *)m_pPrev;
	}

	CHAR *GetFileName() {
		return &m_szFileName[0];
	}

	VOID SetFlags(WORD wFlags) {
		m_wFlags = wFlags;
	}
	WORD GetFlags() {
		return m_wFlags;
	}

	BOOL Playing() {
		return IsPlaying();
	}

	BOOL IsPlaying() {
		return m_bPlaying;
	}
	BOOL IsQueued() {
		return m_bInQueue;
	}

	BOOL Paused() {
		return m_bPaused;
	}

	VOID SetQSlot(INT nSlot) {
		m_iQSlot = nSlot;
	}
	INT GetQSlot() {
		return m_iQSlot;
	}

	VOID SetVolume(INT nVol);
	INT GetVolume() {
		return m_nVol;
	}

	static VOID Initialize();
	static VOID UnInitialize();
	static VOID SetVolume(INT MidiVolume, INT WaveVolume);
	static BOOL SoundAvailable();
	static BOOL MidiAvailable();
	static BOOL SoundVolumeAvailable() {
		return m_bWaveVolume;
	}
	static BOOL MidiVolumeAvailable() {
		return m_bMidiVolume;
	}

	static BOOL SoundPlaying() {
		return (m_nCount > 0) ? TRUE : FALSE;
	}

	static BOOL WaveSoundPlaying();
	static BOOL MidiSoundPlaying();
	static BOOL SoundsPlayingNotOver();

	static VOID SetQVol(INT nSlot, INT nVol);

	ERROR_CODE PlayWAV();

	static ERROR_CODE FlushQueue(INT nSlot);

	static VOID ResetQVolumes();

	static BOOL PauseSounds();
	static BOOL ResumeSounds();
	static BOOL StopSounds();
	static BOOL StopWaveSounds();
	static BOOL StopMidiSounds();
	static VOID ClearSounds();
	static VOID ClearWaveSounds();
	static VOID ClearMidiSounds();
	static VOID WaitSounds();
	static VOID WaitWaveSounds();
	static VOID WaitMidiSounds();
	static BOOL HandleMessages();
	static BOOL Sleep(DWORD dwTime);
	static VOID AudioTask();
	static BOOL SoundsPlaying();

private:
	BOOL LoadSound();
	BOOL ReleaseSound();

private:
	CHAR m_szFileName[MAX_FNAME]; // path spec for sound file
	int8 m_chType = 0;            // type of sound commands used

	WORD m_wLoops = 0;   // number of times to loop the sound (0xFFFF means infinite)
	WORD m_wFlags = 0;   // flags for playing
	BOOL m_bPaused = FALSE;  // whether its paused
	BOOL m_bPlaying = FALSE; // whether its playing

	BOOL m_bExtensionsUsed = FALSE;
	DWORD m_dwPlayStart = 0;
	DWORD m_dwRePlayStart = 0;
	DWORD m_dwRePlayEnd = 0;
	Audio::SoundHandle m_handle;
	UBYTE *m_pFileBuf = nullptr;
	UINT m_iFileSize = 0;

	INT m_iQSlot = 0;
	BOOL m_bInQueue = FALSE;
	BOOL m_bStarted = FALSE;
	INT m_nVol = 0;

	CBofWindow *m_pWnd = nullptr; // parent window for messages

	static CHAR m_szDrivePath[MAX_DIRPATH]; // path spec to drive
	static CBofSound *m_pSoundChain;        // first item in chain or nullptr
	static INT m_nCount;                    // count of active sounds
	static INT m_nWavCount;                 // number of wave sound devices
	static INT m_nMidiCount;                // number of midi sound devices
	static BOOL m_bSoundAvailable;          // whether wave sound is available
	static BOOL m_bMidiAvailable;           // whether midi sound is available
	static BOOL m_bWaveVolume;              // whether wave volume can be set
	static BOOL m_bMidiVolume;              // whether midi volume can be set
	static CBofWindow *m_pMainWnd;          // window for message processing
	static BOOL m_bInit;

	static INT m_nSlotVol[NUM_QUEUES];
	static CQueue *m_cQueue[NUM_QUEUES];
};

BOOL BofPlaySound(const CHAR *pszSoundFile, UINT nFlags, INT iQSlot = 0);
BOOL BofPlaySoundEx(const CHAR *pszSoundFile, UINT nFlags, INT iQSlot = 0, BOOL bWait = FALSE);

// support legacy code
#define CSound CBofSound

} // namespace Bagel

#endif
