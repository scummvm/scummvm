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
#include "bagel/boflib/sound.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/string_functions.h"
#include "bagel/boflib/log.h"

namespace Bagel {

#define MEMORY_THRESHOLD    20000L
#define MEMORY_MARGIN       100000L

char    CBofSound::m_szDrivePath[MAX_DIRPATH];
CBofSound  *CBofSound::m_pSoundChain = nullptr;  // Pointer to chain of linked Sounds
int     CBofSound::m_nCount = 0;                 // Count of currently active Sounds
int     CBofSound::m_nWavCount = 0;              // Available wave sound devices
int     CBofSound::m_nMidiCount = 0;             // Available midi sound devices
bool    CBofSound::m_bSoundAvailable = false;    // Whether wave sound is available
bool    CBofSound::m_bMidiAvailable = false;     // Whether midi sound is available
bool    CBofSound::m_bWaveVolume = false;        // Whether wave volume can be set
bool    CBofSound::m_bMidiVolume = false;        // Whether midi volume can be set
CBofWindow   *CBofSound::m_pMainWnd = nullptr;   // Window for message processing

bool    CBofSound::m_bInit = false;

CQueue *CBofSound::m_cQueue[NUM_QUEUES];
int CBofSound::m_nSlotVol[NUM_QUEUES];


CBofSound::CBofSound(CBofWindow *pWnd, const char *pszPathName, uint16 wFlags, const int nLoops) {
	char szTempPath[MAX_DIRPATH];

	// Validate input
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

	m_wLoops = (uint16)nLoops;

	m_bPlaying = false;                 // Not yet playing
	m_bStarted = false;
	m_wFlags = wFlags;                  // Flags for playing
	m_bPaused = false;                  // Not suspended
	m_bExtensionsUsed = false;          // No extended flags used.
	m_szFileName[0] = '\0';

	m_handle = {};
	m_pFileBuf = nullptr;
	m_nVol = VOLUME_INDEX_DEFAULT;
	m_bInQueue = false;
	m_iQSlot = 0;

	int i;
	for (i = 0; i < NUM_QUEUES; i++) {
		m_nSlotVol[i] = VOLUME_INDEX_DEFAULT;
	}

	// Mixed assumes Asynchronous
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

		// Continue as long as this file exists
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

	// Insert this sound into the sound list
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
	m_bSoundAvailable = true;
	m_bMidiAvailable = false;

	for (int i = 0; i < NUM_QUEUES; ++i)
		m_cQueue[i] = new CQueue();

	ResetQVolumes();
}

void CBofSound::ResetQVolumes() {
	// Set Q Volumes to default
	for (int i = 0; i < NUM_QUEUES; i++) {
		m_nSlotVol[i] = VOLUME_INDEX_DEFAULT;
	}
}


void CBofSound::shutdown() {
	// Auto-delete any remaining sounds
	ClearSounds();

	for (int i = 0; i < NUM_QUEUES; ++i)
		delete m_cQueue[i];
}



void CBofSound::SetVolume(int nVolume) {
	Assert(nVolume >= VOLUME_INDEX_MIN && nVolume <= VOLUME_INDEX_MAX);
	int nLocalVolume = nVolume;

	if (nLocalVolume < VOLUME_INDEX_MIN) {
		nLocalVolume = VOLUME_INDEX_MIN;

	} else if (nLocalVolume > VOLUME_INDEX_MAX) {
		nLocalVolume = VOLUME_INDEX_MAX;
	}

	m_nVol = nLocalVolume;

	g_system->getMixer()->setChannelVolume(m_handle, VOLUME_SVM(m_nVol));

	// TODO: MIDI volume
}


void CBofSound::SetVolume(int nMidiVolume, int nWaveVolume) {
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
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, VOLUME_SVM(nWaveVolume));

	if (nMidiVolume < VOLUME_INDEX_MIN) {
		nMidiVolume = VOLUME_INDEX_MIN;

	} else if (nMidiVolume > VOLUME_INDEX_MAX) {
		nMidiVolume = VOLUME_INDEX_MAX;
	}

	// Set master Midi volume
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, VOLUME_SVM(nMidiVolume));
}


bool CBofSound::Play(uint32 dwBeginHere, uint32 TimeFormatFlag) {
	Assert(IsValidObject(this));

	// Assume failure
	bool bSuccess = false;

	if (m_errCode == ERR_NONE) {
		// We must be attached to a valid window
		Assert((m_pWnd != nullptr) || (m_pMainWnd != nullptr));

		// If already playing, then stop and start again
		if (Playing()) {
			// Can't replay an autodelete sound
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

		// Make sure this sound is still valid
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
			m_bPlaying = true;

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
					m_bPlaying = false;
				}
			}

		} else if (m_wFlags & SOUND_MIX) {
			if (!(m_wFlags & SOUND_QUEUE)) {
				PlayWAV();

			} else {
				Assert(m_iQSlot >= 0 && m_iQSlot < NUM_QUEUES);

				m_cQueue[m_iQSlot]->AddItem(this);
				m_bPlaying = true;
				m_bInQueue = true;
				SetVolume(m_nSlotVol[m_iQSlot]);
			}
		}

	}

	return bSuccess;
}


bool CBofSound::MidiLoopPlaySegment(uint32 dwLoopFrom, uint32 dwLoopTo, uint32 dwBegin, uint32 TimeFmt) {
	Assert(IsValidObject(this));

	m_wFlags |= SOUND_LOOP;
	m_dwRePlayStart = dwLoopFrom;
	m_dwRePlayEnd = dwLoopTo;
	m_bExtensionsUsed = true;

	bool bSuccess = Play(dwBegin, TimeFmt);

	return bSuccess;
}


bool CBofSound::PauseSounds() {
	bool bSuccess = true;

	// Thumb through all the sounds
	CBofSound *pSound = m_pSoundChain;
	while (pSound != nullptr) {
		// If one is playing and not paused try to suspend it
		if (pSound->Playing() && (pSound->m_bPaused == false)) {
			bool bStatus = pSound->Pause();
			if (bStatus)
				pSound->m_bPaused = true;
			else
				bSuccess = false;
		}

		pSound = (CBofSound *)pSound->GetNext();
	}

	return bSuccess;
}


bool CBofSound::Pause() {
	Assert(IsValidObject(this));

	bool bSuccess = false;

	// Must be playing to be paused and not already paused
	if (Playing() && (m_bPaused == false)) {
		bSuccess = true;
		if (m_wFlags & SOUND_MIDI) {
			g_engine->_midi->pause();
		} else {
			g_system->getMixer()->pauseHandle(m_handle, true);
		}
	}

	if (bSuccess)
		m_bPaused = true;

	return bSuccess;
}


bool CBofSound::ResumeSounds() {
	bool bSuccess = true;

	CBofSound *pSound = m_pSoundChain;					// Thumb through all the sounds
	while (pSound != nullptr) {
		if (pSound->m_bPaused) {
			// If one is paused
			bool bStatus = pSound->Resume();		// ... try to get it going again
			if (bStatus)
				pSound->m_bPaused = false;	// success
			else
				bSuccess = false;			// failure
		}

		pSound = (CBofSound *)pSound->GetNext();
	}

	return bSuccess;
}


bool CBofSound::Resume() {
	Assert(IsValidObject(this));

	bool bSuccess = false;

	if (m_bPaused) {                        // must be paused to resume
		bSuccess = true;
		if (m_wFlags & SOUND_MIDI) {
			g_engine->_midi->resume();
		} else {
			g_system->getMixer()->pauseHandle(m_handle, false);
		}
	}

	if (bSuccess)
		m_bPaused = false;

	return bSuccess;
}


void CBofSound::StopSounds() {
	CBofSound *pSound = m_pSoundChain;	// Thumb through all the sounds
	while (pSound != nullptr) {
		if (pSound->Playing()) {		// If one is playing
			pSound->m_bPaused = false;	// ... its no longer paused
			pSound->Stop();	// Stop it
		}

		pSound = (CBofSound *)pSound->GetNext();
	}
}


void CBofSound::StopWaveSounds() {
	CBofSound *pSound = m_pSoundChain;			// Find this Sound is the queue
	while (pSound != nullptr) {
		CBofSound *pNextSound = (CBofSound *)pSound->GetNext();

		if (pSound->Playing() && ((pSound->m_wFlags & SOUND_WAVE) || (pSound->m_wFlags & SOUND_MIX))) {
			pSound->Stop();
			if (pSound->m_wFlags & SOUND_AUTODELETE)
				delete pSound;
		}

		pSound = pNextSound;
	}
}


void CBofSound::StopMidiSounds() {
	CBofSound *pSound = m_pSoundChain;			// Find this Sound is the queue
	while (pSound != nullptr) {
		CBofSound *pNextSound = (CBofSound *)pSound->GetNext();

		if (pSound->Playing() && (pSound->m_wFlags & SOUND_MIDI)) {
			pSound->Stop();
			if (pSound->m_wFlags & SOUND_AUTODELETE)
				delete pSound;
		}

		pSound = pNextSound;
	}
}


void CBofSound::Stop() {
	Assert(IsValidObject(this));

	// If this sound is currently playing
	if (m_wFlags & SOUND_MIDI) {
		g_engine->_midi->stop();
	} else {
		g_system->getMixer()->stopHandle(m_handle);
		m_handle = {};
	}

	if (m_bInQueue) {
		Assert(m_iQSlot >= 0 && m_iQSlot < NUM_QUEUES);
		m_cQueue[m_iQSlot]->DeleteItem(this);
		m_bInQueue = false;
	}

	m_bPlaying = false;
	m_bStarted = false;
	m_bPaused = false;

	// One less audio playing
	m_nCount -= 1;
}


void CBofSound::ClearSounds() {
	StopSounds();                               // Stop all active sounds

	CBofSound *pSound = m_pSoundChain;
	while (pSound != nullptr) {                 // Delete all sound entries
		CBofSound *pNextSound = pSound->GetNext();
		delete pSound;
		pSound = pNextSound;
	}

	Assert(m_pSoundChain == nullptr);
}


void CBofSound::ClearWaveSounds() {

	CBofSound *pSound = m_pSoundChain;                     // Find this Sound in the queue
	while (pSound != nullptr) {
		CBofSound *pNextSound = pSound->GetNext();
		if ((pSound->m_wFlags & SOUND_WAVE) || (pSound->m_wFlags & SOUND_MIX))
			delete pSound;

		pSound = pNextSound;
	}
}


void CBofSound::ClearMidiSounds() {
	CBofSound *pSound = m_pSoundChain;                     // find this Sound is the queue
	while (pSound != nullptr) {
		CBofSound *pNextSound = (CBofSound *)pSound->GetNext();

		if (pSound->m_wFlags & SOUND_MIDI)
			delete pSound;

		pSound = pNextSound;
	}
}


bool CBofSound::SoundAvailable() {
	return m_bSoundAvailable;                     // Return requested info
}


bool CBofSound::MidiAvailable() {
	return m_bMidiAvailable;                      // Return requested info
}


void CBofSound::WaitSounds() {
	WaitWaveSounds();
	WaitMidiSounds();
}


void CBofSound::WaitWaveSounds() {
	uint32 dwTickCount = 0;

	for (;;) {
		AudioTask();

		CBofSound *pSound = m_pSoundChain;
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
		if (dwTickCount == 0)
			dwTickCount = g_system->getMillis() + 1000 * 60 * 3;

		if (g_system->getMillis() > dwTickCount) {
			pSound->Stop();
			pSound->m_bPlaying = false;

			pSound->ReportError(ERR_UNKNOWN, "CBofSound::WaitWaveSounds() timeout!");
		}
	}
}

bool CBofSound::SoundsPlaying() {
	AudioTask();

	CBofSound *pSound = m_pSoundChain;
	while (pSound != nullptr) {
		if (pSound->Playing() && (pSound->m_wFlags & SOUND_WAVE || pSound->m_wFlags & SOUND_MIX)) {
			return true;
		}
		pSound = (CBofSound *)pSound->GetNext();
	}

	return false;
}


void CBofSound::WaitMidiSounds() {
	uint32 dwTickCount = 0;

	for (;;) {
		CBofSound *pSound = m_pSoundChain;
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
	}

	g_system->delayMillis(10);
	if (g_engine->shouldQuit())
		return true;

	return false;
}



bool CBofSound::Sleep(uint32 wait) {
	uint32 goal = wait + g_system->getMillis();
	while (goal > g_system->getMillis()) {
		if (HandleMessages())
			return true;
	}

	return false;
}


bool BofPlaySound(const char *pszSoundFile, uint32 nFlags, int iQSlot) {
	// Assume failure
	bool bSuccess = false;

	if (pszSoundFile != nullptr) {
		nFlags |= SOUND_AUTODELETE;

		if (!FileExists(pszSoundFile)) {
			LogError(BuildString("Warning: Sound File '%s' not found", pszSoundFile));
			return false;
		}

		CBofWindow *pWnd = CBofApp::GetApp()->GetMainWindow();

		// Take care of any last minute cleanup before we start this new sound
		CBofSound::AudioTask();
		CBofSound::StopWaveSounds();

		CBofSound *pSound = new CBofSound(pWnd, (char *)pszSoundFile, (uint16)nFlags);
		if (pSound != nullptr) {
			if ((nFlags & SOUND_QUEUE) == SOUND_QUEUE) {
				pSound->SetQSlot(iQSlot);
			}

			bSuccess = pSound->Play();
		}

	} else {
		bSuccess = true;
		CBofSound::StopWaveSounds();
	}

	return bSuccess;
}

bool BofPlaySoundEx(const char *pszSoundFile, uint32 nFlags, int iQSlot, bool bWait) {
	// Assume failure
	bool bSuccess = false;

	if (pszSoundFile != nullptr) {
		if ((nFlags & SOUND_MIX) == 0) {
			bWait = false;
		}

		if (!bWait) {
			nFlags |= SOUND_AUTODELETE;
		}

		if (!FileExists(pszSoundFile)) {
			LogError(BuildString("Warning: Sound File '%s' not found", pszSoundFile));
			return false;
		}

		CBofWindow *pWnd = CBofApp::GetApp()->GetMainWindow();

		// Take care of any last minute cleanup before we start this new sound
		CBofSound::AudioTask();

		CBofSound *pSound = new CBofSound(pWnd, (char *)pszSoundFile, (uint16)nFlags);
		if (pSound != nullptr) {
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

	// Assume failure
	bool bSuccess = false;

	bSuccess = true;
	if (m_pFileBuf == nullptr) {
		bSuccess = false;

		Common::File in;

		if (in.open(m_szFileName)) {
			m_iFileSize = in.size();

			m_pFileBuf = (byte *)malloc(m_iFileSize);

			if (in.read(m_pFileBuf, m_iFileSize) == m_iFileSize)
				bSuccess = true;
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

	return true;
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
	// Assume no wave sounds are playing
	bool bPlaying = false;

	// Walk through sound list, and check for sounds that need attention
	CSound *pSound = m_pSoundChain;
	while (pSound != nullptr) {
		if (pSound->Playing() &&
		        (pSound->m_wFlags & SOUND_WAVE || pSound->m_wFlags & SOUND_MIX) &&
		        !(pSound->m_wFlags & SOUND_OVEROK)) {
			bPlaying = true;
			break;
		}

		pSound = (CBofSound *)pSound->GetNext();
	}

	return bPlaying;
}


bool CBofSound::WaveSoundPlaying() {
	// Assume no wave sounds are playing
	bool bPlaying = false;

	// Walk through sound list, and check for sounds that need attention
	CSound *pSound = m_pSoundChain;
	while (pSound != nullptr) {
		if (pSound->Playing() && (pSound->m_wFlags & SOUND_WAVE || pSound->m_wFlags & SOUND_MIX)) {
			bPlaying = true;
			break;
		}

		pSound = (CBofSound *)pSound->GetNext();
	}

	return bPlaying;
}


bool CBofSound::MidiSoundPlaying() {
	// Assume no wave sounds are playing
	bool bPlaying = false;

	// Walk through sound list, and check for sounds that need attention
	CSound *pSound = m_pSoundChain;
	while (pSound != nullptr) {
		if (pSound->Playing() && (pSound->m_wFlags & SOUND_MIDI)) {
			bPlaying = true;
			break;
		}

		pSound = (CBofSound *)pSound->GetNext();
	}

	return bPlaying;
}


void CBofSound::AudioTask() {
	static bool bAlready = false;

	// Don't allow recursion here
	Assert(!bAlready);

	bAlready = true;

	// Walk through sound list, and check for sounds that need attention
	CBofSound *pSound = m_pSoundChain;
	while (pSound != nullptr) {
		if (!pSound->Paused()) {
			if ((pSound->m_wFlags & SOUND_WAVE) || (pSound->m_wFlags & SOUND_MIX)) {

				// Has this sound been played?
				if (pSound->m_bStarted) {
					// And, Is it done?
					if (!g_system->getMixer()->isSoundHandleActive(pSound->m_handle)) {
						// Kill it
						pSound->Stop();
					}

				} else {

					// If this is a Queued sound, and has not already started
					if (pSound->m_bInQueue && !pSound->m_bStarted) {
						// And it is time to play
						if ((CBofSound *)m_cQueue[pSound->m_iQSlot]->GetQItem() == pSound) {
							pSound->PlayWAV();
						}
					}
				}

			} else if (pSound->m_wFlags & SOUND_MIDI) {
				if (pSound->m_bPlaying) {
					// And, Is it done?
					if (!g_engine->_midi->isPlaying()) {
						// Kill it
						pSound->Stop();
					}
				}
			}
		}

		pSound = (CBofSound *)pSound->GetNext();
	}

	bAlready = false;
}

ErrorCode CBofSound::PlayWAV() {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {
		// If it's not yet loaded, then load it now
		if (m_pFileBuf == nullptr) {
			LoadSound();
		}

		Assert(m_pFileBuf != nullptr);

		if (m_pFileBuf != nullptr) {
			if (m_bInQueue) {
				SetVolume(m_nSlotVol[m_iQSlot]);
			}

			// Then, Play it
			Common::SeekableReadStream *stream = new Common::MemoryReadStream(m_pFileBuf, m_iFileSize);
			Audio::AudioStream *audio = Audio::makeLoopingAudioStream(Audio::makeWAVStream(stream, DisposeAfterUse::YES), m_wLoops);

			if (audio == nullptr) {
				ReportError(ERR_UNKNOWN, "Could not allocate audio stream.");
			}

			g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &m_handle, audio, -1, VOLUME_SVM(m_nVol));
			m_bPlaying = true;
			m_bStarted = true;
		}
	}

	return m_errCode;
}


ErrorCode CBofSound::FlushQueue(int nSlot) {
	Assert(nSlot >= 0 && nSlot < NUM_QUEUES);

	// Assume no error
	ErrorCode errCode = ERR_NONE;

	// Remove all queued sounds
	m_cQueue[nSlot]->Flush();

	// Including any that are currently playing
	CBofSound *pSound = m_pSoundChain;
	while (pSound != nullptr) {
		// Prefetch next sound in case Stop() deletes this one
		CBofSound *pNextSound = pSound->GetNext();

		// If this sound is playing from specified queue
		if (pSound->IsPlaying() && pSound->m_bInQueue && pSound->m_iQSlot == nSlot) {
			// Then chuck it
			pSound->Stop();
		}

		// Next
		pSound = pNextSound;
	}

	return errCode;
}

void CBofSound::SetQVol(int nSlot, int nVol) {
	// Validate input
	Assert(nSlot >= 0 && nSlot < NUM_QUEUES);
	Assert(nVol >= 0 && nVol <= VOLUME_INDEX_MAX);

	m_nSlotVol[nSlot] = nVol;

	// Set all Queued sounds in specified slot to this volume
	CBofSound *pSound = m_pSoundChain;
	while (pSound != nullptr) {
		if (pSound->m_bInQueue && pSound->m_iQSlot == nSlot) {
			pSound->SetVolume(nVol);
		}
		pSound = pSound->GetNext();
	}
}

} // namespace Bagel
