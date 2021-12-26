/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SHERLOCK_SCALPEL_DEBUGGER_H
#define SHERLOCK_SCALPEL_DEBUGGER_H

#include "sherlock/debugger.h"

namespace Sherlock {

class SherlockEngine;

namespace Scalpel {

class ScalpelDebugger : public Debugger {
private:
	/**
	 * Plays a 3DO movie
	 */
	bool cmd3DO_PlayMovie(int argc, const char **argv);

	/**
	 * Plays a 3DO audio
	 */
	bool cmd3DO_PlayAudio(int argc, const char **argv);
public:
	ScalpelDebugger(SherlockEngine *vm);
	~ScalpelDebugger() override {}
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif	/* SHERLOCK_DEBUGGER_H */
