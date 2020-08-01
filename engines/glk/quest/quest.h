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

 /* Based on Geas interpreter version 3.53 */

#ifndef GLK_QUEST_QUEST
#define GLK_QUEST_QUEST

#include "glk/glk_api.h"
#include "glk/quest/string.h"
#include "glk/quest/geas_runner.h"

namespace Glk {
namespace Quest {

/**
 * Quest game interpreter
 */
class Quest : public GlkAPI {
private:
	int _saveSlot;
	GeasRunner *_runner;
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
	void runGame() override;

	/**
	 * Returns the running interpreter type
	 */
	InterpreterType getInterpreterType() const override {
		return INTERPRETER_QUEST;
	}

	/**
	 * Returns true if a savegame can be loaded
	 */
	bool canLoadGameStateCurrently() override {
		return _runner != nullptr && GlkAPI::canLoadGameStateCurrently();
	}

	/**
	 * Returns true if the game can be saved
	 */
	bool canSaveGameStateCurrently() override {
		return _runner != nullptr && GlkAPI::canLoadGameStateCurrently();
	}

	/**
	 * Savegames aren't supported for Quest games
	 */
	Common::Error readSaveData(Common::SeekableReadStream *rs) override;

	/**
	 * Savegames aren't supported for Quest games
	 */
	Common::Error writeGameData(Common::WriteStream *ws) override;

	/**
	 * Returns true if a savegame is being loaded directly from the ScummVM launcher
	 */
	bool loadingSavegame() const { return _saveSlot != -1; }
};

extern Quest *g_vm;

} // End of namespace Quest
} // End of namespace Glk

#endif
