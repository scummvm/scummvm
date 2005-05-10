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

// Sound resource management class

#include "saga/saga.h"

#include "saga/rscfile_mod.h"

#include "saga/sndres.h"
#include "saga/sound.h"
#include "saga/stream.h"

#include "common/file.h"

#include "sound/voc.h"
#include "sound/wave.h"

namespace Saga {

SndRes::SndRes(SagaEngine *vm) : _vm(vm) {
	/* Load sound module resource file contexts */
	_sfx_ctxt = _vm->getFileContext(GAME_SOUNDFILE, 0);
	if (_sfx_ctxt == NULL) {
		return;
	}

	_voice_ctxt = _vm->getFileContext(GAME_VOICEFILE, 0);
	if (_voice_ctxt == NULL) {
		return;
	}

	// Grab sound resource information for the current game
	_snd_info = *_vm->getSoundInfo();

	_init = 1;
}

int SndRes::playSound(uint32 sound_rn, int volume, bool loop) {
	SOUNDBUFFER snd_buffer;

	debug(0, "SndRes::playSound(%ld)", sound_rn);

	if (load(_sfx_ctxt, sound_rn, &snd_buffer) != SUCCESS) {
		debug(0, "Failed to load sound");
		return FAILURE;
	}

	_vm->_sound->playSound(&snd_buffer, volume, loop);

	return SUCCESS;
}

int SndRes::playVoice(uint32 voice_rn) {
	SOUNDBUFFER snd_buffer;
	int result = FAILURE;
	bool voiceFile = false;

	debug(0, "SndRes::playVoice(%ld)", voice_rn);

	if (_vm->getGameType() == GType_ITE && voice_rn == 4) {
		// The Wyrmkeep release of Inherit the Earth provides a
		// separate file (p2_a.voc or P2_A.iaf), to correct voice 4 in
		// the intro. Use that, if available.

		Common::File f;
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

	if (_vm->getFeatures() & GF_VOX_VOICES && !voiceFile)
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
	MemoryReadStreamEndian readS(snd_res, snd_res_len, IS_BIG_ENDIAN);
	byte *data;
	int rate;
	int len;

	data = loadVOCFromStream(readS, len, rate);

	if (!data) {
		return FAILURE;
	}

	snd_buf_i->s_freq = rate;
	snd_buf_i->s_samplebits = 8;
	snd_buf_i->s_stereo = 0;
	snd_buf_i->s_signed = 0;
	snd_buf_i->s_buf = data;
	snd_buf_i->s_buf_len = len;

	return SUCCESS;

}

int SndRes::loadWavSound(byte *snd_res, size_t snd_res_len, SOUNDBUFFER *snd_buf_i) {
	Common::MemoryReadStream readS(snd_res, snd_res_len);
	int rate, size;
	byte flags;

	if (!loadWAVFromStream(readS, size, rate, flags)) {
		return FAILURE;
	}

	snd_buf_i->s_stereo = ((flags & SoundMixer::FLAG_STEREO) != 0);
	snd_buf_i->s_freq = rate;
	snd_buf_i->s_samplebits = 16;
	snd_buf_i->s_signed = 1;
	snd_buf_i->s_buf_len = size;

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

	Common::File f;

	// The Wyrmkeep release of Inherit the Earth provides a separate file
	// (p2_a.voc or P2_A.iaf), to correct voice 4 in the intro. Use that,
	// if available.

	if (_vm->getGameType() == GType_ITE && voice_rn == 4) {
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

	if (_vm->getFeatures() & GF_VOX_VOICES && !voiceFile) {
		// Rough hack, fix this to be accurate
		ms_f = (double)length / 22050 * 2000.0;
		ms_i = (int)ms_f;
	} else if (res_type == GAME_SOUND_PCM) {
		ms_f = (double)length / (_snd_info.sample_size / 8) / (_snd_info.freq) * 1000.0;
		ms_i = (int)ms_f;
	} else if (res_type == GAME_SOUND_VOC) {		
		// Rough hack, fix this to be accurate
		ms_f = (double)length / 14705 * 1000.0;
		ms_i = (int)ms_f;
	} else if (res_type == GAME_SOUND_WAV) {
		// IHNM does not use the same format for all its WAV sounds, so
		// we just have to load the stupid thing and see what that will
		// tell us about it.

		SOUNDBUFFER snd_buffer;

		result = load(_voice_ctxt, voice_rn, &snd_buffer);
		if (result != SUCCESS) {
			return -1;
		}

		free(snd_buffer.s_buf);

		ms_f = (double)snd_buffer.s_buf_len;
		if (snd_buffer.s_samplebits == 16) {
			ms_f /= 2.0;
		}
		if (snd_buffer.s_stereo) {
			ms_f /= 2.0;
		}

		ms_f = ms_f / snd_buffer.s_freq * 1000.0;
		ms_i = (int)ms_f;
	} else {
		return -1;
	}

	return ms_i;
}

} // End of namespace Saga
