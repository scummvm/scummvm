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

#ifndef GLK_GLULXE
#define GLK_GLULXE

#include "common/scummsys.h"
#include "glk/glk_api.h"

namespace Glk {
namespace Glulxe {

/**
 * Glulxe game interpreter
 */
class Glulxe : public GlkAPI {
public:
	/**
	 * Constructor
	 */
	Glulxe(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Run the game
	 */
	void runGame(Common::SeekableReadStream *gameFile);

	/**
	 * Returns the running interpreter type
	 */
	virtual InterpreterType getInterpreterType() const override { return INTERPRETER_GLULXE; }

	/**
	 * Load a savegame from the passed stream
	 */
	virtual Common::Error loadGameData(strid_t file) override;

	/**
	 * Save the game to the passed stream
	 */
	virtual Common::Error saveGameData(strid_t file, const Common::String &desc) override;
};

} // End of namespace Glulxe
} // End of namespace Glk

#endif
