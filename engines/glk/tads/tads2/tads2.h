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

#ifndef GLK_TADS_TADS2
#define GLK_TADS_TADS2

#include "glk/tads/tads.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/**
 * TADS 2 game interpreter
 */
class TADS2 : public TADS {
public:
	/**
	 * Constructor
	 */
	TADS2(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Execute the game
	 */
	void runGame() override;

	/**
	 * Returns the running interpreter type
	 */
	InterpreterType getInterpreterType() const override { return INTERPRETER_TADS2; }
};

//typedef TADS2 appctxdef;

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
