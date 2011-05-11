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
 */

#ifndef SCUMM_RESOURCE_H
#define SCUMM_RESOURCE_H

#include "scumm/scumm.h"	// for rtNumTypes

namespace Scumm {

enum {
	OF_OWNER_MASK = 0x0F,
	OF_STATE_MASK = 0xF0,

	OF_STATE_SHL = 4
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

enum {
	RES_INVALID_OFFSET = 0xFFFFFFFF
};

class ScummEngine;

/**
 * The 'resource manager' class. Currently doesn't really deserve to be called
 * a 'class', at least until somebody gets around to OOfying this more.
 */
class ResourceManager {
	//friend class ScummDebugger;
	//friend class ScummEngine;
protected:
	ScummEngine *_vm;

public:
	byte mode[rtNumTypes];
	uint16 num[rtNumTypes];
	uint32 tags[rtNumTypes];
	const char *name[rtNumTypes];
	byte **address[rtNumTypes];
protected:
	byte *flags[rtNumTypes];
	byte *status[rtNumTypes];
public:
	byte *roomno[rtNumTypes];
	uint32 *roomoffs[rtNumTypes];
	uint32 *globsize[rtNumTypes];

protected:
	uint32 _allocatedSize;
	uint32 _maxHeapThreshold, _minHeapThreshold;
	byte _expireCounter;

public:
	ResourceManager(ScummEngine *vm);
	~ResourceManager();

	void setHeapThreshold(int min, int max);

	void allocResTypeData(int id, uint32 tag, int num, const char *name, int mode);
	void freeResources();

	byte *createResource(int type, int index, uint32 size);
	void nukeResource(int type, int i);

	bool isResourceLoaded(int type, int index) const;

	void lock(int type, int i);
	void unlock(int type, int i);
	bool isLocked(int type, int i) const;

	void setModified(int type, int i);
	bool isModified(int type, int i) const;

	void increaseExpireCounter();
	void setResourceCounter(int type, int index, byte flag);
	void increaseResourceCounter();

	void resourceStats();

//protected:
	bool validateResource(const char *str, int type, int index) const;
protected:
	void expireResources(uint32 size);
};

} // End of namespace Scumm

#endif
