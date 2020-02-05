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

#ifndef GLK_FROTZ_FROTZ
#define GLK_FROTZ_FROTZ

#include "glk/frotz/processor.h"

namespace Glk {
namespace Frotz {

class FrotzScreen;

/**
 * Frotz interpreter for Z-code games
 */
class Frotz : public Processor {
	friend class FrotzScreen;
protected:
	/**
	 * Setup the video mode
	 */
	void initGraphicsMode() override;

	/**
	 * Create the screen class
	 */
	Screen *createScreen() override;
public:
	/**
	 * Constructor
	 */
	Frotz(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Destructor
	 */
	~Frotz() override;

	/**
	 * Initialization
	 */
	void initialize();

	/**
	 * Returns the running interpreter type
	 */
	InterpreterType getInterpreterType() const override { return INTERPRETER_FROTZ; }

	/**
	 * Execute the game
	 */
	void runGame() override;

	/**
	 * Load a savegame from a given slot
	 */
	Common::Error loadGameState(int slot) override;

	/**
	 * Save the game to a given slot
	 */
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;

	/**
	 * Loading method not used for Frotz sub-engine
	 */
	Common::Error readSaveData(Common::SeekableReadStream *rs) override { return Common::kReadingFailed; }

	/**
	 * Saving method not used for Frotz sub-engine
	 */
	Common::Error writeGameData(Common::WriteStream *ws) override { return Common::kWritingFailed; }

};

extern Frotz *g_vm;

} // End of namespace Frotz
} // End of namespace Glk

#endif
