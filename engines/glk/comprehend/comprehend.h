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

namespace Glk {
namespace Comprehend {

#undef printf
#define printf debugN
#undef getchar
#define getchar() (0)

#define PATH_MAX 256

struct comprehend_game;
struct game_info;
struct game_state;

#define EXTRA_STRING_TABLE(x) (0x8200 | (x))

struct game_strings {
	uint16 game_restart;
};

#define ROOM_IS_NORMAL 0
#define ROOM_IS_DARK 1
#define ROOM_IS_TOO_BRIGHT 2

struct game_ops {
	void (*before_game)(struct comprehend_game *game);
	void (*before_prompt)(struct comprehend_game *game);
	bool (*before_turn)(struct comprehend_game *game);
	bool (*after_turn)(struct comprehend_game *game);
	int (*room_is_special)(struct comprehend_game *game,
	                       unsigned room_index,
	                       unsigned *room_desc_string);
	void (*handle_special_opcode)(struct comprehend_game *game,
	                              uint8 operand);
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

	comprehend_game *createGame();

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
