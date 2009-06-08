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
 */

#ifndef ASYLUM_BUNDLE_H_
#define ASYLUM_BUNDLE_H_

#include "common/endian.h"
#include "common/file.h"
#include "common/stream.h"

#include "common/str.h"
#include "common/array.h"

#include "asylum/resources/resource.h"

namespace Asylum {

class Bundle {

public:
	Bundle();
	Bundle(Common::String filename, uint32 index);
	virtual ~Bundle() {}

	Common::String id;
	uint32 size;
	uint32 numEntries;
	Common::Array<Resource> entries;

	uint8* getData() { return data; }
protected:
	int loadRaw(Common::String filename, uint32 index);
	virtual void update(){}
	uint8 *data;

private:

}; // end of class Bundle


} // end of namespace Asylum

#endif
