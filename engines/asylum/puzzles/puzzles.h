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

#ifndef ASYLUM_PUZZLES_H
#define ASYLUM_PUZZLES_H

#include "asylum/console.h"
#include "asylum/shared.h"

#include "common/serializer.h"

namespace Asylum {

class EventHandler;
class Puzzle;

class Puzzles : public Common::Serializable {
public:
	Puzzles(AsylumEngine *engine);
	~Puzzles();

	/**
	 * Resets puzzles
	 */
	void reset();

	/**
	 * Gets a message handler.
	 *
	 * @param index Zero-based index of the message handler
	 *
	 * @return The message handler.
	 */
	EventHandler* getPuzzle(uint32 index) const;

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s);

private:
	AsylumEngine* _vm;
	Puzzle *_puzzles[17];

	/**
	 * Initializes the puzzles
	 */
	void initPuzzles();

	// Debug
	friend class Console;
};

} // End of namespace Asylum

#endif // ASYLUM_PUZZLES_H
