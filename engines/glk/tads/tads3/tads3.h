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

#ifndef GLK_TADS_TADS3
#define GLK_TADS_TADS3

#include "glk/tads/tads.h"

namespace Glk {
namespace TADS {
namespace TADS3 {

/**
 * TADS 3 game interpreter
 */
class TADS3 : public TADS {
public:
	/**
	 * Constructor
	 */
	TADS3(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Execute the game
	 */
	void runGame() override;

	/**
	 * Returns the running interpreter type
	 */
	InterpreterType getInterpreterType() const override { return INTERPRETER_TADS3; }
};

} // End of namespace TADS3
} // End of namespace TADS
} // End of namespace Glk

#endif
