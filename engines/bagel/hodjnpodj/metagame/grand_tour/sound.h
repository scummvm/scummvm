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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BAGEL_METAGAME_GTL_SOUND_H
#define BAGEL_METAGAME_GTL_SOUND_H

#include <limits.h>
#include <mmsystem.h>

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace GrandTour {

#define VOLUME_INDEX_MIN        1
#define VOLUME_INDEX_MAX        10

#define VOLUME_LEVEL            0xFFFF
#define VOLUME_INDEX_FACTOR     0x10

#define SOUND_BUFFERED          0x0001
#define SOUND_ASYNCH            0x0002
#define SOUND_NOTIFY            0x0004
#define SOUND_AUTODELETE        0x0008
#define SOUND_LOOP              0x0010
#define SOUND_WAVE              0x0020
#define SOUND_MIDI              0x0040
#define SOUND_DONT_LOOP_TO_END  0x0080
#define SOUND_QUEUE             0x0100

#define SOUND_TYPE_MCI          1
#define SOUND_TYPE_SND          2
#define SOUND_TYPE_MMIO         3

#define FMT_MILLISEC    1           //time format specifiers.
#define FMT_BPM             2


typedef struct waveInst {
	HANDLE hWaveInst;
	HANDLE hWaveHdr;
	HANDLE hWaveData;
} WAVEINST;

typedef WAVEINST FAR *LPWAVEINST;

#define LPGLOBALENTRY   GLOBALENTRY FAR *
#define OBJECTSIZE      544


class CSound : public CObject {
	DECLARE_DYNCREATE(CSound)

public:
	CSound();
	CSound(CWnd *pWnd, char *pszPathName, WORD wFlags);
	~CSound();

	void Initialize(char *pszPathName, WORD wFlags) {
		Initialize(NULL, pszPathName, wFlags);
	}
	void Initialize(CWnd *pWnd, char *pszPathName, WORD wFlags);
	int  GetID() {
		return (m_nID);
	}

	void SetDrivePath(char *pszDrivePath) {
		Common::strcpy_s(m_chDrivePath, pszDrivePath);
	}

	BOOL MidiLoopPlaySegment(DWORD LoopBegin, DWORD LoopEnd = 0L, DWORD FirstPassBegin = 0L, DWORD TimeFmt = FMT_MILLISEC);
	BOOL Play(DWORD StartOfPlay = 0L, DWORD TimeFmtFlag = FMT_MILLISEC);
	BOOL Pause(void);
	BOOL Resume(void);
	BOOL Stop(void);

	void SetFlags(WORD wFlags) {
		m_wFlags = wFlags;
	}
	WORD GetFlags(void) {
		return (m_wFlags);
	}

	BOOL Error() {
		return (m_bError);
	}
	BOOL Playing(void) {
		return (m_bPlaying);
	}
	BOOL Paused(void) {
		return (m_bPaused);
	}

	static  void InitializeSound(CWnd *pWnd);
	static  void SetVolume(int MidiVolume, int WaveVolume);
	static  BOOL SoundAvailable(void);
	static  BOOL MidiAvailable(void);
	static  BOOL SoundVolumeAvailable(void) {
		return (m_bWaveVolume);
	}
	static  BOOL MidiVolumeAvailable(void) {
		return (m_bMidiVolume);
	}
	static  BOOL SoundPlaying(void) {
		return ((m_nCount > 0) ? TRUE : FALSE);
	}
	static  CSound *OnMCIStopped(WPARAM wParam, LPARAM lParam);
	static  CSound *OnMMIOStopped(WPARAM wParam, LPARAM lParam);
	static  BOOL PauseSounds(void);
	static  BOOL ResumeSounds(void);
	static  BOOL StopSounds(void);
	static  BOOL StopWaveSounds(void);
	static  BOOL StopMidiSounds(void);
	static  void ClearSounds(void);
	static  void ClearWaveSounds(void);
	static  void ClearMidiSounds(void);
	static  void WaitSounds(void);
	static  void WaitWaveSounds(void);
	static  void WaitMidiSounds(void);
	static  BOOL HandleMessages(void);

private:
	BOOL PlaySndSound(void);
	BOOL PlayMMIOSound(void);
	BOOL PlayMCISound(DWORD dwStartOfPlay, DWORD TimeFormat);
	BOOL RePlayMCISound(DWORD wFlags, DWORD From = 0L, DWORD To = 0L);
	DWORD SetMCITimeFormat(UINT wDeviceID, DWORD FmtFlag = FMT_MILLISEC);

private:
	int         m_nID;          // unique identifier
	char        *m_pszPathName; // path spec for sound file
	char        m_chType;       // type of sound commands used
	UINT        m_wDeviceID;    // MCI device id
	LPWAVEINST  m_lpWaveInst;   // MMIO wave instance
	HWAVEOUT    m_hWaveOut;     // MMIO wave device
	LPWAVEHDR   m_lpWaveHdr;    // MMIO wave header
	WORD        m_wFlags;       // flags for playing
	BOOL        m_bPlaying;     // whether its playing
	BOOL        m_bPaused;      // whether its paused
	BOOL        m_bError;       // whether an error occurred
	DWORD       m_dwErrorCode;  // error code

	BOOL        m_bExtensionsUsed;
	DWORD       m_dwPlayStart;
	DWORD       m_dwRePlayStart;
	DWORD       m_dwRePlayEnd;

public:
	HMODULE     m_hMCIModule;       // handle for MCI driver module
	HGLOBAL     m_hMCIMemory[32];   // handles for MCI memory buffers
	int         m_nMCIHandleCount;  // number of items in memory list

private:
	char        m_chDrivePath[128]; // path spec to drive

	CWnd        *m_pWnd;        // parent window for messages

	CSound      *m_pNext;       // next sound in chain or NULL
	CSound      *m_pPrev;       // previous sound in chain or NULL

	static  BOOL    m_bInited;      // whether sounds have been inited
	static  CSound  *m_pSoundChain; // first item in chain or NULL
	static  int     m_nIndex;       // used to create IDs
	static  int     m_nCount;       // count of active sounds
	static  int     m_nWavCount;    // number of wave sound devices
	static  int     m_nMidiCount;   // number of midi sound devices
	static  BOOL    m_bSoundAvailable;  // whether wave sound is available
	static  BOOL    m_bMidiAvailable;   // whether midi sound is available
	static  BOOL    m_bWaveVolume;  // whether wave volume can be set
	static  BOOL    m_bMidiVolume;  // whether midi volume can be set
	static  CWnd    *m_pMainWnd;    // window for message processing

	#ifdef BAGEL_DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
	#endif

};

} // namespace GrandTour
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
