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

#ifndef TITANIC_GAME_MANAGER_H
#define TITANIC_GAME_MANAGER_H

#include "common/scummsys.h"
#include "titanic/game_state.h"
#include "titanic/input_handler.h"
#include "titanic/input_translator.h"
#include "titanic/simple_file.h"
#include "titanic/video_surface.h"
#include "titanic/true_talk/true_talk_manager.h"
#include "titanic/sound/background_sound_maker.h"
#include "titanic/sound/music_room.h"
#include "titanic/sound/sound.h"

namespace Titanic {

class CProjectItem;
class CGameView;

class CGameManagerListItem : public ListItem {
private:
	static int _v1;
public:
	/**
	 * Called after loading a game has finished
	 */
	void postLoad(uint ticks, CProjectItem *project);

	/**
	 * Called when a game is about to be saved
	 */
	void preSave();

	/**
	 * Called when a game has finished being saved
	 */
	void postSave();
};

class CGameManagerList : public List<CGameManagerListItem> {
public:
	/**
	 * Called after loading a game has finished
	 */
	void postLoad(uint ticks, CProjectItem *project);

	/**
	 * Called when a game is about to be saved
	 */
	void preSave();

	/**
	 * Called when a game has finished being saved
	 */
	void postSave();

	/**
	 * Handles an update
	 */
	void update(uint ticks);
};

class CGameManager {
private:
	CTrueTalkManager _trueTalkManager;
	CGameManagerList _list;
	int _field30;
	CBackgroundSoundMaker *_soundMaker;
	CVideoSurface *_videoSurface1;
	int _field4C;
	int _field54;
	CVideoSurface *_videoSurface2;
	uint _lastDiskTicksCount;
	uint _tickCount2;
private:
	/**
	 * Return the current room
	 */
	CRoomItem *getRoom();

	/**
	 * Generates a message for the next game frame
	 */
	void frameMessage(CRoomItem *room);

	/**
	 * Handles any ongoing movie playback
	 */
	void handleMovies();
public:
	CProjectItem *_project;
	CGameView *_gameView;
	CGameState _gameState;
	Rect _bounds;
	CInputHandler _inputHandler;
	CInputTranslator _inputTranslator;
	CTreeItem *_dragItem;
	CMusicRoom _musicRoom;
	CSound _sound;
public:
	CGameManager(CProjectItem *project, CGameView *gameView);
	~CGameManager();

	/**
	 * Load data from a save file
	 */
	void load(SimpleFile *file);

	/**
	 * Called when a game is about to be loaded
	 */
	void preLoad();

	/**
	 * Called after loading a game has finished
	 */
	void postLoad(CProjectItem *project);

	/**
	 * Called when a game is about to be saved
	 */
	void preSave(CProjectItem *project);

	/**
	 * Called when a game has finished being saved
	 */
	void postSave();

	/**
	 * Updates the game time when the last disk access started
	 */
	void updateDiskTicksCount();

	CViewItem *getView() { return _gameState._gameLocation.getView(); }

	/**
	 * Lock the input handler
	 */
	void lockInputHandler() { _inputHandler.incLockCount(); }

	/**
	 * Unlock the input handler
	 */
	void unlockInputHandler() { _inputHandler.decLockCount(); }

	/**
	 * Set default screen bounds
	 */
	void initBounds();

	void fn2();

	/**
	 * Plays a movie clip
	 */
	void playClip(CMovieClip *clip, CRoomItem *oldRoom, CRoomItem *newRoom);

	/**
	 * Main frame update method for the game
	 */
	void update();

	/**
	 * Called when the view changes
	 */
	void viewChange();

	bool test54() const { return !_field54; }

	void inc54() { ++_field54; }

	void dec54() { --_field54; }
};

} // End of namespace Titanic

#endif /* TITANIC_GAME_MANAGER_H */
