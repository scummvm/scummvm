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

#ifndef MADE_DATABASE_H
#define MADE_DATABASE_H

#include "common/array.h"
#include "common/util.h"
#include "common/file.h"
#include "common/stream.h"

#include "made/redreader.h"

namespace Made {

class Object {
public:
	Object();
	~Object();
	void load(Common::SeekableReadStream &source);
	void load(byte *source);

	uint16 getFlags() const;
	uint16 getClass() const;
	uint16 getSize() const;

	byte getCount1() const;
	byte getCount2() const;

	byte *getData();
	const char *getString();
	void setString(const char *str);

	bool isObject();
	bool isVector();
	bool isConstant() const { return !(getFlags() & 1); }
	
	int16 getVectorSize();
	int16 getVectorItem(int16 index);
	void setVectorItem(int16 index, int16 value);

	void dump(const char *filename);

protected:
	bool _freeData;
	uint16 _objSize;
	byte *_objData;
};

class GameDatabase {
public:

	GameDatabase();
	~GameDatabase();

	void open(const char *filename);
	void openFromRed(const char *redFilename, const char *filename);

	Object *getObject(int16 index) const { 
		if (index >= 1)
			return _objects[index - 1];
		else
			return NULL;
	}

	int16 getMainCodeObjectIndex() const { return _mainCodeObjectIndex; }

	int16 getVar(int16 index);
	void setVar(int16 index, int16 value);
	
	int16 *getObjectPropertyPtr(int16 objectIndex, int16 propertyId, int16 &propertyFlag);
	int16 getObjectProperty(int16 objectIndex, int16 propertyId);
	int16 setObjectProperty(int16 objectIndex, int16 propertyId, int16 value);

	void dumpObject(int16 index);
	
protected:
	Common::Array<Object*> _objects;
	byte *_gameState;
	uint32 _gameStateSize;
	int16 _mainCodeObjectIndex;
	void load(Common::SeekableReadStream &sourceS);
};

} // End of namespace Made

#endif /* MADE_H */
