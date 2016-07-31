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

int CSoundItem::fn1() {
	// TODO
	return 0;
}

CSound::CSound(CGameManager *owner) : _gameManager(owner) {
	g_vm->_movieManager.setSoundManager(&_soundManager);
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
		_gameManager->_musicRoom.preLoad();
}

void CSound::preEnterView(CViewItem *newView, bool isNewRoom) {
	warning("CSound::preEnterView");
}

bool CSound::fn1(int val) {
	if (val == 0 || val == -1) {
		if (!_soundManager.proc14())
			return true;
	}

	return false;
}

void CSound::fn2(int handle) {
	warning("TODO: CSound::fn3");
}

void CSound::fn3(int handle, int val2, int val3) {
	warning("TODO: CSound::fn3");
}

int CSound::playSpeech(CDialogueFile *dialogueFile, int speechId, const CProximity &prox) {
	warning("TODO: CSound::playSpeech");
	return 0;
}

uint CSound::loadSound(const CString &name) {
	checkSounds();

	// Check whether an entry for the given name is already active
	for (CSoundItemList::iterator i = _sounds.begin(); i != _sounds.end(); ++i) {
		CSoundItem *soundItem = *i;
		if (soundItem->_name == name) {
			// Found it, so move it to the front of the list and return
			_sounds.remove(soundItem);
			_sounds.push_front(soundItem);
			return soundItem->_soundHandle;
		}
	}

	// Create new sound item
	CSoundItem *soundItem = new CSoundItem(name);
	soundItem->_soundHandle = _soundManager.loadSound(name);

	if (!soundItem->_soundHandle) {
		// Could load sound, so destroy new item and return
		delete soundItem;
		return 0;
	}

	// Add the item to the list of sounds
	_sounds.push_front(soundItem);

	// If there are more than 10 sounds loaded, remove the last one,
	// which is the least recently used of all of them
	if (_sounds.size() > 10)
		removeOldest();

	return soundItem->_soundHandle;
}

void CSound::checkSounds() {
	for (CSoundItemList::iterator i = _sounds.begin(); i != _sounds.end(); ++i) {
		CSoundItem *soundItem = *i;
		if (soundItem->_field24 && soundItem->_field28) {
			if (_soundManager.isActive(soundItem->_soundHandle)) {
				_sounds.remove(soundItem);
				delete soundItem;
			}
		}
	}
}

void CSound::removeOldest() {
	for (CSoundItemList::iterator i = _sounds.reverse_begin();
			i != _sounds.end(); --i) {
		CSoundItem *soundItem = *i;
		if (soundItem->_field28 && !_soundManager.isActive(soundItem->_soundHandle)) {
			_sounds.remove(soundItem);
			delete soundItem;
			break;
		}
	}
}

CSoundItem *CSound::getTrueTalkSound(CDialogueFile *dialogueFile, int index) {
	warning("TODO: CSound::getTrueTalkSound");
	return nullptr;
}

} // End of namespace Titanic z
