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

/* Based on Alan2 interpreter version 2.8(6) */

#ifndef GLK_ALAN2
#define GLK_ALAN2

#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/stack.h"
#include "glk/glk_api.h"

namespace Glk {
namespace Alan2 {

/**
 * Alan2 game interpreter
 */
class Alan2 : public GlkAPI {
private:
	bool _restartFlag;
public:
	bool vm_exited_cleanly;
	Common::String _advName;
	int _saveSlot;
	bool _pendingLook;
private:
	/**
	 * Initialization
	 */
	bool initialize();

	/**
	 * Deinitialization
	 */
	void deinitialize();

	/**
	 * Synchronize data to or from a save file
	 */
	void synchronizeSave(Common::Serializer &s);
public:
	/**
	 * Constructor
	 */
	Alan2(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Run the game
	 */
	void runGame() override;

	/**
	 * Flag for the game to restart
	 */
	void setRestart(bool flag) { _restartFlag = flag; }

	/**
	 * Returns whether the game should restart
	 */
	bool shouldRestart() const { return _restartFlag; }

	/**
	 * Returns the running interpreter type
	 */
	InterpreterType getInterpreterType() const override {
		return INTERPRETER_ALAN2;
	}

	/**
	 * Load a savegame from the passed Quetzal file chunk stream
	 */
	Common::Error readSaveData(Common::SeekableReadStream *rs) override;

	/**
	 * Save the game. The passed write stream represents access to the UMem chunk
	 * in the Quetzal save file that will be created
	 */
	Common::Error writeGameData(Common::WriteStream *ws) override;

};

extern Alan2 *g_vm;

} // End of namespace Alan2
} // End of namespace Glk

#endif
