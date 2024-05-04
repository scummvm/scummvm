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

char    CBofSound::_szDrivePath[MAX_DIRPATH];
CBofSound  *CBofSound::_pSoundChain = nullptr;  // Pointer to chain of linked Sounds
int     CBofSound::_nCount = 0;                 // Count of currently active Sounds
int     CBofSound::_nWavCount = 0;              // Available wave sound devices
int     CBofSound::_nMidiCount = 0;             // Available midi sound devices
bool    CBofSound::_bsoundAvailable = false;    // Whether wave sound is available
bool    CBofSound::_bmidiAvailable = false;     // Whether midi sound is available
bool    CBofSound::_bWaveVolume = false;        // Whether wave volume can be set
bool    CBofSound::_bMidiVolume = false;        // Whether midi volume can be set
CBofWindow   *CBofSound::_pMainWnd = nullptr;   // Window for message processing

bool    CBofSound::_bInit = false;

CQueue *CBofSound::_cQueue[NUM_QUEUES];
int CBofSound::_nSlotVol[NUM_QUEUES];


CBofSound::CBofSound(CBofWindow *pWnd, const char *pszPathName, uint16 wFlags, const int nLoops) {
	// Validate input
	assert(pszPathName != nullptr);
	assert(strlen(pszPathName) < MAX_FNAME);

	//
	// Initialize data fields
	//

	_pWnd = _pMainWnd;
	if (pWnd != nullptr) {
		_pWnd = pWnd;
		if (_pMainWnd == nullptr)
			_pMainWnd = pWnd;
	}

	_wLoops = (uint16)nLoops;

	_bPlaying = false;                 // Not yet playing
	_bStarted = false;
	_wFlags = wFlags;                  // Flags for playing
	_bPaused = false;                  // Not suspended
	_bExtensionsUsed = false;          // No extended flags used.
	_szFileName[0] = '\0';

	_handle = {};
	_pFileBuf = nullptr;
	_nVol = VOLUME_INDEX_DEFAULT;
	_bInQueue = false;
	_iQSlot = 0;

	int i;
	for (i = 0; i < NUM_QUEUES; i++) {
		_nSlotVol[i] = VOLUME_INDEX_DEFAULT;
	}

	// Mixed assumes Asynchronous
	if ((_wFlags & SOUND_MIX) == SOUND_MIX) {
		_wFlags |= SOUND_ASYNCH;
	}

	if (_wFlags & SOUND_MIDI) {
		_chType = SOUND_TYPE_XM;
	} else {
		_chType = SOUND_TYPE_WAV;
	}

	if (pszPathName != nullptr) {

		if ((_szDrivePath[0] != '\0') && (*pszPathName == '.'))
			pszPathName++;

		char szTempPath[MAX_DIRPATH];
		snprintf(szTempPath, MAX_DIRPATH, "%s%s", _szDrivePath, pszPathName);
		StrReplaceStr(szTempPath, "\\\\", "\\");

		// Continue as long as this file exists
		if (fileExists(szTempPath)) {
			fileGetFullPath(_szFileName, szTempPath);

			if (!(_wFlags & SOUND_QUEUE)) {
				if (_wFlags & SOUND_WAVE || _wFlags & SOUND_MIX) {
					loadSound();
				}
			}

		} else {
			// Try both MIDI formats
			if (_wFlags & SOUND_MIDI) {
				StrReplaceStr(szTempPath, ".MID", ".MOV");
				if (fileExists(szTempPath)) {
					fileGetFullPath(_szFileName, szTempPath);
					_chType = SOUND_TYPE_QT;
				} else {
					reportError(ERR_FFIND, szTempPath);
				}
			} else {
				reportError(ERR_FFIND, szTempPath);
			}
		}
	}

	// Insert this sound into the sound list
	if (_pSoundChain != nullptr) {
		_pSoundChain->Insert(this);

		// _pSoundchain must always be the head of the list
		assert(_pSoundChain == _pSoundChain->getHead());
	} else {
		_pSoundChain = this;
	}
}


CBofSound::~CBofSound() {
	assert(isValidObject(this));

	stop();
	releaseSound();

	if (this == _pSoundChain) {              // special case head of chain

		_pSoundChain = (CBofSound *)_pSoundChain->getNext();
	}
}


void CBofSound::initialize() {
	_bsoundAvailable = true;
	_bmidiAvailable = false;

	for (int i = 0; i < NUM_QUEUES; ++i)
		_cQueue[i] = new CQueue();

	resetQVolumes();
}

void CBofSound::resetQVolumes() {
	// Set Q Volumes to default
	for (int i = 0; i < NUM_QUEUES; i++) {
		_nSlotVol[i] = VOLUME_INDEX_DEFAULT;
	}
}


void CBofSound::shutdown() {
	// Auto-delete any remaining sounds
	clearSounds();

	for (int i = 0; i < NUM_QUEUES; ++i)
		delete _cQueue[i];
}



void CBofSound::setVolume(int nVolume) {
	assert(nVolume >= VOLUME_INDEX_MIN && nVolume <= VOLUME_INDEX_MAX);
	int nLocalVolume = nVolume;

	if (nLocalVolume < VOLUME_INDEX_MIN) {
		nLocalVolume = VOLUME_INDEX_MIN;

	} else if (nLocalVolume > VOLUME_INDEX_MAX) {
		nLocalVolume = VOLUME_INDEX_MAX;
	}

	_nVol = nLocalVolume;

	g_system->getMixer()->setChannelVolume(_handle, VOLUME_SVM(_nVol));

	// TODO: MIDI volume
}


void CBofSound::setVolume(int nMidiVolume, int nWaveVolume) {
	assert(nMidiVolume >= VOLUME_INDEX_MIN && nMidiVolume <= VOLUME_INDEX_MAX);
	assert(nWaveVolume >= VOLUME_INDEX_MIN && nWaveVolume <= VOLUME_INDEX_MAX);

    assert(nMidiVolume >= VOLUME_INDEX_MIN && nMidiVolume <= VOLUME_INDEX_MAX);
	assert(nWaveVolume >= VOLUME_INDEX_MIN && nWaveVolume <= VOLUME_INDEX_MAX);

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
	g_engine->_midi->setVolume(VOLUME_SVM(nMidiVolume));
}


bool CBofSound::play(uint32 dwBeginHere, uint32 TimeFormatFlag) {
	assert(isValidObject(this));

	// Assume failure
	bool bSuccess = false;

	if (_errCode == ERR_NONE) {
		// We must be attached to a valid window
		assert((_pWnd != nullptr) || (_pMainWnd != nullptr));

		// If already playing, then stop and start again
		if (playing()) {
			// Can't replay an autodelete sound
			assert(!(_wFlags & SOUND_AUTODELETE));

			stop();
		}

		// WAVE and MIX are mutually exclusive
		assert(!((_wFlags & SOUND_WAVE) && (_wFlags & SOUND_MIX)));

		if (_wFlags & SOUND_WAVE) {
			if (_wFlags & SOUND_QUEUE)
				waitWaveSounds();
			else
				stopWaveSounds();

		} else if (_wFlags & SOUND_MIDI) {

			if (_wFlags & SOUND_QUEUE)
				waitMidiSounds();
			else
				stopMidiSounds();
		}

		// Make sure this sound is still valid
		assert(_pSoundChain != nullptr);

		if (_pFileBuf == nullptr) {
			if ((_wFlags & (SOUND_MIX | SOUND_QUEUE)) == (SOUND_MIX | SOUND_QUEUE)) {
				// Don't pre-load it

			} else {
				loadSound();
			}
		}

		if (_wFlags & SOUND_MIDI) {

			g_engine->_midi->play(this);
			_bPlaying = true;

		} else if (_wFlags & SOUND_WAVE) {

			playWAV();

			if (_bPlaying) {

				if (!(_wFlags & SOUND_ASYNCH)) {
					EventLoop eventLoop;
					auto *mixer = g_system->getMixer();

					while (mixer->isSoundHandleActive(_handle)) {
						if (eventLoop.frame()) {
							stop();
							break;
						}
					}

					_handle = {};
					_bPlaying = false;
				}
			}

		} else if (_wFlags & SOUND_MIX) {
			if (!(_wFlags & SOUND_QUEUE)) {
				playWAV();

			} else {
				assert(_iQSlot >= 0 && _iQSlot < NUM_QUEUES);

				_cQueue[_iQSlot]->addItem(this);
				_bPlaying = true;
				_bInQueue = true;
				setVolume(_nSlotVol[_iQSlot]);
			}
		}

	}

	return bSuccess;
}


bool CBofSound::midiLoopPlaySegment(uint32 dwLoopFrom, uint32 dwLoopTo, uint32 dwBegin, uint32 TimeFmt) {
	assert(isValidObject(this));

	_wFlags |= SOUND_LOOP;
	_dwRePlayStart = dwLoopFrom;
	_dwRePlayEnd = dwLoopTo;
	_bExtensionsUsed = true;

	bool bSuccess = play(dwBegin, TimeFmt);

	return bSuccess;
}


bool CBofSound::pauseSounds() {
	bool bSuccess = true;

	// Thumb through all the sounds
	CBofSound *pSound = _pSoundChain;
	while (pSound != nullptr) {
		// If one is playing and not paused try to suspend it
		if (pSound->playing() && (pSound->_bPaused == false)) {
			bool bStatus = pSound->pause();
			if (bStatus)
				pSound->_bPaused = true;
			else
				bSuccess = false;
		}

		pSound = (CBofSound *)pSound->getNext();
	}

	return bSuccess;
}


bool CBofSound::pause() {
	assert(isValidObject(this));

	bool bSuccess = false;

	// Must be playing to be paused and not already paused
	if (playing() && (_bPaused == false)) {
		bSuccess = true;
		if (_wFlags & SOUND_MIDI) {
			g_engine->_midi->pause();
		} else {
			g_system->getMixer()->pauseHandle(_handle, true);
		}
	}

	if (bSuccess)
		_bPaused = true;

	return bSuccess;
}


bool CBofSound::resumeSounds() {
	bool bSuccess = true;

	CBofSound *pSound = _pSoundChain;					// Thumb through all the sounds
	while (pSound != nullptr) {
		if (pSound->_bPaused) {
			// If one is paused
			bool bStatus = pSound->resume();		// ... try to get it going again
			if (bStatus)
				pSound->_bPaused = false;	// success
			else
				bSuccess = false;			// failure
		}

		pSound = (CBofSound *)pSound->getNext();
	}

	return bSuccess;
}


bool CBofSound::resume() {
	assert(isValidObject(this));

	bool bSuccess = false;

	if (_bPaused) {                        // must be paused to resume
		bSuccess = true;
		if (_wFlags & SOUND_MIDI) {
			g_engine->_midi->resume();
		} else {
			g_system->getMixer()->pauseHandle(_handle, false);
		}
	}

	if (bSuccess)
		_bPaused = false;

	return bSuccess;
}


void CBofSound::stopSounds() {
	CBofSound *pSound = _pSoundChain;	// Thumb through all the sounds
	while (pSound != nullptr) {
		if (pSound->playing()) {		// If one is playing
			pSound->_bPaused = false;	// ... its no longer paused
			pSound->stop();	// Stop it
		}

		pSound = (CBofSound *)pSound->getNext();
	}
}


void CBofSound::stopWaveSounds() {
	CBofSound *pSound = _pSoundChain;			// Find this Sound is the queue
	while (pSound != nullptr) {
		CBofSound *pNextSound = (CBofSound *)pSound->getNext();

		if (pSound->playing() && ((pSound->_wFlags & SOUND_WAVE) || (pSound->_wFlags & SOUND_MIX))) {
			pSound->stop();
			if (pSound->_wFlags & SOUND_AUTODELETE)
				delete pSound;
		}

		pSound = pNextSound;
	}
}


void CBofSound::stopMidiSounds() {
	CBofSound *pSound = _pSoundChain;			// Find this Sound is the queue
	while (pSound != nullptr) {
		CBofSound *pNextSound = (CBofSound *)pSound->getNext();

		if (pSound->playing() && (pSound->_wFlags & SOUND_MIDI)) {
			pSound->stop();
			if (pSound->_wFlags & SOUND_AUTODELETE)
				delete pSound;
		}

		pSound = pNextSound;
	}
}


void CBofSound::stop() {
	assert(isValidObject(this));

	// If this sound is currently playing
	if (_wFlags & SOUND_MIDI) {
		g_engine->_midi->stop();
	} else {
		g_system->getMixer()->stopHandle(_handle);
		_handle = {};
	}

	if (_bInQueue) {
		assert(_iQSlot >= 0 && _iQSlot < NUM_QUEUES);
		_cQueue[_iQSlot]->deleteItem(this);
		_bInQueue = false;
	}

	_bPlaying = false;
	_bStarted = false;
	_bPaused = false;

	// One less audio playing
	_nCount -= 1;
}


void CBofSound::clearSounds() {
	stopSounds();                               // Stop all active sounds

	CBofSound *pSound = _pSoundChain;
	while (pSound != nullptr) {                 // Delete all sound entries
		CBofSound *pNextSound = pSound->getNext();
		delete pSound;
		pSound = pNextSound;
	}

	assert(_pSoundChain == nullptr);
}


void CBofSound::clearWaveSounds() {

	CBofSound *pSound = _pSoundChain;                     // Find this Sound in the queue
	while (pSound != nullptr) {
		CBofSound *pNextSound = pSound->getNext();
		if ((pSound->_wFlags & SOUND_WAVE) || (pSound->_wFlags & SOUND_MIX))
			delete pSound;

		pSound = pNextSound;
	}
}


void CBofSound::clearMidiSounds() {
	CBofSound *pSound = _pSoundChain;                     // find this Sound is the queue
	while (pSound != nullptr) {
		CBofSound *pNextSound = (CBofSound *)pSound->getNext();

		if (pSound->_wFlags & SOUND_MIDI)
			delete pSound;

		pSound = pNextSound;
	}
}


bool CBofSound::soundAvailable() {
	return _bsoundAvailable;                     // Return requested info
}


bool CBofSound::midiAvailable() {
	return _bmidiAvailable;                      // Return requested info
}


void CBofSound::waitSounds() {
	waitWaveSounds();
	waitMidiSounds();
}


void CBofSound::waitWaveSounds() {
	uint32 dwTickCount = 0;

	for (;;) {
		audioTask();

		CBofSound *pSound = _pSoundChain;
		while (pSound != nullptr) {
			if (pSound->playing() && (pSound->_wFlags & SOUND_WAVE || pSound->_wFlags & SOUND_MIX)) {
				break;
			}
			pSound = (CBofSound *)pSound->getNext();
		}

		if (pSound == nullptr)
			break;

		if (handleMessages())
			break;

		// If 3 minutes go by, then just bolt
		if (dwTickCount == 0)
			dwTickCount = g_system->getMillis() + 1000 * 60 * 3;

		if (g_system->getMillis() > dwTickCount) {
			pSound->stop();
			pSound->_bPlaying = false;

			pSound->reportError(ERR_UNKNOWN, "CBofSound::waitWaveSounds() timeout!");
		}
	}
}

bool CBofSound::soundsPlaying() {
	audioTask();

	CBofSound *pSound = _pSoundChain;
	while (pSound != nullptr) {
		if (pSound->playing() && (pSound->_wFlags & SOUND_WAVE || pSound->_wFlags & SOUND_MIX)) {
			return true;
		}
		pSound = (CBofSound *)pSound->getNext();
	}

	return false;
}


void CBofSound::waitMidiSounds() {
	uint32 dwTickCount = 0;

	for (;;) {
		CBofSound *pSound = _pSoundChain;
		while (pSound != nullptr) {
			if (pSound->playing() && (pSound->_wFlags & SOUND_MIDI)) {
				break;
			}
			pSound = (CBofSound *)pSound->getNext();
		}

		if (pSound == nullptr)
			break;

		if (handleMessages())
			break;

		if (dwTickCount == 0)
			dwTickCount = g_system->getMillis() + 1000 * 60;

		if (g_system->getMillis() > dwTickCount) {
			pSound->stop();

			pSound->reportError(ERR_UNKNOWN, "CBofSound::waitMidiSounds() timeout");
			break;
		}
	}
}


bool CBofSound::handleMessages() {
	Common::Event e;
	while (g_system->getEventManager()->pollEvent(e)) {
	}

	g_system->delayMillis(10);
	if (g_engine->shouldQuit())
		return true;

	return false;
}



bool CBofSound::bofSleep(uint32 wait) {
	uint32 goal = wait + g_system->getMillis();
	while (goal > g_system->getMillis()) {
		if (handleMessages())
			return true;
	}

	return false;
}


bool BofPlaySound(const char *pszSoundFile, uint32 nFlags, int iQSlot) {
	// Assume failure
	bool bSuccess = false;

	if (pszSoundFile != nullptr) {
		nFlags |= SOUND_AUTODELETE;

		if (!fileExists(pszSoundFile)) {
			logError(buildString("Warning: Sound File '%s' not found", pszSoundFile));
			return false;
		}

		CBofWindow *pWnd = CBofApp::getApp()->getMainWindow();

		// Take care of any last minute cleanup before we start this new sound
		CBofSound::audioTask();
		CBofSound::stopWaveSounds();

		CBofSound *pSound = new CBofSound(pWnd, pszSoundFile, (uint16)nFlags);
		if (pSound != nullptr) {
			if ((nFlags & SOUND_QUEUE) == SOUND_QUEUE) {
				pSound->setQSlot(iQSlot);
			}

			bSuccess = pSound->play();
		}

	} else {
		bSuccess = true;
		CBofSound::stopWaveSounds();
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

		if (!fileExists(pszSoundFile)) {
			logError(buildString("Warning: Sound File '%s' not found", pszSoundFile));
			return false;
		}

		CBofWindow *pWnd = CBofApp::getApp()->getMainWindow();

		// Take care of any last minute cleanup before we start this new sound
		CBofSound::audioTask();

		CBofSound *pSound = new CBofSound(pWnd, pszSoundFile, (uint16)nFlags);
		if (pSound != nullptr) {
			if ((nFlags & SOUND_QUEUE) == SOUND_QUEUE) {
				pSound->setQSlot(iQSlot);
			}

			bSuccess = pSound->play();

			if (bWait) {
				while (pSound->isPlaying()) {
					CBofSound::audioTask();
				}
				delete pSound;
			}
		}
	}

	return bSuccess;
}


bool CBofSound::loadSound() {
	assert(isValidObject(this));
	assert(_szFileName[0] != '\0');

	// Assume failure
	bool bSuccess = false;

	bSuccess = true;
	if (_pFileBuf == nullptr) {
		bSuccess = false;

		Common::File in;

		if (in.open(_szFileName)) {
			_iFileSize = in.size();

			_pFileBuf = (byte *)malloc(_iFileSize);

			if (in.read(_pFileBuf, _iFileSize) == _iFileSize)
				bSuccess = true;
		}
	}

	return bSuccess;
}


bool CBofSound::releaseSound() {
	assert(isValidObject(this));

	if (_pFileBuf != nullptr) {
		free(_pFileBuf);
		_pFileBuf = nullptr;
	}

	return true;
}


bool CBofSound::soundsPlayingNotOver() {
	// Assume no wave sounds are playing
	bool bPlaying = false;

	// Walk through sound list, and check for sounds that need attention
	CSound *pSound = _pSoundChain;
	while (pSound != nullptr) {
		if (pSound->playing() &&
		        (pSound->_wFlags & SOUND_WAVE || pSound->_wFlags & SOUND_MIX) &&
		        !(pSound->_wFlags & SOUND_OVEROK)) {
			bPlaying = true;
			break;
		}

		pSound = (CBofSound *)pSound->getNext();
	}

	return bPlaying;
}


bool CBofSound::waveSoundPlaying() {
	// Assume no wave sounds are playing
	bool bPlaying = false;

	// Walk through sound list, and check for sounds that need attention
	CSound *pSound = _pSoundChain;
	while (pSound != nullptr) {
		if (pSound->playing() && (pSound->_wFlags & SOUND_WAVE || pSound->_wFlags & SOUND_MIX)) {
			bPlaying = true;
			break;
		}

		pSound = (CBofSound *)pSound->getNext();
	}

	return bPlaying;
}


bool CBofSound::midiSoundPlaying() {
	// Assume no wave sounds are playing
	bool bPlaying = false;

	// Walk through sound list, and check for sounds that need attention
	CSound *pSound = _pSoundChain;
	while (pSound != nullptr) {
		if (pSound->playing() && (pSound->_wFlags & SOUND_MIDI)) {
			bPlaying = true;
			break;
		}

		pSound = (CBofSound *)pSound->getNext();
	}

	return bPlaying;
}


void CBofSound::audioTask() {
	static bool bAlready = false;

	// Don't allow recursion here
	assert(!bAlready);

	bAlready = true;

	// Walk through sound list, and check for sounds that need attention
	CBofSound *pSound = _pSoundChain;
	while (pSound != nullptr) {
		if (!pSound->paused()) {
			if ((pSound->_wFlags & SOUND_WAVE) || (pSound->_wFlags & SOUND_MIX)) {

				// Has this sound been played?
				if (pSound->_bStarted) {
					// And, Is it done?
					if (!g_system->getMixer()->isSoundHandleActive(pSound->_handle)) {
						// Kill it
						pSound->stop();
					}

				} else {

					// If this is a Queued sound, and has not already started
					if (pSound->_bInQueue && !pSound->_bStarted) {
						// And it is time to play
						if ((CBofSound *)_cQueue[pSound->_iQSlot]->getQItem() == pSound) {
							pSound->playWAV();
						}
					}
				}

			} else if (pSound->_wFlags & SOUND_MIDI) {
				if (pSound->_bPlaying) {
					// And, Is it done?
					if (!g_engine->_midi->isPlaying()) {
						// Kill it
						pSound->stop();
					}
				}
			}
		}

		pSound = (CBofSound *)pSound->getNext();
	}

	bAlready = false;
}

ErrorCode CBofSound::playWAV() {
	assert(isValidObject(this));

	if (!errorOccurred()) {
		// If it's not yet loaded, then load it now
		if (_pFileBuf == nullptr) {
			loadSound();
		}

		assert(_pFileBuf != nullptr);

		if (_pFileBuf != nullptr) {
			if (_bInQueue) {
				setVolume(_nSlotVol[_iQSlot]);
			}

			// Then, Play it
			Common::SeekableReadStream *stream = new Common::MemoryReadStream(_pFileBuf, _iFileSize);
			Audio::AudioStream *audio = Audio::makeLoopingAudioStream(Audio::makeWAVStream(stream, DisposeAfterUse::YES), _wLoops);

			if (audio == nullptr) {
				reportError(ERR_UNKNOWN, "Could not allocate audio stream.");
			}

			g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_handle, audio, -1, VOLUME_SVM(_nVol));
			_bPlaying = true;
			_bStarted = true;
		}
	}

	return _errCode;
}


ErrorCode CBofSound::flushQueue(int nSlot) {
	assert(nSlot >= 0 && nSlot < NUM_QUEUES);

	// Assume no error
	ErrorCode errCode = ERR_NONE;

	// Remove all queued sounds
	_cQueue[nSlot]->flush();

	// Including any that are currently playing
	CBofSound *pSound = _pSoundChain;
	while (pSound != nullptr) {
		// Prefetch next sound in case stop() deletes this one
		CBofSound *pNextSound = pSound->getNext();

		// If this sound is playing from specified queue
		if (pSound->isPlaying() && pSound->_bInQueue && pSound->_iQSlot == nSlot) {
			// Then chuck it
			pSound->stop();
		}

		// Next
		pSound = pNextSound;
	}

	return errCode;
}

void CBofSound::setQVol(int nSlot, int nVol) {
	// Validate input
	assert(nSlot >= 0 && nSlot < NUM_QUEUES);
	assert(nVol >= 0 && nVol <= VOLUME_INDEX_MAX);

	_nSlotVol[nSlot] = nVol;

	// Set all Queued sounds in specified slot to this volume
	CBofSound *pSound = _pSoundChain;
	while (pSound != nullptr) {
		if (pSound->_bInQueue && pSound->_iQSlot == nSlot) {
			pSound->setVolume(nVol);
		}
		pSound = pSound->getNext();
	}
}

} // namespace Bagel
