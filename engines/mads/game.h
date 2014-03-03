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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MADS_GAME_H
#define MADS_GAME_H

#include "common/scummsys.h"
#include "mads/scene.h"
#include "mads/game_data.h"
#include "mads/player.h"

namespace MADS {

class MADSEngine;

enum {
	PLAYER_INVENTORY = 2
};

enum Difficulty {
	DIFFICULTY_HARD = 1, DIFFICULTY_MEDIUM = 2, DIFFICULTY_EASY = 3
};

class Game {
private:
	/**
	 * Main game loop
	 */
	void gameLoop();

	/**
	 * Inner game loop for executing gameplay within a game section
	 */
	void sectionLoop();
protected:
	MADSEngine *_vm;
	MSurface *_surface;
	Difficulty _difficultyLevel;
	int _saveSlot;
	int _statusFlag;
	SectionHandler *_sectionHandler;
	VisitedScenes _visitedScenes;
	byte *_quotes;
	int _v1;
	int _v3;
	int _v5;
	int _v6;
	bool _updateSceneFlag;
	Common::String _aaName;
	bool _playerSpritesFlag;
	int _objectHiliteVocabIdx;

	/**
	 * Constructor
	 */
	Game(MADSEngine *vm);

	/**
	 * Initialises the current section number of the game
	 */
	void initSection(int sectionNumber);

	void loadResourceSequence(const Common::String prefix, int v);

	//@{
	/** @name Virtual Method list */

	/**
	 * Perform any copy protection check
	 */
	virtual int checkCopyProtection() = 0;

	/**
	 * Initialises global variables for a new game
	 */
	virtual void initialiseGlobals() = 0;

	/**
	 * Set up the section handler specific to each section
	 */
	virtual void setSectionHandler() = 0;
	//@}

public:
	static Game *init(MADSEngine *vm);
public:
	Player _player;
	int _sectionNumber;
	int _priorSectionNumber;
	int _currentSectionNumber;
	Common::Array<uint16> _globalFlags;
	InventoryObjects _objects;
	Scene _scene;
	int _v2;
	int _v4;
	int _abortTimers;
	int _abortTimers2;
	AbortTimerMode _abortTimersMode;
	AbortTimerMode _abortTimersMode2;
	uint32 _currentTimer;
public:
	virtual ~Game();

	/**
	 * Run the game
	 */
	void run();
};

} // End of namespace MADS

#endif /* MADS_GAME_H */
