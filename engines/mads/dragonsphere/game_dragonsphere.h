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

#ifndef MADS_GAME_DRAGONSPHERE_H
#define MADS_GAME_DRAGONSPHERE_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/globals.h"
//#include "mads/nebular/globals_nebular.h"

namespace MADS {

namespace Dragonsphere {

// TODO: Adapt for Dragonsphere's difficulty setting
enum StoryMode { STORYMODE_NAUGHTY = 1, STORYMODE_NICE = 2 };

enum InventoryObject { OBJ_NONE = -1
	// TODO
};

// HACK: A stub for now, remove from here once it's implemented properly
class DragonsphereGlobals: public Globals {
public:
	DragonsphereGlobals() {}
	virtual ~DragonsphereGlobals() {}
};

class GameDragonsphere: public Game {
	friend class Game;
protected:
	GameDragonsphere(MADSEngine *vm);

	virtual ProtectionResult checkCopyProtection();

	virtual void initialiseGlobals();

	virtual void setSectionHandler();

	virtual void checkShowDialog();
public:
	DragonsphereGlobals _globals;
	StoryMode _storyMode;

	virtual Globals &globals() { return _globals; }

	virtual void doObjectAction();

	virtual void unhandledAction();

	virtual void step();

	virtual void synchronize(Common::Serializer &s, bool phase1);
};


class Section1Handler: public SectionHandler {
public:
	Section1Handler(MADSEngine *vm): SectionHandler(vm) {}

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
typedef Section1Handler Section6Handler;
typedef Section1Handler Section7Handler;
typedef Section1Handler Section8Handler;

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_GAME_DRAGONSPHERE_H */
