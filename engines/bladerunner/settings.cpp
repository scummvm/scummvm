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

#include "bladerunner/settings.h"

#include "bladerunner/actor.h"
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/chapters.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/game_info.h"
#include "bladerunner/music.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"

#include "common/debug.h"

namespace BladeRunner {

Settings::Settings(BladeRunnerEngine *vm) {
	_vm = vm;

	_difficulty = kGameDifficultyMedium;
	_playerAgenda = kPlayerAgendaNormal;

	_chapter = 1;
	_scene = -1;
	_set = -1;
	_unk0 = 0;
	_gamma = 1.0f;

	_ammoType = 0;
	_ammoAmounts[0] = 1;
	_ammoAmounts[1] = 0;
	_ammoAmounts[2] = 0;

	// The remaining fields are not reset in BLADE.EXE
	_chapterChanged = false;
	_newChapter = -1;
	_newScene = -1;
	_newSet = -1;

	_startingGame = false;
	_loadingGame = false;

	_fullHDFrames = true;
	_mst3k = false;

#if BLADERUNNER_ORIGINAL_BUGS
	// Probably a bug. Assigning these here again, overrides the initialization above.
	//       Also note: the reset() method assigns "_ammoAmounts[0] = 1" like above!
	_ammoType = 0;
	_ammoAmounts[0] = 0;
	_ammoAmounts[1] = 0;
	_ammoAmounts[2] = 0;
#endif // BLADERUNNER_ORIGINAL_BUGS

	_learyMode = false;
}

void Settings::reset() {
	_ammoType = 0;
	_ammoAmounts[0] = 1;
	_ammoAmounts[1] = 0;
	_ammoAmounts[2] = 0;
}

bool Settings::openNewScene() {
	if (_newSet == -1) {
		assert(_newScene == -1);
		return true;
	}
	assert(_newScene != -1);

	if (_startingGame) {
		_vm->_ambientSounds->removeAllNonLoopingSounds(true);
		_vm->_ambientSounds->removeAllLoopingSounds(1);
		_vm->_music->stop(2);
	}

	int currentSet = _vm->_scene->getSetId();
	int newSet     = _newSet;
	int newScene   = _newScene;

	_newSet = -1;
	_newScene = -1;
	if (currentSet != -1) {
		_vm->_scene->close(!_loadingGame && !_startingGame);
	}
	if (_chapterChanged) {
		if (_vm->_chapters->hasOpenResources()) {
			_vm->_chapters->closeResources();
		}

		int newChapter = _newChapter;
		_chapterChanged = false;
		_newChapter = 0;
		if (!_vm->_chapters->enterChapter(newChapter)) {
			_vm->_gameIsRunning = false;
			return false;
		}
		_chapter = newChapter;
		if (_startingGame) {
			_startingGame = false;
		}
	}

	if (!_vm->_scene->open(newSet, newScene, _loadingGame)) {
		_vm->_gameIsRunning = false;
		return false;
	}

	_set = newSet;
	_scene = newScene;

	if (!_loadingGame && currentSet != newSet) {
		for (int i = 0; i < (int)_vm->_gameInfo->getActorCount(); ++i) {
			Actor *actor = _vm->_actors[i];
			if (i != kActorMcCoy && actor->getSetId() == currentSet) {
				if (!actor->isRetired()) {
					actor->stopWalking(false);
					actor->movementTrackWaypointReached();
				}
				if (actor->inCombat()) {
					actor->setSetId(kSetFreeSlotG);
					actor->combatModeOff();
				}
			}
		}
	}

	_loadingGame = false;
	return true;
}

int Settings::getAmmoTypesCount() {
	return kAmmoTypesCount;
}

int Settings::getAmmoType() const {
	return _ammoType;
}

void Settings::setAmmoType(int ammoType) {
	if (_ammoAmounts[ammoType] > 0) {
		_ammoType = ammoType;
	}
}

int Settings::getAmmo(int ammoType) const {
	return _ammoAmounts[ammoType];
}

void Settings::addAmmo(int ammoType, int ammo) {
	if (ammoType > _ammoType || _ammoAmounts[_ammoType] == 0) {
		_ammoType = ammoType;
	}
	_ammoAmounts[ammoType] += ammo;
}

void Settings::decreaseAmmo() {
	if (_difficulty == kGameDifficultyEasy || _ammoType == 0) {
		return;
	}

	if (_ammoAmounts[_ammoType] > 0) {
		--_ammoAmounts[_ammoType];
	}

	if (_ammoAmounts[_ammoType] == 0) {
		for (int i = 2; i >= 0; --i) {
			if (_ammoAmounts[i] > 0) {
				_ammoType = i;
				return;
			}
		}
	}
}

int Settings::getDifficulty() const {
	return _difficulty;
}

void Settings::setDifficulty(int difficulty) {
	_difficulty = difficulty;
}

int Settings::getPlayerAgenda() const {
	return _playerAgenda;
}

void Settings::setPlayerAgenda(int agenda) {
	_playerAgenda = agenda;
}

bool Settings::getLearyMode() const {
	return _learyMode;
}

void Settings::setLearyMode(bool learyMode) {
	_learyMode = learyMode;
}

void Settings::save(SaveFileWriteStream &f) {
	f.writeInt(_scene);
	f.writeInt(_set);
	f.writeInt(_chapter);
	f.writeInt(_playerAgenda);
	f.writeInt(_unk0);
	f.writeInt(_difficulty);
	f.writeInt(_ammoType);
	for (int i = 0; i != kAmmoTypesCount; ++i) {
		f.writeInt(_ammoAmounts[i]);
	}
}

void Settings::load(SaveFileReadStream &f) {
	_scene = f.readInt();
	_set = f.readInt();
	_chapter = f.readInt();
	_playerAgenda = f.readInt();
	_unk0 = f.readInt();
	_difficulty = f.readInt();
	_ammoType = f.readInt();
	for (int i = 0; i != kAmmoTypesCount; ++i) {
		_ammoAmounts[i] = f.readInt();
	}
}

} // End of namespace BladeRunner
