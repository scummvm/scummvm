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

#ifndef ASYLUM_BUNDLE_H_
#define ASYLUM_BUNDLE_H_

#include "common/endian.h"
#include "common/file.h"
#include "common/stream.h"

#include "common/str.h"
#include "common/array.h"

#include "asylum/resource.h"

namespace Asylum {

#define RESMASK "res.0%02d"

class Bundle {

public:
	Bundle();
	Bundle(uint8 fileNum);
	virtual ~Bundle() {}

	uint8*  getData() { return data; }
	Bundle* getEntry(uint32 index) { return entries[index]; }
	void    setEntry(uint32 index, Bundle* value);

	uint8  id;
	uint32 size;
	uint32 offset;
	uint32 numEntries;
	bool   initialized;


protected:
	Common::Array<Bundle*> entries;
	Common::String parseFilename(uint8 fileNum);
	void loadRawRecord(Common::String filename, uint32 index, uint32 length);
	uint32 getNextValidOffset(uint8 index);
	virtual void update(){}

	uint8 *data;

private:

}; // end of class Bundle


} // end of namespace Asylum

#endif
