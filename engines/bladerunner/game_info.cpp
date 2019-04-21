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

#include "bladerunner/game_info.h"

#include "bladerunner/bladerunner.h"

#include "common/debug.h"
#include "common/substream.h"

namespace BladeRunner {

GameInfo::GameInfo(BladeRunnerEngine *vm) {
	_vm = vm;
	_actorCount         = 0;
	_playerId           = 0;
	_flagCount          = 0;
	_clueCount          = 0;
	_globalVarCount     = 0;
	_sceneNamesCount    = 0;
	_initialSceneId     = 0;
	_initialSetId       = 0;
	_waypointCount      = 0;
	_sfxTrackCount      = 0;
	_musicTrackCount    = 0;
	_outtakeCount       = 0;
	_crimeCount         = 0;
	_suspectCount       = 0;
	_coverWaypointCount = 0;
	_fleeWaypointCount  = 0;
}

bool GameInfo::open(const Common::String &name) {
	Common::SeekableReadStream *s = _vm->getResourceStream(name);

	if (!s) {
		return false;
	}

	_actorCount           = s->readUint32LE();   /* 00 */
	_playerId             = s->readUint32LE();   /* 01 */
	_flagCount            = s->readUint32LE();   /* 02 */
	_clueCount            = s->readUint32LE();   /* 03 */
	_globalVarCount       = s->readUint32LE();   /* 04 */
	_sceneNamesCount      = s->readUint32LE();   /* 05 */
	_initialSceneId       = s->readUint32LE();   /* 06 */
	                        s->skip(4);          /* 07 */
	_initialSetId         = s->readUint32LE();   /* 08 */
	                        s->skip(4);          /* 09 */
	_waypointCount        = s->readUint32LE();   /* 10 */
	_sfxTrackCount        = s->readUint32LE();   /* 11 */
	_musicTrackCount      = s->readUint32LE();   /* 12 */
	_outtakeCount         = s->readUint32LE();   /* 13 */
	_crimeCount           = s->readUint32LE();   /* 14 */
	_suspectCount         = s->readUint32LE();   /* 15 */
	_coverWaypointCount   = s->readUint32LE();   /* 16 */
	_fleeWaypointCount    = s->readUint32LE();   /* 17 */

	char buf[9];

	_sceneNames.resize(_sceneNamesCount);
	for (uint32 i = 0; i != _sceneNamesCount; ++i) {
		s->read(buf, 5);
		_sceneNames[i] = buf;
	}

	_sfxTracks.resize(_sfxTrackCount);
	for (uint32 i = 0; i != _sfxTrackCount; ++i) {
		s->read(buf, 9);
		_sfxTracks[i] = buf;
		_sfxTracks[i] += ".AUD";
	}

	_musicTracks.resize(_musicTrackCount);
	for (uint32 i = 0; i != _musicTrackCount; ++i) {
		s->read(buf, 9);
		_musicTracks[i] = buf;
		_musicTracks[i] += ".AUD";
	}

	_outtakes.resize(_outtakeCount);
	for (uint32 i = 0; i != _outtakeCount; ++i) {
		s->read(buf, 9);
		_outtakes[i] = buf;
	}

#if BLADERUNNER_DEBUG_CONSOLE
	debug("\nScene names\n----------------");
	for (uint32 i = 0; i != _sceneNamesCount; ++i) {
		debug("%3d: %s", i, _sceneNames[i].c_str());
	}

	debug("\nSfx tracks\n----------------");
	for (uint32 i = 0; i != _sfxTrackCount; ++i) {
		debug("%3d: %s", i, _sfxTracks[i].c_str());
	}

	debug("\nMusic tracks\n----------------");
	for (uint32 i = 0; i != _musicTrackCount; ++i) {
		debug("%3d: %s", i, _musicTracks[i].c_str());
	}

	debug("\nOuttakes\n----------------");
	for (uint32 i = 0; i != _outtakeCount; ++i) {
		debug("%2d: %s.VQA", i, _outtakes[i].c_str());
	}
#endif

	bool err = s->err();
	delete s;
	return !err;
}

const Common::String &GameInfo::getSceneName(int i) const {
	if (i < 0 || i >= (int)_sceneNamesCount) {
		warning("GameInfo::getSceneName: unknown id \"%i\"", i);
		static Common::String str("UNKNOWN_SCENE");
		return str;
	}
	return _sceneNames[i];
}

const Common::String &GameInfo::getSfxTrack(int i) const {
	if (i < 0 || i >= (int)_sfxTrackCount) {
		warning("GameInfo::getSfxTrack: unknown id \"%i\"", i);
		static Common::String str("UNKNOWN_SFX_TRACK");
		return str;
	}
	return _sfxTracks[i];
}

const Common::String &GameInfo::getMusicTrack(int i) const {
	if (i < 0 || i >= (int)_musicTrackCount) {
		warning("GameInfo::getMusicTrack: unknown id \"%i\"", i);
		static Common::String str("UNKNOWN_MUSIC_TRACK");
		return str;
	}
	return _musicTracks[i];
}

const Common::String &GameInfo::getOuttake(int i) const {
	if (i < 0 || i >= (int)_outtakeCount) {
		warning("GameInfo::getOuttake: unknown id \"%i\"", i);
		static Common::String str("UNKNOWN_OUTTAKE");
		return str;
	}
	return _outtakes[i];
}

} // End of namespace BladeRunner
