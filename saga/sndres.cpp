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
	/* Load sound module resource file contexts */
	_sfx_ctxt = GAME_GetFileContext(GAME_SOUNDFILE, 0);
	if (_sfx_ctxt == NULL) {
		return;
	}

	_voice_ctxt = GAME_GetFileContext(GAME_VOICEFILE, 0);
	if (_voice_ctxt == NULL) {
		return;
	}

	// Grab sound resource information for the current game
	GAME_GetSoundInfo(&_snd_info);

	_init = 1;
}

int SndRes::playSound(uint32 sound_rn, int volume) {
	SOUNDBUFFER snd_buffer;

	debug(0, "SndRes::playSound(%ld)", sound_rn);

	if (load(_sfx_ctxt, sound_rn, &snd_buffer) != SUCCESS) {
		debug(0, "Failed to load sound");
		return FAILURE;
	}

	_vm->_sound->playSound(&snd_buffer, volume);

	return SUCCESS;
}

int SndRes::playVoice(uint32 voice_rn) {
	SOUNDBUFFER snd_buffer;
	int result = FAILURE;
	bool voiceFile = false;

	debug(0, "SndRes::playVoice(%ld)", voice_rn);

	if (GAME_GetGameType() == GID_ITE && voice_rn == 4) {
		// The Wyrmkeep release of Inherit the Earth provides a
		// separate file (p2_a.voc or P2_A.iaf), to correct voice 4 in
		// the intro. Use that, if available.

		File f;
		uint32 size;
		bool voc = false;

		if (f.open("p2_a.voc"))
			voc = true;
		else
			f.open("P2_A.iaf");

		if (f.isOpen()) {
			size = f.size();
			byte *snd_res = (byte *)malloc(size);
			f.read(snd_res, size);
			f.close();

			if (!voc) {
				snd_buffer.s_stereo = 0;
				snd_buffer.s_samplebits = 16;
				snd_buffer.s_freq = 22050;
				snd_buffer.s_buf = snd_res;
				snd_buffer.s_buf_len = size;
				snd_buffer.s_signed = 1;
				result = SUCCESS;
			} else {
				result = loadVocSound(snd_res, size, &snd_buffer);
				RSC_FreeResource(snd_res);
			}
			voiceFile = true;
		}
	}

	// File is not present. It is DOS CD or Floppy
	if (result != SUCCESS)
		result = load(_voice_ctxt, voice_rn, &snd_buffer);

	if (result != SUCCESS) {
		return FAILURE;
	}

	if (GAME_GetFeatures() & GF_VOX_VOICES && !voiceFile)
		_vm->_sound->playVoxVoice(&snd_buffer);
	else
		_vm->_sound->playVoice(&snd_buffer);

	return SUCCESS;
}

int SndRes::load(RSCFILE_CONTEXT *snd_ctxt, uint32 snd_rn, SOUNDBUFFER *snd_buf_i) {
	byte *snd_res;
	size_t snd_res_len;

	int result;

	assert((snd_ctxt != NULL) && (snd_buf_i != NULL));

	result = RSC_LoadResource(snd_ctxt, snd_rn, &snd_res, &snd_res_len);
	if (result != SUCCESS) {
		return FAILURE;
	}

	switch (_snd_info.res_type) {
	case GAME_SOUND_PCM:
		snd_buf_i->s_freq = _snd_info.freq;
		snd_buf_i->s_samplebits = _snd_info.sample_size;
		snd_buf_i->s_stereo = _snd_info.stereo;
		snd_buf_i->s_buf = snd_res;
		snd_buf_i->s_buf_len = snd_res_len;
		snd_buf_i->s_signed = 1;
		break;
	case GAME_SOUND_VOC:
		result = loadVocSound(snd_res, snd_res_len, snd_buf_i);
		RSC_FreeResource(snd_res);
		if (result != SUCCESS) {
			return FAILURE;
		}
		break;
	case GAME_SOUND_WAV:
		result = loadWavSound(snd_res, snd_res_len, snd_buf_i);
		RSC_FreeResource(snd_res);
		if (result != SUCCESS) {
			return FAILURE;
		}
		break;
	default:
		/* Unknown sound type */
		RSC_FreeResource(snd_res);
		return FAILURE;
	}

	return SUCCESS;
}

int SndRes::loadVocSound(byte *snd_res, size_t snd_res_len, SOUNDBUFFER *snd_buf_i) {
	VOC_HEADER_BLOCK voc_hb;
	VOC_GENBLOCK voc_gb;
	VOC_BLOCK1 voc_b1;
	byte *data;

	long byte_rate;
	size_t i;

	if (snd_res_len < VOC_HEADER_BLOCK_LEN) {
		return FAILURE;
	}

	MemoryReadStream readS(snd_res, snd_res_len);

	for (i = 0; i < VOC_FILE_DESC_LEN; i++)
		voc_hb.ft_desc[i] = readS.readByte();

	if (memcmp(voc_hb.ft_desc, VOC_FILE_DESC, VOC_FILE_DESC_LEN) != 0) {
		/* Voc file desc string not found */
		return FAILURE;
	}

	voc_hb.db_offset = readS.readUint16LE();
	voc_hb.voc_version = readS.readUint16LE();
	voc_hb.voc_fileid = readS.readUint16LE();

	if ((int32)(snd_res_len - readS.pos()) < (int32)(voc_hb.db_offset + VOC_GENBLOCK_LEN)) {
		return FAILURE;
	}

	while (readS.pos() < voc_hb.db_offset)
		readS.readByte();

	for (;;) {
		/* Read generic block header */
		if (snd_res_len - readS.pos() < VOC_GENBLOCK_LEN) {
			return FAILURE;
		}

		voc_gb.block_id = readS.readByte();
		if (voc_gb.block_id == 0) {
			return FAILURE;
		}

		voc_gb.block_len = readS.readUint24LE();

		/* Process block */
		switch (voc_gb.block_id) {
		case 1:	/* Sound data block */
			voc_b1.time_constant = readS.readByte();
			voc_b1.pack_method = readS.readByte();

			if (voc_b1.pack_method != 0) {
				debug(0, "Packed VOC files not supported");
				return FAILURE;
			}

			byte_rate = VOC_TIME_BASE / (VOC_TIME_CBASE - (voc_b1.time_constant << 8));

			snd_buf_i->s_stereo = 0;
			snd_buf_i->s_samplebits = 8;
			snd_buf_i->s_freq = byte_rate;
			snd_buf_i->s_buf_len = snd_res_len - readS.pos() - 1;	/* -1 for end block */

			data = (byte *)malloc(snd_buf_i->s_buf_len);
			if (!data) {
				return FAILURE;
			}

			readS.read(data, snd_buf_i->s_buf_len);

			snd_buf_i->s_buf = data;
			snd_buf_i->s_signed = 0;
			return SUCCESS;
		default:
			for (i = 0; i < voc_gb.block_len; i++)
				readS.readByte();
			break;
		}
	}

	return SUCCESS;
}

int SndRes::loadWavSound(byte *snd_res, size_t snd_res_len, SOUNDBUFFER *snd_buf_i) {
	// TODO: This function should, perhaps, be made more robust.

	MemoryReadStream readS(snd_res, snd_res_len);

	byte buf[4];

	readS.read(buf, sizeof(buf));
	if (memcmp(buf, "RIFF", sizeof(buf)) != 0) {
		return FAILURE;
	}

	readS.readUint32LE();

	readS.read(buf, sizeof(buf));
	if (memcmp(buf, "WAVE", sizeof(buf)) != 0) {
		return FAILURE;
	}

	readS.read(buf, sizeof(buf));
	if (memcmp(buf, "fmt ", sizeof(buf)) != 0) {
		return FAILURE;
	}

	uint32 len = readS.readUint32LE();
	uint32 pos = readS.pos();

	readS.readUint16LE();

	snd_buf_i->s_stereo = (readS.readUint16LE() == 2) ? 1 : 0;
	snd_buf_i->s_freq = readS.readUint16LE();
	snd_buf_i->s_samplebits = 16;
	snd_buf_i->s_signed = 1;

	readS.seek(pos + len);

	for (;;) {
		readS.read(buf, sizeof(buf));
		if (memcmp(buf, "data", sizeof(buf)) == 0) {
			break;
		}

		len = readS.readUint32LE();
		readS.seek(len, SEEK_CUR);
	}

	snd_buf_i->s_buf_len = readS.readUint32LE();

	byte *data = (byte *)malloc(snd_buf_i->s_buf_len);
	if (!data) {
		return FAILURE;
	}

	readS.read(data, snd_buf_i->s_buf_len);
	snd_buf_i->s_buf = data;
	return SUCCESS;
}

int SndRes::getVoiceLength(uint32 voice_rn) {
	int res_type = _snd_info.res_type;
	uint32 length = 0;
	bool voiceFile = false;

	double ms_f;
	int ms_i = -1;

	int result = FAILURE;

	assert(_init);

	File f;

	// The Wyrmkeep release of Inherit the Earth provides a separate file
	// (p2_a.voc or P2_A.iaf), to correct voice 4 in the intro. Use that,
	// if available.

	if (GAME_GetGameType() == GID_ITE && voice_rn == 4) {
		if (f.open("p2_a.voc")) {
			result = SUCCESS;
			length = f.size();
			res_type = GAME_SOUND_VOC;
			f.close();
			voiceFile = true;
		} else if (f.open("P2_A.iaf")) {
			result = SUCCESS;
			length = f.size();
			res_type = GAME_SOUND_PCM;
			f.close();
			voiceFile = true;
		}
	}

	if (result == FAILURE) {
		result = RSC_GetResourceSize(_voice_ctxt, voice_rn, &length);

		if (result != SUCCESS) {
			return -1;
		}
	}

	if (GAME_GetFeatures() & GF_VOX_VOICES && !voiceFile) {
		// Rough hack, fix this to be accurate
		ms_f = (double)length / 22050 * 2000.0;
		ms_i = (int)ms_f;
	} else if (res_type == GAME_SOUND_PCM) {
		ms_f = (double)length / (_snd_info.sample_size / CHAR_BIT) / (_snd_info.freq) * 1000.0;
		ms_i = (int)ms_f;
	} else if (res_type == GAME_SOUND_VOC) {
		// Rough hack, fix this to be accurate
		ms_f = (double)length / 14705 * 1000.0;
		ms_i = (int)ms_f;
	} else if (res_type == GAME_SOUND_WAV) {
		// TODO!
		return -1;
	} else {
		return -1;
	}

	return ms_i;
}

} // End of namespace Saga
