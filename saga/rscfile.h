/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*

 Description:   
 
    RSC Resource file management header file

 Notes: 
*/

#ifndef SAGA_RSCFILE_H__
#define SAGA_RSCFILE_H__

namespace Saga {

#define RSC_TABLEINFO_SIZE 8
#define RSC_TABLEENTRY_SIZE 8

#define RSC_MIN_FILESIZE (RSC_TABLEINFO_SIZE + RSC_TABLEENTRY_SIZE + 1)

typedef struct R_RSCFILE_RESOURCE_tag {

	int res_type;

	size_t res_offset;
	size_t res_size;

} R_RSCFILE_RESOURCE;

struct R_RSCFILE_CONTEXT_tag {

	const char *rc_file_fspec;
	FILE *rc_file_p;
	int rc_file_open;
	unsigned long rc_file_size;

	int rc_file_loaded;
	R_RSCFILE_RESOURCE *rc_res_table;
	size_t rc_res_ct;

};

int RSC_LoadRSC(R_RSCFILE_CONTEXT * rsc_context);

int RSC_FreeRSC(R_RSCFILE_CONTEXT * rsc);

} // End of namespace Saga

#endif				/* SAGA_RSCFILE_H__ */
