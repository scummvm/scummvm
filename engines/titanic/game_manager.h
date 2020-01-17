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
#include "titanic/support/time_event_info.h"
#include "titanic/true_talk/true_talk_manager.h"
#include "titanic/sound/music_room.h"
#include "titanic/sound/sound.h"

namespace Titanic {

class CBackgroundSoundMaker;
class CGameView;
class CMovie;
class CMovieClip;
class CProjectItem;
class CRoomItem;
class CScreenManager;
class CTreeItem;
class CViewItem;
class CVideoSurface;
class SimpleFile;

class CGameManager {
private:
	CTrueTalkManager _trueTalkManager;
	CTimeEventInfoList _timers;
	CTreeItem *_treeItem;
	CBackgroundSoundMaker *_soundMaker;
	CMovie *_movie;
	CRoomItem *_movieRoom;
	int _transitionCtr;
	CVideoSurface *_movieSurface;
	uint _lastDiskTicksCount;
	uint _tickCount2;
private:
	/**
	 * Generates a message for the next game frame
	 */
	void frameMessage(CRoomItem *room);

	/**
	 * Handles any ongoing movie playback
	 */
	void updateMovies();

	/**
	 * Handles a room transition
	 */
	void roomTransition(CRoomItem *oldRoom, CRoomItem *newRoom);
public:
	CProjectItem *_project;
	CGameView *_gameView;
	CGameState _gameState;
	Rect _bounds;
	CInputHandler _inputHandler;
	CInputTranslator _inputTranslator;
	CTreeItem *_dragItem;
	CSound _sound;
	CMusicRoom _musicRoom;
public:
	CGameManager(CProjectItem *project, CGameView *gameView, Audio::Mixer *mixer);
	~CGameManager();

	/**
	 * Destroys and allocated tree item
	 */
	void destroyTreeItem();

	/**
	 * Save data to a save file
	 */
	void save(SimpleFile *file);

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

	/**
	 * Gets the current view
	 */
	CViewItem *getView() { return _gameState._gameLocation.getView(); }

	/**
	 * Gets the current room node
	 */
	CNodeItem *getNode() { return _gameState._gameLocation.getNode(); }

	/**
	 * Gets the current room
	 */
	CRoomItem *getRoom() { return _gameState._gameLocation.getRoom(); }

	/**
	 * Lock the input handler
	 */
	void lockInputHandler() { _inputHandler.incLockCount(); }

	/**
	 * Unlock the input handler
	 */
	void unlockInputHandler() { _inputHandler.decLockCount(); }

	/**
	 * Plays a movie clip
	 */
	void playClip(CMovieClip *clip, CRoomItem *oldRoom, CRoomItem *newRoom);

	/**
	 * Main frame update method for the game
	 */
	void update();

	/**
	 * Called when the room changes
	 */
	void roomChange();

	/**
	 * Returns true if no transition is currently in progress
	 */
	bool isntTransitioning() const { return !_transitionCtr; }

	/**
	 * Increments the number of active transitions
	 */
	void incTransitions() { ++_transitionCtr; }

	/**
	 * Decremenst the number of active transitions
	 */
	void decTransitions() { --_transitionCtr; }

	/**
	 * Extends the bounds of the currently dirty area of the
	 * game screen to include the specified area
	 */
	void addDirtyRect(const Rect &r);

	/**
	 * Marks the entire screen as dirty, requiring redraw
	 */
	void markAllDirty();

	/**
	 * Set and return the current screen manager
	 */
	CScreenManager *setScreenManager() const;

	/**
	 * Adds a timer
	 */
	void addTimer(CTimeEventInfo *timer) { _timers.push_back(timer); }

	/**
	 * Stops a timer
	 */
	void stopTimer(uint id) { _timers.stop(id); }

	/**
	 * Flags whether the timer will be persisent across save & loads
	 */
	void setTimerPersisent(uint id, bool flag) {
		_timers.setPersisent(id, flag);
	}

	/**
	 * Return the true talk manager
	 */
	CTrueTalkManager *getTalkManager() { return &_trueTalkManager; }

	/**
	 * Return the full Id of the current view in a
	 * room.node.view tuplet form
	 */
	CString getFullViewName();
};

} // End of namespace Titanic

#endif /* TITANIC_GAME_MANAGER_H */
