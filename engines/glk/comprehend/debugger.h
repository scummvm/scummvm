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

#ifndef GLK_COMPREHEND_DEBUGGER_H
#define GLK_COMPREHEND_DEBUGGER_H

#include "glk/debugger.h"
#include "glk/comprehend/debugger_dumper.h"

namespace Glk {
namespace Comprehend {

class Debugger : public Glk::Debugger, public DebuggerDumper {
private:
	/**
	 * Dump data
	 */
	bool cmdDump(int argc, const char **argv);

	/**
	 * Sets whether floodfills are done when rendering images
	 */
	bool cmdFloodfills(int argc, const char **argv);

	/**
	 * Sets or lists the current room
	 */
	bool cmdRoom(int argc, const char **argv);

	/**
	 * Sets or lists the room for an item
	 */
	bool cmdItemRoom(int argc, const char **argv);

	/**
	 * Find a string given a partial specified
	 */
	bool cmdFindString(int argc, const char **argv);

	/**
	 * Draw an image to the screen
	 */
	bool cmdDraw(int argc, const char **argv);

	/**
	 * Flags whether to turn the inventory limit on or off
	 */
	bool cmdInventoryLimit(int argc, const char **argv);

protected:
	void print(const char *fmt, ...) override;

public:
	bool _invLimit;
public:
	Debugger();
	virtual ~Debugger();
};

extern Debugger *g_debugger;

} // End of namespace Comprehend
} // End of namespace Glk

#endif
