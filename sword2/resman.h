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

#define	RES_locked	1
#define	RES_perm	2


class resMan {
public:
	void InitResMan(void);		// read in the config file
	void Close_ResMan(void);

	// Returns ad of resource. Loads if not in memory. Retains a count.
	// Resource can be aged out of memory if count = 0
	// The resource is locked while count != 0
	// Resource floats when count = 0

	uint8 *Res_open(uint32 res);
	void Res_close(uint32 res);	// decrements the count

	// returns '0' if resource out of range or null, otherwise '1' for ok

	uint8 Res_check_valid(uint32 res);

	//for mem_view to query the owners of mem blocs

	char *Fetch_cluster(uint32 res);
	uint32 Fetch_age(uint32 res);
	uint32 Fetch_count(uint32 count);

	uint32 Help_the_aged_out(void);

	uint32 Res_fetch_len(uint32 res);

	void Res_next_cycle( void );
	uint32 Res_fetch_useage( void );

	// Prompts the user for the specified CD.
	void GetCd(int cd);

	int WhichCd() {
		return curCd;
	}

	// ----console commands

	void Print_console_clusters(void);
	void Examine_res(uint8 *input);
	void Kill_all_res(uint8 wantInfo);
	void Kill_all_objects(uint8 wantInfo);
	void Remove_res(uint32 res);
	void Remove_all_res(void);
	void Kill_res(uint8 *res);
	char *GetCdPath(void);

	// pointer to a pointer (or list of pointers in-fact)
	mem **resList;

private:
	int curCd;
	uint32 total_res_files;
	uint32 total_clusters;
	uint32 current_memory_useage;

	// Inc's each time Res_open is called and is given to resource as its
	// age. Ccannot be allowed to start at 0! (A pint if you can tell me
	// why)

	uint32 resTime;

	uint32 *age;

	// Gode generated res-id to res number/rel number conversion table

	uint16 *res_conv_table;

	uint16 *count;
	char resource_files[MAX_res_files][20];
	uint8 cdTab[MAX_res_files];		// Location of each cluster.

	// Drive letter of the CD-ROM drive or false CD path.

	char cdPath[256];

	void CacheNewCluster(uint32 newCluster);
	char cdDrives[24];
};							

extern resMan res_man;	//declare the object global

#endif
