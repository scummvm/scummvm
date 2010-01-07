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
 * $URL$
 * $Id$
 *
 */

#ifndef WAGE_H
#define WAGE_H
 
#include "engines/engine.h"
#include "common/debug.h"
#include "gui/debugger.h"
#include "common/endian.h"

struct ADGameDescription;

namespace Wage {
 
class Console;
class MacResManager;
class World;

using Common::String;
 
// our engine debug levels
enum {
	kWageDebugExample = 1 << 0,
	kWageDebugExample2 = 1 << 1
	// next new level must be 1 << 2 (4)
	// the current limitation is 32 debug levels (1 << 31 is the last one)
};

Common::String readPascalString(Common::SeekableReadStream &in);

class WageEngine : public Engine {
public:
	WageEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~WageEngine();
 
	virtual bool hasFeature(EngineFeature f) const;

	virtual Common::Error run();
 
	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();

	const char *getGameFile() const;

private:
	bool loadWorld(MacResManager *resMan);

private:
	Console *_console;
 
	// We need random numbers
	Common::RandomSource _rnd;

	const ADGameDescription *_gameDescription;

	MacResManager *_resManager;

	World *_world;
};
 
// Example console class
class Console : public GUI::Debugger {
public:
	Console(WageEngine *vm) {}
	virtual ~Console(void) {}
};
 
} // End of namespace Wage
 
#endif
