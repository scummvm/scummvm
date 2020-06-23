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

#ifndef GLK_ZCODE_QUETZAL
#define GLK_ZCODE_QUETZAL

#include "glk/glk_types.h"
#include "glk/quetzal.h"
#include "glk/zcode/frotz_types.h"

namespace Glk {
namespace ZCode {

class Processor;

class Quetzal {
private:
	Common::SeekableReadStream *_storyFile;
	QuetzalReader _reader;
	QuetzalWriter _writer;
	zword frames[STACK_SIZE / 4 + 1];
public:
	/**
	 * Constructor
	 */
	Quetzal(Common::SeekableReadStream *storyFile) : _storyFile(storyFile) {}

	/*
	 * Save a game using Quetzal format.
	 * @param svf	Savegame file
	 * @param proc	Pointer to the Frotz processor
	 * @param desc	Savegame description
	 * @returns		Returns true if OK, false if failed
	 */
	bool save(Common::WriteStream *svf, Processor *proc, const Common::String &desc);

	/**
	 * Restore a saved game using Quetzal format
	 * @param svf	Savegame file
	 * @param proc	Pointer to the Frotz processor
	 * @returns		Return 2 if OK, 0 if an error occurred before any damage was done,
	 *				-1 on a fatal error
	 */
	int restore(Common::SeekableReadStream *svf, Processor *proc);
};

} // End of namespace ZCode
} // End of namespace Glk

#endif
