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

#ifndef MADS_GAME_NEBULAR_H
#define MADS_GAME_NEBULAR_H

#include "common/scummsys.h"
#include "mads/game.h"

namespace MADS {

namespace Nebular {

class GameNebular: public Game {
	friend class Game;
protected:
	GameNebular(MADSEngine *vm);

	virtual int checkCopyProtection();

	virtual void initialiseGlobals();

	virtual void setSectionHandler();
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

#endif /* MADS_GAME_NEBULAR_H */
