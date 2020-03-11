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
 */
#ifndef DRAGONS_DRAGONFLG_H
#define DRAGONS_DRAGONFLG_H

#include "common/stream.h"
#include "common/system.h"

namespace Dragons {

class BigfileArchive;

class Properties {
public:
	Properties(uint count);
	~Properties();
	void init(uint count, byte *properties);
	void clear();
	bool get(uint32 propertyId);
	void set(uint32 propertyId, bool value);
	void save(uint numberToWrite, Common::WriteStream *out);
	void print(char *prefix);

private:
	uint _count;
	byte *_properties;
	uint32 getSize();
	void getProperyPos(uint32 propertyId, uint &index, byte &mask);
};

class DragonFLG {
private:
	byte *_data;
	uint32 _dataSize;
	Properties *_properties;

public:
	virtual ~DragonFLG();

	DragonFLG(BigfileArchive *bigfileArchive);
	bool get(uint32 propertyId);
	void set(uint32 propertyId, bool value);

	void saveState(Common::WriteStream *out);
	void loadState(Common::ReadStream *in);
};

} // End of namespace Dragons

#endif //DRAGONS_DRAGONFLG_H
