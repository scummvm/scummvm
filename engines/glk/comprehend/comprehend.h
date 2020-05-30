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

#ifndef GLK_COMPREHEND_COMPREHEND_H
#define GLK_COMPREHEND_COMPREHEND_H

#include "common/scummsys.h"
#include "glk/glk_api.h"
#include "glk/window_graphics.h"
#include "glk/window_text_buffer.h"
#include "glk/comprehend/game.h"

namespace Glk {
namespace Comprehend {

#undef printf
#define printf debugN
#undef getchar
#define getchar() (0)

#define PATH_MAX 256

struct GameInfo;
struct gameState;

#define EXTRA_STRING_TABLE(x) (0x8200 | (x))

struct GameStrings {
	uint16 game_restart;
};

/**
 * Comprehend engine
 */
class Comprehend : public GlkAPI {
private:
	int _saveSlot;		 ///< Save slot when loading savegame from launcher
public:
	GraphicsWindow *_graphicsWindow;
	TextBufferWindow *_textBufferWindow;
private:
	/**
	 * Initialization code
	 */
	void initialize();

	/**
	 * Deinitialization
	 */
	void deinitialize();

	ComprehendGame *createGame();

public:
	/**
	 * Constructor
	 */
	Comprehend(OSystem *syst, const GlkGameDescription &gameDesc);

	~Comprehend() override;

	/**
	 * Returns the running interpreter type
	 */
	InterpreterType getInterpreterType() const override { return INTERPRETER_SCOTT; }

	/**
	 * Execute the game
	 */
	void runGame() override;

	/**
	 * Load a savegame from the passed Quetzal file chunk stream
	 */
	Common::Error readSaveData(Common::SeekableReadStream *rs) override {
		return Common::kReadingFailed;
	}

	/**
	 * Save the game. The passed write stream represents access to the UMem chunk
	 * in the Quetzal save file that will be created
	 */
	Common::Error writeGameData(Common::WriteStream *ws) override {
		return Common::kWritingFailed;
	}
};

extern Comprehend *g_comprehend;

} // End of namespace Comprehend
} // End of namespace Glk

#endif
