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
 
    Sound resource management module - private header

 Notes: 
*/

#ifndef SAGA_SNDRES_H_
#define SAGA_SNDRES_H_

namespace Saga {

#define R_VOC_TIME_BASE  256000000L
#define R_VOC_TIME_CBASE 65536L

#define R_VOC_FILE_DESC_LEN 20
#define R_VOC_FILE_DESC "Creative Voice File\x1A"

typedef struct R_VOC_HEADER_BLOCK_tag {

	char ft_desc[20];	/* BYTE [20] */
	uint db_offset;		/* WORD */
	uint voc_version;	/* WORD */
	uint voc_fileid;	/* WORD */

} R_VOC_HEADER_BLOCK;

#define R_VOC_HEADER_BLOCK_LEN 26

typedef struct R_VOC_GENBLOCK_tag {

	int block_id;		/* BYTE */
	ulong block_len;	/* BYTE[3] */

} R_VOC_GENBLOCK;

#define R_VOC_GENBLOCK_LEN 4

typedef struct R_VOC_BLOCK1_tag {

	int block_id;		/* BYTE */
	ulong block_len;	/* BYTE[3] */
	uint time_constant;	/* BYTE */
	int pack_method;	/* BYTE */

} R_VOC_BLOCK1;

typedef struct R_SNDRES_MODULE_tag {

	int init;

	R_RSCFILE_CONTEXT *sfx_ctxt;
	R_RSCFILE_CONTEXT *voice_ctxt;

	R_GAME_SOUNDINFO snd_info;

} R_SNDRES_MODULE;

int
SND_Load(R_RSCFILE_CONTEXT * snd_ctxt,
    ulong snd_rn, R_SOUNDBUFFER * snd_buf_i);

int
LoadVocSound(const uchar * snd_res,
    size_t snd_res_len, R_SOUNDBUFFER * snd_buf_i);

} // End of namespace Saga

#endif				/* SAGA_SNDRES_H_ */
