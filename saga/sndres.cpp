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
 
    Sound resource management class

 Notes: 
*/

#include "saga.h"
#include "reinherit.h"

#include "yslib.h"

#include <limits.h>

/*
 * Uses the following modules:
\*--------------------------------------------------------------------------*/
#include "game_mod.h"
#include "rscfile_mod.h"

/*
 * Begin module component
\*--------------------------------------------------------------------------*/
#include "sndres.h"
#include "sound.h"

namespace Saga {

SndRes::SndRes(SagaEngine *vm) {
	int result;

	/* Load sound module resource file contexts */
	result = GAME_GetFileContext(&_sfx_ctxt, R_GAME_SOUNDFILE, 0);
	if (result != R_SUCCESS) {
		return;
	}

	result = GAME_GetFileContext(&_voice_ctxt,
	    R_GAME_VOICEFILE, 0);
	if (result != R_SUCCESS) {
		return;
	}

	/* Grab sound resource information for the current game */
	GAME_GetSoundInfo(&_snd_info);

	_vm = vm;
	_init = 1;
}

int SndRes::playVoice(ulong voice_rn) {
	R_SOUNDBUFFER snd_buffer;
	int result;

	debug(0, "SndRes::playVoice(%ld)", voice_rn);

	// FIXME: In the Wyrmkeep re-release of Inherit the Earth, voices 4 and
	// 5 are identical, even though their resources are stored at different
	// offsets in the resource file. The correct sound for voice 4 is
	// provided as a separate file, sound/p2_a.voc

	result = load(_voice_ctxt, voice_rn, &snd_buffer);
	if (result != R_SUCCESS) {
		return R_FAILURE;
	}

	_vm->_sound->playVoice(&snd_buffer);

	return R_SUCCESS;
}

int SndRes::load(R_RSCFILE_CONTEXT *snd_ctxt, ulong snd_rn, R_SOUNDBUFFER *snd_buf_i) {
	uchar *snd_res;
	size_t snd_res_len;

	int result;

	assert((snd_ctxt != NULL) && (snd_buf_i != NULL));

	result = RSC_LoadResource(snd_ctxt, snd_rn, &snd_res, &snd_res_len);
	if (result != R_SUCCESS) {
		return R_FAILURE;
	}

	switch (_snd_info.res_type) {
	case R_GAME_SOUND_PCM:
		snd_buf_i->s_freq = _snd_info.freq;
		snd_buf_i->s_samplebits = _snd_info.sample_size;
		snd_buf_i->s_stereo = _snd_info.stereo;

		snd_buf_i->res_data = snd_res;
		snd_buf_i->res_len = snd_res_len;

		snd_buf_i->s_buf = snd_res;
		snd_buf_i->s_buf_len = snd_res_len;

		snd_buf_i->s_signed = 1;

		break;

	case R_GAME_SOUND_VOC:
		if (loadVocSound(snd_res, snd_res_len, snd_buf_i) != R_SUCCESS) {

			RSC_FreeResource(snd_res);

			return R_FAILURE;
		}

		break;

	default:
		/* Unknown sound type */
		RSC_FreeResource(snd_res);

		return R_FAILURE;
		break;
	}

	return R_SUCCESS;
}

int SndRes::loadVocSound(byte *snd_res, size_t snd_res_len, R_SOUNDBUFFER *snd_buf_i) {
	R_VOC_HEADER_BLOCK voc_hb;
	R_VOC_GENBLOCK voc_gb;
	R_VOC_BLOCK1 voc_b1;

	long byte_rate;

	const uchar *read_p;
	size_t read_len;

	read_p = snd_res;
	read_len = snd_res_len;

	if (read_len < R_VOC_HEADER_BLOCK_LEN) {
		return R_FAILURE;
	}

	memcpy(voc_hb.ft_desc, read_p, R_VOC_FILE_DESC_LEN);
	read_p += R_VOC_FILE_DESC_LEN;
	read_len -= R_VOC_FILE_DESC_LEN;

	if (memcmp(voc_hb.ft_desc, R_VOC_FILE_DESC, R_VOC_FILE_DESC_LEN) != 0) {
		/* Voc file desc string not found */
		return R_FAILURE;
	}

	voc_hb.db_offset = ys_read_u16_le(read_p, &read_p);
	voc_hb.voc_version = ys_read_u16_le(read_p, &read_p);
	voc_hb.voc_fileid = ys_read_u16_le(read_p, &read_p);

	if (read_len < voc_hb.db_offset + R_VOC_GENBLOCK_LEN) {
		return R_FAILURE;
	}

	read_p = snd_res + voc_hb.db_offset;
	read_len = snd_res_len - voc_hb.db_offset;

	for (;;) {
		/* Read generic block header */
		if (read_len < R_VOC_GENBLOCK_LEN) {
			return R_FAILURE;
		}

		voc_gb.block_id = ys_read_u8(read_p, &read_p);
		if (voc_gb.block_id == 0) {
			return R_FAILURE;
		}

		voc_gb.block_len = ys_read_u24_le(read_p, &read_p);

		read_len -= R_VOC_GENBLOCK_LEN;

		/* Process block */
		switch (voc_gb.block_id) {
		case 1:	/* Sound data block */
			voc_b1.time_constant = ys_read_u8(read_p, &read_p);
			voc_b1.pack_method = ys_read_u8(read_p, &read_p);
			read_len -= 2;

			if (voc_b1.pack_method != 0) {
				debug(0, "Packed VOC files not supported");
				return R_FAILURE;
			}

			byte_rate = R_VOC_TIME_BASE / (R_VOC_TIME_CBASE -
			    (voc_b1.time_constant << 8));

			snd_buf_i->s_stereo = 0;
			snd_buf_i->s_samplebits = 8;
			snd_buf_i->s_freq = byte_rate;

			snd_buf_i->res_data = snd_res;
			snd_buf_i->res_len = snd_res_len;

			snd_buf_i->s_buf = read_p;
			snd_buf_i->s_buf_len = read_len - 1;	/* -1 for end block */

			snd_buf_i->s_signed = 0;

			return R_SUCCESS;

			break;

		default:
			read_p += voc_gb.block_len;
			read_len -= voc_gb.block_len;
			break;
		}
	}

	return R_SUCCESS;
}

int SndRes::getVoiceLength(ulong voice_rn) {
	ulong length;

	double ms_f;
	int ms_i = -1;

	int result;

	assert(_init);

	result = RSC_GetResourceSize(_voice_ctxt, voice_rn, &length);
	if (result != R_SUCCESS) {
		return -1;
	}

	if (_snd_info.res_type == R_GAME_SOUND_PCM) {
		ms_f = (double)length /
		    (_snd_info.sample_size / CHAR_BIT) /
		    (_snd_info.freq) * 1000.0;

		ms_i = (int)ms_f;
	} else if (_snd_info.res_type == R_GAME_SOUND_VOC) {
		/* Rough hack, fix this to be accurate */
		ms_f = (double)length / 14705 * 1000.0;
		ms_i = (int)ms_f;
	} else {
		return -1;
	}

	return ms_i;
}

int SndRes::ITEVOC_Resample(long src_freq, long dst_freq, uchar *src_buf, 
						size_t src_buf_len, uchar **dst_buf, size_t *dst_buf_len) {
	uchar *resamp_buf;
	size_t resamp_len;

	uchar src_samp_a;
	uchar src_samp_b;

	const uchar *read_pa;
	const uchar *read_pb;

	uchar *write_pa;
	uchar *write_pb;
	uchar *write_pc;

	size_t src_i;

	assert(src_freq == 14705);
	assert(dst_freq == 22050);

	resamp_len = (size_t) (src_buf_len * 1.5);
	resamp_buf = (uchar *)malloc(resamp_len);
	if (resamp_buf == NULL) {
		return R_FAILURE;
	}

	read_pa = src_buf;
	read_pb = src_buf + 1;

	write_pa = resamp_buf;
	write_pb = resamp_buf + 1;
	write_pc = resamp_buf + 2;

	for (src_i = 0; src_i < src_buf_len / 2; src_i++) {
		src_samp_a = *read_pa;
		src_samp_b = *read_pb;

		read_pa += 2;
		read_pb += 2;

		*write_pa = src_samp_a;
		*write_pb = (uchar) ((src_samp_a / 2) + (src_samp_b / 2));
		*write_pc = src_samp_b;

		write_pa += 3;
		write_pb += 3;
		write_pc += 3;
	}

	*dst_buf = resamp_buf;
	*dst_buf_len = resamp_len;

	return R_SUCCESS;
}

} // End of namespace Saga
