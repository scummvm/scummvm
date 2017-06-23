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

#define MO_CLUES_COUNT 10
#define WHEREABOUTS_CLUES_COUNT 10
#define REPLICANT_CLUES_COUNT 20
#define NONREPLICANT_CLUES_COUNT 20
#define OTHER_CLUES_COUNT 20
#define IDENTITY_CLUES_COUNT 10
#define PHOTO_CLUES_COUNT 6

class SuspectDatabaseEntry {
	BladeRunnerEngine *_vm;

	int _actorId;
	int _sex;
	int _moClues[MO_CLUES_COUNT];
	int _whereaboutsClues[WHEREABOUTS_CLUES_COUNT];
	int _replicantClues[REPLICANT_CLUES_COUNT];
	int _nonReplicantClues[NONREPLICANT_CLUES_COUNT];
	int _otherClues[OTHER_CLUES_COUNT];
	int _identityClues[IDENTITY_CLUES_COUNT];
	int _photoClues[PHOTO_CLUES_COUNT][3];
	int _moCluesCount;
	int _whereaboutsCluesCount;
	int _replicantCluesCount;
	int _nonReplicantCluesCount;
	int _otherCluesCount;
	int _identityCluesCount;
	int _photoCluesCount;

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
