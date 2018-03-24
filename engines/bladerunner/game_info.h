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

#ifndef BLADERUNNER_GAME_INFO_H
#define BLADERUNNER_GAME_INFO_H

#include "common/array.h"
#include "common/str.h"

namespace BladeRunner {

class BladeRunnerEngine;

class GameInfo {
	BladeRunnerEngine *_vm;

	uint32 _actorCount;
	uint32 _playerId;
	uint32 _flagCount;
	uint32 _clueCount;
	uint32 _globalVarCount;
	uint32 _sceneNamesCount;
	uint32 _initialSceneId;
	uint32 _initialSetId;
	uint32 _waypointCount;
	uint32 _sfxTrackCount;
	uint32 _musicTrackCount;
	uint32 _outtakeCount;
	uint32 _crimeCount;
	uint32 _suspectCount;
	uint32 _coverWaypointCount;
	uint32 _fleeWaypointCount;

	Common::Array<Common::String> _sceneNames;
	Common::Array<Common::String> _sfxTracks;
	Common::Array<Common::String> _musicTracks;
	Common::Array<Common::String> _outtakes;

public:
	GameInfo(BladeRunnerEngine *vm);

	bool open(const Common::String &name);

	uint32 getActorCount() const           { return _actorCount; }
	uint32 getPlayerId() const             { return _playerId; }
	uint32 getFlagCount() const            { return _flagCount; }
	uint32 getClueCount() const            { return _clueCount; }
	uint32 getGlobalVarCount() const       { return _globalVarCount; }
	uint32 getSceneNamesCount() const      { return _sceneNamesCount; }
	uint32 getInitialSceneId() const       { return _initialSceneId; }
	uint32 getInitialSetId() const         { return _initialSetId; }
	uint32 getWaypointCount() const        { return _waypointCount; }
	uint32 getSfxTrackCount() const        { return _sfxTrackCount; }
	uint32 getMusicTrackCount() const      { return _musicTrackCount; }
	uint32 getOuttakeCount() const         { return _outtakeCount; }
	uint32 getCrimeCount() const           { return _crimeCount; }
	uint32 getSuspectCount() const         { return _suspectCount; }
	uint32 getCoverWaypointCount() const   { return _coverWaypointCount; }
	uint32 getFleeWaypointCount() const    { return _fleeWaypointCount; }

	const Common::String &getSceneName(int i) const;
	const Common::String &getSfxTrack(int i) const;
	const Common::String &getMusicTrack(int i) const;
	const Common::String &getOuttake(int i) const;
};

} // End of namespace BladeRunner

#endif
