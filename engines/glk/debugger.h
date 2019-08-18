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

#ifndef GLK_DEBUGGER_H
#define GLK_DEBUGGER_H

#include "common/scummsys.h"
#include "common/stream.h"
#include "gui/debugger.h"
#include "glk/raw_decoder.h"

namespace Glk {

class GlkEngine;

class Debugger : public GUI::Debugger {
private:
	/**
	 * Saves a decoded raw image to a PNG file
	 */
	void saveRawPicture(const RawDecoder &rd, Common::WriteStream &ws);

	/**
	 * Dump a picture
	 */
	bool cmdDumpPic(int argc, const char **argv);
protected:
	/**
	 * Convert a numeric string to an integer
	 */
	int strToInt(const char *s);
public:
	Debugger();
};

} // End of namespace Glk

#endif
