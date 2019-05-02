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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "titanic/sound/sound.h"
#include "titanic/game_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

CSoundItem::~CSoundItem() {
	delete _waveFile;
}

/*------------------------------------------------------------------------*/

CSound::CSound(CGameManager *owner, Audio::Mixer *mixer) :
		_gameManager(owner), _soundManager(mixer) {
	g_vm->_movieManager.setSoundManager(&_soundManager);
}

CSound::~CSound() {
	_soundManager.qsWaveMixCloseSession();
	_sounds.destroyContents();
}

void CSound::save(SimpleFile *file) const {
	_soundManager.save(file);
}

void CSound::load(SimpleFile *file) {
	_soundManager.load(file);
}

void CSound::preLoad() {
	_soundManager.preLoad();

	if (_gameManager)
		_gameManager->_musicRoom.destroyMusicHandler();
}

void CSound::preEnterView(CViewItem *newView, bool isNewRoom) {
	CNodeItem *node = newView->findNode();
	double xp, yp, zp;
	node->getPosition(xp, yp, zp);

	double cosVal = cos(newView->_angle);
	double sinVal = -sin(newView->_angle);

	_soundManager.setListenerPosition(xp, yp, zp, cosVal, sinVal, 0, isNewRoom);
}

bool CSound::isActive(int handle) {
	if (handle != 0 && handle != -1)
		return _soundManager.isActive(handle);

	return false;
}

void CSound::setVolume(uint handle, uint volume, uint seconds) {
	_soundManager.setVolume(handle, volume, seconds);
}

void CSound::activateSound(CWaveFile *waveFile, DisposeAfterUse::Flag disposeAfterUse) {
	for (CSoundItemList::iterator i = _sounds.begin(); i != _sounds.end(); ++i) {
		CSoundItem *sound = *i;
		if (sound->_waveFile == waveFile) {
			sound->_active = true;
			sound->_disposeAfterUse = disposeAfterUse;

			// Anything bigger than 50Kb is automatically flagged to be free when finished
			if (waveFile->size() > (50 * 1024))
				sound->_disposeAfterUse = DisposeAfterUse::YES;
			break;
		}
	}
}

void CSound::stopChannel(int channel) {
	_soundManager.stopChannel(channel);
}

void CSound::checkSounds() {
	for (CSoundItemList::iterator i = _sounds.begin(); i != _sounds.end(); ) {
		CSoundItem *soundItem = *i;

		if (soundItem->_active && soundItem->_disposeAfterUse == DisposeAfterUse::YES) {
			if (!_soundManager.isActive(soundItem->_waveFile)) {
				i = _sounds.erase(i);
				delete soundItem;
				continue;
			}
		}

		++i;
	}
}

void CSound::removeOldest() {
	for (CSoundItemList::iterator i = _sounds.reverse_begin();
			i != _sounds.end(); --i) {
		CSoundItem *soundItem = *i;
		if (soundItem->_active && !_soundManager.isActive(soundItem->_waveFile)) {
			_sounds.remove(soundItem);
			delete soundItem;
			break;
		}
	}
}

CWaveFile *CSound::getTrueTalkSound(CDialogueFile *dialogueFile, int index) {
	return loadSpeech(dialogueFile, index);
}

CWaveFile *CSound::loadSound(const CString &name) {
	checkSounds();

	// Check whether an entry for the given name is already active
	for (CSoundItemList::iterator i = _sounds.begin(); i != _sounds.end(); ++i) {
		CSoundItem *soundItem = *i;
		if (soundItem->_name == name) {
			// Found it, so move it to the front of the list and return
			_sounds.remove(soundItem);
			_sounds.push_front(soundItem);
			return soundItem->_waveFile;
		}
	}

	// Create new sound item
	CSoundItem *soundItem = new CSoundItem(name);
	soundItem->_waveFile = _soundManager.loadSound(name);

	if (!soundItem->_waveFile) {
		// Couldn't load sound, so destroy new item and return
		delete soundItem;
		return 0;
	}

	// Add the item to the list of sounds
	_sounds.push_front(soundItem);

	// If there are more than 10 sounds loaded, remove the last one,
	// which is the least recently used of all of them
	if (_sounds.size() > 10)
		removeOldest();

	return soundItem->_waveFile;
}

int CSound::playSound(const CString &name, CProximity &prox) {
	CWaveFile *waveFile  = loadSound(name);
	if (!waveFile)
		return -1;

	prox._soundDuration = waveFile->getDurationTicks();
	if (prox._soundType != Audio::Mixer::kPlainSoundType)
		waveFile->_soundType = prox._soundType;

	activateSound(waveFile, prox._disposeAfterUse);

	return _soundManager.playSound(*waveFile, prox);
}

CWaveFile *CSound::loadSpeech(CDialogueFile *dialogueFile, int speechId) {
	checkSounds();

	// Check whether an entry for the given name is already active
	for (CSoundItemList::iterator i = _sounds.begin(); i != _sounds.end(); ++i) {
		CSoundItem *soundItem = *i;
		if (soundItem->_dialogueFileHandle == dialogueFile->getFile()
				&& soundItem->_speechId == speechId) {
			// Found it, so move it to the front of the list and return
			_sounds.remove(soundItem);
			_sounds.push_front(soundItem);
			return soundItem->_waveFile;
		}
	}

	// Create new sound item
	CSoundItem *soundItem = new CSoundItem(dialogueFile->getFile(), speechId);
	soundItem->_waveFile = _soundManager.loadSpeech(dialogueFile, speechId);

	if (!soundItem->_waveFile) {
		// Couldn't load speech, so destroy new item and return
		delete soundItem;
		return 0;
	}

	// Add the item to the list of sounds
	_sounds.push_front(soundItem);

	// If there are more than 10 sounds loaded, remove the last one,
	// which is the least recently used of all of them
	if (_sounds.size() > 10)
		removeOldest();

	return soundItem->_waveFile;
}

int CSound::playSpeech(CDialogueFile *dialogueFile, int speechId, CProximity &prox) {
	CWaveFile *waveFile = loadSpeech(dialogueFile, speechId);
	if (!waveFile)
		return -1;

	prox._soundDuration = waveFile->getDurationTicks();
	if (prox._soundType != Audio::Mixer::kPlainSoundType)
		waveFile->_soundType = prox._soundType;

	activateSound(waveFile, prox._disposeAfterUse);
	return _soundManager.playSound(*waveFile, prox);
}

void CSound::stopSound(uint handle) {
	_soundManager.stopSound(handle);
}

void CSound::setCanFree(int handle) {
	if (handle != 0 && handle != -1)
		_soundManager.setCanFree(handle);
}

void CSound::updateMixer() {
	_soundManager.waveMixPump();
}

} // End of namespace Titanic
