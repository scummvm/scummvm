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

#ifndef MADS_NEBULAR_ROOMS_DIALOG_H
#define MADS_NEBULAR_ROOMS_DIALOG_H

#include "mads/mads.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

#define MAX_CONV_MESSAGE 16

class Dialog {
public:
	int _globalId;
	int _count;
	int16 _quotes[MAX_CONV_MESSAGE];

	/**
	 * Set up a conversation sequence
	 */
	void setup(int globalId, ...);

	/**
	 * Activates the passed set of quotes in the given conversation node
	 */
	void set(int quoteId, ...);

	/**
	 * Returns the bit for a given quote to indicate whether it's active or not or,
	 * if 0 is passed, returns the number of currently active quotes
	 */
	int read(int quoteId);

	/**
	 * Activates or deactivates the specified quote in the given conversation node
	 */
	void write(int quoteId, bool flag);

	/**
	 * Starts the conversation
	 */
	void start();
};

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS

#endif
