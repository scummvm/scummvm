/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $Header$
 */

#ifndef RESOURCE_H
#define RESOURCE_H

namespace Scumm {

enum {
	OF_OWNER_MASK = 0x0F,
	OF_STATE_MASK = 0xF0,

	OF_STATE_SHL = 4
};

enum {
	RF_LOCK = 0x80,
	RF_USAGE = 0x7F,
	RF_USAGE_MAX = RF_USAGE
};


class ResourceIterator {
	uint32 _size;
	uint32 _pos;
	const byte *_ptr;
	bool _smallHeader;
public:
	ResourceIterator(const byte *searchin, bool smallHeader);
	const byte *findNext(uint32 tag);
};

} // End of namespace Scumm

#endif
