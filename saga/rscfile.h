/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

// RSC Resource file management header file

#ifndef SAGA_RSCFILE_H__
#define SAGA_RSCFILE_H__

#include "backends/fs/fs.h"
#include "common/file.h"

namespace Saga {

#define RSC_TABLEINFO_SIZE 8
#define RSC_TABLEENTRY_SIZE 8

#define RSC_MIN_FILESIZE (RSC_TABLEINFO_SIZE + RSC_TABLEENTRY_SIZE + 1)

struct RSCFILE_RESOURCE {
	int res_type;
	size_t res_offset;
	size_t res_size;
};

struct RSCFILE_CONTEXT {
	const char *rc_file_fspec;
	Common::File *rc_file;
	int rc_file_loaded;
	RSCFILE_RESOURCE *rc_res_table;
	size_t rc_res_ct;
};

int RSC_LoadRSC(RSCFILE_CONTEXT *rsc_context);
int RSC_FreeRSC(RSCFILE_CONTEXT *rsc);

} // End of namespace Saga

#endif
