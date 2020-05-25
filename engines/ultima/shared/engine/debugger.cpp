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

#include "ultima/shared/engine/debugger.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/shared/early/game.h"
#include "ultima/shared/maps/map.h"

namespace Ultima {
namespace Shared {

Debugger::Debugger() : GUI::Debugger() {
}

int Debugger::strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

void Debugger::splitString(const Common::String &str,
		Common::StringArray &argv) {
	// Clear the vector
	argv.clear();

	bool quoted = false;
	Common::String::const_iterator it;
	int ch;
	Common::String arg;

	for (it = str.begin(); it != str.end(); ++it) {
		ch = *it;

		// Toggle quoted string handling
		if (ch == '\"') {
			quoted = !quoted;
			continue;
		}

		// Handle \\, \", \', \n, \r, \t
		if (ch == '\\') {
			Common::String::const_iterator next = it + 1;
			if (next != str.end()) {
				if (*next == '\\' || *next == '\"' || *next == '\'') {
					ch = *next;
					++it;
				} else if (*next == 'n') {
					ch = '\n';
					++it;
				} else if (*next == 'r') {
					ch = '\r';
					++it;
				} else if (*next == 't') {
					ch = '\t';
					++it;
				} else if (*next == ' ') {
					ch = ' ';
					++it;
				}
			}
		}

		// A space, a tab, line feed, carriage return
		if (!quoted && (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')) {
			// If we are not empty then we are at the end of the arg
			// otherwise we will ignore the extra chars
			if (!arg.empty()) {
				argv.push_back(arg);
				arg.clear();
			}

			continue;
		}

		// Add the charater to the string
		arg += ch;
	}

	// Push any arg if it's left
	if (!arg.empty())
		argv.push_back(arg);
}

void Debugger::executeCommand(const Common::String &cmd) {
	// Split up the command, and form a const char * array
	Common::StringArray args;
	splitString(cmd, args);

	Common::Array<const char *> argv;
	for (uint idx = 0; idx < args.size(); ++idx)
		argv.push_back(args[idx].c_str());

	// Execute the command
	executeCommand(argv.size(), &argv[0]);
}

void Debugger::executeCommand(int argc, const char **argv) {
	if (argc <= 0)
		return;

	bool keepRunning = false;
	if (!handleCommand(argc, argv, keepRunning)) {
		debugPrintf("Unknown command - %s\n", argv[0]);
		keepRunning = true;
	}

	// If any message occurred, then we need to ensure the debugger is opened if it isn't already
	if (keepRunning)
		attach();
}

} // End of namespace Shared
} // End of namespace Ultima
