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
#include "common/memstream.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/wave.h"

#include "bagel/bagel.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/event_loop.h"
#include "bagel/boflib/misc.h"

#include "bagel/boflib/sound.h"

namespace Bagel {

#define MEMORY_THRESHOLD    20000L
#define MEMORY_MARGIN       100000L

char    CBofSound::m_szDrivePath[MAX_DIRPATH];
CBofSound  *CBofSound::m_pSoundChain = nullptr;  // pointer to chain of linked Sounds
INT     CBofSound::m_nCount = 0;           // count of currently active Sounds
INT     CBofSound::m_nWavCount = 0;        // available wave sound devices
INT     CBofSound::m_nMidiCount = 0;       // available midi sound devices
bool    CBofSound::m_bSoundAvailable = FALSE;  // whether wave sound is available
bool    CBofSound::m_bMidiAvailable = FALSE;   // whether midi sound is available
bool    CBofSound::m_bWaveVolume = FALSE;  // whether wave volume can be set
bool    CBofSound::m_bMidiVolume = FALSE;  // whether midi volume can be set
CBofWindow   *CBofSound::m_pMainWnd = nullptr;         // window for message processing

bool    CBofSound::m_bInit = FALSE;

CQueue *CBofSound::m_cQueue[NUM_QUEUES];
INT CBofSound::m_nSlotVol[NUM_QUEUES];


CBofSound::CBofSound(CBofWindow *pWnd, const char *pszPathName, WORD wFlags, const INT nLoops) {
	char szTempPath[MAX_DIRPATH];

	// validate input
	//
	Assert(pszPathName != nullptr);
	Assert(strlen(pszPathName) < MAX_FNAME);

	//
	// Initialize data fields
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

	m_handle = {};
	m_pFileBuf = nullptr;
	m_nVol = VOLUME_INDEX_DEFAULT;
	m_bInQueue = FALSE;
	m_iQSlot = 0;

	INT i;
	for (i = 0; i < NUM_QUEUES; i++) {
		m_nSlotVol[i] = VOLUME_INDEX_DEFAULT;
	}

	// Mixed assumes Asynchronous
	//
	if ((m_wFlags & SOUND_MIX) == SOUND_MIX) {
		m_wFlags |= SOUND_ASYNCH;
	}

	if (m_wFlags & SOUND_MIDI) {
		m_chType = SOUND_TYPE_XM;
	} else {
		m_chType = SOUND_TYPE_WAV;
	}

	if (pszPathName != nullptr) {

		if ((m_szDrivePath[0] != '\0') && (*pszPathName == '.'))
			pszPathName++;

		snprintf(szTempPath, MAX_DIRPATH, "%s%s", m_szDrivePath, pszPathName);
		StrReplaceStr(szTempPath, "\\\\", "\\");

		// continue as long as this file exists
		//
		if (FileExists(szTempPath)) {

			FileGetFullPath(m_szFileName, szTempPath);

			if (!(m_wFlags & SOUND_QUEUE)) {

				if (m_wFlags & SOUND_WAVE || m_wFlags & SOUND_MIX) {
					LoadSound();
				}
			}

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
	ReleaseSound();

	if (this == m_pSoundChain) {              // special case head of chain

		m_pSoundChain = (CBofSound *)m_pSoundChain->GetNext();
	}
}


void CBofSound::initialize() {
	m_bSoundAvailable = TRUE;
	m_bMidiAvailable = FALSE;

	for (int i = 0; i < NUM_QUEUES; ++i)
		m_cQueue[i] = new CQueue();

	ResetQVolumes();
}

void CBofSound::ResetQVolumes() {
	INT i;

	// Set Q Volumes to default
	//
	for (i = 0; i < NUM_QUEUES; i++) {
		m_nSlotVol[i] = VOLUME_INDEX_DEFAULT;
	}
}


void CBofSound::shutdown() {
	// Auto-delete any remaining sounds
	ClearSounds();

	for (int i = 0; i < NUM_QUEUES; ++i)
		delete m_cQueue[i];
}



void CBofSound::SetVolume(INT nVolume) {
	Assert(nVolume >= VOLUME_INDEX_MIN && nVolume <= VOLUME_INDEX_MAX);
	INT nLocalVolume = nVolume;

	if (nLocalVolume < VOLUME_INDEX_MIN) {
		nLocalVolume = VOLUME_INDEX_MIN;

	} else if (nLocalVolume > VOLUME_INDEX_MAX) {
		nLocalVolume = VOLUME_INDEX_MAX;
	}

	m_nVol = nLocalVolume;

	g_system->getMixer()->setChannelVolume(m_handle, VOLUME_SVM(m_nVol));

	// TODO: MIDI volume
}


void CBofSound::SetVolume(INT nMidiVolume, INT nWaveVolume) {
	Assert(nMidiVolume >= VOLUME_INDEX_MIN && nMidiVolume <= VOLUME_INDEX_MAX);
	Assert(nWaveVolume >= VOLUME_INDEX_MIN && nWaveVolume <= VOLUME_INDEX_MAX);

    Assert(nMidiVolume >= VOLUME_INDEX_MIN && nMidiVolume <= VOLUME_INDEX_MAX);
	Assert(nWaveVolume >= VOLUME_INDEX_MIN && nWaveVolume <= VOLUME_INDEX_MAX);

	if (nWaveVolume < VOLUME_INDEX_MIN) {
		nWaveVolume = VOLUME_INDEX_MIN;

	} else if (nWaveVolume > VOLUME_INDEX_MAX) {
		nWaveVolume = VOLUME_INDEX_MAX;
	}

	// Set master wave volume
	//
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, VOLUME_SVM(nWaveVolume));

	if (nMidiVolume < VOLUME_INDEX_MIN) {
		nMidiVolume = VOLUME_INDEX_MIN;

	} else if (nMidiVolume > VOLUME_INDEX_MAX) {
		nMidiVolume = VOLUME_INDEX_MAX;
	}

	// Set master Midi volume
	//
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, VOLUME_SVM(nMidiVolume));
}


bool CBofSound::Play(uint32 dwBeginHere, uint32 TimeFormatFlag) {
	Assert(IsValidObject(this));

	bool    bSuccess;

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

		if (m_pFileBuf == nullptr) {

			if ((m_wFlags & (SOUND_MIX | SOUND_QUEUE)) == (SOUND_MIX | SOUND_QUEUE)) {

				// Don't pre-load it

			} else {
				LoadSound();
			}
		}

		if (m_wFlags & SOUND_MIDI) {

			g_engine->_midi->play(this);
			m_bPlaying = TRUE;

		} else if (m_wFlags & SOUND_WAVE) {

			PlayWAV();

			if (m_bPlaying) {

				if (!(m_wFlags & SOUND_ASYNCH)) {
					EventLoop eventLoop;
					auto *mixer = g_system->getMixer();

					while (mixer->isSoundHandleActive(m_handle)) {
						if (eventLoop.frame()) {
							Stop();
							break;
						}
					}

					m_handle = {};
					m_bPlaying = FALSE;
				}
			}

		} else if (m_wFlags & SOUND_MIX) {

			if (!(m_wFlags & SOUND_QUEUE)) {

				PlayWAV();

			} else {
				Assert(m_iQSlot >= 0 && m_iQSlot < NUM_QUEUES);

				m_cQueue[m_iQSlot]->AddItem(this);
				m_bPlaying = TRUE;
				m_bInQueue = TRUE;
				SetVolume(m_nSlotVol[m_iQSlot]);
			}
		}

	}

	return bSuccess;
}


bool CBofSound::MidiLoopPlaySegment(uint32 dwLoopFrom, uint32 dwLoopTo, uint32 dwBegin, uint32 TimeFmt) {
	Assert(IsValidObject(this));

	bool    bSuccess;

	m_wFlags |= SOUND_LOOP;
	m_dwRePlayStart = dwLoopFrom;
	m_dwRePlayEnd = dwLoopTo;
	m_bExtensionsUsed = TRUE;

	bSuccess = Play(dwBegin, TimeFmt);

	return bSuccess;
}


bool CBofSound::PauseSounds() {
	bool    bSuccess = TRUE;
	bool    bStatus;
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


bool CBofSound::Pause() {
	Assert(IsValidObject(this));

	bool bSuccess = FALSE;

	// must be playing to be paused and not already paused
	//
	if (Playing() && (m_bPaused == FALSE)) {
		bSuccess = TRUE;
		if (m_wFlags & SOUND_MIDI) {
			g_engine->_midi->pause();
		} else {
			g_system->getMixer()->pauseHandle(m_handle, true);
		}
	}

	if (bSuccess)
		m_bPaused = TRUE;

	return bSuccess;
}


bool CBofSound::ResumeSounds() {
	bool    bSuccess = TRUE;
	bool    bStatus;
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


bool CBofSound::Resume() {
	Assert(IsValidObject(this));

	bool bSuccess = FALSE;

	if (m_bPaused) {                        // must be paused to resume
		bSuccess = TRUE;
		if (m_wFlags & SOUND_MIDI) {
			g_engine->_midi->resume();
		} else {
			g_system->getMixer()->pauseHandle(m_handle, false);
		}
	}


	if (bSuccess)
		m_bPaused = FALSE;

	return bSuccess;
}


bool CBofSound::StopSounds() {
	bool    bSuccess = TRUE;
	bool    bStatus;
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


bool CBofSound::StopWaveSounds() {
	bool    bSuccess = TRUE;
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


bool CBofSound::StopMidiSounds() {
	bool    bSuccess = TRUE;
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


bool CBofSound::Stop() {
	Assert(IsValidObject(this));

	bool        bSuccess;

	// assume success
	bSuccess = TRUE;

	// if this sound is currently playing
	//

	if (m_wFlags & SOUND_MIDI) {
		g_engine->_midi->stop();
	} else {
		g_system->getMixer()->stopHandle(m_handle);
		m_handle = {};
	}

	if (m_bInQueue) {
		Assert(m_iQSlot >= 0 && m_iQSlot < NUM_QUEUES);
		m_cQueue[m_iQSlot]->DeleteItem(this);
		m_bInQueue = FALSE;
	}

	m_bPlaying = FALSE;
	m_bStarted = FALSE;

	if (bSuccess) {                     // no longer playing
		m_bPaused = FALSE;

		// One less audio playing
		m_nCount -= 1;
	}

	return bSuccess;
}


void CBofSound::ClearSounds() {
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


void CBofSound::ClearWaveSounds() {
	CBofSound  *pSound, *pNextSound;

	pSound = m_pSoundChain;                     // find this Sound in the queue
	while (pSound != nullptr) {

		pNextSound = pSound->GetNext();
		if ((pSound->m_wFlags & SOUND_WAVE) || (pSound->m_wFlags & SOUND_MIX))
			delete pSound;

		pSound = pNextSound;
	}
}


void CBofSound::ClearMidiSounds() {
	CBofSound  *pSound, *pNextSound;

	pSound = m_pSoundChain;                     // find this Sound is the queue
	while (pSound != nullptr) {

		pNextSound = (CBofSound *)pSound->GetNext();
		if (pSound->m_wFlags & SOUND_MIDI)
			delete pSound;

		pSound = pNextSound;
	}
}


bool CBofSound::SoundAvailable() {
	return m_bSoundAvailable;                     // return requested info
}


bool CBofSound::MidiAvailable() {
	return m_bMidiAvailable;                      // return requested info
}


void CBofSound::WaitSounds() {
	WaitWaveSounds();
	WaitMidiSounds();
}


void CBofSound::WaitWaveSounds() {
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

bool CBofSound::SoundsPlaying() {
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


void CBofSound::WaitMidiSounds() {
	uint32   dwTickCount = 0;
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


bool CBofSound::HandleMessages() {
	Common::Event e;
	while (g_system->getEventManager()->pollEvent(e)) {
		;
	}

	g_system->delayMillis(10);
	if (g_engine->shouldQuit())
		return TRUE;

	return FALSE;
}



bool CBofSound::Sleep(uint32 wait) {
	uint32 goal;

	goal = wait + g_system->getMillis();
	while (goal > g_system->getMillis()) {
		if (HandleMessages())
			return TRUE;
	}
	return FALSE;
}


bool BofPlaySound(const char *pszSoundFile, uint32 nFlags, INT iQSlot) {
	CBofSound *pSound;
	CBofWindow *pWnd;
	bool bSuccess;

	// assume failure
	bSuccess = FALSE;

	if (pszSoundFile != nullptr) {

		nFlags |= SOUND_AUTODELETE;

		if (!FileExists(pszSoundFile)) {

			LogError(BuildString("Warning: Sound File '%s' not found", pszSoundFile));
			return FALSE;
		}

		pWnd = CBofApp::GetApp()->GetMainWindow();

		// take care of any last minute cleanup before we start this new sound
		CBofSound::AudioTask();
		CBofSound::StopWaveSounds();

		if ((pSound = new CBofSound(pWnd, (char *)pszSoundFile, (WORD)nFlags)) != nullptr) {

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

bool BofPlaySoundEx(const char *pszSoundFile, uint32 nFlags, INT iQSlot, bool bWait) {
	CBofSound *pSound;
	CBofWindow *pWnd;
	bool bSuccess;

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

		if ((pSound = new CBofSound(pWnd, (char *)pszSoundFile, (WORD)nFlags)) != nullptr) {

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


bool CBofSound::LoadSound() {
	Assert(IsValidObject(this));
	Assert(m_szFileName[0] != '\0');

	bool bSuccess;

	// assume failure
	bSuccess = FALSE;

	bSuccess = TRUE;
	if (m_pFileBuf == nullptr) {
		bSuccess = FALSE;

		Common::File in;

		if (in.open(m_szFileName)) {
			m_iFileSize = in.size();

			m_pFileBuf = (byte *)malloc(m_iFileSize);

			if (in.read(m_pFileBuf, m_iFileSize) == m_iFileSize)
				bSuccess = TRUE;
		}
	}

	return bSuccess;
}


bool CBofSound::ReleaseSound() {
	Assert(IsValidObject(this));

	if (m_pFileBuf != nullptr) {
		free(m_pFileBuf);
		m_pFileBuf = nullptr;
	}
	return TRUE;
}


void CBofSound::SetDrivePath(const char *pszDrivePath) {
	m_szDrivePath[0] = '\0';
	if (pszDrivePath != nullptr) {
		Common::strlcpy(m_szDrivePath, pszDrivePath, MAX_DIRPATH);
	}
}

void CBofSound::GetDrivePath(char *pszDrivePath) {
	Assert(pszDrivePath != nullptr);

	*pszDrivePath = '\0';
	if (m_szDrivePath[0] != '\0') {
		Common::strlcpy(pszDrivePath, m_szDrivePath, MAX_DIRPATH);
	}
}

bool CBofSound::SoundsPlayingNotOver() {
	CSound *pSound;
	bool bPlaying;

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


bool CBofSound::WaveSoundPlaying() {
	CSound *pSound;
	bool bPlaying;

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


bool CBofSound::MidiSoundPlaying() {
	CSound *pSound;
	bool bPlaying;

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


void CBofSound::AudioTask() {
	static bool bAlready = FALSE;
	CBofSound *pSound;

	// don't allow recursion here
	Assert(!bAlready);

	bAlready = TRUE;

	// walk through sound list, and check for sounds that need attention
	//
	pSound = m_pSoundChain;
	while (pSound != nullptr) {

		if (!pSound->Paused()) {

			if ((pSound->m_wFlags & SOUND_WAVE) || (pSound->m_wFlags & SOUND_MIX)) {

				// Has this sound been played?
				//
				if (pSound->m_bStarted) {

					// And, Is it done?
					//
					if (!g_system->getMixer()->isSoundHandleActive(pSound->m_handle)) {

						// Kill it
						pSound->Stop();
					}

				} else {

					// If this is a Queued sound, and has not already started
					//
					if (pSound->m_bInQueue && !pSound->m_bStarted) {

						// And it is time to play
						//
						if ((CBofSound *)m_cQueue[pSound->m_iQSlot]->GetQItem() == pSound) {

							pSound->PlayWAV();
						}
					}
				}

			} else if (pSound->m_wFlags & SOUND_MIDI) {

				if (pSound->m_bPlaying) {

					// And, Is it done?
					//
					if (!g_engine->_midi->isPlaying()) {

						// Kill it
						pSound->Stop();
					}
				}

			}
		}

		pSound = (CBofSound *)pSound->GetNext();
	}

	bAlready = FALSE;
}

ERROR_CODE CBofSound::PlayWAV() {
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
			Common::SeekableReadStream *stream = new Common::MemoryReadStream(m_pFileBuf, m_iFileSize);
			Audio::AudioStream *audio = Audio::makeLoopingAudioStream(Audio::makeWAVStream(stream, DisposeAfterUse::YES), m_wLoops);
			if (audio == nullptr) {
				ReportError(ERR_UNKNOWN, "Could not allocate audio stream.");
			}

			g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &m_handle, audio, -1, VOLUME_SVM(m_nVol));
			m_bPlaying = TRUE;
			m_bStarted = TRUE;
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
	m_cQueue[nSlot]->Flush();

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

void CBofSound::SetQVol(INT nSlot, INT nVol) {
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

} // namespace Bagel
