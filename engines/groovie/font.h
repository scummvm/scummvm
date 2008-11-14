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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GROOVIE_FONT_H
#define GROOVIE_FONT_H

#include "common/stream.h"
#include "common/system.h"

namespace Groovie {

class Font {
public:
	Font(OSystem *syst);
	~Font();
	void printstring(char *messagein);

private:
	OSystem *_syst;
	Common::MemoryReadStream *_sphinxfnt;

	uint16 letteroffset(char letter);
	uint8 letterwidth(char letter);
	uint8 letterheight(char letter);
	uint8 printletter(char letter, uint16 xoffset);
};

} // End of Groovie namespace

#endif // GROOVIE_FONT_H
