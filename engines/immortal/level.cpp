/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "immortal/room.h"
#include "immortal/immortal.h"

namespace Immortal {

struct Flame;

void ImmortalEngine::levelInitAtStartOfGameOnly() {
	initStoryDynamic();
	_lastLevelLoaded = -1;
	_lastSongLoaded = -1;
}

void ImmortalEngine::levelInit() {
	_count = 0;
}

void ImmortalEngine::levelNew(int l) {
	stopMusic();
	clearScreen();

	levelStory(l);
	if (kLevelToMaze[l] != _lastLevelLoaded) {
		_lastLevelLoaded = kLevelToMaze[l];
		//loadMaze(l);
	}

	if (_level != _lastSongLoaded) {
		//loadSong(l);
	}

	//startMusic();
	//monstSetXY -> _rooms[_currentRoom].monsters[kPlayerID].setXY(_initialBX, _initialBY);

	//univSetXY(_initialX << 3, _initialY << 3);

	levelShowRoom(_initialRoom, _initialBX, _initialBY);
}

void ImmortalEngine::levelStory(int l) {
	levelLoadFile(l);
}

void ImmortalEngine::levelLoadFile(int l) {
	/* This was originally a large branching tree that checked the identifier of each entry and
	 * Processed them all for the story. Once again, this would have been better as an indexed
	 * JSR instead of a set of comparisons and branches. Regardless, we instead use the information
	 * in the story struct to create the rooms and then populate them.
	 */

	// Create the rooms and doors, then populate the rooms with their objects and actors

	for (int d = 0; d < _stories[l]._doors.size(); d++) {
		doorNew(_stories[l]._doors[d]);
	}

	for (int r = 0; r < _stories[l]._rooms.size(); r++) {
		_rooms[r] = new Room(_stories[l]._rooms[r]._x, _stories[l]._rooms[r]._y, _stories[l]._rooms[r]._flags, _sprites, _dataSprites, _cycles, _cycPtrs, &_numSprites);

		Common::Array<SFlame> allFlames(_stories[l]._flames[r].size());
		if (_stories[l]._flames[r].size() > 0) {
			for (int f = 0; f < _stories[l]._flames[r].size(); f++) {
				SFlame sf;
				sf._p = _stories[l]._flames[r][f]._p;
				sf._x = _stories[l]._flames[r][f]._x;
				sf._y = _stories[l]._flames[r][f]._y;
				allFlames[f] = sf;
			}
		}
		_allFlames[r] = allFlames;

		if (_stories[l]._objects[r].size() > 0) {
			for (int o = 0; o < _stories[l]._objects[r].size(); o++) {
				//objNew(_stories[l]._objects[r][o]);
			}
		}

		if (_stories[l]._monsters[r].size() > 0) {
			for (int m = 0; m < _stories[l]._monsters[r].size(); m++) {
				//monstNew(_stories[l]._monsters[r][m]);
			}
		}
	}

	// Set up the _initial variables for the engine scope
	univAtNew(l);
}

void ImmortalEngine::univAtNew(int l) {
	_initialRoom = _dRoomNum;
	_initialX    = _stories[l]._initialUnivX;
	_initialY    = _stories[l]._initialUnivY;
	_initialBX   = _stories[l]._playerPointX;
	_initialBY   = _stories[l]._playerPointY;

	//doorToNextLevel(_stories[l]._doorToNextLevel, _initialBX, _initialBY);
	//doorSetLadders(_stories[l]._doorSetLadders);
	//roomSetHole(_stories[l]._setHole, _stories[l]._setHoleX, _stories[l]._setHoleY);
	//monstRepos(kPlayerID);
}

void ImmortalEngine::levelDrawAll() {
	_count++;
	//univAutoCenter();
	clearSprites();
	// Room needs to be able to add to the sprite list, so we need to give it a pointer to it first
	_rooms[_currentRoom]->drawContents(_viewPortX, _viewPortY);
}

void ImmortalEngine::levelShowRoom(int r, int bX, int bY) {
	_currentRoom = r;
	cycleFreeAll();		// This may not be needed, or it may need to be changed slightly
	_rooms[_currentRoom]->flameSetRoom(_allFlames[r]);
	//univSetRoom(r, bX, bY);
	//fset, spark, bullet, and door get set to the current room
	//roomGetCell(r, bX, bY);
	//x, y <- roomGetXY(r, bX, bY);
	//x += bX;
	//y += bY;
	//x <<= 1;
	//blister();
}

bool ImmortalEngine::levelIsLoaded(int l) {
	if (l == _storyLevel) {
		return true;
	}
	return false;
}

bool ImmortalEngine::levelIsShowRoom(int r) {
	if (r == _currentRoom) {
		return true;
	}
	return false;
}

} // namespace immortal


















