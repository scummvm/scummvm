/* Copyright (C) 1994-2003 Revolution Software Ltd
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

#include "memory.h"

#define	MAX_res_files	20

class Sword2ResourceManager {
public:
	void init(void);		// read in the config file
	void exit(void);

	// Returns ad of resource. Loads if not in memory. Retains a count.
	// Resource can be aged out of memory if count = 0
	// The resource is locked while count != 0
	// Resource floats when count = 0

	uint8 *open(uint32 res);
	void close(uint32 res);		// decrements the count

	// returns '0' if resource out of range or null, otherwise '1' for ok

	uint8 checkValid(uint32 res);

	//for mem_view to query the owners of mem blocs

	char *fetchCluster(uint32 res);
	uint32 fetchAge(uint32 res);
	uint32 fetchCount(uint32 count);

	uint32 helpTheAgedOut(void);

	uint32 fetchLen(uint32 res);

	void nextCycle(void);
	uint32 fetchUsage(void);

	// Prompts the user for the specified CD.
	void getCd(int cd);

	int whichCd() {
		return _curCd;
	}

	// ----console commands

	void printConsoleClusters(void);
	void examine(uint8 *input);
	void kill(uint8 *res);
	void killAll(uint8 wantInfo);
	void killAllObjects(uint8 wantInfo);
	void remove(uint32 res);
	void removeAll(void);

	// pointer to a pointer (or list of pointers in-fact)
	mem **_resList;

private:
	int _curCd;
	uint32 _totalResFiles;
	uint32 _totalClusters;
	uint32 _currentMemoryUsage;

	// Inc's each time open is called and is given to resource as its age.
	// Cannot be allowed to start at 0! (A pint if you can tell me why)

	uint32 _resTime;

	uint32 *_age;

	// Gode generated res-id to res number/rel number conversion table

	uint16 *_resConvTable;

	uint16 *_count;
	char _resourceFiles[MAX_res_files][20];
	uint8 _cdTab[MAX_res_files];		// Location of each cluster.

	// Drive letter of the CD-ROM drive or false CD path.

	char _cdPath[256];

	void cacheNewCluster(uint32 newCluster);
	char _cdDrives[24];
};							

extern Sword2ResourceManager res_man;	//declare the object global

#endif
