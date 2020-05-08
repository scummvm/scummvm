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

#ifndef ULTIMA_SHARED_ENGINE_DEBUGGER_H
#define ULTIMA_SHARED_ENGINE_DEBUGGER_H

#include "common/scummsys.h"
#include "gui/debugger.h"

namespace Ultima {
namespace Shared {

class UltimaEngine;

/**
 * Debugger base class
 */
class Debugger : public GUI::Debugger {
protected:
	/**
	 * Converts a string to an integer
	 */
	int strToInt(const char *s);

	/**
	 * Converts a string to a boolean
	 */
	bool strToBool(const char *s) {
		return s && tolower(*s) == 't';
	}

	/**
	 * Split up a command string into arg values
	 */
	void splitString(const Common::String &str, Common::StringArray &argv);
public:
	Debugger();
    ~Debugger() override {}

	/**
	 * Executes the given command
	 */
	void executeCommand(const Common::String &cmd);

	/**
	 * Executes the given command
	 */
	void executeCommand(int argc, const char **argv);
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
