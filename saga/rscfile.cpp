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

// RSC Resource file management module
#include "saga/saga.h"

#include "saga/rscfile_mod.h"
#include "saga/rscfile.h"
#include "saga/stream.h"

namespace Saga {

static struct Substitutes {
	uint32 id;
	const char *fname;
} substitutes[] = {
	{ 1529, "wyrm.pak" },
	{ 1530, "wyrm1.dlt" },
	{ 1531, "wyrm2.dlt" },
	{ 1532, "wyrm3.dlt" },
	{ 1533, "wyrm4.dlt" },
	{ 1796, "credit3n.dlt" },
	{ 1797, "credit4n.dlt" }
};
	

RSCFILE_CONTEXT *RSC_CreateContext() {
	RSCFILE_CONTEXT empty_context;
	empty_context.rc_file_fspec = NULL;
	empty_context.rc_file_loaded = 0;
	empty_context.rc_res_table = NULL;
	empty_context.rc_res_ct = 0;
	empty_context.rc_file = new Common::File();
	RSCFILE_CONTEXT *new_context;

	new_context = (RSCFILE_CONTEXT *)malloc(sizeof(*new_context));
	if (new_context == NULL) {
		return NULL;
	}

	*new_context = empty_context;

	return new_context;
}

int RSC_OpenContext(RSCFILE_CONTEXT *rsc_context, const char *fspec) {
	if (rsc_context->rc_file->isOpen()) {
		return FAILURE;
	}

	if (!rsc_context->rc_file->open(fspec)) {
		return FAILURE;
	}

	rsc_context->rc_file_fspec = fspec;

	if (RSC_LoadRSC(rsc_context) != SUCCESS) {
		return FAILURE;
	}

	rsc_context->rc_file_loaded = 1;

	return SUCCESS;
}

int RSC_CloseContext(RSCFILE_CONTEXT *rsc_context) {
	if (rsc_context->rc_file->isOpen()) {
		rsc_context->rc_file->close();
	}

	RSC_FreeRSC(rsc_context);

	rsc_context->rc_file_loaded = 0;

	return SUCCESS;
}

int RSC_DestroyContext(RSCFILE_CONTEXT *rsc_context) {
	RSC_CloseContext(rsc_context);

	if (rsc_context->rc_file_loaded) {
		free(rsc_context->rc_res_table);
	}

	free(rsc_context);

	return SUCCESS;
}

int RSC_LoadRSC(RSCFILE_CONTEXT *rsc) {
	uint32 res_tbl_ct;
	uint32 res_tbl_offset;

	byte tblinfo_buf[RSC_TABLEINFO_SIZE];
	byte *tbl_buf;
	size_t tbl_len;
	uint32 i;

	RSCFILE_RESOURCE *rsc_restbl;

	if (rsc->rc_file->size() < RSC_MIN_FILESIZE) {
		return FAILURE;
	}

	// Read resource table info from the rear end of file
	rsc->rc_file->seek((long)(rsc->rc_file->size() - 8), SEEK_SET);

	if (rsc->rc_file->read(tblinfo_buf, RSC_TABLEINFO_SIZE) != RSC_TABLEINFO_SIZE) {
		return FAILURE;
	}
	
	MemoryReadStreamEndian readS(tblinfo_buf, RSC_TABLEINFO_SIZE, IS_BIG_ENDIAN);

	res_tbl_offset = readS.readUint32();
	res_tbl_ct = readS.readUint32();

	// Check for sane table offset
	if (res_tbl_offset != rsc->rc_file->size() - RSC_TABLEINFO_SIZE - RSC_TABLEENTRY_SIZE * res_tbl_ct) {

		return FAILURE;
	}

	// Load resource table
	tbl_len = RSC_TABLEENTRY_SIZE * res_tbl_ct;

	tbl_buf = (byte *)malloc(tbl_len);
	if (tbl_buf == NULL) {
		return FAILURE;
	}

	rsc->rc_file->seek((long)res_tbl_offset, SEEK_SET);

	if (rsc->rc_file->read(tbl_buf, tbl_len) != tbl_len) {
		free(tbl_buf);
		return FAILURE;
	}

	rsc_restbl = (RSCFILE_RESOURCE *)malloc(res_tbl_ct * sizeof(*rsc_restbl));
	if (rsc_restbl == NULL) {
		free(tbl_buf);
		return FAILURE;
	}

	MemoryReadStreamEndian readS1(tbl_buf, tbl_len, IS_BIG_ENDIAN);

	debug(9, "RSC %s", rsc->rc_file_fspec);
	for (i = 0; i < res_tbl_ct; i++) {
		rsc_restbl[i].res_offset = readS1.readUint32();
		rsc_restbl[i].res_size = readS1.readUint32();
		//debug(9, "#%x Offset:%x Size:%x", i, rsc_restbl[i].res_offset, rsc_restbl[i].res_size);
		if ((rsc_restbl[i].res_offset > rsc->rc_file->size()) || (rsc_restbl[i].res_size > rsc->rc_file->size())) {
			free(tbl_buf);
			free(rsc_restbl);
			return FAILURE;
		}
	}

	rsc->rc_res_table = rsc_restbl;
	rsc->rc_res_ct = res_tbl_ct;

	free(tbl_buf);

	return SUCCESS;
}

int RSC_FreeRSC(RSCFILE_CONTEXT *rsc) {
	if (!rsc->rc_file_loaded) {
		return FAILURE;
	}

	delete rsc->rc_file;
	rsc->rc_file = NULL;

	free(rsc->rc_res_table);

	return SUCCESS;
}

uint32 RSC_GetResourceCount(RSCFILE_CONTEXT *rsc) {
	return (rsc == NULL) ? 0 : rsc->rc_res_ct;
}

int RSC_GetResourceSize(RSCFILE_CONTEXT *rsc, uint32 res_num, uint32 *res_size) {
	if ((rsc == NULL) || (res_size == NULL)) {
		return FAILURE;
	}

	if (res_num > (rsc->rc_res_ct - 1)) {
		return FAILURE;
	}

	*res_size = rsc->rc_res_table[res_num].res_size;

	return SUCCESS;
}

int RSC_GetResourceOffset(RSCFILE_CONTEXT *rsc, uint32 res_num, uint32 *res_offset) {
	if ((rsc == NULL) || (res_offset == NULL)) {
		return FAILURE;
	}

	if (res_num > (rsc->rc_res_ct - 1)) {
		return FAILURE;
	}

	*res_offset = rsc->rc_res_table[res_num].res_offset;

	return SUCCESS;
}

const char *RSC_FileName(RSCFILE_CONTEXT *rsc) {
	return rsc->rc_file_fspec;
}

int RSC_LoadResource(RSCFILE_CONTEXT *rsc, uint32 res_num, byte **res_p, size_t *res_size_p) {
	uint32 res_offset;
	size_t res_size = 0;
	byte *res_buf = NULL;
	int substnum = -1;

	if ((rsc == NULL) || (res_p == NULL)) {
		return FAILURE;
	}

	if (res_num > (rsc->rc_res_ct - 1)) {
		return FAILURE;
	}

	debug(8, "LoadResource %d", res_num);
	for (int i = 0; i < ARRAYSIZE(substitutes); i++)
		if (substitutes[i].id == res_num) {
			substnum = i;
			break;
		}

	if (!(_vm->getFeatures() & GF_WYRMKEEP))
		substnum = -1;

	if (substnum != -1) {
		Common::File in;

		if (in.open(substitutes[substnum].fname)) {
			res_size = in.size();
			if ((res_buf = (byte *)malloc(res_size)) == NULL)
				return MEM;

			in.read(res_buf, res_size);
			in.close();
			debug(8, "LoadResource: substituted resource by %s", substitutes[substnum].fname);
		} else {
			substnum = -1;
		}
	}

	if (substnum == -1) {
		res_offset = rsc->rc_res_table[res_num].res_offset;
		res_size = rsc->rc_res_table[res_num].res_size;

		if ((res_buf = (byte *)malloc(res_size)) == NULL)
			return MEM;

		rsc->rc_file->seek((long)res_offset, SEEK_SET);

		if (rsc->rc_file->read(res_buf, res_size) != res_size) {
			free(res_buf);
			return FAILURE;
		}
	}

	*res_p = res_buf;

	if (res_size_p != NULL) {
		*res_size_p = res_size;
	}

	return SUCCESS;
}

int RSC_FreeResource(byte *resource_ptr) {
	free(resource_ptr);

	return SUCCESS;
}

int RSC_ConvertID(int id) {
	int res = id;

	if (_vm->getFeatures() & GF_MAC_RESOURCES) {
		if (res > 1537)
			res -= 2;
		else if (res == 1535 || res == 1536) {
			error ("Wrong resource number %d for Mac ITE");
		}
	}

	return res;
}

} // End of namespace Saga
