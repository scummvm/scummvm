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

#ifndef GARGOYLE_FROTZ_FROTZ
#define GARGOYLE_FROTZ_FROTZ

#include "gargoyle/frotz/processor.h"

namespace Gargoyle {
namespace Frotz {

/**
 * Frotz interpreter for Z-code games
 */
class Frotz : public Processor {
public:
	/**
	 * Constructor
	 */
	Frotz(OSystem *syst, const GargoyleGameDescription *gameDesc);

	/**
	 * Destructor
	 */
	virtual ~Frotz();

	/**
	 * Initialization
	 */
	void initialize();

	/**
	 * Execute the game
	 */
	virtual void runGame(Common::SeekableReadStream *gameFile) override;

	/**
	 * Load a savegame
	 */
	virtual Common::Error loadGameState(int slot) override;

	/**
	 * Save the game
	 */
	virtual Common::Error saveGameState(int slot, const Common::String &desc) override;
};

extern Frotz *g_vm;

} // End of namespace Frotz
} // End of namespace Gargoyle

#endif
