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
#include "reinherit.h"

#include "yslib.h"

/*
 * Uses the following modules:
\*--------------------------------------------------------------------------*/
#include "game_mod.h"
#include "rscfile_mod.h"

namespace Saga {

/*
 * Begin module component
\*--------------------------------------------------------------------------*/

static int SoundInitialized = 0;

static R_RSCFILE_CONTEXT *SoundContext;
static R_RSCFILE_CONTEXT *VoiceContext;

int SYSSOUND_Init(int enabled)
{

	int result;

	YS_IGNORE_PARAM(enabled);

	if (SoundInitialized) {
		return R_FAILURE;
	}

	/* Load sound module resource file contexts
	 * \*------------------------------------------------------------- */
	result = GAME_GetFileContext(&SoundContext, R_GAME_SOUNDFILE, 0);
	if (result != R_SUCCESS) {
		return R_FAILURE;
	}

	result = GAME_GetFileContext(&VoiceContext, R_GAME_VOICEFILE, 0);
	if (result != R_SUCCESS) {
		return R_FAILURE;
	}

	SoundInitialized = 1;
	return R_SUCCESS;
}

int SYSSOUND_Shutdown()
{
	if (!SoundInitialized) {
		return R_FAILURE;
	}

	SoundInitialized = 0;

	return R_SUCCESS;
}

int SYSSOUND_Play(int sound_rn, int channel)
{

	(void)sound_rn;
	(void)channel;

	if (!SoundInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int SYSSOUND_Pause(int channel)
{

	(void)channel;

	if (!SoundInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int SYSSOUND_Resume(int channel)
{

	(void)channel;

	if (!SoundInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int SYSSOUND_Stop(int channel)
{

	(void)channel;

	if (!SoundInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int SYSSOUND_PlayVoice(R_SOUNDBUFFER * buf)
{

	(void)buf;

	if (!SoundInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int SYSSOUND_PauseVoice(void)
{

	if (!SoundInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int SYSSOUND_ResumeVoice(void)
{

	if (!SoundInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int SYSSOUND_StopVoice(void)
{

	if (!SoundInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

} // End of namespace Saga
