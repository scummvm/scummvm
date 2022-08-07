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

#include "immortal/immortal.h"

namespace Immortal {

void ImmortalEngine::levelInitAtStartOfGameOnly() {
	_lastLevelLoaded = -1;
	_lastSongLoaded = -1;
}

void ImmortalEngine::levelInit() {
	_count = 0;
}

void ImmortalEngine::levelNew(int l) {
	stopMusic();
	clearScreen();
	/* commented out in the source for some reason? */
	for (int i = 0; i < kMaxRooms; i++) {
		//_rooms[i].delete();
	}

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

//loadStoryFiles() {}

void ImmortalEngine::levelLoadFile(int l) {
	/* Originally, this searched through story.gs and ignored the data entries.
	 * However, we have the STR entries separate from the story entries, so
	 * we can just index the story files array directly.
	 * It also used a 16 byte buffer to read in a story entry, but again this
	 * is equivalent to the overhead of reading the array entry I think.
	 */

	_dRoomNum = 0;
	bool done = false;
	int type = 0;//story[l];

	// instead of switch statement, just make a room for each, because the rooms will have the relevant info

	// Once again, would be better as an indexed JSR instead of a set of comparisons and branches
	while (done == false) {
		switch (type & kOPMaskRecord) {
			case kOPMaskRoom:
//				roomNew();
				break;
			case kOPMaskInRoom:
//				inRoomNew();
				break;
			case kOPMaskFlame:
//				fsetNew();
				break;
			case kOPMaskUnivAt:
				univAtNew(l);
				break;
			case kOPMaskMonster:
//				monstNew();
				break;
			case kOPMaskDoor:
//				doorNew();
				break;
			case kOPMaskObject:
//				objectNew();
				break;
			default:
				done = true;
		}
	}
}

void ImmortalEngine::univAtNew(int l) {
	_initialRoom = _dRoomNum;
	_initialX = 0;//_stories[l]._initialX;
	_initialY = 0;//_stories[l]._initialY;
	_initialBX = 0;//_stories[l]._initialBX;
	_initialBY = 0;//_stories[l]._initialBY;
	//doorToNextLevel(_stories[l]._doorToNextLevel, _initialBX, _initialBY);
	//doorSetLadders(_stories[l]._doorSetLadders);
	//roomSetHole(_stories[l]._setHole, _stories[l]._setHoleX, _stories[l]._setHoleY);
	//monstRepos(kPlayerID);

}

void ImmortalEngine::levelDrawAll() {
	_count++;
	//univAutoCenter();
	clearSprites();
	//rooms[_currentRoom].drawContents();
}

void ImmortalEngine::levelShowRoom(int r, int bX, int bY) {
	_currentRoom = r;
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


















