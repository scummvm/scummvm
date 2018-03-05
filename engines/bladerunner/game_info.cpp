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
	_sceneNames         = nullptr;
	_sfxTracks          = nullptr;
	_musicTracks        = nullptr;
	_outtakes           = nullptr;
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

GameInfo::~GameInfo() {
	delete[] _sceneNames;
	delete[] _sfxTracks;
	delete[] _musicTracks;
	delete[] _outtakes;
}

bool GameInfo::open(const Common::String &name) {
	Common::SeekableReadStream *s = _vm->getResourceStream(name);

	if (!s)
		return false;

	uint32 unk;
	_actorCount           = s->readUint32LE();   /* 00 */
	_playerId             = s->readUint32LE();   /* 01 */
	_flagCount            = s->readUint32LE();   /* 02 */
	_clueCount            = s->readUint32LE();   /* 03 */
	_globalVarCount       = s->readUint32LE();   /* 04 */
	_sceneNamesCount      = s->readUint32LE();   /* 05 */
	_initialSceneId       = s->readUint32LE();   /* 06 */
	unk                   = s->readUint32LE();   /* 07 */
	_initialSetId         = s->readUint32LE();   /* 08 */
	unk                   = s->readUint32LE();   /* 09 */
	_waypointCount        = s->readUint32LE();   /* 10 */
	_sfxTrackCount        = s->readUint32LE();   /* 11 */
	_musicTrackCount      = s->readUint32LE();   /* 12 */
	_outtakeCount         = s->readUint32LE();   /* 13 */
	_crimeCount           = s->readUint32LE();   /* 14 */
	_suspectCount         = s->readUint32LE();   /* 15 */
	_coverWaypointCount   = s->readUint32LE();   /* 16 */
	_fleeWaypointCount    = s->readUint32LE();   /* 17 */

	(void)unk;

	_sceneNames = new char[_sceneNamesCount][5];
	for (uint32 i = 0; i != _sceneNamesCount; ++i)
		s->read(_sceneNames[i], 5);

	_sfxTracks = new char[_sfxTrackCount][13];
	for (uint32 i = 0; i != _sfxTrackCount; ++i) {
		s->read(_sfxTracks[i], 9);
		strcat(_sfxTracks[i], ".AUD");
	}

	_musicTracks = new char[_musicTrackCount][13];
	for (uint32 i = 0; i != _musicTrackCount; ++i) {
		s->read(_musicTracks[i], 9);
		strcat(_musicTracks[i], ".AUD");
	}

	_outtakes = new char[_outtakeCount][13];
	for (uint32 i = 0; i != _outtakeCount; ++i)
		s->read(_outtakes[i], 9);

#if BLADERUNNER_DEBUG_CONSOLE
	debug("\nScene names\n----------------");
	for (uint32 i = 0; i != _sceneNamesCount; ++i) {
		debug("%3d: %s", i, _sceneNames[i]);
	}

	debug("\nSfx tracks\n----------------");
	for (uint32 i = 0; i != _sfxTrackCount; ++i) {
		debug("%3d: %s", i, _sfxTracks[i]);
	}

	debug("\nMusic tracks\n----------------");
	for (uint32 i = 0; i != _musicTrackCount; ++i) {
		debug("%3d: %s", i, _musicTracks[i]);
	}

	debug("\nOuttakes\n----------------");
	for (uint32 i = 0; i != _outtakeCount; ++i) {
		debug("%2d: %s.VQA", i, _outtakes[i]);
	}
#endif

	bool err = s->err();
	delete s;
	return !err;
}

const char *GameInfo::getSceneName(int i) const {
	if (i < 0 || i >= (int)_sceneNamesCount) {
		warning("GameInfo::getSceneName: unknown id \"%i\"", i);
		return nullptr;
	}
	return _sceneNames[i];
}

const char *GameInfo::getSfxTrack(int i) const {
	if (i < 0 || i >= (int)_sfxTrackCount) {
		warning("GameInfo::getSfxTrack: unknown id \"%i\"", i);
		return nullptr;
	}
	return _sfxTracks[i];
}

const char *GameInfo::getMusicTrack(int i) const {
	if (i < 0 || i >= (int)_musicTrackCount) {
		warning("GameInfo::getMusicTrack: unknown id \"%i\"", i);
		return nullptr;
	}
	return _musicTracks[i];
}

const char *GameInfo::getOuttake(int i) const {
	if (i < 0 || i >= (int)_outtakeCount) {
		warning("GameInfo::getOuttake: unknown id \"%i\"", i);
		return nullptr;
	}
	return _outtakes[i];
}

} // End of namespace BladeRunner
