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

#include "bladerunner/suspects_database.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/text_resource.h"

namespace BladeRunner {

SuspectDatabaseEntry::SuspectDatabaseEntry(BladeRunnerEngine *vm) {
	_vm = vm;
	reset();
}

SuspectDatabaseEntry::~SuspectDatabaseEntry() {
}

void SuspectDatabaseEntry::setActor(int actorId) {
	_actorId = actorId;
}

void SuspectDatabaseEntry::setSex(int sex) {
	_sex = sex;
}

bool SuspectDatabaseEntry::addMOClue(int clueId) {
	if (_moCluesCount >= MO_CLUES_COUNT) {
		return false;
	}
	_moClues[_moCluesCount++] = clueId;
	return true;
}

bool SuspectDatabaseEntry::addWhereaboutsClue(int clueId) {
	if (_whereaboutsCluesCount >= WHEREABOUTS_CLUES_COUNT) {
		return false;
	}
	_whereaboutsClues[_whereaboutsCluesCount++] = clueId;
	return true;
}

bool SuspectDatabaseEntry::addReplicantClue(int clueId) {
	if (_replicantCluesCount >= REPLICANT_CLUES_COUNT) {
		return false;
	}
	_replicantClues[_replicantCluesCount++] = clueId;
	return true;
}

bool SuspectDatabaseEntry::addNonReplicantClue(int clueId) {
	if (_nonReplicantCluesCount >= NONREPLICANT_CLUES_COUNT) {
		return false;
	}
	_nonReplicantClues[_nonReplicantCluesCount++] = clueId;
	return true;
}

bool SuspectDatabaseEntry::addOtherClue(int clueId) {
	if (_otherCluesCount >= OTHER_CLUES_COUNT) {
		return false;
	}
	_otherClues[_otherCluesCount++] = clueId;
	return true;
}

bool SuspectDatabaseEntry::addIdentityClue(int clueId) {
	if (_identityCluesCount >= IDENTITY_CLUES_COUNT) {
		return false;
	}
	_identityClues[_identityCluesCount++] = clueId;
	return true;
}

bool SuspectDatabaseEntry::addPhotoClue(int shapeId, int clueId) {
	if (_photoCluesCount >= PHOTO_CLUES_COUNT) {
		return false;
	}
	_photoClues[_photoCluesCount][0] = clueId;
	_photoClues[_photoCluesCount][1] = shapeId;
	_photoClues[_photoCluesCount][2] = -1;

	_photoCluesCount++;
	return true;
}

const char *SuspectDatabaseEntry::getName() {
	return _vm->_textActorNames->getText(_actorId);
}

bool SuspectDatabaseEntry::hasMOClue(int clueId) {
	for (int i = 0; i < _moCluesCount; i++) {
		if (_moClues[i] == clueId) {
			return true;
		}
	}
	return false;
}

bool SuspectDatabaseEntry::hasWhereaboutsClue(int clueId) {
	for (int i = 0; i < _whereaboutsCluesCount; i++) {
		if (_whereaboutsClues[i] == clueId) {
			return true;
		}
	}
	return false;
}

bool SuspectDatabaseEntry::hasReplicantClue(int clueId) {
	for (int i = 0; i < _replicantCluesCount; i++) {
		if (_replicantClues[i] == clueId) {
			return true;
		}
	}
	return false;
}

bool SuspectDatabaseEntry::hasNonReplicantClue(int clueId) {
	for (int i = 0; i < _nonReplicantCluesCount; i++) {
		if (_nonReplicantClues[i] == clueId) {
			return true;
		}
	}
	return false;
}

bool SuspectDatabaseEntry::hasOtherClue(int clueId) {
	for (int i = 0; i < _otherCluesCount; i++) {
		if (_otherClues[i] == clueId) {
			return true;
		}
	}
	return false;
}

bool SuspectDatabaseEntry::hasIdentityClue(int clueId) {
	for (int i = 0; i < _identityCluesCount; i++) {
		if (_identityClues[i] == clueId) {
			return true;
		}
	}
	return false;
}

bool SuspectDatabaseEntry::hasClue(int clueId) {
	return hasMOClue(clueId)
		|| hasWhereaboutsClue(clueId)
		|| hasReplicantClue(clueId)
		|| hasNonReplicantClue(clueId)
		|| hasOtherClue(clueId);
}

int SuspectDatabaseEntry::getPhotoClue1(int photoId) {
	return _photoClues[photoId][0];
}

int SuspectDatabaseEntry::getPhotoClue2(int photoId) {
	return _photoClues[photoId][1];
}

int SuspectDatabaseEntry::getPhotoClue3(int photoId) {
	return _photoClues[photoId][2];
}

void SuspectDatabaseEntry::reset() {
	_actorId = -1;
	_sex = -1;
	for (int i = 0; i < MO_CLUES_COUNT; i++) {
		_moClues[i] = -1;
	}
	for (int i = 0; i < WHEREABOUTS_CLUES_COUNT; i++) {
		_whereaboutsClues[i] = -1;
	}
	for (int i = 0; i < IDENTITY_CLUES_COUNT; i++) {
		_identityClues[i] = -1;
	}
	for (int i = 0; i < REPLICANT_CLUES_COUNT; i++) {
		_replicantClues[i] = -1;
	}
	for (int i = 0; i < NONREPLICANT_CLUES_COUNT; i++) {
		_nonReplicantClues[i] = -1;
	}
	for (int i = 0; i < OTHER_CLUES_COUNT; i++) {
		_otherClues[i] = -1;
	}

	// photo clues are not reseted in original game

	_moCluesCount = 0;
	_whereaboutsCluesCount = 0;
	_replicantCluesCount = 0;
	_nonReplicantCluesCount = 0;
	_otherCluesCount = 0;
	_identityCluesCount = 0;
	_photoCluesCount = 0;
}

SuspectsDatabase::SuspectsDatabase(BladeRunnerEngine *vm, int size) {
	_vm = vm;
	for (int i = 0; i < size; i++) {
		_suspects.push_back(new SuspectDatabaseEntry(_vm));
	}
}

SuspectsDatabase::~SuspectsDatabase() {
	for (int i = _suspects.size() - 1; i >= 0; i--) {
		delete _suspects.remove_at(i);
	}
	_suspects.clear();
}

SuspectDatabaseEntry *SuspectsDatabase::get(int suspectId) {
	return _suspects[suspectId];
}

} // End of namespace BladeRunner
