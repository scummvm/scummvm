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
 * $URL$
 * $Id$
 */


#ifndef TEENAGENT_PACK_H__
#define TEENAGENT_PACK_H__

#include "common/file.h"

namespace TeenAgent {
class Pack {
	mutable Common::File file;
	uint32 count;
	uint32 *offsets;

public: 
	Pack();
	~Pack();
	void open(const Common::String &filename);
	void close();
	uint32 get_size(uint32 id) const;
	uint32 read(uint32 id, byte *dst, uint32 size) const;
	Common::SeekableReadStream * getStream(uint32 id) const;
};

}

#endif
