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

namespace Sword2 {

#define	MAX_res_files	20

class Sword2Engine;

struct Resource {
	byte *ptr;
	uint32 size;
	uint32 refCount;
	uint32 refTime;
};

class ResourceManager {
public:
	ResourceManager(Sword2Engine *vm);	// read in the config file
	~ResourceManager(void);

	byte *openResource(uint32 res, bool dump = false);
	void closeResource(uint32 res);

	bool checkValid(uint32 res);
	uint32 fetchLen(uint32 res);

	void expireOldResources(void);

	void passTime(void);

	// Prompts the user for the specified CD.
	void getCd(int cd);

	int whichCd() {
		return _curCd;
	}

	void remove(int res);

	// ----console commands

	void printConsoleClusters(void);
	void listResources(uint minCount);
	void examine(int res);
	void kill(int res);
	void killAll(bool wantInfo);
	void killAllObjects(bool wantInfo);
	void removeAll(void);

	Resource *_resList;

private:
	Sword2Engine *_vm;

	int _curCd;
	uint32 _totalResFiles;
	uint32 _totalClusters;

	uint32 _resTime;

	// Gode generated res-id to res number/rel number conversion table

	uint16 *_resConvTable;

	char _resourceFiles[MAX_res_files][20];
	uint8 _cdTab[MAX_res_files];		// Location of each cluster.
};							

} // End of namespace Sword2

#endif
