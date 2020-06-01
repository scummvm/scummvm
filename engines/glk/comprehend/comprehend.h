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

#define PATH_MAX 256

struct GameInfo;
struct gameState;
class DrawSurface;
class Pics;

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
	GraphicsWindow *_topWindow;
	TextBufferWindow *_bottomWindow;
	DrawSurface *_drawSurface;
	ComprehendGame *_game;
	Pics *_pics;
	bool _graphicsEnabled;
	uint _drawFlags;

private:
	/**
	 * Initialization code
	 */
	void initialize();

	/**
	 * Deinitialization
	 */
	void deinitialize();

	/**
	 * Create the debugger
	 */
	void createDebugger() override;

	/**
	 * Creates the appropriate game class
	 */
	void createGame();

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
	Common::Error readSaveData(Common::SeekableReadStream *rs) override;

	/**
	 * Save the game. The passed write stream represents access to the UMem chunk
	 * in the Quetzal save file that will be created
	 */
	Common::Error writeGameData(Common::WriteStream *ws) override;

	/**
	 * Print string to the buffer window
	 */
	void print(const char *fmt, ...);

	/**
	 * Read an input line
	 */
	void readLine(char *buffer, size_t maxLen);

	/**
	 * Read in a character
	 */
	int readChar();

	/**
	 * Draw a location image
	 */
	void drawLocationPicture(int pictureNum, bool clearBg = true);

	/**
	 * Draw an item image
	 */
	void drawItemPicture(int pictureNum);

	/**
	 * Clear the picture area
	 */
	void clearScreen(bool isBright);
};

extern Comprehend *g_comprehend;

} // End of namespace Comprehend
} // End of namespace Glk

#endif
