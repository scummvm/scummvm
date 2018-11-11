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

#ifndef GARGOYLE_FROTZ_MEM
#define GARGOYLE_FROTZ_MEM

#include "gargoyle/frotz/frotz_types.h"

namespace Gargoyle {
namespace Frotz {

class Mem {
	struct StoryEntry {
		Story _storyId;
		zword _release;
		char _serial[7];
	};
	static const StoryEntry RECORDS[25];
private:
	Common::SeekableReadStream *story_fp;
	uint blorb_ofs, blorb_len;
private:
	/**
	 * Handles setting the story file, parsing it if it's a Blorb file
	 */
	void initializeStoryFile();
public:
	/**
	 * Constructor
	 */
	Mem();

	/**
	 * Initialize
	 */
	void initialize();
};

} // End of namespace Frotz
} // End of namespace Gargoyle

#endif
