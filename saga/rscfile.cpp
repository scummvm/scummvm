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
 
    RSC Resource file management module

 Notes: 
*/

#include "reinherit.h"

#include "yslib.h"

/*
 * Uses the following modules:
\*--------------------------------------------------------------------------*/

/*
 * Begin module
\*--------------------------------------------------------------------------*/
#include "rscfile_mod.h"
#include "rscfile.h"

namespace Saga {

R_RSCFILE_CONTEXT *RSC_CreateContext(void)
{
	R_RSCFILE_CONTEXT empty_context = { 0 };
	R_RSCFILE_CONTEXT *new_context;

	new_context = (R_RSCFILE_CONTEXT *)malloc(sizeof *new_context);
	if (new_context == NULL) {
		return NULL;
	}

	*new_context = empty_context;

	return new_context;
}

int RSC_OpenContext(R_RSCFILE_CONTEXT * rsc_context, const char *fspec)
{
	FILE *rsc_fp;
	int result;

	rsc_fp = fopen(fspec, "rb");
	if (rsc_fp == NULL) {
		return R_FAILURE;
	}

	if (rsc_context->rc_file_open) {
		return R_FAILURE;
	}

	rsc_context->rc_file_fspec = fspec;
	rsc_context->rc_file_p = rsc_fp;

	result = ys_get_filesize(rsc_fp, &rsc_context->rc_file_size, NULL);
	if (result != YS_E_SUCCESS) {
		fclose(rsc_fp);
		return R_FAILURE;
	}

	if (RSC_LoadRSC(rsc_context) != R_SUCCESS) {
		return R_FAILURE;
	}

	rsc_context->rc_file_open = 1;
	rsc_context->rc_file_loaded = 1;

	return R_SUCCESS;
}

int RSC_CloseContext(R_RSCFILE_CONTEXT * rsc_context)
{
	if (rsc_context->rc_file_open) {
		fclose(rsc_context->rc_file_p);
	}

	rsc_context->rc_file_open = 0;

	RSC_FreeRSC(rsc_context);

	rsc_context->rc_file_loaded = 0;

	return R_SUCCESS;
}

int RSC_DestroyContext(R_RSCFILE_CONTEXT * rsc_context)
{
	RSC_CloseContext(rsc_context);

	if (rsc_context->rc_file_loaded) {
		free(rsc_context->rc_res_table);
	}

	free(rsc_context);

	return R_SUCCESS;
}

int RSC_LoadRSC(R_RSCFILE_CONTEXT * rsc)
{
	ulong res_tbl_ct;
	ulong res_tbl_offset;

	uchar tblinfo_buf[RSC_TABLEINFO_SIZE];
	uchar *tbl_buf;
	size_t tbl_len;
	ulong i;

	R_RSCFILE_RESOURCE *rsc_restbl;

	const uchar *read_p;

	read_p = tblinfo_buf;

	if (rsc->rc_file_size < RSC_MIN_FILESIZE) {
		return R_FAILURE;
	}

	/* Read resource table info from the rear end of file
	 * \*------------------------------------------------------------- */
	fseek(rsc->rc_file_p, (long)(rsc->rc_file_size - 8), SEEK_SET);

	if (fread(tblinfo_buf,
		1, RSC_TABLEINFO_SIZE, rsc->rc_file_p) != RSC_TABLEINFO_SIZE) {

		return R_FAILURE;
	}

	res_tbl_offset = ys_read_u32_le(read_p, &read_p);
	res_tbl_ct = ys_read_u32_le(read_p, NULL);

	/* Check for sane table offset
	 * \*------------------------------------------------------------- */
	if (res_tbl_offset != rsc->rc_file_size - RSC_TABLEINFO_SIZE -
	    RSC_TABLEENTRY_SIZE * res_tbl_ct) {

		return R_FAILURE;
	}

	/* Load resource table
	 * \*------------------------------------------------------------- */
	tbl_len = RSC_TABLEENTRY_SIZE * res_tbl_ct;

	tbl_buf = (uchar *)malloc(tbl_len);
	if (tbl_buf == NULL) {
		return R_FAILURE;
	}

	fseek(rsc->rc_file_p, (long)res_tbl_offset, SEEK_SET);

	if (fread(tbl_buf, 1, tbl_len, rsc->rc_file_p) != tbl_len) {
		free(tbl_buf);
		return R_FAILURE;
	}

	rsc_restbl = (R_RSCFILE_RESOURCE *)malloc(res_tbl_ct * sizeof *rsc_restbl);
	if (rsc_restbl == NULL) {
		free(tbl_buf);
		return R_FAILURE;
	}

	read_p = tbl_buf;

	for (i = 0; i < res_tbl_ct; i++) {

		rsc_restbl[i].res_offset = ys_read_u32_le(read_p, &read_p);
		rsc_restbl[i].res_size = ys_read_u32_le(read_p, &read_p);

		if ((rsc_restbl[i].res_offset > rsc->rc_file_size) ||
		    (rsc_restbl[i].res_size > rsc->rc_file_size)) {

			free(tbl_buf);
			free(rsc_restbl);
			return R_FAILURE;
		}
	}

	rsc->rc_res_table = rsc_restbl;
	rsc->rc_res_ct = res_tbl_ct;

	free(tbl_buf);

	return R_SUCCESS;
}

int RSC_FreeRSC(R_RSCFILE_CONTEXT * rsc)
{
	if (!rsc->rc_file_loaded) {
		return R_FAILURE;
	}

	free(rsc->rc_res_table);

	return R_SUCCESS;
}

ulong RSC_GetResourceCount(R_RSCFILE_CONTEXT * rsc)
{
	return (rsc == NULL) ? 0 : rsc->rc_res_ct;
}

int
RSC_GetResourceSize(R_RSCFILE_CONTEXT * rsc, ulong res_num, ulong * res_size)
{
	if ((rsc == NULL) || (res_size == NULL)) {
		return R_FAILURE;
	}

	if (res_num > (rsc->rc_res_ct - 1)) {
		return R_FAILURE;
	}

	*res_size = rsc->rc_res_table[res_num].res_size;

	return R_SUCCESS;
}

int
RSC_GetResourceOffset(R_RSCFILE_CONTEXT * rsc,
    ulong res_num, ulong * res_offset)
{
	if ((rsc == NULL) || (res_offset == NULL)) {
		return R_FAILURE;
	}

	if (res_num > (rsc->rc_res_ct - 1)) {
		return R_FAILURE;
	}

	*res_offset = rsc->rc_res_table[res_num].res_offset;

	return R_SUCCESS;
}

int
RSC_LoadResource(R_RSCFILE_CONTEXT * rsc,
    ulong res_num, uchar ** res_p, size_t * res_size_p)
{
	ulong res_offset;
	size_t res_size;
	uchar *res_buf;

	if ((rsc == NULL) || (res_p == NULL)) {
		return R_FAILURE;
	}

	if (res_num > (rsc->rc_res_ct - 1)) {
		return R_FAILURE;
	}

	res_offset = rsc->rc_res_table[res_num].res_offset;
	res_size = rsc->rc_res_table[res_num].res_size;

	if (fseek(rsc->rc_file_p, (long)res_offset, SEEK_SET) != 0) {
		return R_FAILURE;
	}

	res_buf = (uchar *)malloc(res_size);
	if (res_buf == NULL) {
		return R_MEM;
	}

	if (fread(res_buf, 1, res_size, rsc->rc_file_p) != res_size) {
		free(res_buf);
		return R_FAILURE;
	}

	*res_p = res_buf;

	if (res_size_p != NULL) {
		*res_size_p = res_size;
	}

	return R_SUCCESS;
}

int RSC_FreeResource(uchar * resource_ptr)
{

	free(resource_ptr);

	return R_SUCCESS;
}

} // End of namespace Saga
