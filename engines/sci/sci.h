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

#ifndef SCI_H
#define SCI_H

#include "engines/engine.h"
#include "gui/debugger.h"

//namespace Sci {

// our engine debug levels
enum {
	SCI_DEBUG_RESOURCES = 1 << 0,
	SCI_DEBUG_todo = 1 << 1
};

struct GameFlags {
	//int gameType;
	//int gameId;
	//uint32 features;
	// SCI Version
	// Resource Map Version
	// etc...
};

struct SciGameDescription {
	ADGameDescription desc;
	GameFlags flags;
};

//class Console;

class SciEngine : public Engine {
public:
	SciEngine(OSystem *syst, const SciGameDescription *desc);
	~SciEngine();

	virtual Common::Error init(void);
	virtual Common::Error go(void);

private:
	//Console *_console;
};

/*
// Example console
class Console : public GUI::Debugger {
	public:
		//Console(SciEngine *vm);
		//virtual ~Console(void);
};
*/

//} // End of namespace Sci

#endif // SCI_H
