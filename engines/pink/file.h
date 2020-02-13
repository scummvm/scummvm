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

#ifndef PINK_FILE_H
#define PINK_FILE_H

#include "common/file.h"

namespace Pink {

struct ObjectDescription {
	void load(Common::File &file);

	char name[16];
	uint32 objectsOffset;
	//uint32 objectsCount; never used
	uint32 resourcesOffset;
	uint32 resourcesCount;
};

struct ResourceDescription {
	void load(Common::File &file);

	char name[16];
	uint32 offset;
	uint32 size;
	bool inBro;
};

class PinkEngine;
class Object;

class OrbFile : public Common::File {
public:
	OrbFile();
	~OrbFile() override;
	bool open(const Common::String &name) override;

public:
	void loadGame(PinkEngine *game);
	void loadObject(Object *obj, const Common::String &name);
	void loadObject(Object *obj, ObjectDescription *objDesc);

	ResourceDescription *createResDescTable(ObjectDescription *objDesc);

	ObjectDescription *getObjDesc(const char *name);
	uint32 getTimestamp() { return _timestamp; }

private:
	void seekToObject(const char *name);

private:
	ObjectDescription *_table;
	uint32 _tableSize;
	uint32 _timestamp;
};

class BroFile : public Common::File {
public:
	BroFile() : _timestamp(0) {}
	bool open(const Common::String &name) override;
	uint32 getTimestamp() { return _timestamp; }

private:
	uint32 _timestamp;
};

} // End of namespace Pink

#endif
