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

// TODO: Adapt for Phantom's difficulty setting
enum StoryMode { STORYMODE_NAUGHTY = 1, STORYMODE_NICE = 2 };

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

class GamePhantom : public Game {
	friend class Game;
protected:
	GamePhantom(MADSEngine *vm);

	virtual void startGame();

	virtual void initializeGlobals();

	virtual void setSectionHandler();

	virtual void checkShowDialog();
public:
	PhantomGlobals _globals;
	StoryMode _storyMode;

	virtual Globals &globals() { return _globals; }

	virtual void doObjectAction();

	virtual void unhandledAction();

	virtual void step();

	virtual void synchronize(Common::Serializer &s, bool phase1);
};


class Section1Handler : public SectionHandler {
public:
	Section1Handler(MADSEngine *vm) : SectionHandler(vm) {}

	// TODO: Properly implement handler methods
	virtual void preLoadSection() {}
	virtual void sectionPtr2() {}
	virtual void postLoadSection() {}
};

// TODO: Properly implement handler classes
typedef Section1Handler Section2Handler;
typedef Section1Handler Section3Handler;
typedef Section1Handler Section4Handler;
typedef Section1Handler Section5Handler;

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_GAME_PHANTOM_H */
