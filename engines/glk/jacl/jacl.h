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

#ifndef JACL_JACL
#define JACL_JACL

#include "glk/glk_api.h"

namespace Glk {
namespace JACL {

#define GLK

/**
 * JACL game interpreter
 */
class JACL : public GlkAPI {
private:
	int _saveSlot;
private:
	/**
	 * Engine initialization
	 */
	bool initialize();

	/**
	 * Engine cleanup
	 */
	void deinitialize();
public:
	/**
	 * Constructor
	 */
	JACL(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Run the game
	 */
	virtual void runGame() override;

	/**
	 * Returns the running interpreter type
	 */
	virtual InterpreterType getInterpreterType() const override {
		return INTERPRETER_JACL;
	}

	/**
	 * Savegames aren't supported for JACL games
	 */
	virtual Common::Error readSaveData(Common::SeekableReadStream *rs) override;

	/**
	 * Savegames aren't supported for JACL games
	 */
	virtual Common::Error writeGameData(Common::WriteStream *ws) override;

	/**
	 * Returns true if a savegame is being loaded directly from the ScummVM launcher
	 */
	bool loadingSavegame() const {
		return _saveSlot != -1;
	}
};

extern JACL *g_vm;

} // End of namespace JACL
} // End of namespace Glk

#endif
