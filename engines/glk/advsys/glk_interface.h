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

#ifndef GLK_ADVSYS_GLK_INTERFACE
#define GLK_ADVSYS_GLK_INTERFACE

#include "glk/glk_api.h"

namespace Glk {
namespace AdvSys {

/**
 * Interface class that sits between AdvSys and the GLK base, providing methods for
 * input and output
 */
class GlkInterface : public GlkAPI {
private:
	winid_t _window;
protected:
	int _saveSlot;
	Common::String _pendingLine;
protected:
	/**
	 * GLK initialization
	 */
	bool initialize();

	/**
	 * Print a string
	 * @param msg		String
	 */
	void print(const Common::String &msg);

	/**
	 * Print a unicode string
	 * @param msg		U32String
	 */
	void print(const Common::U32String &msg);

	/**
	 * Print a number
	 * @param number	Number to print
	 */
	void print(int number);

	/**
	 * Get an input line
	 */
	Common::String readLine();
public:
	/**
	 * Constructor
	 */
	GlkInterface(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		_window(nullptr), _saveSlot(-1) {}
};

} // End of namespace AdvSys
} // End of namespace Glk

#endif
