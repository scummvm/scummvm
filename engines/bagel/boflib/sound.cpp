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

#include "common/system.h"
#include "common/file.h"

#include "bagel/boflib/app.h"
#include "bagel/boflib/misc.h"

#include "bagel/boflib/sound.h"

namespace Bagel {

#define MEMORY_THRESHOLD    20000L
#define MEMORY_MARGIN       100000L

CHAR    CBofSound::m_szDrivePath[MAX_DIRPATH];
CBofSound  *CBofSound::m_pSoundChain = nullptr;  // pointer to chain of linked Sounds
INT     CBofSound::m_nCount = 0;           // count of currently active Sounds
INT     CBofSound::m_nWavCount = 0;        // available wave sound devices
INT     CBofSound::m_nMidiCount = 0;       // available midi sound devices
BOOL    CBofSound::m_bSoundAvailable = FALSE;  // whether wave sound is available
BOOL    CBofSound::m_bMidiAvailable = FALSE;   // whether midi sound is available
BOOL    CBofSound::m_bWaveVolume = FALSE;  // whether wave volume can be set
BOOL    CBofSound::m_bMidiVolume = FALSE;  // whether midi volume can be set
CBofWindow   *CBofSound::m_pMainWnd = nullptr;         // window for message processing

BOOL    CBofSound::m_bInit = FALSE;


// Mac stuff
#if BOF_MAC || BOF_WINMAC

#if PLAYWAVONMAC
#include "WAVtoSND.h"
#include "DBFF.h"
#endif
INT     CBofSound::m_nMacSndLev = 0;           // unique Sound indentifier
INT     CBofSound::m_nMacMidiLev = 0;           // unique Sound indentifier
SndChannelPtr CBofSound::m_pSndChan[MAX_CHANNELS];
static SndCallBackUPP           gSndCallBackUPP;
static FilePlayCompletionUPP    gFilePlayCompletionUPP;
#else // PC
CQueue m_cQueue[NUM_QUEUES];
INT m_nSlotVol[NUM_QUEUES];
#endif


//
// this class is designed to help with global initializations
//
class CSoundStartup {
public:
	CSoundStartup();
	~CSoundStartup();
};

CSoundStartup::CSoundStartup() {
	CBofSound::Initialize();
}


CSoundStartup::~CSoundStartup() {
	CBofSound::UnInitialize();
}

static CSoundStartup g_sStartup;


CBofSound::CBofSound(CBofWindow *pWnd, const CHAR *pszPathName, WORD wFlags, const INT nLoops) {
	CHAR szTempPath[MAX_DIRPATH];

	// validate input
	//
	Assert(pszPathName != nullptr);
	Assert(strlen(pszPathName) < MAX_FNAME);

#if BOF_MAC || BOF_WINMAC
	// strip out the QUEUE flag for Macintosh
	// RMS no longer needed: wFlags &= ~SOUND_QUEUE;
	m_pMacSndChan = nullptr;               // ptr to sound channel allocated by this object
	m_hMacSndRes = nullptr;                // ptr to sound resource
	m_pMacMidi = nullptr;                  // ptr to Midi QT movie object
	m_resRefNum = 0;                    // refernce number for resource file
	m_bTempChannel = FALSE;

#if PLAYWAVONMAC
	m_pSoundInfo = nullptr;
#endif
#endif

	//
	// initialize data fields
	//

	m_pWnd = m_pMainWnd;
	if (pWnd != nullptr) {
		m_pWnd = pWnd;
		if (m_pMainWnd == nullptr)
			m_pMainWnd = pWnd;
	}

	m_wLoops = (WORD)nLoops;

	m_bPlaying = FALSE;                 // not yet playing
	m_bStarted = FALSE;
	m_wFlags = wFlags;                  // flags for playing
	m_bPaused = FALSE;                  // not suspended
	m_bExtensionsUsed = FALSE;          // no extended flags used.
	m_szFileName[0] = '\0';

#if BOF_WINDOWS
	m_hSample = nullptr;
	m_hSequence = nullptr;
	m_pFileBuf = nullptr;
	m_nVol = VOLUME_INDEX_DEFAULT;
	m_bInQueue = FALSE;
	m_iQSlot = 0;

	INT i;
	for (i = 0; i < NUM_QUEUES; i++) {
		m_nSlotVol[i] = VOLUME_INDEX_DEFAULT;
	}

#endif

	// Mixed assumes Asynchronous
	//
	if ((m_wFlags & SOUND_MIX) == SOUND_MIX) {
		m_wFlags |= SOUND_ASYNCH;
	}

	if (pszPathName != nullptr) {

#if BOF_MAC || BOF_WINMAC
		// if our number of loops is zero, then this means keep playing it man...
		if (nLoops == 0) {
			m_wFlags |= SOUND_LOOP;
		}

		if (m_szDrivePath[0] != '\0') {
			sprintf(szTempPath, "%s:%s", m_szDrivePath, pszPathName);
		} else {
			strcpy(szTempPath, pszPathName);
		}
		StrReplaceStr(szTempPath, ":::", ":");
		StrReplaceStr(szTempPath, "::", ":");

		// unfortunately, all the scripts are written in PC language, thus
		// our .WLD files will have the sound file specs with .WAV extensions, we don't
		// want that here.
		//
		// might have a midi file...

		// added conditional code to play ".wav" files directly

#if !PLAYWAVONMAC
		StrReplaceStr(szTempPath, ".WAV", ".SND");
#endif
#if USEQUICKTIME
		StrReplaceStr(szTempPath, ".MID", ".MOV");
#endif
#if USESOUNDMUSICSYS
		StrReplaceStr(szTempPath, ".MID", ".SMS");
#endif
#else
		if ((m_szDrivePath[0] != '\0') && (*pszPathName == '.'))
			pszPathName++;

		snprintf(szTempPath, MAX_DIRPATH, "%s%s", m_szDrivePath, pszPathName);
		StrReplaceStr(szTempPath, "\\\\", "\\");
#endif

		// continue as long as this file exists
		//
		if (FileExists(szTempPath)) {

			FileGetFullPath(m_szFileName, szTempPath);

#if BOF_WINDOWS
			if (!(m_wFlags & SOUND_QUEUE)) {
#endif

				if (m_wFlags & SOUND_WAVE || m_wFlags & SOUND_MIX) {
					LoadSound();
				}
#if BOF_WINDOWS
			}
#endif

		} else {
			ReportError(ERR_FFIND, szTempPath);
		}
	}

	// insert this sound into the sound list
	//
	if (m_pSoundChain != nullptr) {
		m_pSoundChain->Insert(this);

		// m_pSoundchain must always be the head of the list
		Assert(m_pSoundChain == m_pSoundChain->GetHead());
	} else {
		m_pSoundChain = this;
	}
}


CBofSound::~CBofSound() {
	Assert(IsValidObject(this));

	Stop();

#if BOF_MAC || BOF_WINMAC  // Mac cleanup

	// release sound channel
	ReleaseMacSndChan();

	// release any sound resources still in memory
	ReleaseSound();

	// Close open res file
	if (m_resRefNum != 0) {
		CloseResFile(m_resRefNum);
		m_resRefNum = 0;
	}

	// close the QuickTime midi stuff
	//
	if (m_pMacMidi != nullptr) {
		delete m_pMacMidi;
		m_pMacMidi = nullptr;
	}

#else // PC

	ReleaseSound();

#endif

	if (this == m_pSoundChain) {              // special case head of chain

		m_pSoundChain = (CBofSound *)m_pSoundChain->GetNext();
	}
}


VOID CBofSound::Initialize() {
#if BOF_MAC || BOF_WINMAC
	OSErr sndErr = 0;
	INT i;

	// assume sound Manager 3.0 available and QT available
	//
	m_bSoundAvailable = TRUE;
	m_bMidiAvailable = TRUE;

	// assume failure
	m_bInit = FALSE;

	//
	// pre-allocate some sound channels
	//
	// for the PPC, make sure that we use Universal proc ptrs to
	// our callbacks
	// change to initialize static UPPs

	gSndCallBackUPP = NewSndCallBackProc(OnMacSndCallback);
	gFilePlayCompletionUPP = NewFilePlayCompletionProc(OnMacFileCallback);

	for (i = 0; i < MAX_CHANNELS; i++) {

		if (m_pSndChan[i] == nullptr) {

			// allocate a sound channel with associate completion callback
			//
			if ((sndErr = SndNewChannel(&m_pSndChan[i], sampledSynth, initMono, gSndCallBackUPP)) != 0) {
				LogError(BuildString("SndNewChannel failed: sndErr = %d, %d", sndErr, i));
				break;
			}
		}
	}

	// if we allocated any
	//
	if (i > 0) {
		m_bInit = TRUE;
	}

	// if error occured then report it
	//
	if (sndErr) {
		LogError(BuildString("CBofSound::Initialize() error: Unable to pre-allocate more than %d sound channels", i));
	}

#else
	m_bSoundAvailable = TRUE;
	m_bMidiAvailable = FALSE;

	//ResetQVolumes();

#endif
}

#if BOF_WINDOWS
VOID CBofSound::ResetQVolumes() {
	INT i;

	// Set Q Volumes to default
	//
	for (i = 0; i < NUM_QUEUES; i++) {
		m_nSlotVol[i] = VOLUME_INDEX_DEFAULT;
	}
}
#else
void CBofSound::ResetQVolumes() {
}
#endif


VOID CBofSound::UnInitialize() {
#if BOF_MAC || BOF_WINMAC
	OSErr sndErr;
	const int MAXTRIES = 64;
	INT i = MAXTRIES;

	// make sure all sounds have been deleted
	if (m_pSoundChain) {
		AudioTask();
		ClearSounds();
	}

	for (i = 0; i < MAX_CHANNELS; i++) {

		if (m_pSndChan[i] != nullptr) {
			sndErr = SndDisposeChannel(m_pSndChan[i], kQuietNow);
			m_pSndChan[i] = nullptr;
		}
	}
#else
	// Auto-delete any remaining sounds
	ClearSounds();
#endif
}



VOID CBofSound::SetVolume(INT nVolume) {
	Assert(nVolume >= VOLUME_INDEX_MIN && nVolume <= VOLUME_INDEX_MAX);
	INT nLocalVolume = nVolume;

	if (nLocalVolume < VOLUME_INDEX_MIN) {
		nLocalVolume = VOLUME_INDEX_MIN;

	} else if (nLocalVolume > VOLUME_INDEX_MAX) {
		nLocalVolume = VOLUME_INDEX_MAX;
	}

	m_nVol = nLocalVolume;

	warning("STUB: CBofSound::SetVolume(INT nVolume)");

#if 0
	if (m_hSample != nullptr) {
		AIL_set_sample_volume(m_hSample, m_nVol * 10);
	} else if (m_hSequence != nullptr) {
		AIL_set_sequence_volume(m_hSequence, m_nVol * 10, 0);
	}
#endif
}


VOID CBofSound::SetVolume(INT nMidiVolume, INT nWaveVolume) {
	Assert(nMidiVolume >= VOLUME_INDEX_MIN && nMidiVolume <= VOLUME_INDEX_MAX);
	Assert(nWaveVolume >= VOLUME_INDEX_MIN && nWaveVolume <= VOLUME_INDEX_MAX);

	if (nWaveVolume < VOLUME_INDEX_MIN) {
		nWaveVolume = VOLUME_INDEX_MIN;

	} else if (nWaveVolume > VOLUME_INDEX_MAX) {
		nWaveVolume = VOLUME_INDEX_MAX;
	}

	warning("STUB: CBofSound::SetVolume(INT nMidiVolume, INT nWaveVolume)");

#if 0
	MacQT::SetMacMasterVolume(nWaveVolume);
#endif

	if (nMidiVolume < VOLUME_INDEX_MIN) {
		nMidiVolume = VOLUME_INDEX_MIN;

	} else if (nMidiVolume > VOLUME_INDEX_MAX) {
		nMidiVolume = VOLUME_INDEX_MAX;
	}

	// Set master Midi volume
	//
#if 0
	MacQT::SetMacMasterMidiVolume(nMidiVolume);
#endif
}


BOOL CBofSound::Play(DWORD dwBeginHere, DWORD TimeFormatFlag) {
	Assert(IsValidObject(this));

	BOOL    bSuccess;

	// assume failure
	bSuccess = FALSE;

	if (m_errCode == ERR_NONE) {

		// we must be attached to a valid window
		Assert((m_pWnd != nullptr) || (m_pMainWnd != nullptr));

		// if already playing, then stop and start again
		//
		if (Playing()) {

			// can't replay an autodelete sound
			Assert(!(m_wFlags & SOUND_AUTODELETE));

			Stop();
		}

		// WAVE and MIX are mutually exclusive
		Assert(!((m_wFlags & SOUND_WAVE) && (m_wFlags & SOUND_MIX)));

		// Strip off any flags that conflict with MIX
		//
		/*if ((m_wFlags & SOUND_MIX) != 0) {
		    m_wFlags &= ~SOUND_MIX;
		}*/

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

		// make sure this sound is still valid
		Assert(m_pSoundChain != nullptr);

#if BOF_MAC || BOF_WINMAC // MAC VERSION

		// Mac treats wave and mix as equal snd's

		// If Wave or Mix see if we can play it
		//
		if ((m_wFlags & SOUND_WAVE) || (m_wFlags & SOUND_MIX)) {

			if (!m_bSoundAvailable)
				return FALSE;

			// first try play it loaded in memory
			//
			if (!(m_wFlags & SOUND_BUFFERED)) {

				// If not pre-loaded
				//
				bSuccess = TRUE;
				if (m_hMacSndRes == nullptr) {

					// try loading it now
					bSuccess = LoadSound();
				}

				if (bSuccess) {
					return PlayMacSnd();
				}
			}

			// try to play it buffered from file
			//
			return PlayMacSndFile();

			// if it is a MIDI file
			//
		} else if (m_wFlags & SOUND_MIDI) {

			if (!m_bMidiAvailable)                  // ... see if we can play it
				return FALSE;

			// play a MIDI file
			//
			return PlayMacMidi();
		}

#else  // PC  VERSION

		warning("STUB: CBofSound::Play()");

#if 0
		if (m_pFileBuf == nullptr) {

			if ((m_wFlags & (SOUND_MIX | SOUND_QUEUE)) == (SOUND_MIX | SOUND_QUEUE)) {

				// Don't pre-load it

			} else {
				LoadSound();
			}
		}
		//if (m_pFileBuf != nullptr) {

		if (m_wFlags & SOUND_MIDI) {

			HMDIDRIVER hMidiDriver;

			if ((hMidiDriver = CBofApp::GetApp()->GetMidiDriver()) != nullptr) {

				if ((m_hSequence = AIL_allocate_sequence_handle(hMidiDriver)) != nullptr) {
					INT nError;

					nError = AIL_init_sequence(m_hSequence, m_pFileBuf, 0);
					AIL_set_sequence_volume(m_hSequence, m_nVol * 10, 0);
					AIL_set_sequence_loop_count(m_hSequence, m_wLoops);
					AIL_start_sequence(m_hSequence);
					m_bPlaying = TRUE;

				} else {
					ReportError(ERR_UNKNOWN, "Could not allocate an HSEQUENCE. (%s)", AIL_last_error());
				}
			}

		} else if (m_wFlags & SOUND_WAVE) {

			PlayMSS();

			if (m_hSample != nullptr) {

				if (!(m_wFlags & SOUND_ASYNCH)) {

					while (AIL_sample_status(m_hSample) == SMP_PLAYING)
						;

					AIL_release_sample_handle(m_hSample);
					m_hSample = nullptr;
					m_bPlaying = FALSE;
				}
			}

		} else if (m_wFlags & SOUND_MIX) {

			if (!(m_wFlags & SOUND_QUEUE)) {

				PlayMSS();

			} else {
				Assert(m_iQSlot >= 0 && m_iQSlot < NUM_QUEUES);

				m_cQueue[m_iQSlot].AddItem(this);
				m_bPlaying = TRUE;
				m_bInQueue = TRUE;
				SetVolume(m_nSlotVol[m_iQSlot]);
			}
		}
		//}
#endif

#endif // MAC/PC

	}

	return bSuccess;
}


BOOL CBofSound::MidiLoopPlaySegment(DWORD dwLoopFrom, DWORD dwLoopTo, DWORD dwBegin, DWORD TimeFmt) {
	Assert(IsValidObject(this));

	BOOL    bSuccess;

	m_wFlags |= SOUND_LOOP;
	m_dwRePlayStart = dwLoopFrom;
	m_dwRePlayEnd = dwLoopTo;
	m_bExtensionsUsed = TRUE;

	bSuccess = Play(dwBegin, TimeFmt);

	return bSuccess;
}


BOOL CBofSound::PauseSounds() {
	BOOL    bSuccess = TRUE;
	BOOL    bStatus;
	CBofSound  *pSound;

	// thumb through all the sounds
	//
	pSound = m_pSoundChain;
	while (pSound != nullptr) {

		// if one is playing and not paused try to suspend it
		if (pSound->Playing() && (pSound->m_bPaused == FALSE)) {
			bStatus = pSound->Pause();
			if (bStatus)
				pSound->m_bPaused = TRUE;
			else
				bSuccess = FALSE;
		}
		pSound = (CBofSound *)pSound->GetNext();
	}

	return bSuccess;
}


BOOL CBofSound::Pause() {
	Assert(IsValidObject(this));

	BOOL bSuccess = FALSE;

	// must be playing to be paused and not already paused
	//
	if (Playing() && (m_bPaused == FALSE)) {

#if BOF_MAC || BOF_WINMAC
		switch (m_chType) {


		case SOUND_TYPE_MAC_SND:
			bSuccess = PauseMacSnd();
			break;

		case SOUND_TYPE_MAC_FILE:
			bSuccess = PauseMacFile();
			break;

		case SOUND_TYPE_MAC_MIDI:
			bSuccess = PauseMacMidi();
			break;
		}

#else //PC

		warning("STUB: CBofSound::Pause()");

#if 0
		if (m_hSample != nullptr) {
			bSuccess = TRUE;
			AIL_stop_sample(m_hSample);

		} else if (m_hSequence != nullptr) {
			bSuccess = TRUE;
			AIL_stop_sequence(m_hSequence);
		}
#endif

#endif

	}

	if (bSuccess)
		m_bPaused = TRUE;

	return bSuccess;
}


BOOL CBofSound::ResumeSounds() {
	BOOL    bSuccess = TRUE;
	BOOL    bStatus;
	CBofSound  *pSound;

	pSound = m_pSoundChain;                 // thumb through all the sounds
	while (pSound != nullptr) {
		if (pSound->m_bPaused) {                  // if one is paused
			bStatus = pSound->Resume();           // ... try to get it going again
			if (bStatus)
				pSound->m_bPaused = FALSE;        // success
			else
				bSuccess = FALSE;                   // failure
		}
		pSound = (CBofSound *)pSound->GetNext();
	}

	return bSuccess;
}


BOOL CBofSound::Resume() {
	Assert(IsValidObject(this));

	BOOL bSuccess = FALSE;

	if (m_bPaused) {                        // must be paused to resume

#if BOF_MAC || BOF_WINMAC
		switch (m_chType) {

		case SOUND_TYPE_MAC_SND:
			bSuccess = ResumeMacSnd();
			break;

		case SOUND_TYPE_MAC_FILE:
			bSuccess = ResumeMacFile();
			break;

		case SOUND_TYPE_MAC_MIDI:
			bSuccess = ResumeMacMidi();
			break;
		}

#else //PC
		warning("STUB: CBofSound::Resume()");

#if 0
		if (m_hSample != nullptr) {
			bSuccess = TRUE;
			AIL_resume_sample(m_hSample);

		} else if (m_hSequence != nullptr) {
			bSuccess = TRUE;
			AIL_resume_sequence(m_hSequence);
		}
#endif
#endif

	}


	if (bSuccess)
		m_bPaused = FALSE;

	return bSuccess;
}


BOOL CBofSound::StopSounds() {
	BOOL    bSuccess = TRUE;
	BOOL    bStatus;
	CBofSound  *pSound;

	pSound = m_pSoundChain;                 // thumb through all the sounds
	while (pSound != nullptr) {

		if (pSound->Playing()) {                 // if one is playing
			pSound->m_bPaused = FALSE;            // ... its no longer paused
			bStatus = pSound->Stop();             // ... try to stop it
			if (bStatus == FALSE)
				bSuccess = FALSE;                   // not the failure
		}
		pSound = (CBofSound *)pSound->GetNext();
	}

	return bSuccess;
}


BOOL CBofSound::StopWaveSounds() {
	BOOL    bSuccess = TRUE;
	CBofSound  *pSound, *pNextSound;

	pSound = m_pSoundChain;                     // find this Sound is the queue
	while (pSound != nullptr) {

		pNextSound = (CBofSound *)pSound->GetNext();

		if (pSound->Playing() && ((pSound->m_wFlags & SOUND_WAVE) || (pSound->m_wFlags & SOUND_MIX))) {
			bSuccess = pSound->Stop();
			if (bSuccess) {
				if (pSound->m_wFlags & SOUND_AUTODELETE)
					delete pSound;
			}
		}
		pSound = pNextSound;
	}

	return bSuccess;
}


BOOL CBofSound::StopMidiSounds() {
	BOOL    bSuccess = TRUE;
	CBofSound  *pSound, *pNextSound;

	pSound = m_pSoundChain;                     // find this Sound is the queue
	while (pSound != nullptr) {

		pNextSound = (CBofSound *)pSound->GetNext();
		if (pSound->Playing() && (pSound->m_wFlags & SOUND_MIDI)) {
			bSuccess = pSound->Stop();
			if (bSuccess) {
				if (pSound->m_wFlags & SOUND_AUTODELETE)
					delete pSound;
			}
		}
		pSound = pNextSound;
	}

	return bSuccess;
}


BOOL CBofSound::Stop() {
	Assert(IsValidObject(this));

	BOOL        bSuccess;

	// assume success
	bSuccess = TRUE;

	// if this sound is currently playing
	//

#if BOF_MAC || BOF_WINMAC

	if (Playing()) {
		switch (m_chType) {

		case SOUND_TYPE_MAC_SND:
			bSuccess = StopMacSnd();
			break;

		case SOUND_TYPE_MAC_FILE:
			bSuccess = StopMacFile();
			break;

		case SOUND_TYPE_MAC_MIDI:
			bSuccess = StopMacMidi();
			break;
		}
	}

#else //PC
//	warning("STUB: CBofSound::Stop()");

#if 0
	if (m_hSequence != nullptr) {
		AIL_stop_sequence(m_hSequence);
		AIL_release_sequence_handle(m_hSequence);
		m_hSequence = nullptr;
	}

	if (m_hSample != nullptr) {
		AIL_stop_sample(m_hSample);
		AIL_release_sample_handle(m_hSample);
		m_hSample = nullptr;
	}

	if (m_bInQueue) {
		Assert(m_iQSlot >= 0 && m_iQSlot < NUM_QUEUES);
		m_cQueue[m_iQSlot].DeleteItem(this);
		m_bInQueue = FALSE;
	}
#endif

#endif  // PC

	m_bPlaying = FALSE;
	m_bStarted = FALSE;

	if (bSuccess) {                     // no longer playing
		m_bPaused = FALSE;

		// One less audio playing
		m_nCount -= 1;
	}

	return bSuccess;
}


VOID CBofSound::ClearSounds() {
	StopSounds();                                   // stop all active sounds

	CBofSound *pSound, *pNextSound;

	pSound = m_pSoundChain;
	while (pSound != nullptr) {                 // delete all sound entries

		pNextSound = pSound->GetNext();

		delete pSound;

		pSound = pNextSound;
	}

	Assert(m_pSoundChain == nullptr);
}


VOID CBofSound::ClearWaveSounds() {
	CBofSound  *pSound, *pNextSound;

	pSound = m_pSoundChain;                     // find this Sound in the queue
	while (pSound != nullptr) {

		pNextSound = pSound->GetNext();
		if ((pSound->m_wFlags & SOUND_WAVE) || (pSound->m_wFlags & SOUND_MIX))
			delete pSound;

		pSound = pNextSound;
	}
}


VOID CBofSound::ClearMidiSounds() {
	CBofSound  *pSound, *pNextSound;

	pSound = m_pSoundChain;                     // find this Sound is the queue
	while (pSound != nullptr) {

		pNextSound = (CBofSound *)pSound->GetNext();
		if (pSound->m_wFlags & SOUND_MIDI)
			delete pSound;

		pSound = pNextSound;
	}
}


BOOL CBofSound::SoundAvailable() {
	return m_bSoundAvailable;                     // return requested info
}


BOOL CBofSound::MidiAvailable() {
	return m_bMidiAvailable;                      // return requested info
}


VOID CBofSound::WaitSounds() {
	WaitWaveSounds();
	WaitMidiSounds();
}


VOID CBofSound::WaitWaveSounds() {
	uint32   dwTickCount = 0;
	CBofSound  *pSound;

	for (;;) {

		AudioTask();

		pSound = m_pSoundChain;
		while (pSound != nullptr) {
			if (pSound->Playing() && (pSound->m_wFlags & SOUND_WAVE || pSound->m_wFlags & SOUND_MIX)) {
				break;
			}
			pSound = (CBofSound *)pSound->GetNext();
		}
		if (pSound == nullptr)
			break;

		if (HandleMessages())
			break;

		// If 3 minutes go by, then just bolt
		//
		if (dwTickCount == 0)
			dwTickCount = g_system->getMillis() + 1000 * 60 * 3;

		if (g_system->getMillis() > dwTickCount) {

			pSound->Stop();
			pSound->m_bPlaying = FALSE;

			pSound->ReportError(ERR_UNKNOWN, "CBofSound::WaitWaveSounds() timeout!");
		}
	}
}

BOOL CBofSound::SoundsPlaying() {
	CBofSound  *pSound;

	AudioTask();

	pSound = m_pSoundChain;
	while (pSound != nullptr) {
		if (pSound->Playing() && (pSound->m_wFlags & SOUND_WAVE || pSound->m_wFlags & SOUND_MIX)) {
			return TRUE;
		}
		pSound = (CBofSound *)pSound->GetNext();
	}

	return FALSE;
}


VOID CBofSound::WaitMidiSounds() {
	UINT32   dwTickCount = 0;
	CBofSound  *pSound;

	for (;;) {

		pSound = m_pSoundChain;
		while (pSound != nullptr) {
			if (pSound->Playing() && (pSound->m_wFlags & SOUND_MIDI)) {
				break;
			}
			pSound = (CBofSound *)pSound->GetNext();
		}
		if (pSound == nullptr)
			break;

		if (HandleMessages())
			break;

		if (dwTickCount == 0)
			dwTickCount = g_system->getMillis() + 1000 * 60;

		if (g_system->getMillis() > dwTickCount) {
			pSound->Stop();

			pSound->ReportError(ERR_UNKNOWN, "CBofSound::WaitMidiSounds() timeout");
			break;
		}
	}
}


BOOL CBofSound::HandleMessages() {
#if BOF_MAC || BOF_WINMAC
	AudioTask();
#else

	warning("STUB: CBofSound::HandleMessages()");

#if 0
	MSG     msg;

	if (PeekMessage(&msg, nullptr, 0, WM_KEYFIRST - 1, PM_REMOVE)) {
		if (msg.message == WM_CLOSE || msg.message == WM_QUIT)
			return TRUE;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (PeekMessage(&msg, nullptr, WM_KEYLAST + 1, WM_MOUSEMOVE, PM_REMOVE)) {
		if (msg.message == WM_CLOSE || msg.message == WM_QUIT)
			return TRUE;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (PeekMessage(&msg, nullptr, WM_PARENTNOTIFY, 0xFFFF, PM_REMOVE)) {
		if (msg.message == WM_CLOSE || msg.message == WM_QUIT)
			return TRUE;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#endif
#endif
	return FALSE;
}



BOOL CBofSound::Sleep(DWORD wait) {
#if BOF_WIN16
	DWORD goal;

	goal = wait + GetTickCount();
	while (goal > GetTickCount()) {
		if (HandleMessages())
			return TRUE;
	}
#else
	uint32 goal;

	goal = wait + g_system->getMillis();
	while (goal > g_system->getMillis()) {
		if (HandleMessages())
			return TRUE;
	}
#endif
	return FALSE;
}


BOOL BofPlaySound(const CHAR *pszSoundFile, UINT nFlags, INT iQSlot) {
	CBofSound *pSound;
	CBofWindow *pWnd;
	BOOL bSuccess;

	// assume failure
	bSuccess = FALSE;

	if (pszSoundFile != nullptr) {

		nFlags |= SOUND_AUTODELETE;

		if (!FileExists(pszSoundFile)) {

			LogError(BuildString("Warning: Sound File '%s' not found", pszSoundFile));
			return FALSE;
		}

		pWnd = CBofApp::GetApp()->GetMainWindow();

//#if BOF_MAC || BOF_WINMAC
		// take care of any last minute cleanup before we start this new sound
		CBofSound::AudioTask();
//#endif
		CBofSound::StopWaveSounds();

		if ((pSound = new CBofSound(pWnd, (CHAR *)pszSoundFile, (WORD)nFlags)) != nullptr) {

			if ((nFlags & SOUND_QUEUE) == SOUND_QUEUE) {
				pSound->SetQSlot(iQSlot);
			}
			bSuccess = pSound->Play();
		}

	} else {
		bSuccess = TRUE;
		CBofSound::StopWaveSounds();
	}

	return bSuccess;
}

BOOL BofPlaySoundEx(const CHAR *pszSoundFile, UINT nFlags, INT iQSlot, BOOL bWait) {
	CBofSound *pSound;
	CBofWindow *pWnd;
	BOOL bSuccess;

	// assume failure
	bSuccess = FALSE;

	if (pszSoundFile != nullptr) {
		if ((nFlags & SOUND_MIX) == 0) {
			bWait = FALSE;
		}

		if (!bWait) {
			nFlags |= SOUND_AUTODELETE;
		}

		if (!FileExists(pszSoundFile)) {

			LogError(BuildString("Warning: Sound File '%s' not found", pszSoundFile));
			return FALSE;
		}

		pWnd = CBofApp::GetApp()->GetMainWindow();

		// take care of any last minute cleanup before we start this new sound
		CBofSound::AudioTask();

		if ((pSound = new CBofSound(pWnd, (CHAR *)pszSoundFile, (WORD)nFlags)) != nullptr) {

			if ((nFlags & SOUND_QUEUE) == SOUND_QUEUE) {
				pSound->SetQSlot(iQSlot);
			}
			bSuccess = pSound->Play();

			if (bWait) {

				while (pSound->IsPlaying()) {
					CBofSound::AudioTask();
				}
				delete pSound;
			}
		}
	}

	return bSuccess;
}


BOOL CBofSound::LoadSound() {
	Assert(IsValidObject(this));
	Assert(m_szFileName[0] != '\0');

	BOOL bSuccess;

	// assume failure
	bSuccess = FALSE;

#if BOF_MAC || BOF_WINMAC

#if PLAYWAVONMAC

//	OSErr err = memFullErr; // use this instead of the next line to test playing from files
	OSErr err = LoadSNDFromWAVFile(m_szFileName, &m_hMacSndRes);

	bSuccess = (err == noErr);

	if (err == noErr) {
		bSuccess = true;
	} else {
		bSuccess = false;
		if (err == memFullErr)
			m_wFlags |= SOUND_BUFFERED;
		else if (err == fnfErr)
			ReportError(ERR_FFIND);
		else
			ReportError(ERR_UNKNOWN);
	}

#else
	//
	// load the SND resource and open SND file and play the sound
	//

	CHAR szFileName[256];
	UCHAR *pStr;

	// set up pathname
	strcpy(szFileName, m_szFileName);

	// convert C string to Pascal String - IN PLACE
	pStr = (UCHAR *)StrCToPascal(szFileName);

	// open the sound file
	//
	if ((m_resRefNum = OpenResFile(pStr)) != -1) {

		// Use the one we just opened
		UseResFile(m_resRefNum);

		// load the sound resource from file and check if successful
		// get the first and only SND
		//
		if ((m_hMacSndRes = GetIndResource('snd ', 1)) != nullptr) {

			// move high in heap so you don't fragment
			MoveHHi(m_hMacSndRes);
			HLock(m_hMacSndRes);

			bSuccess = TRUE;
		} else {

			// force BUFFERED mode since the file was too big to load here
			m_wFlags |= SOUND_BUFFERED;
		}

	} else {

		ReportError(ERR_FFIND);
	}
#endif
#else

	bSuccess = TRUE;
	if (m_pFileBuf == nullptr) {
		bSuccess = FALSE;

		Common::File in;

		if (in.open(m_szFileName)) {
			uint32 fsize = in.size();

			m_pFileBuf = (UBYTE *)malloc(fsize);

			if (in.read(m_pFileBuf, fsize) == fsize)
				bSuccess = TRUE;
		}
	}

#endif
	return bSuccess;
}


BOOL CBofSound::ReleaseSound() {
	Assert(IsValidObject(this));

#if BOF_MAC || BOF_WINMAC

#if PLAYWAVONMAC
	if (m_pSoundInfo != nil) {
		Assert(m_resRefNum == 0);

		OSErr err = ASoundDonePlaying(m_pSoundInfo, kCloseFile + kFreeMem);
		::DisposePtr(m_pSoundInfo);
		m_pSoundInfo = nil;
	}
#endif

	return ReleaseSndResource();
#else

	if (m_pFileBuf != nullptr) {
		free(m_pFileBuf);
		m_pFileBuf = nullptr;
	}
	return TRUE;
#endif
}


#if BOF_MAC || BOF_WINMAC

BOOL CBofSound::CreateMacSndChannel(BOOL) {
	Assert(IsValidObject(this));

	INT i;
	OSErr sndErr;
	BOOL bFound;

	if (m_errCode == ERR_NONE) {

		// get a new sound channel if neccessary
		//
		if (m_pMacSndChan == nullptr) {

			if (!m_bInit) {
				Initialize();
			}

			// assume we won't find an open channel
			bFound = FALSE;

			// find an unused sound channel
			//
			for (i = 0; i < MAX_CHANNELS; i++) {

				// if this one is unused
				//
				if (m_pSndChan[i] != nullptr) {

					// store it with our sound object
					m_pMacSndChan = m_pSndChan[i];

					// mark it as used
					m_pSndChan[i] = nullptr;
					bFound = TRUE;
					break;
				}
			}

			// if we can't use one of the pre-allocated sound channels
			//
			// for the PPC, make sure that we use Universal proc ptrs to
			// our callbacks
			// change to use global gSndCallBackUPP

			if (!bFound) {

				// then we need to allocate a temporary one
				//
				if ((sndErr = SndNewChannel(&m_pMacSndChan, sampledSynth, initMono, gSndCallBackUPP)) == 0) {
					m_bTempChannel = TRUE;
					bFound = TRUE;
				} else {

					LogError(BuildString("SndNewChannel failed on temp channel: sndErr = %d", sndErr));
				}
			}

			// make sure we haven't exceeded our MAX_CHANNELS limit
			Assert(bFound);
		}
	}

	return TRUE;
}


BOOL CBofSound::ReleaseMacSndChan() {
	Assert(IsValidObject(this));

	OSErr sndErr;
	INT i;
	BOOL bSuccess = TRUE;

	if (m_pMacSndChan != nullptr) {

		sndErr = SendFlush(m_pMacSndChan);

		// send it in case
		sndErr = SendQuiet(m_pMacSndChan, kQuietNow);

		// if error, then report it
		//
		if (sndErr) {

			ReportError(ERR_UNKNOWN);
			bSuccess = FALSE;
		}

		// if temporary sound channel then just delete it
		//
		if (m_bTempChannel) {
			sndErr = SndDisposeChannel(m_pMacSndChan, kQuietNow);
			m_bTempChannel = FALSE;

		} else {

			for (i = 0; i < MAX_CHANNELS; i++) {
				if (m_pSndChan[i] == nullptr) {
					m_pSndChan[i] = m_pMacSndChan;
					break;
				}
			}
		}
		m_pMacSndChan = nullptr;
	}

	return bSuccess;
}


BOOL CBofSound::ReleaseSndResource() {
	Assert(IsValidObject(this));

	OSErr   sndErr;
	BOOL    bSuccess = TRUE;

	if (m_hMacSndRes != nullptr) {

#if PLAYWAVONMAC
		sndErr = ReleaseSNDFromWAVFile(m_hMacSndRes);
#else
		ReleaseResource(m_hMacSndRes);
		sndErr = ResError();
#endif

		// if error, report it
		//
		if (sndErr) {
			ReportError(ERR_UNKNOWN);
			bSuccess = FALSE;
		}

		m_hMacSndRes = nullptr;
	}

	return bSuccess;
}


BOOL CBofSound::PlayMacSnd() {
	Assert(IsValidObject(this));

	OSErr   sndErr;
	BOOL    bSuccess;

	// assume success
	bSuccess = TRUE;

	if (m_errCode == ERR_NONE) {

		// mark it as an PlayMacSnd type
		m_chType = SOUND_TYPE_MAC_SND;

		// clean up
		AudioTask();

		// get a new sound channel if neccessary
		//
		if ((bSuccess = CreateMacSndChannel(TRUE)) == FALSE) {

			// set error code
			//
			ReportError(ERR_MEMORY);

			if (m_wFlags & SOUND_AUTODELETE)  // delete object
				delete this;

			return FALSE;                     // return FALSE
		}

		// send the object to the callback
		if (m_wFlags & SOUND_ASYNCH) {
			m_pMacSndChan->userInfo = (LONG) this;
		} else {
			m_pMacSndChan->userInfo = 0;
		}

		sndErr = SndPlay(m_pMacSndChan, (SndListHandle) m_hMacSndRes,
		                 (m_wFlags & SOUND_ASYNCH) ? (unsigned char)1 : (unsigned char)0);

		if (sndErr != noErr) {
			// set error code
			//
			ReportError(ERR_UNKNOWN);

			if (m_wFlags & SOUND_AUTODELETE)  // delete object
				delete this;

			return FALSE;                 // return FALSE
		}

		if (m_wFlags & SOUND_ASYNCH) {

			if ((sndErr = InstallCallBack(m_pMacSndChan)) != 0) {

				ReportError(ERR_UNKNOWN);

				if (m_wFlags & SOUND_AUTODELETE)  // delete object
					delete this;

				return FALSE;                 // return FALSE
			}
		}

		m_nCount += 1;                              // bump count of active Sounds
		m_bPlaying = TRUE;                          // mark that it's now playing
		m_bPaused = FALSE;

		if (!(m_wFlags & SOUND_ASYNCH) && (m_wFlags & SOUND_AUTODELETE))
			delete this;
	}

	return bSuccess;
}


BOOL CBofSound::PlayMacSndFile() {
	Assert(IsValidObject(this));

	BOOL    bSuccess = TRUE;
	OSErr   sndErr;

	m_chType = SOUND_TYPE_MAC_FILE;        // mark it as an PlayMacSndFile type

	if (m_errCode == ERR_NONE) {

		// clean up
		AudioTask();

		// play from resource file
		//

		// first we need to make sure we don't already have it open
		ReleaseSound();

		CHAR szFileName[256];
		UCHAR *pStr;

		// set up pathname - making a copy that you can convert to pascal
		//
		strcpy(szFileName, m_szFileName);

		// convert C string to Pascal String - IN PLACE
		pStr = (UCHAR *)StrCToPascal(szFileName);

#if PLAYWAVONMAC

		FSSpec theFSSpec;

		sndErr = ::FSMakeFSSpec(0, 0, pStr, &theFSSpec);

		if (sndErr != noErr) {
			ReportError(ERR_FFIND);
			if (m_wFlags & SOUND_AUTODELETE)  // delete object
				delete this;

			return FALSE;                 // return FALSE
		}

		m_pSoundInfo = ASoundNew(&sndErr);

		if (sndErr != noErr) {
			ReportError(ERR_UNKNOWN);
			if (m_wFlags & SOUND_AUTODELETE)  // delete object
				delete this;

			return FALSE;                 // return FALSE
		}

		sndErr = ASoundSpecifyWAVFileToPlay(m_pSoundInfo, &theFSSpec);

		if (sndErr != noErr) {
			ReportError(ERR_UNKNOWN);
			if (m_wFlags & SOUND_AUTODELETE)  // delete object
				delete this;

			return FALSE;                 // return FALSE
		}

		// get a new sound channel with no callback function because SndStartFilePlay sets it
		bSuccess = CreateMacSndChannel(FALSE);
		if (!bSuccess) {

			ReportError(ERR_UNKNOWN);

			if (m_wFlags & SOUND_AUTODELETE)  // delete object
				delete this;

			return bSuccess;                 // return FALSE
		}

		sndErr = ASoundSetSoundChannel(m_pSoundInfo, m_pMacSndChan);
		if (sndErr != noErr) {
			ReportError(ERR_UNKNOWN);
			if (m_wFlags & SOUND_AUTODELETE)  // delete object
				delete this;

			return FALSE;                 // return FALSE
		}

		sndErr = ASoundSetSoundCallBack(m_pSoundInfo, OnMacFileCallback);

		if (sndErr != noErr) {
			ReportError(ERR_UNKNOWN);
			if (m_wFlags & SOUND_AUTODELETE)  // delete object
				delete this;

			return FALSE;                 // return FALSE
		}

		m_pMacSndChan->userInfo = SetCurrentA5();

		sndErr = ASoundStartPlaying(m_pSoundInfo, 0);

		if (sndErr == noErr && !(m_wFlags & SOUND_ASYNCH)) {    // synchronous
			while (m_pMacSndChan->userInfo != 0)
				;
		}
#else
		// open the sound file
		//
		if ((m_resRefNum = OpenResFile(pStr)) == -1) {

			ReportError(ERR_FOPEN);

			if (m_wFlags & SOUND_AUTODELETE)  // delete object
				delete this;

			return FALSE;                 // return FALSE
		}

		// get a new sound channel with no callback function because SndStartFilePlay sets it
		bSuccess = CreateMacSndChannel(FALSE);
		if (!bSuccess) {

			ReportError(ERR_UNKNOWN);

			if (m_wFlags & SOUND_AUTODELETE)  // delete object
				delete this;

			return bSuccess;                 // return FALSE
		}

		//  Mark the field for playing
		m_pMacSndChan->userInfo = SetCurrentA5();

		// for the PPC, make sure that we use Universal proc ptrs to
		// our callbacks

		sndErr = SndStartFilePlay(m_pMacSndChan, m_resRefNum, 0, kTotalSize, nullptr, nullptr, gFilePlayCompletionUPP,
		                          (m_wFlags & SOUND_ASYNCH) ? (unsigned char)1 : (unsigned char)0);

#endif
		if (sndErr) {
			ReportError(ERR_UNKNOWN);

			bSuccess = FALSE;                   // set flags

			if (m_wFlags & SOUND_AUTODELETE)  // delete object
				delete this;
			return bSuccess;                 // return FALSE
		}

		m_nCount += 1;                              // bump count of active Sounds
		m_bPlaying = TRUE;                          // mark that it's now playing
		m_bPaused = FALSE;

		if (!(m_wFlags & SOUND_ASYNCH) && (m_wFlags & SOUND_AUTODELETE))
			delete this;
	}

	return bSuccess;
}


BOOL CBofSound::PlayMacMidi() {
	Assert(IsValidObject(this));

	BOOL    bSuccess = TRUE;

	m_chType = SOUND_TYPE_MAC_MIDI;   // mark it as an PlayMacMidi type

	if (m_errCode == ERR_NONE) {

		// clean up
		AudioTask();

		// if we need to load the movie object
		//
		if (!m_pMacMidi) {

			// convert C string to Pascal String
			CHAR szFileName[MAX_DIRPATH];
			CHAR szFullFile[MAX_DIRPATH];
			UCHAR *pStr;

			// set up pathname
			strcpy(szFileName, m_szFileName);

			FileGetFullPath(szFullFile, szFileName);

			// convert C string to Pascal String - IN PLACE
			pStr = (UCHAR *)StrCToPascal(szFullFile);

			if ((m_pMacMidi = new MacQT(pStr, 0, 0)) == nullptr) {

				ReportError(ERR_MEMORY);

				bSuccess = FALSE;                   // set flags

				if (m_wFlags & SOUND_AUTODELETE)  // delete object
					delete this;
			}

		}

		bSuccess =  m_pMacMidi->Play();

		if (!bSuccess) {                          // We failed
			ReportError(ERR_UNKNOWN);

			if (m_wFlags & SOUND_AUTODELETE)  // delete object
				delete this;
			return bSuccess;                  // return FALSE
		}

		m_nCount += 1;                              // bump count of active Sounds
		m_bPlaying = TRUE;                          // mark that it's now playing
		m_bPaused = FALSE;
	}

	return bSuccess;
}


BOOL CBofSound::MacReplay(CBofSound *pSound) {
	BOOL bSuccess = TRUE;
	OSErr  sndErr;

	Assert(pSound != nullptr);

	if (pSound->m_errCode == ERR_NONE) {

		if (pSound->m_chType == SOUND_TYPE_MAC_SND) {

			// use universal callback procs (upp's)
			sndErr = SndPlay(pSound->m_pMacSndChan, (SndListHandle) pSound->m_hMacSndRes,
			                 (pSound->m_wFlags & SOUND_ASYNCH) ? (unsigned char)1 : (unsigned char)0);

			if (sndErr) {
				pSound->ReportError(ERR_UNKNOWN);

				bSuccess = FALSE;

				// don't auto delete here,
				// just return to MacSndStop
				// and it will clean up on fail
				return bSuccess;                            // return FALSE
			}

			sndErr = pSound->InstallCallBack(pSound->m_pMacSndChan);

			if (sndErr) {

				pSound->ReportError(ERR_UNKNOWN);

				bSuccess = FALSE;                   // set flags

				// don't auto delete here,
				// just return to MacSndStop
				// and it will clean up on fail
				return bSuccess;                  // return FALSE
			}

			pSound->m_nCount += 1;                              // bump count of active Sounds
			pSound->m_bPlaying = TRUE;                          // mark that it's now playing
			pSound->m_bPaused = FALSE;
			pSound->m_pMacSndChan->userInfo = SetCurrentA5();   // loops indefinitely without this

		} else if (pSound->m_chType == SOUND_TYPE_MAC_FILE) {

			// for the PPC, make sure that we use Universal proc ptrs to
			// our callbacks

#if PLAYWAVONMAC
			Assert(ASoundIsDone(pSound->m_pSoundInfo));

			pSound->m_pMacSndChan->userInfo = SetCurrentA5();   // loops indefinitely without this

			sndErr = ASoundPlay(pSound->m_pSoundInfo);

			if (sndErr == noErr && !(pSound->m_wFlags & SOUND_ASYNCH)) {    // synchronous
				while (pSound->m_pMacSndChan->userInfo != 0)
					;
			}
#else
			sndErr = SndStartFilePlay(pSound->m_pMacSndChan, pSound->m_resRefNum, 0, kTotalSize, nullptr, nullptr,
			                          gFilePlayCompletionUPP, (pSound->m_wFlags & SOUND_ASYNCH) ? (unsigned char)1 : (unsigned char)0);
#endif
			if (sndErr) {
				pSound->ReportError(ERR_UNKNOWN);

				// don't auto delete here,
				// just return to MacSndStop
				// and it will clean up on fail
				return FALSE;                 // return FALSE
			}

			pSound->m_nCount += 1;                              // bump count of active Sounds
			pSound->m_bPlaying = TRUE;                          // mark that it's now playing
			pSound->m_bPaused = FALSE;

		} else if (pSound->m_chType == SOUND_TYPE_MAC_MIDI) {

			bSuccess =  pSound->m_pMacMidi->Play();

			if (!bSuccess) {

				pSound->ReportError(ERR_UNKNOWN);

				// don't auto delete here,
				// just return to MacSndStop
				// and it will clean up on fail
				return bSuccess;                 // return FALSE
			}

			pSound->m_nCount += 1;                              // bump count of active Sounds
			pSound->m_bPlaying = TRUE;                          // mark that it's now playing
			pSound->m_bPaused = FALSE;
		}
	}

	return bSuccess;
}


pascal VOID CBofSound::OnMacSndCallback(SndChannelPtr SndChan, SndCommand theCmd) {
	// just need to store a nonzero value here, and set it to zero
	// to let the ::audiotask code know when this thing is really done playing.
	CSound      *pSnd = (CSound *) SndChan->userInfo;

	SndChan->userInfo = 0;
}


pascal VOID CBofSound::OnMacFileCallback(SndChannelPtr SndChan) {
	long SaveA5;

	SaveA5 = SetA5(SndChan->userInfo);

	// pull out the userinfo place
	SndChan->userInfo = 0;

	SetA5(SaveA5);
}


CBofSound *CBofSound::OnMacSndStopped(CBofSound *pSound) {
	BOOL        bSuccess = FALSE;                         // success/failure status

	if (pSound != nullptr) {                                 // release the data structures
		if (pSound->Playing()) {

			pSound->m_bPlaying = FALSE;                   // mark sound as no longer playing
			pSound->m_bPaused = FALSE;

			m_nCount -= 1;                                // decrement active sound count

			// if the loop var is non-zero, then keep looping and decrement it.
			// if (pSound->m_wFlags & SOUND_LOOP) {
			if (pSound->m_wLoops > 0) {
				pSound->m_wLoops--;
			}
			if (pSound->m_wFlags & SOUND_LOOP || pSound->m_wLoops) {           // if looping is specified
				bSuccess = pSound->MacReplay(pSound);
				if (bSuccess) {
					return pSound;
				} else {
					pSound->m_wFlags ^= SOUND_LOOP;
				}
			}
		}

		// release sound channel
		pSound->ReleaseMacSndChan();

		// release any sound resources still in memory
		pSound->ReleaseSndResource();

#if PLAYWAVONMAC
		if (pSound->m_pSoundInfo != nil) {
			Assert(pSound->m_resRefNum == 0);

			OSErr err = ASoundDonePlaying(pSound->m_pSoundInfo, kCloseFile + kFreeMem);
			::DisposePtr(pSound->m_pSoundInfo);
			pSound->m_pSoundInfo = nil;
		}
#else
		// Close open res file
		if (pSound->m_resRefNum) {
			CloseResFile(pSound->m_resRefNum);
			pSound->m_resRefNum = 0;
		}
#endif

		if (pSound->m_wFlags & SOUND_AUTODELETE) {    // scrag it if marked for deletion
			delete pSound;
			pSound = nullptr;

		} else if (!(pSound->m_wFlags & SOUND_NOTIFY))       // return sound pointer on notify
			pSound = nullptr;
	}

	return pSound;
}


BOOL CBofSound::PauseMacSnd() {
	Assert(IsValidObject(this));

	if (SendPause(m_pMacSndChan) != 0) {
		ReportError(ERR_UNKNOWN);
	}

	return !ErrorOccurred();
}


BOOL CBofSound::PauseMacFile() {
	Assert(IsValidObject(this));

#if PLAYWAVONMAC
	if (ASoundPause(m_pSoundInfo) != 0) {
		ReportError(ERR_UNKNOWN);
	}
#else
	if (SndPauseFilePlay(m_pMacSndChan) != 0) {
		ReportError(ERR_UNKNOWN);
	}
#endif

	return !ErrorOccurred();
}


BOOL CBofSound::PauseMacMidi() {
	Assert(IsValidObject(this));
	Assert(m_pMacMidi != nullptr);

	m_pMacMidi->Stop();

	return TRUE;
}


BOOL CBofSound::ResumeMacSnd() {
	Assert(IsValidObject(this));

	if (SendResume(m_pMacSndChan) != 0) {
		ReportError(ERR_UNKNOWN);
	}

	return !ErrorOccurred();
}


BOOL CBofSound::ResumeMacFile() {
	Assert(IsValidObject(this));

	// calling this again will resume play
#if PLAYWAVONMAC
	if (ASoundPause(m_pSoundInfo) != 0) {
		ReportError(ERR_UNKNOWN);
	}
#else
	if (SndPauseFilePlay(m_pMacSndChan) != 0) {
		ReportError(ERR_UNKNOWN);
	}
#endif

	return !ErrorOccurred();
}


BOOL CBofSound::ResumeMacMidi() {
	Assert(IsValidObject(this));

	m_pMacMidi->Resume();

	return TRUE;
}


BOOL CBofSound::StopMacSnd() {
	Assert(IsValidObject(this));

	// first flush out all other commands
	SendFlush(m_pMacSndChan);

	// then send a quiet command to the channel
	//
	if (SendQuiet(m_pMacSndChan, kQuietNow) != 0) {
		ReportError(ERR_UNKNOWN);
	}

	return !ErrorOccurred();
}


BOOL CBofSound::StopMacFile() {
	Assert(IsValidObject(this));

#if PLAYWAVONMAC
	if (ASoundStop(m_pSoundInfo) != 0) {
		ReportError(ERR_UNKNOWN);
	}
#else
	if (SndStopFilePlay(m_pMacSndChan, TRUE) != 0) {
		ReportError(ERR_UNKNOWN);
	}
#endif

	return !ErrorOccurred();
}


BOOL CBofSound::StopMacMidi() {
	Assert(IsValidObject(this));

	Assert(m_pMacMidi != nullptr);

	m_pMacMidi->Stop();

	return TRUE;
}


OSErr CBofSound::InsertAmp(SndChannelPtr aSndChan, short level) {
	SndCommand mySndCmd;    // command record

	mySndCmd.cmd = ampCmd;  // install the callback command
	mySndCmd.param1 = level;    // last command for this channel
	mySndCmd.param2 = 0;

	return SndDoImmediate(aSndChan, &mySndCmd);
}


OSErr CBofSound::InstallCallBack(SndChannelPtr aSndChan) {
	OSErr cbErr;
	SndCommand mySndCmd;    // command record

	mySndCmd.cmd = callBackCmd; // install the callback command
	mySndCmd.param1 = 0x1234;   // mark this with our unique value
	mySndCmd.param2 = SetCurrentA5();   // pass the callback the A5

	// the final parameter to do command is true if you
	// want the sound manager to return a queue full error immediately.
	//
	// cbErr = SndDoCommand (aSndChan, &mySndCmd, kWaitIfFull);
	cbErr = SndDoCommand(aSndChan, &mySndCmd, FALSE);

	return cbErr;
}


OSErr CBofSound::SendQuiet(SndChannelPtr aSndChan, int immediate) {
	SndCommand theCmd;

	theCmd.cmd = quietCmd;
	theCmd.param1 = 0;
	theCmd.param2 = 0;

	if (immediate)
		return SndDoImmediate(aSndChan, &theCmd);
	else
		return SndDoCommand(aSndChan, &theCmd, !kQuietNow);
}


OSErr CBofSound::SendPause(SndChannelPtr aSndChan) {
	SndCommand mySndCmd;    // command record

	mySndCmd.cmd = pauseCmd;    // install the callback command
	mySndCmd.param1 = 0;    // last command for this channel
	mySndCmd.param2 = 0;
	return SndDoImmediate(aSndChan, &mySndCmd);
}


OSErr CBofSound::SendResume(SndChannelPtr aSndChan) {
	SndCommand mySndCmd;    // command record

	mySndCmd.cmd = resumeCmd;   // install the callback command
	mySndCmd.param1 = 0;    // last command for this channel
	mySndCmd.param2 = 0;
	return SndDoImmediate(aSndChan, &mySndCmd);
}


OSErr CBofSound::SendFlush(SndChannelPtr aSndChan) {
	SndCommand theCmd;

	theCmd.cmd = flushCmd;
	theCmd.param1 = 0;
	theCmd.param2 = 0;

	return SndDoCommand(aSndChan, &theCmd, !kQuietNow);
}
#endif // MAC


VOID CBofSound::SetDrivePath(const CHAR *pszDrivePath) {
	m_szDrivePath[0] = '\0';
	if (pszDrivePath != nullptr) {
		Common::strlcpy(m_szDrivePath, pszDrivePath, MAX_DIRPATH);
	}
}

VOID CBofSound::GetDrivePath(CHAR *pszDrivePath) {
	Assert(pszDrivePath != nullptr);

	*pszDrivePath = '\0';
	if (m_szDrivePath[0] != '\0') {
		Common::strlcpy(pszDrivePath, m_szDrivePath, MAX_DIRPATH);
	}
}

BOOL CBofSound::SoundsPlayingNotOver() {
	CSound *pSound;
	BOOL bPlaying;

#if BOF_MAC || BOF_WINMAC
	AudioTask();
#endif

	// assume no wave sounds are playing
	bPlaying = FALSE;

	// walk through sound list, and check for sounds that need attention
	//
	pSound = m_pSoundChain;
	while (pSound != nullptr) {

		if (pSound->Playing() &&
		        (pSound->m_wFlags & SOUND_WAVE || pSound->m_wFlags & SOUND_MIX) &&
		        !(pSound->m_wFlags & SOUND_OVEROK)) {
			bPlaying = TRUE;
			break;
		}

		pSound = (CBofSound *)pSound->GetNext();
	}

	return bPlaying;
}


BOOL CBofSound::WaveSoundPlaying() {
	CSound *pSound;
	BOOL bPlaying;

#if BOF_MAC || BOF_WINMAC
	AudioTask();
#endif

	// assume no wave sounds are playing
	bPlaying = FALSE;

	// walk through sound list, and check for sounds that need attention
	//
	pSound = m_pSoundChain;
	while (pSound != nullptr) {

		if (pSound->Playing() && (pSound->m_wFlags & SOUND_WAVE || pSound->m_wFlags & SOUND_MIX)) {
			bPlaying = TRUE;
			break;
		}

		pSound = (CBofSound *)pSound->GetNext();
	}

	return bPlaying;
}


BOOL CBofSound::MidiSoundPlaying() {
	CSound *pSound;
	BOOL bPlaying;

#if BOF_MAC || BOF_WINMAC
	AudioTask();
#endif

	// assume no wave sounds are playing
	bPlaying = FALSE;

	// walk through sound list, and check for sounds that need attention
	//
	pSound = m_pSoundChain;
	while (pSound != nullptr) {

		if (pSound->Playing() && (pSound->m_wFlags & SOUND_MIDI)) {
			bPlaying = TRUE;
			break;
		}

		pSound = (CBofSound *)pSound->GetNext();
	}

	return bPlaying;
}


VOID CBofSound::AudioTask() {
	static BOOL bAlready = FALSE;
	CBofSound *pSound;

	// don't allow recursion here
	Assert(!bAlready);

	bAlready = TRUE;

#if BOF_MAC || BOF_WINMAC
	// let midi/movies have some time to cleanup
	MacQT::Task();
#endif

	// walk through sound list, and check for sounds that need attention
	//
	pSound = m_pSoundChain;
	while (pSound != nullptr) {

#if BOF_MAC || BOF_WINMAC

		// Determine what type of sound it is
		//
		if ((pSound->m_chType == SOUND_TYPE_MAC_SND) || (pSound->m_chType == SOUND_TYPE_MAC_FILE)) {

			// if it is playing and it has a valid sound channel
			//
			if (pSound->Playing() && (pSound->m_pMacSndChan != nullptr)) {

				// if the channel flag has been set to FALSE, it's finished playing
				//
				if (pSound->m_pMacSndChan->userInfo == 0) {

					// call the clean up function for the sound
					OnMacSndStopped(pSound);

					// go back to beginning of list
					pSound = m_pSoundChain;

					// back up to top of while
					continue;
				}
			}

		} else if (pSound->m_chType == SOUND_TYPE_MAC_MIDI) {

			// if it is playing and it has a valid movie object
			//
			if ((pSound->Playing()) && (pSound->m_pMacMidi)) {

				// if the movie has finished up
				//
				if (pSound->m_pMacMidi->Playing() == FALSE) {

					// call the clean up function
					OnMacSndStopped(pSound);

					// go back to beginning of list
					pSound = m_pSoundChain;

					// back up to top of while
					continue;
				}
			}
		}
#else
		if (!pSound->Paused()) {

			debug(1, "STUB: CBofSound::MidiSoundPlaying()");

#if 0

			if ((pSound->m_wFlags & SOUND_WAVE) || (pSound->m_wFlags & SOUND_MIX)) {

				// Has this sound been played?
				//
				if (pSound->m_hSample != nullptr) {

					// And, Is it done?
					//
					if (AIL_sample_status(pSound->m_hSample) != SMP_PLAYING) {

						// Kill it
						pSound->Stop();
					}

				} else {

					// If this is a Queued sound, and has not already started
					//
					if (pSound->m_bInQueue && !pSound->m_bStarted) {

						// And it is time to play
						//
						if ((CBofSound *)m_cQueue[pSound->m_iQSlot].GetQItem() == pSound) {
							pSound->m_bStarted = TRUE;

							pSound->PlayMSS();
						}
					}
				}

			} else if (pSound->m_wFlags & SOUND_MIDI) {

				if (pSound->m_hSequence != nullptr) {

					// And, Is it done?
					//
					if (AIL_sequence_status(pSound->m_hSequence) != SEQ_PLAYING) {

						// Kill it
						pSound->Stop();
					}
				}
			}
#endif
		}

#endif

		pSound = (CBofSound *)pSound->GetNext();
	}

	bAlready = FALSE;
}

#if BOF_WINDOWS && !BOF_WINMAC
ERROR_CODE CBofSound::PlayMSS() {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {

		// If it's not yet loaded, then load it now
		//
		if (m_pFileBuf == nullptr) {
			LoadSound();
		}

		Assert(m_pFileBuf != nullptr);

		if (m_pFileBuf != nullptr) {

			if (m_bInQueue) {
				SetVolume(m_nSlotVol[m_iQSlot]);
			}

			// Then, Play it
			//
			HDIGDRIVER hDriver;
			if ((hDriver = CBofApp::GetApp()->GetDriver()) != nullptr) {

				if ((m_hSample = AIL_allocate_sample_handle(hDriver)) != nullptr) {
					AIL_init_sample(m_hSample);
					AIL_set_sample_file(m_hSample, m_pFileBuf, 0);
					AIL_set_sample_loop_count(m_hSample, m_wLoops);
					AIL_set_sample_volume(m_hSample, m_nVol * 10);
					AIL_start_sample(m_hSample);
					m_bPlaying = TRUE;

				} else {
					ReportError(ERR_UNKNOWN, "Could not allocate an HSAMPLE. (%s)", AIL_last_error());
				}
			}
		}
	}

	return m_errCode;
}


ERROR_CODE CBofSound::FlushQueue(INT nSlot) {
	Assert(nSlot >= 0 && nSlot < NUM_QUEUES);

	CBofSound *pSound, *pNextSound;
	ERROR_CODE errCode;

	// Assume no error
	errCode = ERR_NONE;

	// Remove all queued sounds
	//
	m_cQueue[nSlot].Flush();

	// Including any that are currently playing
	//
	pSound = m_pSoundChain;
	while (pSound != nullptr) {

		// Prefetch next sound in case Stop() deletes this one
		pNextSound = pSound->GetNext();

		// If this sound is playing from specified queue
		//
		if (pSound->IsPlaying() && pSound->m_bInQueue && pSound->m_iQSlot == nSlot) {

			// Then chuck it
			pSound->Stop();
		}

		// Next
		pSound = pNextSound;
	}

	return errCode;
}

#endif
/*
BOOL CBofSound::Playing()
{
    BOOL bPlaying;

    bPlaying = FALSE;

    if (m_hSample != nullptr && (AIL_sample_status(m_hSample) == SMP_PLAYING)) {
        bPlaying = TRUE;
    }

    return(bPlaying);
}
*/

#if BOF_WINDOWS
VOID CBofSound::SetQVol(INT nSlot, INT nVol) {
	// Validate input
	Assert(nSlot >= 0 && nSlot < NUM_QUEUES);
	Assert(nVol >= 0 && nVol <= VOLUME_INDEX_MAX);

	CBofSound *pSound;

	m_nSlotVol[nSlot] = nVol;

	// Set all Queued sounds in specified slot to this volume
	//
	pSound = m_pSoundChain;
	while (pSound != nullptr) {
		if (pSound->m_bInQueue && pSound->m_iQSlot == nSlot) {
			pSound->SetVolume(nVol);
		}
		pSound = pSound->GetNext();
	}
}
#endif

} // namespace Bagel
