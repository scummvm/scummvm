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

#ifndef MADS_GAME_PHANTOM_H
#define MADS_GAME_PHANTOM_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/globals.h"
#include "mads/phantom/globals_phantom.h"

namespace MADS {

namespace Phantom {

enum Difficulty {
	DIFFICULTY_HARD = 1, DIFFICULTY_MEDIUM = 2, DIFFICULTY_EASY = 3
};

enum InventoryObject {
	OBJ_NONE = -1,
	OBJ_KEY = 0,
	OBJ_LANTERN = 1,
	OBJ_RED_FRAME = 2,
	OBJ_SANDBAG = 3,
	OBJ_YELLOW_FRAME = 4,
	OBJ_FIRE_AXE = 5,
	OBJ_SMALL_NOTE = 6,
	OBJ_ROPE = 7,
	OBJ_SWORD = 8,
	OBJ_ENVELOPE = 9,
	OBJ_TICKET = 10,
	OBJ_PIECE_OF_PAPER = 11,
	OBJ_PARCHMENT = 12,
	OBJ_LETTER = 13,
	OBJ_NOTICE = 14,
	OBJ_BOOK = 15,
	OBJ_CRUMPLED_NOTE = 16,
	OBJ_BLUE_FRAME = 17,
	OBJ_LARGE_NOTE = 18,
	OBJ_GREEN_FRAME = 19,
	OBJ_MUSIC_SCORE = 20,
	OBJ_WEDDING_RING = 21,
	OBJ_CABLE_HOOK = 22,
	OBJ_ROPE_WITH_HOOK = 23,
	OBJ_OAR = 24
};

enum MazeEvent {
	MAZE_EVENT_NONE = 0,
	MAZE_EVENT_PUDDLE = 0x0001,
	MAZE_EVENT_RAT_NEST = 0x0002,
	MAZE_EVENT_SKULL = 0x0004,
	MAZE_EVENT_POT = 0x0008,
	MAZE_EVENT_BRICK = 0x0010,
	MAZE_EVENT_HOLE = 0x0020,
	MAZE_EVENT_WEB = 0x0040,
	MAZE_EVENT_PLANK = 0x0080,
	MAZE_EVENT_DRAIN = 0x0100,
	MAZE_EVENT_STONE = 0x0200,
	MAZE_EVENT_BLOCK = 0x0400,
	MAZE_EVENT_FALLEN_BLOCK = 0x0800
};

struct Catacombs {
	int _sceneNum;
	int _exit[4];
	int _fromDirection[4];
	int _flags;
};

class GamePhantom : public Game {
	friend class Game;

private:
	void genericObjectExamine();
	void stopWalker();
	void stopWalkerBasic();

	Catacombs *_catacombs;
	int _catacombSize;

protected:
	GamePhantom(MADSEngine *vm);

	void startGame() override;

	void initializeGlobals() override;

	void setSectionHandler() override;

	void checkShowDialog() override;
public:
	PhantomGlobals _globals;
	Difficulty _difficulty;


	Globals &globals() override { return _globals; }

	void doObjectAction() override;

	void unhandledAction() override;

	void step() override;

	void synchronize(Common::Serializer &s, bool phase1) override;

	void setupCatacombs();
	void enterCatacombs(bool val);
	void initCatacombs();
	void moveCatacombs(int dir);
	int exitCatacombs(int dir);
	void newCatacombRoom(int fromRoom, int fromExit);
};

// Section handlers aren't needed in ScummVM implementation
class Section1Handler : public SectionHandler {
public:
	Section1Handler(MADSEngine *vm) : SectionHandler(vm) {}

	void preLoadSection() override {}
	void sectionPtr2() override {}
	void postLoadSection() override {}
};

typedef Section1Handler Section2Handler;
typedef Section1Handler Section3Handler;
typedef Section1Handler Section4Handler;
typedef Section1Handler Section5Handler;

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_GAME_PHANTOM_H */
