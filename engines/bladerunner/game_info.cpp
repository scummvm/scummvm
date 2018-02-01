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
	_setNamesCount      = 0;
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
	_setNamesCount        = s->readUint32LE();   /* 05 */
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

	_sceneNames = new char[_setNamesCount][5];
	for (uint32 i = 0; i != _setNamesCount; ++i)
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

	if (false) {
		for (uint32 i = 0; i != _setNamesCount; ++i)
			debug("%3d: %s", i, _sceneNames[i]);
		for (uint32 i = 0; i != _sfxTrackCount; ++i)
			debug("%3d: %s", i, _sfxTracks[i]);
		for (uint32 i = 0; i != _musicTrackCount; ++i)
			debug("%s", _musicTracks[i]);
		for (uint32 i = 0; i != _outtakeCount; ++i)
			debug("%2d: %s.VQA", i, _outtakes[i]);
	}

	bool err = s->err();
	delete s;
	return !err;
}

} // End of namespace BladeRunner
