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

#ifndef TITANIC_MUSIC_ROOM_H
#define TITANIC_MUSIC_ROOM_H

#include "common/array.h"
#include "titanic/sound/music_handler.h"

namespace Titanic {

class CGameManager;
class CSound;

class CMusicRoom {
	struct Entry {
		uint _val1;
		uint _val2;
		uint _val3;
		uint _val4;
		uint _val5;

		Entry() : _val1(0), _val2(0), _val3(0), _val4(0), _val5(0) {}
	};
private:
	Common::Array<Entry> _items;
public:
	static CMusicHandler *_musicHandler;
public:
	CGameManager *_gameManager;
	CSound *_sound;
public:
	CMusicRoom(CGameManager *owner);
	~CMusicRoom();

	/**
	 * Creates a music handler
	 */
	CMusicHandler *createMusicHandler();

	/**
	 * Destroys and currently active music handler
	 */
	void destroyMusicHandler();

	void setItem1(int index, int val) { _items[index]._val1 = val; }
	void setItem2(int index, int val) { _items[index]._val2 = val; }
	void setItem3(int index, int val) { _items[index]._val3 = val; }
	void setItem4(int index, int val) { _items[index]._val4 = val; }
	void setItem5(int index, int val) { _items[index]._val5 = val; }

	/**
	 * Start playing a given music number
	 */
	void startMusic(int musicId);

	/**
	 * Stop playing music
	 */
	void stopMusic();
};

} // End of namespace Titanic

#endif /* TITANIC_MUSIC_ROOM_H */
