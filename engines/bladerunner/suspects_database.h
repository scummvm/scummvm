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

#ifndef BLADERUNNER_SUSPECTS_DATABASE_H
#define BLADERUNNER_SUSPECTS_DATABASE_H
#include "common/array.h"

namespace BladeRunner {

class BladeRunnerEngine;
class TextResource;

class SuspectDatabaseEntry {
private:
	static const int kMOClueCount = 10;
	static const int kWhereaboutsClueCount = 10;
	static const int kReplicantClueCount = 20;
	static const int kNonReplicantClueCount = 20;
	static const int kOtherClueCount = 20;
	static const int kIdentityClueCount = 10;
	static const int kPhotoClueCount = 6;

	BladeRunnerEngine *_vm;

	int _actorId;
	int _sex;

	int _moClues[kMOClueCount];
	int _whereaboutsClues[kWhereaboutsClueCount];
	int _replicantClues[kReplicantClueCount];
	int _nonReplicantClues[kNonReplicantClueCount];
	int _otherClues[kOtherClueCount];
	int _identityClues[kIdentityClueCount];
	int _photoClues[kPhotoClueCount][3];

	int _moClueCount;
	int _whereaboutsClueCount;
	int _replicantClueCount;
	int _nonReplicantClueCount;
	int _otherClueCount;
	int _identityClueCount;
	int _photoClueCount;

public:
	SuspectDatabaseEntry(BladeRunnerEngine *_vm);
	~SuspectDatabaseEntry();

	void setActor(int actorId);
	void setSex(int sex);
	bool addMOClue(int clueId);
	bool addWhereaboutsClue(int clueId);
	bool addReplicantClue(int clueId);
	bool addNonReplicantClue(int clueId);
	bool addOtherClue(int clueId);
	bool addIdentityClue(int clueId);
	bool addPhotoClue(int shapeId, int clueIndex);

	const char *getName();
	bool hasMOClue(int clueId);
	bool hasWhereaboutsClue(int clueId);
	bool hasReplicantClue(int clueId);
	bool hasNonReplicantClue(int clueId);
	bool hasOtherClue(int clueId);
	bool hasIdentityClue(int clueId);
	bool hasClue(int clueId);
	int getPhotoClue1(int photoId);
	int getPhotoClue2(int photoId);
	int getPhotoClue3(int photoId);

private:
	void reset();
};

class SuspectsDatabase {
	BladeRunnerEngine *_vm;

	Common::Array<SuspectDatabaseEntry*> _suspects;

public:
	SuspectsDatabase(BladeRunnerEngine *_vm, int size);
	~SuspectsDatabase();

	SuspectDatabaseEntry *get(int suspectId);
};

} // End of namespace BladeRunner

#endif
