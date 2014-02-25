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

namespace MADS {

class MADSEngine;

enum {
	PLAYER_INVENTORY = 2
};

enum Difficulty {
	DIFFICULTY_HARD = 1, DIFFICULTY_MEDIUM = 2, DIFFICULTY_EASY = 3
};

class InventoryObject {
public:
	int _descId;
	int _roomNumber;
	int _article;
	int _vocabCount;
	struct {
		int _actionFlags1;
		int _actionFlags2;
		int _vocabId;
	} _vocabList[3];
	char _mutilateString[10];	// ???
	const byte *_objFolder;		// ???

	/**
	 * Loads the data for a given object
	 */
	void load(Common::SeekableReadStream &f);
};

class Player {
public:
	int _direction;
	int _newDirection;
	bool _spritesLoaded;
	int _spriteListStart;
	int _numSprites;
	bool _stepEnabled;
	bool _spritesChanged;
	bool _visible;
public:
	Player();

	void loadSprites(const Common::String &prefix) {
		warning("TODO: Player::loadSprites");
	}
};

class SectionHandler {
protected:
	MADSEngine *_vm;
public:
	SectionHandler(MADSEngine *vm): _vm(vm) {}
	virtual ~SectionHandler() {}

	virtual void preLoadSection() = 0;
	virtual void sectionPtr2() = 0;
	virtual void postLoadSection() = 0;
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

	/**
	 * Returns true if a given Scene Id exists in the listed of previously visited scenes.
	 */
	bool visitedScenesExists(int sceneId);

	/**
	 * Adds a scene Id to the list of previously visited scenes, if it doesn't already exist
	 */
	void addVisitedScene(int sceneId);
protected:
	MADSEngine *_vm;
	MSurface *_surface;
	Difficulty _difficultyLevel;
	Player _player;
	Scene _scene;
	int _saveSlot;
	int _statusFlag;
	SectionHandler *_sectionHandler;
	Common::Array<int> _visitedScenes;
	byte *_quotes;
	int _v1;
	int _v2;
	int _v3;
	int _v4;
	int _v5;
	int _v6;
	Common::String _aaName;
	bool _playerSpritesFlag;

	/**
	 * Constructor
	 */
	Game(MADSEngine *vm);

	/**
	 * Loads the game's object list
	 */
	void loadObjects();

	/**
	 * Set the associated data? pointer with an inventory object
	 */
	void setObjectData(int objIndex, int id, const byte *p);

	/**
	 * Sets the room number
	 */
	void setObjectRoom(int objectId, int roomNumber);

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
	int _sectionNumber;
	int _priorSectionNumber;
	int _currentSectionNumber;
	Common::Array<uint16> _globalFlags;
	Common::Array<InventoryObject> _objects;
	Common::Array<int> _inventoryList;
public:
	virtual ~Game();

	/**
	 * Run the game
	 */
	void run();

	Player &player() { return _player; }
};

} // End of namespace MADS

#endif /* MADS_GAME_H */
