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

#include "gargoyle/glk.h"
#include "gargoyle/frotz/frotz_types.h"
#include "gargoyle/frotz/buffer.h"
#include "gargoyle/frotz/err.h"
#include "gargoyle/frotz/mem.h"

namespace Gargoyle {
namespace Frotz {

/**
 * Frotz interpreter for Z-code games
 */
class Frotz : public Glk {
private:
	/**
	 * Perform any initialization
	 */
	void initialize();
public:
	UserOptions _options;
	Header _header;
	Buffer _buffer;
	Mem _mem;

	// Story file name, id number and size
	Common::SeekableReadStream *_gameFile;
	Story _storyId;
	size_t _storySize;

	// Stack data
	zword _stack[STACK_SIZE];
	zword *_sp;
	zword *_fp;
	zword _frameCount;

	// IO streams
	bool _ostream_screen;
	bool _ostream_script;
	bool _ostream_memory;
	bool _ostream_record;
	bool _istream_replay;
	bool _message;

	// Current window and mouse data
	int _cwin;
	int _mwin;
	int _mouse_y;
	int _mouse_x;
	int _menu_selected;

	// Window attributes
	bool _enableWrapping;
	bool _enableScripting;
	bool _enableScrolling;
	bool _enableBuffering;

	// Size of memory to reserve (in bytes)
	size_t _reserveMem;
public:
	/**
	 * Constructor
	 */
	Frotz(OSystem *syst, const GargoyleGameDescription *gameDesc);

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
