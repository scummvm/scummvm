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

#include "saga/rscfile_mod.h"
#include "saga/game_mod.h"
#include "saga/sound.h"

namespace Saga {

class SndRes {
public:

	SndRes(SagaEngine *vm);

	int loadSound(uint32 sound_rn);
	int playSound(uint32 sound_rn, int volume, bool loop);
	int playVoice(uint32 voice_rn);
	int getVoiceLength(uint32 voice_rn);

 private:
	int load(RSCFILE_CONTEXT *snd_ctxt, uint32 snd_rn, SOUNDBUFFER *snd_buf_i);
	int loadVocSound(byte *snd_res, size_t snd_res_len, SOUNDBUFFER *snd_buf_i);
	int loadWavSound(byte *snd_res, size_t snd_res_len, SOUNDBUFFER *snd_buf_i);

	int _init;

	RSCFILE_CONTEXT *_sfx_ctxt;
	RSCFILE_CONTEXT *_voice_ctxt;

	GAME_SOUNDINFO _snd_info;

	SagaEngine *_vm;
};

} // End of namespace Saga

#endif
