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

//#include "src\driver96.h"
#include "memory.h"

#define	MAX_res_files	20

#define	RES_locked	1
#define	RES_perm	2


class	resMan
{
	public:

		void	InitResMan(void);				//read in the config file
		void	Close_ResMan(void);				//Tony29May96
//----
		uint8	*Res_open(uint32 res);			//returns ad of resource. Loads if not in memory
												//retains a count
												//resource can be aged out of memory if count=0
												//the resource is locked while count!=0

		void	Res_close(uint32 res);			//decrements the count

//----

		uint8	Res_check_valid( uint32 res );	// returns '0' if resource out of range or null, otherwise '1' for ok

												//resource floats when count=0
//----
		char	*Fetch_cluster(uint32 res);		//for mem_view to query the owners of mem blocs
		uint32	Fetch_age(uint32 res);			//
		uint32	Fetch_count(uint32 count);		//

		uint32	Help_the_aged_out(void);		//Tony10Oct96

		uint32	Res_fetch_len( uint32 res );	//Tony27Jan96

		void	Res_next_cycle( void );
		uint32	Res_fetch_useage( void );

		void	GetCd(int cd);				// Prompts the user for the specified CD.
		int		WhichCd() {return curCd;}

//----console commands
		void	Print_console_clusters(void);	//Tony10Oct96
		void	Examine_res(uint8 *input);		//Tony23Oct96
		void	Kill_all_res(uint8 wantInfo);	//Tony29Nov96
		void	Kill_all_objects(uint8 wantInfo);	// James17jan97
		void	Remove_res(uint32	res);		//Tony10Jan97
		void	Remove_all_res(void);			// James24mar97
		void	Kill_res(uint8 *res);			//Tony23Oct96
		char	*GetCdPath( void );				// Chris 9Apr97


		mem	**resList;	//pointer to a pointer (or list of pointers in-fact)

	private:

		int		curCd;
		uint32	total_res_files;
		uint32	total_clusters;
		uint32	current_memory_useage;
		uint32	resTime;	//inc's each time Res_open is called and is given to resource as its age
							//cannot be allowed to start at 0! (a pint if you can tell me why)
		uint32	*age;
		uint16	*res_conv_table;	//Gode generated res-id to res number/rel number conversion table
		uint16	*count;
		char	resource_files[MAX_res_files][20];
		uint8	cdTab[MAX_res_files];		// Location of each cluster.
		char	cdPath[256];				// Drive letter of the CD-ROM drive or false CD path.
		void	CacheNewCluster(uint32 newCluster);
		char	cdDrives[24];
};							



extern	resMan	res_man;	//declare the object global

#endif
