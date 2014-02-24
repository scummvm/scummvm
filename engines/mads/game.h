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

enum DialogId {
	DIALOG_NONE = 0, DIALOG_GAME_MENU = 1, DIALOG_SAVE = 2, DIALOG_RESTORE = 3,
	DIALOG_OPTIONS = 4, DIALOG_DIFFICULTY = 5, DIALOG_ERROR = 6
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
public:
	Player();
};

class SectionHandler {
protected:
	MADSEngine *_vm;
public:
	SectionHandler(MADSEngine *vm): _vm(vm) {}

	virtual void loadSection() = 0;
	virtual void sectionPtr2() = 0;
	virtual void sectionPtr3() = 0;
};

class Game {
private:
	/**
	 * Main game loop
	 */
	void gameLoop();
protected:
	MADSEngine *_vm;
	MSurface *_surface;
	Difficulty _difficultyLevel;
	Common::Array<uint16> _globalFlags;
	Common::Array<InventoryObject> _objects;
	Common::Array<int> _inventoryList;
	Player _player;
	Scene _scene;
	int _saveSlot;
	int _statusFlag;
	DialogId _pendingDialog;

	SectionHandler *_sectionHandler;

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
	 * Show a game dialog
	 */
	virtual void showDialog() = 0;

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
public:
	virtual ~Game();

	/**
	 * Run the game
	 */
	void run();
};

} // End of namespace MADS

#endif /* MADS_GAME_H */
