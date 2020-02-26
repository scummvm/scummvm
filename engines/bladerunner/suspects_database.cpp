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
	if (_moClueCount >= kMOClueCount) {
		return false;
	}
	_moClues[_moClueCount++] = clueId;
	return true;
}

bool SuspectDatabaseEntry::addWhereaboutsClue(int clueId) {
	if (_whereaboutsClueCount >= kWhereaboutsClueCount) {
		return false;
	}
	_whereaboutsClues[_whereaboutsClueCount++] = clueId;
	return true;
}

bool SuspectDatabaseEntry::addReplicantClue(int clueId) {
	if (_replicantClueCount >= kReplicantClueCount) {
		return false;
	}
	_replicantClues[_replicantClueCount++] = clueId;
	return true;
}

bool SuspectDatabaseEntry::addNonReplicantClue(int clueId) {
	if (_nonReplicantClueCount >= kNonReplicantClueCount) {
		return false;
	}
	_nonReplicantClues[_nonReplicantClueCount++] = clueId;
	return true;
}

bool SuspectDatabaseEntry::addOtherClue(int clueId) {
	if (_otherClueCount >= kOtherClueCount) {
		return false;
	}
	_otherClues[_otherClueCount++] = clueId;
	return true;
}

bool SuspectDatabaseEntry::addIdentityClue(int clueId) {
	if (_identityClueCount >= kIdentityClueCount) {
		return false;
	}
	_identityClues[_identityClueCount++] = clueId;
	return true;
}

bool SuspectDatabaseEntry::addPhotoClue(int shapeId, int clueId) {
	if (_photoClueCount >= kPhotoClueCount) {
		return false;
	}
	_photoClues[_photoClueCount].clueId = clueId;
	_photoClues[_photoClueCount].shapeId = shapeId;
	_photoClues[_photoClueCount].notUsed = -1;

	++_photoClueCount;
	return true;
}

const char *SuspectDatabaseEntry::getName() const {
	return _vm->_textActorNames->getText(_actorId);
}

int SuspectDatabaseEntry::getSex() const {
	return _sex;
}

bool SuspectDatabaseEntry::hasMOClue(int clueId) const {
	for (int i = 0; i < _moClueCount; ++i) {
		if (_moClues[i] == clueId) {
			return true;
		}
	}
	return false;
}

bool SuspectDatabaseEntry::hasWhereaboutsClue(int clueId) const {
	for (int i = 0; i < _whereaboutsClueCount; ++i) {
		if (_whereaboutsClues[i] == clueId) {
			return true;
		}
	}
	return false;
}

bool SuspectDatabaseEntry::hasReplicantClue(int clueId) const {
	for (int i = 0; i < _replicantClueCount; ++i) {
		if (_replicantClues[i] == clueId) {
			return true;
		}
	}
	return false;
}

bool SuspectDatabaseEntry::hasNonReplicantClue(int clueId) const {
	for (int i = 0; i < _nonReplicantClueCount; ++i) {
		if (_nonReplicantClues[i] == clueId) {
			return true;
		}
	}
	return false;
}

bool SuspectDatabaseEntry::hasOtherClue(int clueId) const {
	for (int i = 0; i < _otherClueCount; ++i) {
		if (_otherClues[i] == clueId) {
			return true;
		}
	}
	return false;
}

bool SuspectDatabaseEntry::hasIdentityClue(int clueId) const {
	for (int i = 0; i < _identityClueCount; ++i) {
		if (_identityClues[i] == clueId) {
			return true;
		}
	}
	return false;
}

bool SuspectDatabaseEntry::hasClue(int clueId) const {
	return hasMOClue(clueId)
		|| hasWhereaboutsClue(clueId)
		|| hasReplicantClue(clueId)
		|| hasNonReplicantClue(clueId)
		|| hasOtherClue(clueId);
}

int SuspectDatabaseEntry::getPhotoCount() const {
	return _photoClueCount;
}

int SuspectDatabaseEntry::getPhotoClueId(int photoId) const {
	return _photoClues[photoId].clueId;
}

int SuspectDatabaseEntry::getPhotoShapeId(int photoId) const {
	return _photoClues[photoId].shapeId;
}

int SuspectDatabaseEntry::getPhotoNotUsed(int photoId) const {
	return _photoClues[photoId].notUsed;
}

void SuspectDatabaseEntry::reset() {
	_actorId = -1;
	_sex = -1;
	for (int i = 0; i < kMOClueCount; ++i) {
		_moClues[i] = -1;
	}
	for (int i = 0; i < kWhereaboutsClueCount; ++i) {
		_whereaboutsClues[i] = -1;
	}
	for (int i = 0; i < kIdentityClueCount; ++i) {
		_identityClues[i] = -1;
	}
	for (int i = 0; i < kReplicantClueCount; ++i) {
		_replicantClues[i] = -1;
	}
	for (int i = 0; i < kNonReplicantClueCount; ++i) {
		_nonReplicantClues[i] = -1;
	}
	for (int i = 0; i < kOtherClueCount; ++i) {
		_otherClues[i] = -1;
	}
	for (int i = 0; i < kPhotoClueCount; ++i) {
		_photoClues[i].clueId  = -1;
		_photoClues[i].shapeId = -1;
		_photoClues[i].notUsed = -1;
	}

	// photo clues are not reseted in original game

	_moClueCount = 0;
	_whereaboutsClueCount = 0;
	_replicantClueCount = 0;
	_nonReplicantClueCount = 0;
	_otherClueCount = 0;
	_identityClueCount = 0;
	_photoClueCount = 0;
}

SuspectsDatabase::SuspectsDatabase(BladeRunnerEngine *vm, int size) {
	_vm = vm;
	for (int i = 0; i < size; ++i) {
		_suspects.push_back(new SuspectDatabaseEntry(_vm));
	}
}

SuspectsDatabase::~SuspectsDatabase() {
	for (int i = _suspects.size() - 1; i >= 0; --i) {
		delete _suspects.remove_at(i);
	}
	_suspects.clear();
}

SuspectDatabaseEntry *SuspectsDatabase::get(int suspectId) {
	return _suspects[suspectId];
}

} // End of namespace BladeRunner
