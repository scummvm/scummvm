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

#include "bladerunner/gameinfo.h"

#include "bladerunner/bladerunner.h"

#include "common/debug.h"
#include "common/substream.h"

namespace BladeRunner {

GameInfo::GameInfo(BladeRunnerEngine *vm)
		: _vm(vm) {
	_scene_names    = nullptr;
	_sfx_tracks   = nullptr;
	_music_tracks = nullptr;
	_outtakes     = nullptr;
}

GameInfo::~GameInfo() {
	delete[] _scene_names;
	delete[] _sfx_tracks;
	delete[] _music_tracks;
	delete[] _outtakes;
}

bool GameInfo::open(const Common::String &name) {
	Common::SeekableReadStream *s = _vm->getResourceStream(name);

	if (!s)
		return false;

	uint32 unk;
	_actor_count          = s->readUint32LE();   /* 00 */
	_player_id            = s->readUint32LE();   /* 01 */
	_flag_count           = s->readUint32LE();   /* 02 */
	_clue_count           = s->readUint32LE();   /* 03 */
	_global_var_count     = s->readUint32LE();   /* 04 */
	_set_names_count      = s->readUint32LE();   /* 05 */
	_initial_scene_id     = s->readUint32LE();   /* 06 */
	unk                   = s->readUint32LE();   /* 07 */
	_initial_set_id       = s->readUint32LE();   /* 08 */
	unk                   = s->readUint32LE();   /* 09 */
	_waypoint_count       = s->readUint32LE();   /* 10 */
	_sfx_track_count      = s->readUint32LE();   /* 11 */
	_music_track_count    = s->readUint32LE();   /* 12 */
	_outtake_count        = s->readUint32LE();   /* 13 */
	unk                   = s->readUint32LE();   /* 14 */
	_suspectsDatabaseSize = s->readUint32LE();   /* 15 */
	_cover_waypoint_count = s->readUint32LE();   /* 16 */
	_flee_waypoint_count  = s->readUint32LE();   /* 17 */

	(void)unk;

	_scene_names = new char[_set_names_count][5];
	for (uint32 i = 0; i != _set_names_count; ++i)
		s->read(_scene_names[i], 5);

	_sfx_tracks = new char[_sfx_track_count][13];
	for (uint32 i = 0; i != _sfx_track_count; ++i) {
		s->read(_sfx_tracks[i], 9);
		strcat(_sfx_tracks[i], ".AUD");
	}

	_music_tracks = new char[_music_track_count][13];
	for (uint32 i = 0; i != _music_track_count; ++i) {
		s->read(_music_tracks[i], 9);
		strcat(_music_tracks[i], ".AUD");
	}

	_outtakes = new char[_outtake_count][13];
	for (uint32 i = 0; i != _outtake_count; ++i)
		s->read(_outtakes[i], 9);

	if (false) {
		for (uint32 i = 0; i != _set_names_count; ++i)
			debug("%3d: %s", i, _scene_names[i]);
		for (uint32 i = 0; i != _sfx_track_count; ++i)
			debug("%3d: %s", i, _sfx_tracks[i]);
		for (uint32 i = 0; i != _music_track_count; ++i)
			debug("%s", _music_tracks[i]);
		for (uint32 i = 0; i != _outtake_count; ++i)
			debug("%2d: %s.VQA", i, _outtakes[i]);
	}

	bool err = s->err();
	delete s;
	return !err;
}

} // End of namespace BladeRunner
