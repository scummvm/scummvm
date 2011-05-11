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
 * The mode of a resource type indicates whether the resource can be restored
 * from the game data files or not.
 * This affects for example whether the resource is stored in savestates.
 *
 * Note that we treat sound resources somewhat differently: On the one hand,
 * these behave mostly like a kStaticResTypeMode res type. However, when we
 * create a savestate, we do save *some* information about them: Namely, which
 * sound resources are loaded in memory at the time the save is made. And when
 * loading, we invoke ensureResourceLoaded() for each sound resource that was
 * marked in this way.
 */
enum ResTypeMode {
	kDynamicResTypeMode = 0,	///!< Resource is generated during runtime and may change
	kStaticResTypeMode = 1,		///!< Resource comes from data files, does not change
	kSoundResTypeMode = 2		///!< Resource comes from data files, but may change
};

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
	/**
	 * This struct represents a resource type and all resource of that type.
	 */
	class ResTypeData {
	friend class ResourceManager;
	public:
		ResTypeMode _mode;
		/**
		 * The maximal number of resource of this type. Determines the size
		 * of various arrays.
		 */
		uint16 _num;

		/**
		 * The 4-byte tag or chunk type associated to this resource type, if any.
		 * Only applies to resources that are loaded from the game data files.
		 */
		uint32 _tag;

		/**
		 * Array of size _num containing pointers to each resource of this type.
		 */
		byte **_address;

		/**
		 * Array of size _num containing the sizes of each resource of this type.
		 */
		uint32 *_size;
	protected:
		/**
		 * Array of size _num containing TODO of each resource of this type.
		 */
		byte *flags;

		/**
		 * Array of size _num containing the status of each resource of this type.
		 * This is a bitfield of which currently only one bit is used, which indicates
		 * whether the resource is modified.
		 */
		byte *_status;
	public:
		/**
		 * Array of size _num containing for each resource of this type the
		 * id of the room (resp. the disk) the resource is contained in.
		 */
		byte *roomno;

		/**
		 * Array of size _num containing room offsets of each resource of this type.
		 * That is the offset (in bytes) where the data for this resources
		 * can be found in the game data file(s), relative to the start
		 * of the room the resource is contained in.
		 *
		 * A value of RES_INVALID_OFFSET indicates a resources that is not contained
		 * in the game data files.
		 */
		uint32 *roomoffs;

		/**
		 * Array of size _num. Occurs in HE 70+, but we don't use it for anything.
		 */
		uint32 *globsize;

	public:
		ResTypeData();
		~ResTypeData();
	};
	ResTypeData _types[rtNumTypes];

protected:
	uint32 _allocatedSize;
	uint32 _maxHeapThreshold, _minHeapThreshold;
	byte _expireCounter;

public:
	ResourceManager(ScummEngine *vm);
	~ResourceManager();

	void setHeapThreshold(int min, int max);

	void allocResTypeData(int id, uint32 tag, int num, ResTypeMode mode);
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
