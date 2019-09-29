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

#ifndef GLK_QUEST_QUEST
#define GLK_QUEST_QUEST

#include "glk/glk_api.h"
#include "glk/quest/string.h"

namespace Glk {
namespace Quest {

/**
 * Quest game interpreter
 */
class Quest : public GlkAPI {
private:
	int _saveSlot;
public:
	String banner;
private:
	/**
	 * Engine initialization
	 */
	bool initialize();

	/**
	 * Engine cleanup
	 */
	void deinitialize();

	/**
	 * Inner gameplay method
	 */
	void playGame();
public:
	/**
	 * Constructor
	 */
	Quest(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Run the game
	 */
	virtual void runGame() override;

	/**
	 * Returns the running interpreter type
	 */
	virtual InterpreterType getInterpreterType() const override {
		return INTERPRETER_QUEST;
	}

	/**
	 * Load a savegame from the passed Quetzal file chunk stream
	 */
	virtual Common::Error readSaveData(Common::SeekableReadStream *rs) override;

	/**
	 * Save the game. The passed write stream represents access to the UMem chunk
	 * in the Quetzal save file that will be created
	 */
	virtual Common::Error writeGameData(Common::WriteStream *ws) override;
};

extern Quest *g_vm;

} // End of namespace Quest
} // End of namespace Glk

#endif
