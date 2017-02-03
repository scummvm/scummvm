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

#ifndef BLADERUNNER_GAMEINFO_H
#define BLADERUNNER_GAMEINFO_H

#include "common/str.h"

#include "common/str.h"

namespace BladeRunner {

class BladeRunnerEngine;

class GameInfo {
	BladeRunnerEngine *_vm;

	uint32 _actor_count;
	uint32 _player_id;
	uint32 _flag_count;
	uint32 _clue_count;
	uint32 _global_var_count;
	uint32 _set_names_count;
	uint32 _initial_scene_id;
	uint32 _initial_set_id;
	uint32 _waypoint_count;
	uint32 _sfx_track_count;
	uint32 _music_track_count;
	uint32 _outtake_count;
	uint32 _suspectsDatabaseSize;
	uint32 _cover_waypoint_count;
	uint32 _flee_waypoint_count;

	char (*_scene_names)[5];
	char (*_sfx_tracks)[13];
	char (*_music_tracks)[13];
	char (*_outtakes)[13];
	
public:
	GameInfo(BladeRunnerEngine *vm);
	~GameInfo();

	bool open(const Common::String &name);

	uint32 getActorCount()           { return _actor_count; }
	uint32 getPlayerId()             { return _player_id; }
	uint32 getFlagCount()            { return _flag_count; }
	uint32 getClueCount()            { return _clue_count; }
	uint32 getGlobalVarCount()       { return _global_var_count; }
	uint32 getSetNamesCount()        { return _set_names_count; }
	uint32 getInitialSceneId()       { return _initial_scene_id; }
	uint32 getInitialSetId()         { return _initial_set_id; }
	uint32 getWaypointCount()        { return _waypoint_count; }
	uint32 getSfxTrackCount()        { return _sfx_track_count; }
	uint32 getMusicTrackCount()      { return _music_track_count; }
	uint32 getOuttakeCount()         { return _outtake_count; }
	uint32 getSuspectsDatabaseSize() { return _suspectsDatabaseSize; }
	uint32 getCoverWaypointCount()   { return _cover_waypoint_count; }
	uint32 getFleeWaypointCount()    { return _flee_waypoint_count; }

	const char *getSceneName(int i)  { return _scene_names[i]; }
	const char *getSfxTrack(int i)   { return _sfx_tracks[i]; }
	const char *getMusicTrack(int i) { return _music_tracks[i]; }
	const char *getOuttake(int i)    { return _outtakes[i]; }
};

} // End of namespace BladeRunner

#endif
