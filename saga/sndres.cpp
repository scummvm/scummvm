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

// Sound resource management class

#include "saga/saga.h"

#include "saga/game_mod.h"
#include "saga/rscfile_mod.h"

#include "saga/sndres.h"
#include "saga/sound.h"

#include "common/file.h"

namespace Saga {

SndRes::SndRes(SagaEngine *vm) : _vm(vm) {
	int result;

	/* Load sound module resource file contexts */
	result = GAME_GetFileContext(&_sfx_ctxt, R_GAME_SOUNDFILE, 0);
	if (result != R_SUCCESS) {
		return;
	}

	result = GAME_GetFileContext(&_voice_ctxt, R_GAME_VOICEFILE, 0);
	if (result != R_SUCCESS) {
		return;
	}

	// Grab sound resource information for the current game
	GAME_GetSoundInfo(&_snd_info);

	_init = 1;
}

int SndRes::playVoice(uint32 voice_rn) {
	R_SOUNDBUFFER snd_buffer;
	int result;

	debug(0, "SndRes::playVoice(%ld)", voice_rn);

	// The Wyrmkeep release of Inherit the Earth provides a separate VOC
	// file, sound/p2_a.voc, to correct voice 4 in the intro. Use that, if
	// available.
	// 
	// FIXME: There's a nasty 'pop' at the beginning of the sound, and a
	// smaller one at the end. I don't know if that's in the file, or in
	// our playback code.

	File f;

	if (GAME_GetGameType() == GID_ITE && voice_rn == 4 && f.open("sound/p2_a.voc")) {
		uint32 size = f.size();
		byte *snd_res = (byte *)malloc(size);
		f.read(snd_res, size);
		result = loadVocSound(snd_res, size, &snd_buffer);
		f.close();
	} else
		result = load(_voice_ctxt, voice_rn, &snd_buffer);

	if (result != R_SUCCESS) {
		return R_FAILURE;
	}

	_vm->_sound->playVoice(&snd_buffer);

	return R_SUCCESS;
}

int SndRes::load(R_RSCFILE_CONTEXT *snd_ctxt, uint32 snd_rn, R_SOUNDBUFFER *snd_buf_i) {
	byte *snd_res;
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
	size_t i;

	if (snd_res_len < R_VOC_HEADER_BLOCK_LEN) {
		return R_FAILURE;
	}

	MemoryReadStream readS(snd_res, snd_res_len);

	for (i = 0; i < R_VOC_FILE_DESC_LEN; i++)
		voc_hb.ft_desc[i] = readS.readByte();

	if (memcmp(voc_hb.ft_desc, R_VOC_FILE_DESC, R_VOC_FILE_DESC_LEN) != 0) {
		/* Voc file desc string not found */
		return R_FAILURE;
	}

	voc_hb.db_offset = readS.readUint16LE();
	voc_hb.voc_version = readS.readUint16LE();
	voc_hb.voc_fileid = readS.readUint16LE();

	if ((int32)(snd_res_len - readS.pos()) < (int32)(voc_hb.db_offset + R_VOC_GENBLOCK_LEN)) {
		return R_FAILURE;
	}

	while (readS.pos() < voc_hb.db_offset)
		readS.readByte();

	for (;;) {
		/* Read generic block header */
		if (snd_res_len - readS.pos() < R_VOC_GENBLOCK_LEN) {
			return R_FAILURE;
		}

		voc_gb.block_id = readS.readByte();
		if (voc_gb.block_id == 0) {
			return R_FAILURE;
		}

		voc_gb.block_len = readS.readUint24LE();

		/* Process block */
		switch (voc_gb.block_id) {
		case 1:	/* Sound data block */
			voc_b1.time_constant = readS.readByte();
			voc_b1.pack_method = readS.readByte();

			if (voc_b1.pack_method != 0) {
				debug(0, "Packed VOC files not supported");
				return R_FAILURE;
			}

			byte_rate = R_VOC_TIME_BASE / (R_VOC_TIME_CBASE - (voc_b1.time_constant << 8));

			snd_buf_i->s_stereo = 0;
			snd_buf_i->s_samplebits = 8;
			snd_buf_i->s_freq = byte_rate;

			snd_buf_i->res_data = snd_res;
			snd_buf_i->res_len = snd_res_len;

			snd_buf_i->s_buf = snd_res + readS.pos();
			snd_buf_i->s_buf_len = snd_res_len - readS.pos() - 1;	/* -1 for end block */

			snd_buf_i->s_signed = 0;
			return R_SUCCESS;
			break;
		default:
			for (i = 0; i < voc_gb.block_len; i++)
				readS.readByte();
			break;
		}
	}

	return R_SUCCESS;
}

int SndRes::getVoiceLength(uint32 voice_rn) {
	int res_type = _snd_info.res_type;
	uint32 length;

	double ms_f;
	int ms_i = -1;

	int result;

	assert(_init);

	File f;

	// The Wyrmkeep release of Inherit the Earth provides a separate VOC
	// file, sound/p2_a.voc, to correct voice 4 in the intro. Use that, if
	// available.

	if (GAME_GetGameType() == GID_ITE && voice_rn == 4 && f.open("sound/p2_a.voc")) {
		length = f.size();
		res_type = R_GAME_SOUND_VOC;
		f.close();
	} else {
		result = RSC_GetResourceSize(_voice_ctxt, voice_rn, &length);

		if (result != R_SUCCESS) {
			return -1;
		}
	}

	if (res_type == R_GAME_SOUND_PCM) {
		ms_f = (double)length / (_snd_info.sample_size / CHAR_BIT) / (_snd_info.freq) * 1000.0;
		ms_i = (int)ms_f;
	} else if (res_type == R_GAME_SOUND_VOC) {
		// Rough hack, fix this to be accurate
		ms_f = (double)length / 14705 * 1000.0;
		ms_i = (int)ms_f;
	} else {
		return -1;
	}

	return ms_i;
}

int SndRes::ITEVOC_Resample(long src_freq, long dst_freq, byte *src_buf, 
						size_t src_buf_len, byte **dst_buf, size_t *dst_buf_len) {
	byte *resamp_buf;
	size_t resamp_len;

	byte src_samp_a;
	byte src_samp_b;

	const byte *read_pa;
	const byte *read_pb;

	byte *write_pa;
	byte *write_pb;
	byte *write_pc;

	size_t src_i;

	assert(src_freq == 14705);
	assert(dst_freq == 22050);

	resamp_len = (size_t) (src_buf_len * 1.5);
	resamp_buf = (byte *)malloc(resamp_len);
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
		*write_pb = (byte) ((src_samp_a / 2) + (src_samp_b / 2));
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
