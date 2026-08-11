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

#define BUILD_FOR_META_DLL TRUE

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/boflib/sound.h"

#if BUILD_FOR_META_DLL
	#include "bagel/hodjnpodj/metagame/bgen/bgb.h"
#endif

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#ifdef BAGEL_DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CSound, CObject)

#define MEMORY_THRESHOLD    20000L
#define MEMORY_MARGIN       100000L

#if BUILD_FOR_META_DLL
	extern  DWORD GetPhysicalMemory(void);
	extern  CBgbMgr *gpBgbMgr;
#else
	DWORD GetPhysicalMemory(void);
#endif

void PageLockMCISeqBuffers(CSound *pSound);
BOOL MaybePageLockObject(CSound *pSound, LPGLOBALENTRY lpge);
void PageUnlockMCISeqBuffers(CSound *pSound);
BOOL MaybePageUnlock(LPGLOBALENTRY lpge, HGLOBAL hObject);

BOOL    CSound::m_bInited = FALSE;      // whether sounds have been inited
CSound  *CSound::m_pSoundChain = NULL;  // pointer to chain of linked Sounds
int     CSound::m_nIndex = 0;           // unique Sound indentifier
int     CSound::m_nCount = 0;           // count of currently active Sounds
int     CSound::m_nWavCount = 0;        // available wave sound devices
int     CSound::m_nMidiCount = 0;       // available midi sound devices
BOOL    CSound::m_bSoundAvailable = FALSE;  // whether wave sound is available
BOOL    CSound::m_bMidiAvailable = FALSE;   // whether midi sound is available
BOOL    CSound::m_bWaveVolume = FALSE;  // whether wave volume can be set
BOOL    CSound::m_bMidiVolume = FALSE;  // whether midi volume can be set
CWnd    *CSound::m_pMainWnd = NULL;         // window for message processing


/*************************************************************************
 *
 * CSound()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Constructor for Sound class.  Initialize all fields
 *                  to logical NULL.  Calls should then be made to the
 *                  Initialize and Play routines.
 *
 ************************************************************************/

CSound::CSound() {
	Initialize(NULL, NULL, 0);              // initialize to null values

	m_pNext = m_pSoundChain;                // insert at head of chain
	if (m_pSoundChain != NULL)
		(*m_pSoundChain).m_pPrev = this;
	m_pPrev = NULL;
	m_pSoundChain = this;
}


/*************************************************************************
 *
 * CSound()
 *
 * Parameters:
 *
 *  pWnd            pointer to the parent window
 *  pszPathName     pointer to text string that is the path specification
 *                  for the Sound out on disk
 *  wFlags          specialized flags
 *
 * Return Value:    none
 *
 * Description:     Constructor for sound class.  Initialize all fields
 *                  to values specifed.  Calls should then be made to the
 *                  Play routine.
 *
 ************************************************************************/

CSound::CSound(CWnd *pWnd, char *pszPathName, WORD wFlags) {
	Initialize(pWnd, pszPathName, wFlags);  // initialize data fields

	m_pNext = m_pSoundChain;                // insert at head of chain
	if (m_pSoundChain != NULL)
		(*m_pSoundChain).m_pPrev = this;
	m_pPrev = NULL;
	m_pSoundChain = this;
}


/*************************************************************************
 *
 * ~CSound()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destructor for Sound class.  The Sound object is
 *                  removed from the chain.  If it is then stopped and unloaded
 *                  if it is currently active.
 *
 ************************************************************************/

CSound::~CSound() {
	BOOL    bSuccess;
	CSound  *pSound, *pTemp;

	if (m_nID == (*m_pSoundChain).m_nID) {              // special case head of chain
		m_pSoundChain = (*m_pSoundChain).m_pNext;
		if (m_pSoundChain != NULL)
			(*m_pSoundChain).m_pPrev = NULL;
	} else {                                            // find it in the chain and
		pSound = m_pSoundChain;                         // ... modify the linkages of
		while ((*pSound).m_nID != m_nID) {              // ... its neighbors to point
			pSound = (*pSound).m_pNext;                 // ... around it
		}
		pTemp = (*pSound).m_pPrev;
		(*pTemp).m_pNext = m_pNext;
		pTemp = m_pNext;
		if (pTemp != NULL)
			(*pTemp).m_pPrev = m_pPrev;
	}

	if (m_bPlaying == TRUE) {                           // stop it and unload it if
		bSuccess = Stop();                              // ... it is currently playing
		ASSERT(bSuccess);
	}
}


/*************************************************************************
 *
 * InitializeSound()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Do initial sound setup functions.
 *
 ************************************************************************/

void CSound::InitializeSound(CWnd *pWnd) {
	m_nWavCount = ::waveOutGetNumDevs();
	if (m_nWavCount > 0)
		m_bSoundAvailable = TRUE;
	else
		m_bSoundAvailable = FALSE;

	m_nMidiCount = ::midiOutGetNumDevs();
	if (m_nMidiCount > 0)
		m_bMidiAvailable = TRUE;
	else
		m_bMidiAvailable = FALSE;

	if (pWnd != NULL)
		m_pMainWnd = pWnd;
}


/*************************************************************************
 *
 * SetVolume()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Set midi and wave volume levels (0 to 10).
 *
 ************************************************************************/

void CSound::SetVolume(int nMidiVolume, int nWaveVolume) {
	int     nDev;
	UINT    nResult;
	DWORD   nVolumeIndex;
	DWORD   dwVolume;

	m_bWaveVolume = FALSE;
	m_bMidiVolume = FALSE;

	CSound::InitializeSound(m_pMainWnd);

	if (m_bSoundAvailable) {
		nVolumeIndex = nWaveVolume;
		if (nVolumeIndex < VOLUME_INDEX_MIN)
			nVolumeIndex = VOLUME_INDEX_MIN;
		else if (nVolumeIndex > VOLUME_INDEX_MAX)
			nVolumeIndex = VOLUME_INDEX_MAX;
		dwVolume = (nVolumeIndex << 12) + 0x5FFF;
		dwVolume |= (dwVolume << 16);
		nResult = ::waveOutSetVolume(WAVE_MAPPER, dwVolume);
		if (nResult == 0)
			m_bWaveVolume = TRUE;
		for (nDev = 0; nDev < m_nWavCount; nDev++) {
			nResult = ::waveOutSetVolume(nDev, dwVolume);
			if (nResult == 0)
				m_bWaveVolume = TRUE;
		}
	}

	if (m_bMidiAvailable) {
		nVolumeIndex = nMidiVolume;
		if (nVolumeIndex < VOLUME_INDEX_MIN)
			nVolumeIndex = VOLUME_INDEX_MIN;
		else if (nVolumeIndex > VOLUME_INDEX_MAX)
			nVolumeIndex = VOLUME_INDEX_MAX;
		dwVolume = (nVolumeIndex << 12) + 0x5FFF;
		dwVolume |= (dwVolume << 16);
		nResult = ::midiOutSetVolume(MIDI_MAPPER, dwVolume);
		if (nResult == 0)
			m_bMidiVolume = TRUE;
		for (nDev = 0; nDev < m_nMidiCount; nDev++) {
			nResult = ::midiOutSetVolume(nDev, dwVolume);
			if (nResult == 0)
				m_bMidiVolume = TRUE;
		}
	}

	AfxGetApp()->WriteProfileInt("Meta", "MidiVolume", nMidiVolume);
	AfxGetApp()->WriteProfileInt("Meta", "WaveVolume", nWaveVolume);
}


/*****************************************************************
 *
 * Initialize
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Initialize a Sound that is in WAV or MIDI format.
 *
 * FORMAL PARAMETERS:
 *
 *  pWnd            pointer to the parent window
 *  pszPathName     pointer to text string that is the path specification
 *                  for the Sound out on disk
 *  wFlags          specialized flags (SOUND_xyz)
 *
 * RETURN VALUE:
 *
 *      BOOL        success / failure condition
 *
 ****************************************************************/

void CSound::Initialize(CWnd *pWnd, char *pszPathName, WORD wFlags) {
	if (pWnd == NULL)
		m_pWnd = m_pMainWnd;
	else {
		m_pWnd = pWnd;
		if (m_pMainWnd == NULL)
			m_pMainWnd = m_pWnd;
	}

	m_pszPathName = pszPathName;            // path spec for sound file
	m_wFlags = wFlags;                      // flags for playing
	m_lpWaveInst = NULL;                    // no wave data
	m_lpWaveHdr = NULL;
	m_hWaveOut = NULL;
	m_wDeviceID = 0;                        // no device id
	m_nID = ++m_nIndex;                     // unique identifier
	m_bPlaying = FALSE;                     // not yet playing
	m_bPaused = FALSE;                      // not suspended
	m_bError = FALSE;                       // no errors yet
	m_dwErrorCode = 0;                      // ... so no error code
	m_bExtensionsUsed = FALSE;              // no extended flags used.
	m_chDrivePath[0] = '\0';                // just use sound file path
}


/*****************************************************************
 *
 * Play
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Dispatch to the appropriate routine to play a sound object.
 *
 * FORMAL PARAMETERS: (BOTH OPTIONAL)
 *
 *      DWORD dwBeginHere       Begin Playing Sound at this time (fmt as in parameter #2).
 *      DWORD TimeFormatFlag    Specifies TimeFormat; the following constants are defined :
                                                                FMT_MILLISEC ---> all time parameters will be in milliseconds.
 *
 *      dwStartOfPlay is defualted to 0L.
 *      TimeFlag is defaulted to FMT_MILLISEC.
 *
 * RETURN VALUE:
 *
 *      BOOL        success / failure condition
 *
 ****************************************************************/

BOOL CSound::Play(DWORD dwBeginHere, DWORD TimeFormatFlag) {
	BOOL    bRetry = FALSE;
	BOOL    bSuccess = FALSE;

	if (m_bPlaying)                                     // punt if already playing
		return (FALSE);

	if (m_pWnd == NULL) {                               // not valid if no owning window
		if (m_pMainWnd == NULL)
			return (FALSE);
		m_pWnd = m_pMainWnd;
	}

	if (m_bInited == FALSE) {                           // initialize sound info if not done
		InitializeSound(m_pMainWnd);
		m_bInited = TRUE;
	}

	if (m_wFlags & SOUND_WAVE) {
		if (m_wFlags & SOUND_QUEUE)
			WaitWaveSounds();
		else
			StopWaveSounds();
	} else if (m_wFlags & SOUND_MIDI) {
		if (m_wFlags & SOUND_QUEUE)
			WaitMidiSounds();
		else
			StopMidiSounds();
	}

try_again:

	m_bError = TRUE;                                    // assume there is an error
	m_dwErrorCode = 0;

	if (m_wFlags & SOUND_WAVE) {                        // if it is a .WAV file
		if (m_bSoundAvailable == FALSE)                 // ... see if we can play it
			return (FALSE);
		if (!(m_wFlags & SOUND_BUFFERED) || TRUE) {     // first try play it loaded in memory
			if ((m_wFlags & SOUND_NOTIFY) ||            // otherwise method depends on whether
			        ((m_wFlags & SOUND_AUTODELETE) &&       // ... we want to:
			         (m_wFlags & SOUND_ASYNCH)))            // ... play and do something or
				bSuccess = PlayMMIOSound();             // ... play and forget
			else
				bSuccess = PlaySndSound();              // try to play it now
			if (bSuccess == TRUE)                       // Leave if successful
				return (TRUE);                          // ... otherwise try buffered mode
		}
	}

	if (m_wFlags & SOUND_MIDI)                          // if it is a .MID file
		if (m_bMidiAvailable == FALSE)                  // ... see if we can play it
			return (FALSE);

	bSuccess = PlayMCISound(dwBeginHere, TimeFormatFlag);                           // play an .MID file or buffered .WAV

	if (!bSuccess) {
		#ifdef BAGEL_DEBUG
		char    chErrorMessage[132];
		#endif
		if (!bRetry) {
			bRetry = TRUE;
			#if BUILD_FOR_META_DLL
			gpBgbMgr->CacheOptimize(0);
			#else
			(void) GlobalCompact(2000000L);
			(void) LocalCompact((UINT)64000);
			#endif
			goto try_again;
		}
		#ifdef BAGEL_DEBUG
		if (m_dwErrorCode == MCIERR_OUT_OF_MEMORY) {
			Common::sprintf_s(chErrorMessage, "Unable to play sound file %s", m_pszPathName);
			(*m_pWnd).MessageBox(chErrorMessage, "Insufficient System Resources", MB_ICONEXCLAMATION);
		} else if (m_chType == SOUND_TYPE_MCI) {
			Common::sprintf_s(chErrorMessage, "Unable to play sound file %s\nMCI error code %ld occurred", m_pszPathName, m_dwErrorCode);
			(*m_pWnd).MessageBox(chErrorMessage, "Internal Problem", MB_ICONEXCLAMATION);
		} else if (m_chType == SOUND_TYPE_MMIO) {
			Common::sprintf_s(chErrorMessage, "Unable to play sound file %s\nMMIO error code %ld occurred", m_pszPathName, m_dwErrorCode);
			(*m_pWnd).MessageBox(chErrorMessage, "Internal Problem", MB_ICONEXCLAMATION);
		} else if (m_chType == SOUND_TYPE_SND) {
			Common::sprintf_s(chErrorMessage, "Unable to play sound file %s\nsndPlaySound Routine failure %ld occurred", m_pszPathName, m_dwErrorCode);
			(*m_pWnd).MessageBox(chErrorMessage, "Internal Problem", MB_ICONEXCLAMATION);
		} else {
			Common::sprintf_s(chErrorMessage, "Unable to play sound file %s\nFailure code %ld occurred", m_pszPathName, m_dwErrorCode);
			(*m_pWnd).MessageBox(chErrorMessage, "Internal Problem", MB_ICONEXCLAMATION);
		}
		#endif
	}

	return bSuccess;
}


/*****************************************************************
 *
 * PlaySndSound
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Initialize, and begin playing, a WAV formated Sound
 *      via sndPlay commands.  Callbacks are not supported.
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      BOOL        success / failure condition
 *
 ****************************************************************/

BOOL CSound::PlaySndSound(void) {
	BOOL    bSuccess;
	char    chFilePath[256];

	if (m_chDrivePath[0] == '\0')
		Common::strcpy_s(chFilePath, m_pszPathName);
	else {
		Common::strcpy_s(chFilePath, m_chDrivePath);
		if (m_pszPathName[0] == '.')
			strcat(chFilePath, &m_pszPathName[1]);
		else
			strcat(chFilePath, m_pszPathName);
	}

	m_chType = SOUND_TYPE_SND;                      // mark it as an SndPlay type
	bSuccess = sndPlaySound(chFilePath,             // now play the sound
	                        ((m_wFlags & SOUND_ASYNCH) ? SND_ASYNC : 0) |
	                        ((m_wFlags & SOUND_LOOP) ? (SND_LOOP | SND_ASYNC) : 0));

	m_bError = !bSuccess;

	if (bSuccess == TRUE) {
		m_dwErrorCode = 0;
		if (!(m_wFlags & SOUND_ASYNCH) &&
		        (m_wFlags & SOUND_AUTODELETE))
			delete this;
	}

	return bSuccess;
}


/*****************************************************************
 *
 * PlayMMIOSound
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Initialize, and begin playing, a WAV formated Sound
 *      via low-level sound commands.
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      BOOL            success / failure condition
 *
 ****************************************************************/

BOOL CSound::PlayMMIOSound(void) {
	LPWAVEHDR   lpWaveHdr = NULL;
	LPWAVEINST  lpWaveInst = NULL;
	HANDLE      hWaveHdr = NULL;
	HANDLE      hWaveInst = NULL;
	HANDLE      hData = NULL;
	LPSTR       lpData = NULL;
	HWAVEOUT    hWaveOut = NULL;
	HMMIO       hmmio;
	MMCKINFO    mmckinfoParent;
	MMCKINFO    mmckinfoSubchunk;
	DWORD       dwFmtSize;
	WORD        wResult;
	HANDLE      hFormat = NULL;
	WAVEFORMAT  *pFormat = NULL;
	DWORD       dwDataSize;
	WORD        wBlockSize;
	char        chFilePath[256];

	if (m_chDrivePath[0] == '\0')
		Common::strcpy_s(chFilePath, m_pszPathName);
	else {
		Common::strcpy_s(chFilePath, m_chDrivePath);
		if (m_pszPathName[0] == '.')
			strcat(chFilePath, &m_pszPathName[1]);
		else
			strcat(chFilePath, m_pszPathName);
	}

	m_bError = TRUE;                                // assume failure
	m_dwErrorCode = 0;
	m_chType = SOUND_TYPE_MMIO;                     // mark it as a MMIO type

	/* Open the given file for reading using buffered I/O.
	 */
	if (!(hmmio = mmioOpen(chFilePath, NULL, MMIO_READ | MMIO_ALLOCBUF))) {
		return (FALSE);
	}

	/* Locate a 'RIFF' chunk with a 'WAVE' form type
	 * to make sure it's a WAVE file.
	 */
	mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if (mmioDescend(hmmio, (LPMMCKINFO) &mmckinfoParent, NULL, MMIO_FINDRIFF)) {
		mmioClose(hmmio, 0);
		return (FALSE);
	}

	/* Now, find the format chunk (form type 'fmt '). It should be
	 * a subchunk of the 'RIFF' parent chunk.
	 */
	mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent,
	                MMIO_FINDCHUNK)) {
		mmioClose(hmmio, 0);
		return (FALSE);
	}

	/* Get the size of the format chunk, allocate and lock memory for it.
	 */
	dwFmtSize = mmckinfoSubchunk.cksize;
	hFormat = LocalAlloc(LMEM_MOVEABLE, LOWORD(dwFmtSize));
	if (!hFormat) {
		mmioClose(hmmio, 0);
		m_dwErrorCode = MCIERR_OUT_OF_MEMORY;
		return (FALSE);
	}
	pFormat = (WAVEFORMAT *) LocalLock(hFormat);
	if (!pFormat) {
		LocalFree(hFormat);
		mmioClose(hmmio, 0);
		m_dwErrorCode = MCIERR_OUT_OF_MEMORY;
		return (FALSE);
	}

	/* Read the format chunk.
	 */
	if (mmioRead(hmmio, (HPSTR) pFormat, dwFmtSize) != (LONG) dwFmtSize) {
		LocalUnlock(hFormat);
		LocalFree(hFormat);
		mmioClose(hmmio, 0);
		return (FALSE);
	}

	/* Make sure it's a PCM file.
	 */
	if (pFormat->wFormatTag != WAVE_FORMAT_PCM) {
		LocalUnlock(hFormat);
		LocalFree(hFormat);
		mmioClose(hmmio, 0);
		return (FALSE);
	}

	/* Make sure a waveform output device supports this format.
	 */
	if (waveOutOpen(&hWaveOut, (UINT)WAVE_MAPPER, (LPWAVEFORMAT)pFormat, NULL, 0L,
	                (DWORD)WAVE_FORMAT_QUERY)) {
		LocalUnlock(hFormat);
		LocalFree(hFormat);
		mmioClose(hmmio, 0);
		return (FALSE);
	}

	/* Ascend out of the format subchunk.
	 */
	mmioAscend(hmmio, &mmckinfoSubchunk, 0);

	/* Find the data subchunk.
	 */
	mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent,
	                MMIO_FINDCHUNK)) {
		LocalUnlock(hFormat);
		LocalFree(hFormat);
		mmioClose(hmmio, 0);
		return (FALSE);
	}

	/* Get the size of the data subchunk.
	 */
	dwDataSize = mmckinfoSubchunk.cksize;
	if (dwDataSize == 0L) {
		LocalUnlock(hFormat);
		LocalFree(hFormat);
		mmioClose(hmmio, 0);
		return (FALSE);
	}

	#if 0
	/* Check for sufficient memory to lock the waveform data.
	 */
	if ((dwDataSize > MEMORY_THRESHOLD) &&
	        ((dwDataSize + MEMORY_MARGIN > GetPhysicalMemory()) ||
	         (dwDataSize + MEMORY_MARGIN > ::GlobalCompact(0)))) {
		LocalUnlock(hFormat);
		LocalFree(hFormat);
		mmioClose(hmmio, 0);
		m_dwErrorCode = MCIERR_OUT_OF_MEMORY;
		return (FALSE);
	}
	#endif

	/* Open a waveform output device.
	 */
	if (waveOutOpen((LPHWAVEOUT)&hWaveOut, (UINT)WAVE_MAPPER,
	                (LPWAVEFORMAT)pFormat, (UINT)(*m_pWnd).m_hWnd, 0L, (DWORD)CALLBACK_WINDOW)) {
		LocalUnlock(hFormat);
		LocalFree(hFormat);
		mmioClose(hmmio, 0);
		return (FALSE);
	}

	/* Save block alignment info for later use.
	 */
	wBlockSize = pFormat->nBlockAlign;

	/* We're done with the format header, free it.
	 */
	LocalUnlock(hFormat);
	LocalFree(hFormat);

	/* Allocate and lock memory for the waveform data.
	 */
	hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dwDataSize);
	if (!hData) {
		mmioClose(hmmio, 0);
		waveOutClose(hWaveOut);
		m_dwErrorCode = MCIERR_OUT_OF_MEMORY;
		return (FALSE);
	}
	lpData = (char *) GlobalLock(hData);
	if (!lpData) {
		GlobalFree(hData);
		mmioClose(hmmio, 0);
		waveOutClose(hWaveOut);
		m_dwErrorCode = MCIERR_OUT_OF_MEMORY;
		return (FALSE);
	}

	/* Read the waveform data subchunk.
	 */
	if (mmioRead(hmmio, (HPSTR) lpData, dwDataSize) != (LONG) dwDataSize) {
		GlobalUnlock(hData);
		GlobalFree(hData);
		mmioClose(hmmio, 0);
		waveOutClose(hWaveOut);
		return (FALSE);
	}

	/* We're done with the file, close it.
	 */
	mmioClose(hmmio, 0);

	/* Allocate a waveform data header. The WAVEHDR must be
	 * globally allocated and locked.
	 */
	hWaveHdr = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD) sizeof(WAVEHDR));
	if (!hWaveHdr) {
		GlobalUnlock(hData);
		GlobalFree(hData);
		waveOutClose(hWaveOut);
		m_dwErrorCode = MCIERR_OUT_OF_MEMORY;
		return (FALSE);
	}
	lpWaveHdr = (LPWAVEHDR) GlobalLock(hWaveHdr);
	if (!lpWaveHdr) {
		GlobalUnlock(hData);
		GlobalFree(hData);
		GlobalFree(hWaveHdr);
		waveOutClose(hWaveOut);
		m_dwErrorCode = MCIERR_OUT_OF_MEMORY;
		return (FALSE);
	}

	/* Allocate and set up instance data for waveform data block.
	 * This information is needed by the routine that frees the
	 * data block after it has been played.
	 */
	hWaveInst = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD) sizeof(WAVEHDR));
	if (!hWaveInst) {
		GlobalUnlock(hData);
		GlobalFree(hData);
		GlobalUnlock(hWaveHdr);
		GlobalFree(hWaveHdr);
		waveOutClose(hWaveOut);
		m_dwErrorCode = MCIERR_OUT_OF_MEMORY;
		return (FALSE);
	}
	lpWaveInst = (LPWAVEINST) GlobalLock(hWaveInst);
	if (!lpWaveInst) {
		GlobalUnlock(hData);
		GlobalFree(hData);
		GlobalUnlock(hWaveHdr);
		GlobalFree(hWaveHdr);
		GlobalFree(hWaveInst);
		waveOutClose(hWaveOut);
		m_dwErrorCode = MCIERR_OUT_OF_MEMORY;
		return (FALSE);
	}
	lpWaveInst->hWaveInst = hWaveInst;
	lpWaveInst->hWaveHdr = hWaveHdr;
	lpWaveInst->hWaveData = hData;

	/* Set up WAVEHDR structure and prepare it to be written to wave device.
	 */
	lpWaveHdr->lpData = lpData;
	lpWaveHdr->dwBufferLength = dwDataSize;
	lpWaveHdr->dwFlags = (((m_wFlags & SOUND_LOOP) && (m_wFlags & ~SOUND_NOTIFY)) ? (WHDR_BEGINLOOP | WHDR_ENDLOOP) : 0L);
	lpWaveHdr->dwLoops = (((m_wFlags & SOUND_LOOP) && (m_wFlags & ~SOUND_NOTIFY)) ? 2147483647L : 0L);
	lpWaveHdr->dwUser = m_nID;
	if (waveOutPrepareHeader(hWaveOut, lpWaveHdr, sizeof(WAVEHDR))) {
		GlobalUnlock(hData);
		GlobalFree(hData);
		GlobalUnlock(hWaveHdr);
		GlobalFree(hWaveHdr);
		GlobalUnlock(hWaveInst);
		GlobalFree(hWaveInst);
		waveOutClose(hWaveOut);
		return (FALSE);
	}

	/* Then the data block can be sent to the output device.
	 */

	wResult = waveOutWrite(hWaveOut, lpWaveHdr, sizeof(WAVEHDR));
	if (wResult != 0) {
		waveOutUnprepareHeader(hWaveOut, lpWaveHdr, sizeof(WAVEHDR));
		GlobalUnlock(hData);
		GlobalFree(hData);
		GlobalUnlock(hWaveHdr);
		GlobalFree(hWaveHdr);
		GlobalUnlock(hWaveInst);
		GlobalFree(hWaveInst);
		waveOutClose(hWaveOut);
		m_dwErrorCode = wResult;
		return (FALSE);
	}

	m_lpWaveInst = lpWaveInst;                  // retain pointers to sound structures
	m_lpWaveHdr = lpWaveHdr;
	m_hWaveOut = hWaveOut;

	m_nCount += 1;                              // bump count of active Sounds
	m_bPlaying = TRUE;                          // mark that it's now playing
	m_bPaused = FALSE;
	m_bError = FALSE;

	return (TRUE);
}


/*****************************************************************
 *
 * PlayMCISound
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Initialize, and begin playing, a WAV or MIDI formated Sound
 *      via MCI commands.  For asynch sounds, an event is generated
 *      for the main window when the sound terminates; the callback
 *      routine (OnMCIStopped) then releases the resources used to
 *      play that Sound.
 *
 * FORMAL PARAMETERS:   (BOTH OPTOINAL)
 *
 *      DWORD dwStartOfPlay     Begin Playing Sound at this time (fmt as in parameter #2).
 *      DWORD TimeFlag          Specifies TimeFormat; the following constants are defined:
                                                        FMT_MILLISEC ---> all time parameters will be in milliseconds.
 *
 *      dwStartOfPlay is defualted to 0L.
 *      TimeFlag is defaulted to FMT_MILLISEC.
 *
 * RETURN VALUE:
 *
 *      BOOL            success / failure condition
 *
 ****************************************************************/

BOOL CSound::PlayMCISound(DWORD dwStartOfPlay, DWORD TimeFlag) {
	UINT                wDeviceID;
	DWORD               dwReturn;
	MCI_OPEN_PARMS      mciOpenParams;
	MCI_PLAY_PARMS      mciPlayParams;
	MCI_STATUS_PARMS    mciStatusParams;
	char                chFilePath[256];

	if (m_chDrivePath[0] == '\0')
		Common::strcpy_s(chFilePath, m_pszPathName);
	else {
		Common::strcpy_s(chFilePath, m_chDrivePath);
		if (m_pszPathName[0] == '.')
			strcat(chFilePath, &m_pszPathName[1]);
		else
			strcat(chFilePath, m_pszPathName);
	}

	m_bError = TRUE;                                // assume failure
	m_dwErrorCode = 0;

	m_bPlaying = FALSE;                             // not yet playing
	m_bPaused = FALSE;
	m_chType = SOUND_TYPE_MCI;                      // using MCI commands

	if (m_wFlags & SOUND_WAVE)                      // set for .WAV or .MID
		mciOpenParams.lpstrDeviceType = "waveaudio";
	else
		mciOpenParams.lpstrDeviceType = "sequencer";

	mciOpenParams.lpstrElementName = (LPSTR) chFilePath;
	mciOpenParams.lpstrAlias = NULL;
	mciOpenParams.dwCallback = 0;

	dwReturn = mciSendCommand(0, MCI_OPEN,          // open a sound device
	                          MCI_OPEN_TYPE | MCI_OPEN_ELEMENT,
	                          (DWORD)(LPVOID)&mciOpenParams);
	if (dwReturn != 0) {                            // punt if no device acquired
		m_dwErrorCode = dwReturn;
		return (FALSE);
	}

	if (m_wFlags & SOUND_MIDI)
		PageLockMCISeqBuffers(this);

	wDeviceID = mciOpenParams.wDeviceID;            // save the device

	m_dwPlayStart = dwStartOfPlay;
	mciPlayParams.dwFrom = dwStartOfPlay;           // will play the entire file if StartOfPlay is 0L.
	mciPlayParams.dwTo = m_dwRePlayEnd;             // will play to m_dwRePlayEnd if SOUND_DONT_LOOP_TO_END is specified.

	if (m_wFlags & SOUND_MIDI) {                    // if its .MID check for MIDI Mapper
		mciStatusParams.dwCallback = 0;
		mciStatusParams.dwTrack = 0;
		mciStatusParams.dwItem = MCI_SEQ_STATUS_PORT;
		dwReturn = mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM,
		                          (DWORD)(LPVOID) &mciStatusParams);
		if ((dwReturn != 0) ||                      // problem, so punt
		        (LOWORD(mciStatusParams.dwReturn) != MIDI_MAPPER))
			goto punt;
	}

	dwReturn = SetMCITimeFormat(wDeviceID, TimeFlag);      //set time format.
	if (dwReturn != 0)
		goto punt;                                  //return if error.

	if ((m_wFlags & SOUND_NOTIFY) ||                // play the sound file with notify
	        (m_wFlags & SOUND_LOOP) ||
	        ((m_wFlags & SOUND_ASYNCH) &&
	         (m_wFlags & SOUND_AUTODELETE))) {
		if (m_pWnd == NULL)                 // not valid if no owning window
			goto punt;

		mciPlayParams.dwCallback = MAKELONG((*m_pWnd).m_hWnd, 0);
		dwReturn = mciSendCommand(wDeviceID,
		                          MCI_PLAY,
		                          MCI_NOTIFY | (dwStartOfPlay ? MCI_FROM : 0x00) | ((m_wFlags & SOUND_DONT_LOOP_TO_END) ? MCI_TO : 0x00),
		                          (DWORD)(LPVOID) &mciPlayParams);
		if (dwReturn == 0) {
			m_nCount += 1;                          // bump count of active Sounds
			m_bPlaying = TRUE;                      // mark that it's now playing
			m_wDeviceID = wDeviceID;                // save the device id
		}
	} else {

		mciPlayParams.dwCallback = 0;               // play without notification
		dwReturn = mciSendCommand(wDeviceID,
		                          MCI_PLAY,
		                          (m_wFlags & SOUND_ASYNCH) ? 0 : MCI_WAIT,
		                          (DWORD)(LPVOID) &mciPlayParams);

		if ((dwReturn == 0) &&
		        (m_wFlags & SOUND_AUTODELETE)) {        // release it if marked for delete
			if (m_wFlags & SOUND_MIDI)
				PageUnlockMCISeqBuffers(this);
			mciSendCommand(wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
			m_dwErrorCode = dwReturn;
			delete this;                            // ... and was not an asynch sound
			return (TRUE);
		}
	}

	if (dwReturn != 0) {                            // abort if there is a problem
punt:
		if (m_wFlags & SOUND_MIDI)
			PageUnlockMCISeqBuffers(this);
		mciSendCommand(wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
		m_dwErrorCode = dwReturn;
		return (FALSE);
	}

	m_bError = FALSE;                               // mark for no error occurred

	return (TRUE);
}


/*****************************************************************
 *
 * RePlayMCISound
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Replay a WAV or MIDI formated Sound via MCI commands.  An
 *      event is generated for the main window when the sound ends;
 *      the callback routine (OnMCIStopped) is then responsible for
 *      deciding what to do next.
 *
 * FORMAL PARAMETERS:
 *
 *      DWORD wFlags    flags for MCI_PLAY
 *      DWORD From      Time in the Format set in MidiLoopSegment() to begin replay.
 *      DWORD To        Time in the Format set in MidiLoopSegment() to end replay.
 *
 *      The From and To parameters are defaulted to 0L. They are ignored UNLESS the
 *      the MCI_FROM or the MCI_TO flags, RESPECTIVELY  are specified.
 *
 * RETURN VALUE:
 *
 *      BOOL            success / failure condition
 *
 ****************************************************************/

BOOL CSound::RePlayMCISound(DWORD dwFlag, DWORD From, DWORD To) {
	//From and To are defaulted to 0L.
	DWORD               dwReturn;
	MCI_PLAY_PARMS      mciPlayParams;

	m_bError = TRUE;                                // assume failure
	m_dwErrorCode = 0;
	m_bPlaying = FALSE;                             // not yet playing

	mciPlayParams.dwCallback = MAKELONG((*m_pWnd).m_hWnd, 0);
	mciPlayParams.dwFrom = From;
	mciPlayParams.dwTo = To;

	dwReturn = mciSendCommand(m_wDeviceID, MCI_PLAY, dwFlag,
	                          (DWORD)(LPVOID) &mciPlayParams);

	if (dwReturn != 0) {                                // had a problem
		if (m_wFlags & SOUND_MIDI)
			PageUnlockMCISeqBuffers(this);
		mciSendCommand(m_wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
		m_dwErrorCode = dwReturn;
		return (FALSE);
	}

	m_bError = FALSE;                               // mark for no error occurred
	m_bPlaying = TRUE;                              // now playing again
	return (TRUE);
}


/*************************************************************************
 *
 * MidiLoopPlaySegment()
 *
 * Parameters:      ????
 *
 * Return Value:
 *  BOOL            success / failure condition
 *
 * Description:     play a looping midi segment.
 *
 ************************************************************************/

BOOL CSound::MidiLoopPlaySegment(DWORD dwLoopFrom, DWORD dwLoopTo, DWORD dwBegin, DWORD TimeFmt) {
	BOOL    bSuccess;

	m_wFlags |= SOUND_LOOP;
	m_dwRePlayStart = dwLoopFrom;
	m_dwRePlayEnd = dwLoopTo;
	m_bExtensionsUsed = TRUE;

	bSuccess = Play(dwBegin, TimeFmt);

	return bSuccess;
}


/*************************************************************************
 *
 * PauseSounds()
 *
 * Parameters:      none
 *
 * Return Value:
 *  BOOL            success / failure condition
 *
 * Description:     suspend all playing sounds.
 *
 ************************************************************************/

BOOL CSound::PauseSounds(void) {
	BOOL    bSuccess = TRUE;
	BOOL    bStatus;
	CSound  *pSound;

	pSound = CSound::m_pSoundChain;                 // thumb through all the sounds
	while (pSound != NULL) {
		if (((*pSound).m_bPlaying == TRUE) &&       // if one is playing
		        ((*pSound).m_bPaused == FALSE)) {       // ... and not paused
			bStatus = (*pSound).Pause();            // ... try to suspend it
			if (bStatus == TRUE)
				(*pSound).m_bPaused = TRUE;         // success
			else
				bSuccess = FALSE;                   // failure
		}
		pSound = (*pSound).m_pNext;
	}

	return bSuccess;
}


/*************************************************************************
 *
 * Pause()
 *
 * Parameters:      none
 *
 * Return Value:
 *  BOOL            success / failure condition
 *
 * Description:     suspend an playing sound.
 *
 ************************************************************************/

BOOL CSound::Pause(void) {
	BOOL                bSuccess = FALSE;
	DWORD               dwReturn;
	UINT                nResult;
	MCI_GENERIC_PARMS   mciGenericParams;

	if ((m_bPlaying == TRUE) &&                     // must be playing to be paused
	        (m_bPaused == FALSE)) {                     // ... and not already paused
		switch (m_chType) {

		case SOUND_TYPE_SND:                    // can't pause this type of sound
			bSuccess = FALSE;
			break;

		case SOUND_TYPE_MCI:                    // will either pause or abort
			mciGenericParams.dwCallback = NULL; // ... aborts will be treated as pause
			dwReturn = mciSendCommand(m_wDeviceID, MCI_PAUSE, MCI_WAIT,
			                          (DWORD)(LPVOID) &mciGenericParams);
			m_dwErrorCode = dwReturn;
			bSuccess = ((dwReturn == 0) ? TRUE : FALSE);
			break;

		case SOUND_TYPE_MMIO:                   // pauses should be okay
			nResult = waveOutPause((HWAVEOUT) m_hWaveOut);
			bSuccess = ((nResult == 0) ? TRUE : FALSE);
		}
	}

	if (bSuccess == TRUE)
		m_bPaused = TRUE;

	return bSuccess;
}

/*************************************************************************
 *
 * SetMCITimeFormat(DWORD)
 *
 * Formal Parameters(Optional):
 *      DWORD formatFlag    Specifies Time Format. The following constants are defined:
 *                                                      FMT_MILLISEC----> all time formats are now in milliseconds.
 *
 * Return Value:
 *  DWORD           0L if no error in time setting, else returns the error numeral.
 *
 * Description:     Set Time Format.    Note this call is NOT VALID unless a call to MCI_OPEN is made,
 *                                                          such as in PlayMCISound(), or elsewhere,
 *                                                          and the m_wDeviceID member is valid.
 *
 ************************************************************************/
DWORD CSound::SetMCITimeFormat(UINT wDeviceID, DWORD formatFlag) {
	DWORD           dwReturn;
	MCI_SET_PARMS   mciSetParms;

	mciSetParms.dwCallback = 0;
	mciSetParms.dwAudio = 0;

	switch (formatFlag) {
	case FMT_MILLISEC:
		mciSetParms.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
		break;
	default:
		#ifdef BAGEL_DEBUG
		::MessageBox(NULL, "Invalid MCI time format", "Debug Info", MB_OK);
		#endif
		mciSetParms.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	}

	dwReturn = mciSendCommand(wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT | MCI_WAIT, (DWORD)(LPVOID)&mciSetParms);
	return (dwReturn);
}


/*************************************************************************
 *
 * ResumeSounds()
 *
 * Parameters:      none
 *
 * Return Value:
 *  BOOL            success / failure condition
 *
 * Description:     resume all paused sounds.
 *
 ************************************************************************/

BOOL CSound::ResumeSounds(void) {
	BOOL    bSuccess = TRUE;
	BOOL    bStatus;
	CSound  *pSound;

	pSound = CSound::m_pSoundChain;                 // thumb through all the sounds
	while (pSound != NULL) {
		if ((*pSound).m_bPaused) {                  // if one is paused
			bStatus = (*pSound).Resume();           // ... try to get it going again
			if (bStatus == TRUE)
				(*pSound).m_bPaused = FALSE;        // success
			else
				bSuccess = FALSE;                   // failure
		}
		pSound = (*pSound).m_pNext;
	}

	return bSuccess;
}


/*************************************************************************
 *
 * Resume()
 *
 * Parameters:      none
 *
 * Return Value:
 *  BOOL            success / failure condition
 *
 * Description:     resume playing a sound.
 *
 ************************************************************************/

BOOL CSound::Resume(void) {
	BOOL                bSuccess = FALSE;
	DWORD               dwReturn;
	UINT                nResult;
	MCI_GENERIC_PARMS   mciGenericParams;

	if (m_bPaused == TRUE) {                        // must be paused to resume
		switch (m_chType) {

		case SOUND_TYPE_SND:                    // not valid for this type
			bSuccess = FALSE;                   // ... and neither are pauses
			break;

		case SOUND_TYPE_MCI:                    // resume may not be valid
			mciGenericParams.dwCallback = NULL; // ... in which case will use play
			dwReturn = mciSendCommand(m_wDeviceID, MCI_RESUME, MCI_WAIT,
			                          (DWORD)(LPVOID) &mciGenericParams);
			m_dwErrorCode = dwReturn;
			bSuccess = ((dwReturn == 0) ? TRUE : FALSE);
			if (dwReturn == MCIERR_UNSUPPORTED_FUNCTION)    // not supported, so instead
				bSuccess = RePlayMCISound(MCI_NOTIFY | ((m_wFlags & SOUND_DONT_LOOP_TO_END) ? MCI_TO : 0x00), 0L, m_dwRePlayEnd);   // ... play from current position to end of flle or to m_dwRePlayEnd.
			break;

		case SOUND_TYPE_MMIO:                   // resume should be okay
			nResult = waveOutRestart((HWAVEOUT) m_hWaveOut);
			bSuccess = ((nResult == 0) ? TRUE : FALSE);
		}
	}


	if (bSuccess == TRUE)
		m_bPaused = FALSE;

	return bSuccess;
}


/*************************************************************************
 *
 * StopSounds()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Stop all Sounds in the Sound chain.
 *
 ************************************************************************/

BOOL CSound::StopSounds(void) {
	BOOL    bSuccess = TRUE;
	BOOL    bStatus;
	CSound  *pSound;

	pSound = CSound::m_pSoundChain;                 // thumb through all the sounds
	while (pSound != NULL) {
		if ((*pSound).m_bPlaying) {                 // if one is playing
			(*pSound).m_bPaused = FALSE;            // ... its longer paused
			bStatus = (*pSound).Stop();             // ... try to stop it
			if (bStatus == FALSE)
				bSuccess = FALSE;                   // not the failure
		}
		pSound = (*pSound).m_pNext;
	}

	if (bSuccess == TRUE) {                         // now do general stop calls
		(void) sndPlaySound(NULL, 0);               // ... to catch all other sounds
//        (void) mciSendCommand(MCI_ALL_DEVICE_ID, MCI_CLOSE, MCI_WAIT, NULL);
	}

	return bSuccess;
}


/*************************************************************************
 *
 * StopWaveSounds()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Stop all Wave Sounds that are currently active.
 *
 ************************************************************************/

BOOL CSound::StopWaveSounds(void) {
	BOOL    bSuccess = TRUE;
	CSound  *pSound = NULL,
	         *pNextSound = NULL;

	pSound = CSound::m_pSoundChain;                     // find this Sound is the queue
	while (pSound != NULL) {
		if ((*pSound).m_bPlaying &&
		        ((*pSound).m_wFlags & SOUND_WAVE)) {
			bSuccess = (*pSound).Stop();
			if (bSuccess == TRUE) {
				if ((*pSound).m_wFlags & SOUND_AUTODELETE)
					delete pSound;
			}
		}
		pSound = pNextSound;
	}

	return bSuccess;
}


/*************************************************************************
 *
 * StopMidiSounds()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Stop all Midi Sounds that are currently active.
 *
 ************************************************************************/

BOOL CSound::StopMidiSounds(void) {
	BOOL    bSuccess = TRUE;
	CSound  *pSound = NULL,
	         *pNextSound = NULL;

	pSound = CSound::m_pSoundChain;                     // find this Sound is the queue
	while (pSound != NULL) {
		pNextSound = (*pSound).m_pNext;
		if ((*pSound).m_bPlaying &&
		        ((*pSound).m_wFlags & SOUND_MIDI)) {
			bSuccess = (*pSound).Stop();
			if (bSuccess == TRUE) {
				if ((*pSound).m_wFlags & SOUND_AUTODELETE)
					delete pSound;
			}
		}
		pSound = pNextSound;
	}

	return bSuccess;
}


/*****************************************************************
 *
 * Stop
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      This procedure is called to terminate a sound in progress.
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      TRUE / FALSE
 *
 ****************************************************************/

BOOL CSound::Stop(void) {
	BOOL        bSuccess = FALSE;
	DWORD       dwReturn;
	UINT        nResult;
	LPWAVEINST  lpWaveInst = NULL;

	if (m_bPlaying == TRUE) {                       // only stop if it's playing
		switch (m_chType) {

		case SOUND_TYPE_SND:                    // stop it as an SndPlay sound
			bSuccess = sndPlaySound(NULL, 0);
			break;

		case SOUND_TYPE_MCI:                    // stop it as an MCI sound
			if (m_wFlags & SOUND_MIDI)
				PageUnlockMCISeqBuffers(this);
			dwReturn = mciSendCommand(m_wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
			ASSERT(dwReturn == 0);
			bSuccess = TRUE;
			break;

		case SOUND_TYPE_MMIO:                   // stop it as an MMIO sound
			nResult = waveOutReset((HWAVEOUT) m_hWaveOut);
			ASSERT(nResult == 0);
			lpWaveInst = (LPWAVEINST) m_lpWaveInst;
			nResult = waveOutUnprepareHeader((HWAVEOUT) m_hWaveOut, (LPWAVEHDR) m_lpWaveHdr, sizeof(WAVEHDR));
			ASSERT(nResult == 0);
			GlobalUnlock(lpWaveInst->hWaveData);
			GlobalFree(lpWaveInst->hWaveData);
			GlobalUnlock(lpWaveInst->hWaveHdr);
			GlobalFree(lpWaveInst->hWaveHdr);
			GlobalUnlock(lpWaveInst->hWaveInst);
			GlobalFree(lpWaveInst->hWaveInst);
			nResult = waveOutClose((HWAVEOUT) m_hWaveOut);
			ASSERT(nResult == 0);
			bSuccess = TRUE;
		}

		if (bSuccess == TRUE) {                     // no longer playing
			m_bPlaying = FALSE;
			m_bPaused = FALSE;
			m_nCount -= 1;
		}
	}

	m_bError = !bSuccess;
	if (!m_bError)
		m_dwErrorCode = 0;
	return bSuccess;
}


/*****************************************************************
 *
 * OnMCIStopped
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      This procedure is called when a termination message is received;
 *      i.e. an MCI sound has finished playing.  If it is a looping sound,
 *      then an attempt is made to play it again.  Otherwise, cleanup is done,
 *      which simply involves closing the sound file and deleting the resource.
 *
 * FORMAL PARAMETERS:
 *
 *      WPARAM      reason for the termination
 *      LPARAM      the identifier of the Sound that has stopped
 *
 * RETURN VALUE:
 *
 *      always returns TRUE to signify that we processed the message
 *
 ****************************************************************/

CSound *CSound::OnMCIStopped(WPARAM wParam, LPARAM lParam) {
	BOOL    bSuccess = FALSE;                               // success/failure status
	DWORD   dwReturn;
	CSound  *pSound;

	pSound = CSound::m_pSoundChain;                     // find this Sound is the queue
	while (pSound != NULL) {
		if ((*pSound).m_wDeviceID == (UINT) lParam)
			break;
		pSound = (*pSound).m_pNext;
	}

	if (pSound != NULL) {
		if (wParam == MCI_NOTIFY_FAILURE)               // note if terminated by error
			(*pSound).m_bError = TRUE;
		else if ((wParam == MCI_NOTIFY_ABORTED) &&          // punt if just "paused"
		         ((*pSound).m_bPaused == TRUE))
			return (NULL);
		if ((*pSound).m_bPlaying == TRUE) {             // verify it's actually playing
			if (!(*pSound).m_bError &&
			        (*pSound).m_wFlags & SOUND_LOOP) {      // if looping is specified
				if (pSound->m_wFlags & SOUND_DONT_LOOP_TO_END)
					bSuccess = (*pSound).RePlayMCISound(MCI_NOTIFY | MCI_FROM | MCI_TO, (*pSound).m_bExtensionsUsed ? (*pSound).m_dwRePlayStart : (*pSound).m_dwPlayStart, (*pSound).m_dwRePlayEnd);
				else
					bSuccess = (*pSound).RePlayMCISound(MCI_NOTIFY | MCI_FROM, (*pSound).m_bExtensionsUsed ? (*pSound).m_dwRePlayStart : (*pSound).m_dwPlayStart, (*pSound).m_dwRePlayEnd);
				if (bSuccess)
					return (NULL);
			}                                           // time to close the sound file
			(*pSound).m_bPlaying = FALSE;               // mark sound as no longer playing
			(*pSound).m_bPaused = FALSE;
			if (pSound->m_wFlags & SOUND_MIDI)
				PageUnlockMCISeqBuffers(pSound);
			dwReturn = mciSendCommand(LOWORD(lParam), MCI_CLOSE, MCI_WAIT, NULL);
			ASSERT(dwReturn == 0);
			m_nCount -= 1;                              // decrement active sound count
		} else {
			(*pSound).m_bError = FALSE;
			(*pSound).m_dwErrorCode = 0;
		}
		if ((*pSound).m_wFlags & SOUND_AUTODELETE) {    // scrag it if marked for deletion
			delete pSound;
			pSound = NULL;
		} else if (!((*pSound).m_wFlags & SOUND_NOTIFY))
			pSound = NULL;
	}

	return (pSound);                                    // return sound pointer on notify
}


/*****************************************************************
 *
 * OnMMIOStopped
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      This procedure is called when a termination message is received;
 *      i.e. an MMIO sound has finished playing.  If it is a looping sound,
 *      then an attempt is made to play it again.  Otherwise, cleanup is done,
 *      which simply involves closing the sound file and deleting the resource.
 *
 * FORMAL PARAMETERS:
 *
 *      WPARAM      pointer to WAVEOUT information
 *      LPARAM      pointer to WAVEHDR information
 *
 * RETURN VALUE:
 *
 *      always returns TRUE to signify that we processed the message
 *
 ****************************************************************/

CSound *CSound::OnMMIOStopped(WPARAM wParam, LPARAM lParam) {
	BOOL        bSuccess = FALSE;                           // success/failure status
	LPWAVEINST  lpWaveInst = NULL;
	CSound      *pSound = NULL;
	UINT        nResult;

	pSound = CSound::m_pSoundChain;                     // find this Sound is the queue
	while (pSound != NULL) {
		if ((*pSound).m_lpWaveHdr == (LPWAVEHDR)lParam)
			if ((*pSound).m_nID == (int)((LPWAVEHDR)lParam)->dwUser)
				break;
		pSound = (*pSound).m_pNext;
	}

	if (pSound != NULL) {                               // release the data structures
		if ((*pSound).Playing()) {

			(*pSound).m_bPlaying = FALSE;                   // mark sound as no longer playing
			(*pSound).m_bPaused = FALSE;

			nResult = waveOutUnprepareHeader((HWAVEOUT) wParam, (LPWAVEHDR) lParam, sizeof(WAVEHDR));
			ASSERT(nResult == 0);
			lpWaveInst = (*pSound).m_lpWaveInst;
			GlobalUnlock(lpWaveInst->hWaveData);
			GlobalFree(lpWaveInst->hWaveData);
			GlobalUnlock(lpWaveInst->hWaveHdr);
			GlobalFree(lpWaveInst->hWaveHdr);
			GlobalUnlock(lpWaveInst->hWaveInst);
			GlobalFree(lpWaveInst->hWaveInst);
			nResult = waveOutClose((HWAVEOUT) wParam);
			ASSERT(nResult == 0);

			m_nCount -= 1;                                  // decrement active sound count

			(*pSound).m_bError = FALSE;
			(*pSound).m_dwErrorCode = 0;

			if ((*pSound).m_wFlags & SOUND_LOOP) {          // if looping is specified
				bSuccess = (*pSound).Play();
				if (bSuccess == FALSE)
					(*pSound).m_wFlags ^= SOUND_LOOP;
			}
		}

		if ((*pSound).m_wFlags & SOUND_AUTODELETE) {    // scrag it if marked for deletion
			delete pSound;
			pSound = NULL;
		} else if (!((*pSound).m_wFlags & SOUND_NOTIFY))    // return sound pointer on notify
			pSound = NULL;
	}

	return (pSound);
}


/*************************************************************************
 *
 * ClearSounds()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Clear all Sounds from the Sound chain, stopping
 *                  play of any that are currently active.
 *
 ************************************************************************/

void CSound::ClearSounds(void) {
	StopSounds();                                   // stop all active sounds

	while (m_pSoundChain != NULL) {                 // delete all sound entries
		delete m_pSoundChain;
	}
}


/*************************************************************************
 *
 * ClearWaveSounds()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Clear all Wave Sounds from the Sound chain, stopping
 *                  play of any that are currently active.
 *
 ************************************************************************/

void CSound::ClearWaveSounds(void) {
	CSound  *pSound = NULL,
	         *pNextSound = NULL;

	pSound = CSound::m_pSoundChain;                     // find this Sound is the queue
	while (pSound != NULL) {
		pNextSound = (*pSound).m_pNext;
		if ((*pSound).m_wFlags & SOUND_WAVE)
			delete pSound;
		pSound = pNextSound;
	}
}


/*************************************************************************
 *
 * ClearMidiSounds()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Clear all Midi Sounds from the Sound chain, stopping
 *                  play of any that are currently active.
 *
 ************************************************************************/

void CSound::ClearMidiSounds(void) {
	CSound  *pSound = NULL,
	         *pNextSound = NULL;

	pSound = CSound::m_pSoundChain;                     // find this Sound is the queue
	while (pSound != NULL) {
		pNextSound = (*pSound).m_pNext;
		if ((*pSound).m_wFlags & SOUND_MIDI)
			delete pSound;
		pSound = pNextSound;
	}
}


/*************************************************************************
 *
 * SoundAvailable()
 *
 * Parameters:      none
 *
 * Return Value:    TRUE / FALSE
 *
 * Description:     Determines whether .WAV sound output is available.
 *
 ************************************************************************/

BOOL CSound::SoundAvailable(void) {
	if (m_bInited == FALSE) {                       // do sound initialization if needed
		InitializeSound(m_pMainWnd);
		m_bInited = TRUE;
	}

	return (m_bSoundAvailable);                     // return requested info
}


/*************************************************************************
 *
 * MidiAvailable()
 *
 * Parameters:      none
 *
 * Return Value:    TRUE / FALSE
 *
 * Description:     Determines whether .MID sound output is available.
 *
 ************************************************************************/

BOOL CSound::MidiAvailable(void) {
	if (m_bInited == FALSE) {                       // do sound initialization if needed
		InitializeSound(m_pMainWnd);
		m_bInited = TRUE;
	}

	return (m_bMidiAvailable);                      // return requested info
}


/*************************************************************************
 *
 * WaitSounds()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     wait until all sounds have finished playing, allowing
 *                  message traffic to take place.
 *
 ************************************************************************/

void CSound::WaitSounds(void) {
	WaitWaveSounds();
	WaitMidiSounds();
}


/*************************************************************************
 *
 * WaitWaveSounds()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     wait until all wave sounds have finished playing, allowing
 *                  message traffic to take place.
 *
 ************************************************************************/

void CSound::WaitWaveSounds(void) {
	DWORD   dwTickCount = 0;
	CSound  *pSound;
	BOOL    bWaiting = FALSE;

	while (TRUE) {
		if (HandleMessages())
			break;
		pSound = m_pSoundChain;
		while (pSound != NULL) {
			if ((*pSound).m_bPlaying &&
			        ((*pSound).m_wFlags & SOUND_WAVE)) {
				#if BUILD_FOR_META_DLL
				if (!bWaiting) {
					bWaiting = TRUE;
					AfxGetApp()->DoWaitCursor(1);
				}
				#endif
				break;
			}
			pSound = (*pSound).m_pNext;
		}
		if (pSound == NULL)
			break;
		if (dwTickCount == 0)
			dwTickCount = ::GetTickCount() + 60000L;
		if (::GetTickCount() > dwTickCount) {
			#ifdef BAGEL_DEBUG
			::MessageBox(NULL, "Sound messages may have been lost...", "Possible Internal Problem", MB_ICONEXCLAMATION);
			#endif
			(*pSound).Stop();
			(*pSound).m_bPlaying = FALSE;
			(*pSound).m_bError = TRUE;
		}
	}

	#if BUILD_FOR_META_DLL
	if (bWaiting)
		AfxGetApp()->DoWaitCursor(-1);
	#endif
}


/*************************************************************************
 *
 * WaitMidiSounds()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     wait until all midi sounds have finished playing, allowing
 *                  message traffic to take place.
 *
 ************************************************************************/

void CSound::WaitMidiSounds(void) {
	DWORD   dwTickCount = 0;
	CSound  *pSound;
	BOOL    bWaiting = FALSE;

	while (TRUE) {
		if (HandleMessages())
			break;
		pSound = m_pSoundChain;
		while (pSound != NULL) {
			if ((*pSound).m_bPlaying &&
			        ((*pSound).m_wFlags & SOUND_MIDI)) {
				#if BUILD_FOR_META_DLL
				if (!bWaiting) {
					bWaiting = TRUE;
					AfxGetApp()->DoWaitCursor(1);
				}
				#endif
				break;
			}
			pSound = (*pSound).m_pNext;
		}
		if (pSound == NULL)
			break;
		if (dwTickCount == 0)
			dwTickCount = ::GetTickCount() + 60000L;
		if (::GetTickCount() > dwTickCount) {
			#ifdef BAGEL_DEBUG
			::MessageBox(NULL, "Sound messages may have been lost...", "Possible Internal Problem", MB_ICONEXCLAMATION);
			#endif
			(*pSound).Stop();
			(*pSound).m_bPlaying = FALSE;
			(*pSound).m_bError = TRUE;
		}
	}

	#if BUILD_FOR_META_DLL
	if (bWaiting)
		AfxGetApp()->DoWaitCursor(-1);
	#endif
}


/*************************************************************************
 *
 * HandleMessages()
 *
 * Parameters:      none
 *
 * Return Value:
 *  BOOL            whether an urgent message is pending
 *
 * Description:     process pending non-keyboard/mouse messages.
 *
 ************************************************************************/

BOOL CSound::HandleMessages(void) {
	MSG     msg;

	if (PeekMessage(&msg, NULL, 0, WM_KEYFIRST - 1, PM_REMOVE)) {
		if (msg.message == WM_CLOSE || msg.message == WM_QUIT)
			return (TRUE);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (PeekMessage(&msg, NULL, WM_KEYLAST + 1, WM_MOUSEMOVE, PM_REMOVE)) {
		if (msg.message == WM_CLOSE || msg.message == WM_QUIT)
			return (TRUE);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (PeekMessage(&msg, NULL, WM_PARENTNOTIFY, 0xFFFF, PM_REMOVE)) {
		if (msg.message == WM_CLOSE || msg.message == WM_QUIT)
			return (TRUE);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (FALSE);
}


#if !BUILD_FOR_META_DLL
typedef DWORD (FAR PASCAL * FPGETFREEMEMINFO)(void);
FPGETFREEMEMINFO    lpfnGetFreeMemInfo;

DWORD GetPhysicalMemory(void) {
	DWORD   dwInfo;
	WORD    wFreePages;
	DWORD   lFreeBytes = 0;

	lpfnGetFreeMemInfo = (FPGETFREEMEMINFO)GetProcAddress(GetModuleHandle("KERNEL"), "GETFREEMEMINFO");
	if (lpfnGetFreeMemInfo != NULL) {
		dwInfo = lpfnGetFreeMemInfo();
		if (dwInfo != -1L) {
			wFreePages = HIWORD(dwInfo);
			lFreeBytes = (DWORD) wFreePages * 4096L;
		}
	}

	if (lFreeBytes == 0)
		lFreeBytes = GetFreeSpace(0);

	return (lFreeBytes);
}
#endif


void PageLockMCISeqBuffers(CSound *pSound) {
	GLOBALENTRY ge;                                 // where we put object info

	TRACE("PageLocking...\n");

	(*pSound).m_nMCIHandleCount = 0;            // no initial handles
	(*pSound).m_hMCIModule = ::GetModuleHandle("MCISEQ");
	if ((*pSound).m_hMCIModule == NULL)         // get the driver handle or punt
		return;

	TRACE("...got driver...\n");

loop:
	ge.dwSize = sizeof(GLOBALENTRY);            // initialize the data structure
	::GlobalFirst(&ge, GLOBAL_ALL);               // get first object in heap

	TRACE("...got first...\n");

	if (MaybePageLockObject(pSound, &ge))           // lock it if its a midi buffer
		goto loop;

	while (::GlobalNext(&ge, GLOBAL_ALL)) {       // walk remainder of heap, locking
		if (MaybePageLockObject(pSound, &ge))       // ... midi buffers
			goto loop;
	}
	TRACE("PageLocked\n");
}


BOOL MaybePageLockObject(CSound *pSound, LPGLOBALENTRY lpge) {
	if ((lpge->hOwner == (*pSound).m_hMCIModule) && // see if the object is belongs
	        (lpge->dwBlockSize >= OBJECTSIZE) &&        // ... to mciseq and the size we expect
	        (lpge->wType != GT_CODE) &&                 // ... and is not yet locked
	        !lpge->wcPageLock) {
		if (::GlobalPageLock(lpge->hBlock)) {           // found one, so lock it and retain it
			TRACE("...lock...\n");
			if ((*pSound).m_nMCIHandleCount == 32)
				::MessageBox(NULL, "Global Lock Overflow", "Debug Info", MB_OK);
			else
				(*pSound).m_hMCIMemory[(*pSound).m_nMCIHandleCount++] = lpge->hBlock;
			return (TRUE);
		} else
			::MessageBox(NULL, "Global Lock Failed", "Debug Info", MB_OK);
	}

	return (FALSE);
}


void PageUnlockMCISeqBuffers(CSound *pSound) {
	GLOBALENTRY ge;
	int         i;

	TRACE("PageUnLocking...\n");

	ge.dwSize = sizeof(GLOBALENTRY);                // init the data structure

	for (i = 0; i < (*pSound).m_nMCIHandleCount; i++) { // thumb through list of saved memory handles
		::GlobalFirst(&ge, GLOBAL_ALL);                   // get first object in heap
		if (MaybePageUnlock(&ge, (*pSound).m_hMCIMemory[i])) // if we find a handle on the heap
			continue;                                       // ... then will unlock it
		while (::GlobalNext(&ge, GLOBAL_ALL)) {
			if (MaybePageUnlock(&ge, (*pSound).m_hMCIMemory[i]))
				break;
		}
	}

	(*pSound).m_nMCIHandleCount = 0;                // clear the handle count
	TRACE("PageUnLocked\n");
}


BOOL MaybePageUnlock(LPGLOBALENTRY lpge, HGLOBAL hObject) {
	if (lpge->hBlock == hObject) {                  // look for an object match
		if (lpge->wcPageLock)                       // if its lock, then unlock it
			::GlobalPageUnlock(hObject);
		else
			::MessageBox(NULL, "Tried to unlock a non-locked object", "Debug Info", MB_OK);
		return (TRUE);
	}

	return (FALSE);
}






/////////////////////////////////////////////////////////////////////////////
// CSound diagnostics

#ifdef BAGEL_DEBUG
void CSound::AssertValid() const {
	CObject::AssertValid();
}

void CSound::Dump(CDumpContext& dc) const {
	CObject::Dump(dc);
}

#endif //BAGEL_DEBUG

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
