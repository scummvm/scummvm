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

#ifndef ILLUSIONS_SCRIPTRESOURCE_H
#define ILLUSIONS_SCRIPTRESOURCE_H

#include "illusions/resourcesystem.h"

namespace Illusions {

class IllusionsEngine;

class ScriptResourceLoader : public BaseResourceLoader {
public:
	ScriptResourceLoader(IllusionsEngine *vm) : _vm(vm) {}
	virtual ~ScriptResourceLoader() {}
	virtual void load(Resource *resource);
	virtual void unload(Resource *resource);
	virtual void buildFilename(Resource *resource);
	virtual bool isFlag(int flag);
protected:
	IllusionsEngine *_vm;
};

class Properties {
public:
	Properties();
	void init(uint count, byte *properties);
public:
	uint _count;
	byte *_properties;
};

class BlockCounters {
public:
	BlockCounters();
	void init(uint count, byte *blockCounters);
	void clear();
	byte get(uint index);
	void set(uint index, byte value);
public:
	uint _count;
	byte *_blockCounters;
};

struct TriggerCause {
	uint32 _verbId;
	uint32 _objectId2;
	uint32 _codeOffs;
	void load(Common::SeekableReadStream &stream);
};

class TriggerObject {
public:
	TriggerObject();
	~TriggerObject();
	void load(byte *dataStart, Common::SeekableReadStream &stream);
public:
	uint32 _objectId;
	uint _causesCount;
	TriggerCause *_causes;
};

class ProgInfo {
public:
	ProgInfo();
	~ProgInfo();
	void load(byte *dataStart, Common::SeekableReadStream &stream);
protected:
	uint16 _id;
	uint16 _unk;
	byte *_name;
	uint _triggerObjectsCount;
	TriggerObject *_triggerObjects;
};

class ScriptResource {
public:
	ScriptResource();
	~ScriptResource();
	void load(byte *data, uint32 dataSize);
	byte *getThreadCode(uint32 threadId);
	ProgInfo *getProgInfo(uint32 index);
public:
	byte *_data;
	uint32 _dataSize;
	Properties _properties;
	BlockCounters _blockCounters;
	uint _codeCount;
	uint32 *_codeOffsets;
	uint _progInfosCount;
	ProgInfo *_progInfos;
};

} // End of namespace Illusions

#endif // ILLUSIONS_ACTORRESOURCE_H
