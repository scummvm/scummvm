/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef	RESMAN_H
#define	RESMAN_H

namespace Common {
	class File;
}

#define MAX_MEM_CACHE (8 * 1024 * 1024) // we keep up to 8 megs of resource data files in memory
#define	MAX_res_files 20

namespace Sword2 {

class Sword2Engine;

enum {
	BOTH		= 0x0,		// Cluster is on both CDs
	CD1		= 0x1,		// Cluster is on CD1 only
	CD2		= 0x2,		// Cluster is on CD2 only
	LOCAL_CACHE	= 0x4,		// Cluster is cached on HDD
	LOCAL_PERM	= 0x8		// Cluster is on HDD.
};

struct Resource {
	byte *ptr;
	uint32 size;
	uint32 refCount;
	Resource *next, *prev;
};

struct ResourceFile {
	char fileName[20];
	int32 numEntries;
	uint32 *entryTab;
	uint8 cd;
};

class ResourceManager {
private:
	Common::File *openCluFile(uint16 fileNum);
	void readCluIndex(uint16 fileNum, Common::File *file = NULL);
	void removeFromCacheList(Resource *res);
	void addToCacheList(Resource *res);
	void checkMemUsage();

	Sword2Engine *_vm;

	int _curCd;
	uint32 _totalResFiles;
	uint32 _totalClusters;

	// Gode generated res-id to res number/rel number conversion table

	uint16 *_resConvTable;
	ResourceFile _resFiles[MAX_res_files];
	Resource *_resList;

	Resource *_cacheStart, *_cacheEnd;
	uint32 _usedMem; // amount of used memory in bytes

public:
	ResourceManager(Sword2Engine *vm);	// read in the config file
	~ResourceManager();

	uint32 getNumResFiles() { return _totalResFiles; }
	uint32 getNumClusters() { return _totalClusters; }
	ResourceFile *getResFiles() { return _resFiles; }
	Resource *getResList() { return _resList; }

	byte *openResource(uint32 res, bool dump = false);
	void closeResource(uint32 res);

	bool checkValid(uint32 res);
	uint32 fetchLen(uint32 res);

	// Prompts the user for the specified CD.
	void getCd(int cd);

	int whichCd();

	void remove(int res);
	void removeAll();

	// ----console commands

	void killAll(bool wantInfo);
	void killAllObjects(bool wantInfo);
};

} // End of namespace Sword2

#endif
