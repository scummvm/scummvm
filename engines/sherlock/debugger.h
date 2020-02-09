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

#ifndef SHERLOCK_DEBUGGER_H
#define SHERLOCK_DEBUGGER_H

#include "common/scummsys.h"
#include "gui/debugger.h"

namespace Sherlock {

class SherlockEngine;

enum AllLocations { LOC_REFRESH = -1, LOC_DISABLED = 0, LOC_ALL = 1 };

class Debugger : public GUI::Debugger {
private:
	/**
	 * Converts a decimal or hexadecimal string into a number
	 */
	int strToInt(const char *s);

	/**
	 * Switch to another scene
	 */
	bool cmdScene(int argc, const char **argv);

	/**
	 * Plays a song
	 */
	bool cmdSong(int argc, const char **argv);

	/**
	 * Lists all available songs
	 */
	bool cmdListSongs(int argc, const char **argv);

	/**
	 * Lists all files in a library (use at your own risk)
	 */
	bool cmdListFiles(int argc, const char **argv);

	/**
	 * Dumps a file to disk
	 */
	bool cmdDumpFile(int argc, const char **argv);

	/**
	 * Show all locations on the map
	 */
	bool cmdLocations(int argc, const char **argv);

	/**
	 * Get or set the value of a flag
	 */
	bool cmdFlag(int argc, const char **argv);
protected:
	SherlockEngine *_vm;
	Common::String _3doPlayMovieFile;
public:
	AllLocations _showAllLocations;
public:
	Debugger(SherlockEngine *vm);
	~Debugger() override {}
	static Debugger *init(SherlockEngine *vm);

	void postEnter() override;
};

} // End of namespace Sherlock

#endif	/* SHERLOCK_DEBUGGER_H */
