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

// Sound resource class header

#ifndef SAGA_SNDRES_H_
#define SAGA_SNDRES_H_

#include "rscfile_mod.h"
#include "game_mod.h"
#include "sound.h"

namespace Saga {

#define R_VOC_TIME_BASE  256000000L
#define R_VOC_TIME_CBASE 65536L

#define R_VOC_FILE_DESC_LEN 20
#define R_VOC_FILE_DESC "Creative Voice File\x1A"

struct R_VOC_HEADER_BLOCK {
	char ft_desc[20]; // BYTE [20]
	uint16 db_offset; // WORD
	uint16 voc_version;// WORD
	uint16 voc_fileid; // WORD
};

#define R_VOC_HEADER_BLOCK_LEN 26

struct R_VOC_GENBLOCK {
	int block_id;		// BYTE
	uint32 block_len;	// BYTE[3]
};

#define R_VOC_GENBLOCK_LEN 4

struct R_VOC_BLOCK1 {
	int block_id; // BYTE
	uint32 block_len; // BYTE[3]
	uint16 time_constant; // BYTE
	int pack_method; // BYTE
};

class SndRes {
public:

	SndRes(SagaEngine *vm);

	int loadSound(uint32 sound_rn);
	int playVoice(uint32 voice_rn);
	int getVoiceLength(uint32 voice_rn);
	int ITEVOC_Resample(long src_freq, long dst_freq, byte *src_buf,
						size_t src_buf_len, byte **dst_buf, size_t *dst_buf_len);

 private:
	int load(R_RSCFILE_CONTEXT *snd_ctxt, uint32 snd_rn, R_SOUNDBUFFER *snd_buf_i);
	int loadVocSound(byte *snd_res, size_t snd_res_len, R_SOUNDBUFFER *snd_buf_i);

	int _init;

	R_RSCFILE_CONTEXT *_sfx_ctxt;
	R_RSCFILE_CONTEXT *_voice_ctxt;

	R_GAME_SOUNDINFO _snd_info;

	SagaEngine *_vm;
};

} // End of namespace Saga

#endif
